#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "objects.c"
#include "background.c"
#include "effects.c"
#include "menu.c"
#include "shop.c"

#define DEATH_ANIMATION_FRAMES 70
#define DEATH_FADE_FRAMES 20

void StartGame() {
    generateCoin(Gamevars.screenWidth, Gamevars.screenHeight);
    Chicken.dead = false;
    Menuvars.enabled = false;
    Gamevars.score = 0;
    Gamevars.newHighscore = false;
    Chicken.pos = (Vector2){ Gamevars.screenWidth/2.0f, Gamevars.screenHeight/2.0f };
    Chicken.velocityY = 0;
    Chicken.direction = 0;
    Chicken.rotation = 0;
    Chicken.squash = 0;
    Gamevars.startgame = true;
    generateSpikes(Gamevars.screenWidth, Gamevars.screenHeight, false);
}

void KillChicken() {
    Chicken.dead = true;
    Chicken.deathTimer = 0;
    Chicken.velocityY = -11.0f;   // a little hop before falling off the screen
    if (Gamevars.highscore < Gamevars.score) {
        Gamevars.highscore = Gamevars.score;
        Gamevars.newHighscore = true;
    }
    SaveGame();
    StartShake(10.0f, 20);
    Vector2 center = { Chicken.pos.x + CHICKEN_SIZE / 2.0f, Chicken.pos.y + CHICKEN_SIZE / 2.0f };
    SpawnParticles(center, 24, characters[Shopvars.character].body, 6.0f, 7.0f);
    SpawnParticles(center, 8, characters[Shopvars.character].shade, 5.0f, 6.0f);
}

int main(void)
{
    Initgamevars();
    InitWindow(Gamevars.screenWidth, Gamevars.screenHeight, "Jumper Chicken");
    InitAudioDevice();
    SetTargetFPS(60);
    srand(time(NULL));

    LoadGame();
    Initplayer(Gamevars.screenWidth, Gamevars.screenHeight);
    InitSpikes(Gamevars.screenWidth, Gamevars.screenHeight);
    Initcolors();
    InitCoin();
    InitCharacters();
    InitBackground(Gamevars.screenWidth, Gamevars.screenHeight);

    while (!WindowShouldClose() && !Menuvars.quit)
    {
        UpdateBackground(Gamevars.screenWidth, Gamevars.screenHeight);
        UpdateEffects();

        if (!Chicken.dead && Gamevars.startgame) {
            Gamevars.score += 1;
            if (IsKeyPressed(KEY_X)) {
                Chicken.velocityY = Chicken.jumpForce;
                Chicken.squash = 1.0f;
                SpawnParticles((Vector2){ Chicken.pos.x + CHICKEN_SIZE / 2.0f, Chicken.pos.y + CHICKEN_SIZE - 8 },
                               4, Fade(characters[Shopvars.character].body, 0.8f), 2.5f, 6.0f);
            }

            Chicken.velocityY += Chicken.gravity;
            Chicken.pos.y += Chicken.velocityY;

            if (Chicken.pos.y > Gamevars.screenHeight - CHICKEN_SIZE) {
                if (Chicken.velocityY > 4.0f) Chicken.squash = -0.8f;
                Chicken.pos.y = Gamevars.screenHeight - CHICKEN_SIZE;
                Chicken.velocityY = 0;
            }
            if (Chicken.pos.y < 0) { Chicken.pos.y = 0; Chicken.velocityY = 0; }

            if (Chicken.direction == 0) Chicken.pos.x += 8;
            if (Chicken.direction == 2) Chicken.pos.x -= 8;

            if (Chicken.pos.x >= Gamevars.screenWidth - CHICKEN_SIZE) {
                Chicken.direction = 2;
                Chicken.pos.x = Gamevars.screenWidth - CHICKEN_SIZE;
                Chicken.squash = 0.7f;
                generateSpikes(Gamevars.screenWidth, Gamevars.screenHeight, true);
            } else if (Chicken.pos.x <= 0) {
                Chicken.direction = 0;
                Chicken.pos.x = 0;
                Chicken.squash = 0.7f;
                generateSpikes(Gamevars.screenWidth, Gamevars.screenHeight, false);
            }

            // Tilt the beak up while rising and down while falling
            float targetRotation = Chicken.velocityY * 2.5f;
            if (targetRotation < -25.0f) targetRotation = -25.0f;
            if (targetRotation > 45.0f) targetRotation = 45.0f;
            if (Chicken.direction == 2) targetRotation = -targetRotation;
            Chicken.rotation += (targetRotation - Chicken.rotation) * 0.25f;
            Chicken.squash *= 0.85f;

            Rectangle chickenRect = { Chicken.pos.x + 8, Chicken.pos.y + 8, CHICKEN_SIZE-(0.2*CHICKEN_SIZE), CHICKEN_SIZE-(0.2*CHICKEN_SIZE) };

            for (int i = 0; i < NUM_SPIKES; i++) {
                Rectangle leftSpikeRect = { lspikes[i].pos.x, lspikes[i].pos.y + 20, 44, 24 };
                Rectangle rightSpikeRect = { rspikes[i].pos.x -44, rspikes[i].pos.y + 20, 44, 24 };

                if ((lspikes[i].active && CheckCollisionRecs(chickenRect, leftSpikeRect)) ||
                    (rspikes[i].active && CheckCollisionRecs(chickenRect, rightSpikeRect))) {
                    KillChicken();
                    break;
                }
            }
            Rectangle coinRect = { maincoin.pos.x+12, maincoin.pos.y+4, 40, 56};
            if (!Chicken.dead && CheckCollisionRecs(chickenRect, coinRect)) {
                Vector2 coinCenter = { maincoin.pos.x + 32, maincoin.pos.y + 32 };
                SpawnParticles(coinCenter, 16, YELLOW, 4.0f, 6.0f);
                SpawnPopup((Vector2){ coinCenter.x, coinCenter.y - 20 }, "+100", YELLOW);
                generateCoin(Gamevars.screenWidth, Gamevars.screenHeight);
                Gamevars.Coins += 1;
                Gamevars.score += 100;
                PlaySound(maincoin.sound);
            }
        } else if (Menuvars.enabled) {
            if (Menuvars.untermenu == 1) {
                UpdateShop(Gamevars.screenWidth);
            } else if (IsKeyPressed(KEY_X)) {
                if (Menuvars.auswahl == 0) StartGame();
                else if (Menuvars.auswahl == 1) Menuvars.untermenu = 1;
                else if (Menuvars.auswahl == 2) Menuvars.quit = true;
            } else if (IsKeyPressed(KEY_UP)) {
                Menuvars.auswahl = (Menuvars.auswahl - 1 + MENU_OPTIONS) % MENU_OPTIONS;
            } else if (IsKeyPressed(KEY_DOWN)) {
                Menuvars.auswahl = (Menuvars.auswahl + 1) % MENU_OPTIONS;
            }
        } else {
            // The chicken tumbles off the screen before the death screen shows up
            Chicken.deathTimer++;
            if (Chicken.pos.y < Gamevars.screenHeight + CHICKEN_SIZE) {
                Chicken.velocityY += Chicken.gravity;
                Chicken.pos.y += Chicken.velocityY;
                Chicken.rotation += Chicken.direction == 0 ? 12.0f : -12.0f;
            }
            if (Chicken.deathTimer > DEATH_ANIMATION_FRAMES) {
                if (IsKeyPressed(KEY_X)) {
                    StartGame();
                } else if (IsKeyPressed(KEY_BACKSPACE)) {
                    Menuvars.enabled = true;
                    Menuvars.untermenu = 0;
                    Menuvars.auswahl = 0;
                }
            }
        }

        Camera2D camera = { 0 };
        camera.offset = UpdateShake();
        camera.zoom = 1.0f;

        BeginDrawing();
        DrawBackground(Gamevars.screenWidth, Gamevars.screenHeight, themes[PreviewedBackground()]);

        BeginMode2D(camera);
        if (!Menuvars.enabled) {
            for (int i = 0; i < NUM_SPIKES; i++) {
                if (lspikes[i].active) DrawTextureEx(lspikes[i].spiketexture, lspikes[i].pos, 0.0f, 1.0f, WHITE);
                if (rspikes[i].active) {
                        Vector2 rotatedPos = {
                        rspikes[i].pos.x ,
                        rspikes[i].pos.y + SLOT_HEIGHT
                    };
                    DrawTextureEx(lspikes[i].spiketexture, rotatedPos, 180.0f, 1.0f, WHITE);
                }
            }
            if (!Chicken.dead) DrawCoin();
            DrawChicken(CurrentCharacterTexture(Chicken.direction));
        }
        EndMode2D();

        if (Menuvars.enabled) {
            if (Menuvars.untermenu == 0) {
                DrawMainMenu(Gamevars.screenWidth, Gamevars.screenHeight, Menuvars.auswahl,
                             CurrentCharacterTexture(0), Gamevars.highscore, Gamevars.Coins);
            } else {
                DrawShop(Gamevars.screenWidth, Gamevars.screenHeight);
            }
        } else {
            DrawText(TextFormat("Score: %d", Gamevars.score), 8, 8, 20, WHITE);
            DrawText(TextFormat("Coins: %d", Gamevars.Coins), 8, 32, 20, YELLOW);
        }

        DrawEffects();

        if (!Menuvars.enabled && Chicken.dead && Chicken.deathTimer > DEATH_ANIMATION_FRAMES - DEATH_FADE_FRAMES) {
            float fade = (Chicken.deathTimer - (DEATH_ANIMATION_FRAMES - DEATH_FADE_FRAMES)) / (float)DEATH_FADE_FRAMES;
            if (fade > 1.0f) fade = 1.0f;
            DrawDeathScreen(Gamevars.screenWidth, Gamevars.screenHeight, Gamevars.score,
                            Gamevars.highscore, Gamevars.newHighscore, fade);
        }
        EndDrawing();
    }

    SaveGame();

    UnloadCharacters();
    UnloadTexture(spikeLeftTex);
    UnloadTexture(spikeRightTex);
    UnloadTexture(maincoin.texture);
    UnloadSound(maincoin.sound);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
