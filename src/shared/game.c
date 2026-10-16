// Game loop and shared state (platform-independent)
// Original main.c logic minus platform-specific entry point
#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "objects.h"
#include "input.h"
#include "services.h"
#include "audio.h"
#include "background.h"
#include "effects.h"
#include "menu.h"
#include "shop.h"
#include "filerw.h"

#define DEATH_ANIMATION_FRAMES 70
#define DEATH_FADE_FRAMES 20
#define REVIVE_INVULNERABLE_FRAMES 120

// Android: raylib redirects written files to internal storage,
// but rename() doesn't know about it, so we work directly inside that folder
#if defined(PLATFORM_ANDROID)
#include <unistd.h>
#include <sys/stat.h>
#include <android_native_app_glue.h>

// Provided by raylib (rcore_android.c); not declared in raylib.h
struct android_app *GetAndroidApp(void);
#endif

void PrepareSaveDirectory(void) {
#if defined(PLATFORM_ANDROID)
    if (chdir(GetAndroidApp()->activity->internalDataPath) != 0)
        TraceLog(LOG_WARNING, "Could not open internal storage");
    mkdir("data", 0700);
#endif
}

void StartGame(void) {
    generateCoin(GetGamevars()->screenWidth, GetGamevars()->screenHeight);
    GetChicken()->dead = false;
    GetMenuvars()->enabled = false;
    GetGamevars()->score = 0;
    GetGamevars()->newHighscore = false;
    GetGamevars()->bounces = 0;
    GetGamevars()->usedRevive = false;
    GetChicken()->pos = (Vector2){
        GetGamevars()->screenWidth / 2.0f,
        GetGamevars()->screenHeight / 2.0f
    };
    GetChicken()->velocityY = 0;
    GetChicken()->direction = 0;
    GetChicken()->rotation = 0;
    GetChicken()->squash = 0;
    GetChicken()->invulnerable = 0;
    GetGamevars()->startgame = true;
    generateSpikes(GetGamevars()->screenWidth, GetGamevars()->screenHeight, false);
}

void UpdateHighscore(void) {
    if (GetGamevars()->highscore < GetGamevars()->score) {
        GetGamevars()->highscore = GetGamevars()->score;
        GetGamevars()->newHighscore = true;
    }
}

void KillChicken(void) {
    GetChicken()->dead = true;
    GetChicken()->deathTimer = 0;
    GetChicken()->velocityY = -11.0f;   // little hop before falling off screen
    UpdateHighscore();
    SaveGame();
    SubmitLeaderboardScore(GetGamevars()->highscore);
    StartShake(10.0f, 20);
    Vector2 center = {
        GetChicken()->pos.x + CHICKEN_SIZE / 2.0f,
        GetChicken()->pos.y + CHICKEN_SIZE / 2.0f
    };
    SpawnParticles(center, 24, CurrentCharacterBodyColor(), 6.0f, 7.0f);
    SpawnParticles(center, 8, CurrentCharacterShadeColor(), 5.0f, 6.0f);
}

// Continue the run after watching an ad: back to the middle, keep the score
void ReviveChicken(void) {
    GetChicken()->dead = false;
    GetChicken()->pos = (Vector2){
        GetGamevars()->screenWidth / 2.0f - CHICKEN_SIZE / 2.0f,
        GetGamevars()->screenHeight / 2.0f
    };
    GetChicken()->velocityY = GetChicken()->jumpForce * 0.6f;
    GetChicken()->rotation = 0;
    GetChicken()->squash = 1.0f;
    GetChicken()->invulnerable = REVIVE_INVULNERABLE_FRAMES;
    GetGamevars()->usedRevive = true;
    generateSpikes(GetGamevars()->screenWidth, GetGamevars()->screenHeight,
                   GetChicken()->direction == 2);
    SpawnPopup((Vector2){ GetGamevars()->screenWidth / 2.0f,
                          GetGamevars()->screenHeight / 2.0f - 40 },
               "REVIVED!", (Color){ 120, 230, 120, 255 });
}

void BackToMenu(void) {
    GetMenuvars()->enabled = true;
    GetMenuvars()->untermenu = 0;
    GetMenuvars()->auswahl = 0;
    GetGamevars()->startgame = false;
}

static bool DeathScreenActive(void) {
    return !GetMenuvars()->enabled && GetChicken()->dead &&
           GetChicken()->deathTimer > DEATH_ANIMATION_FRAMES;
}

static bool CanRevive(void) {
    return AdsSupported() && !GetGamevars()->usedRevive;
}

void ApplyAdRewards(void) {
    // pendingReward handled in services.c
}

void UpdateGameplay(void) {
    GetGamevars()->score += 1;
    if (BackPressed()) {
        // Leaving a run keeps the collected coins and the highscore
        UpdateHighscore();
        SaveGame();
        SubmitLeaderboardScore(GetGamevars()->highscore);
        BackToMenu();
        return;
    }
    if (IsKeyPressed(KEY_X) || IsKeyPressed(KEY_SPACE) || PointerPressed()) {
        GetChicken()->velocityY = GetChicken()->jumpForce;
        GetChicken()->squash = 1.0f;
        PlayJumpSound();
        SpawnParticles(
            (Vector2){
                GetChicken()->pos.x + CHICKEN_SIZE / 2.0f,
                GetChicken()->pos.y + CHICKEN_SIZE - 8
            },
            4,
            Fade(CurrentCharacterBodyColor(), 0.8f),
            2.5f, 6.0f);
    }

    GetChicken()->velocityY += GetChicken()->gravity;
    GetChicken()->pos.y += GetChicken()->velocityY;

    if (GetChicken()->pos.y > GetGamevars()->screenHeight - CHICKEN_SIZE) {
        if (GetChicken()->velocityY > 4.0f)
            GetChicken()->squash = -0.8f;
        GetChicken()->pos.y = GetGamevars()->screenHeight - CHICKEN_SIZE;
        GetChicken()->velocityY = 0;
    }
    if (GetChicken()->pos.y < 0) {
        GetChicken()->pos.y = 0;
        GetChicken()->velocityY = 0;
    }

    if (GetChicken()->direction == 0)
        GetChicken()->pos.x += ChickenSpeed();
    if (GetChicken()->direction == 2)
        GetChicken()->pos.x -= ChickenSpeed();

    bool hitRightWall = GetChicken()->pos.x >= GetGamevars()->screenWidth - CHICKEN_SIZE;
    if (hitRightWall || GetChicken()->pos.x <= 0) {
        int oldLevel = GameLevel();
        GetGamevars()->bounces++;
        GetChicken()->direction = hitRightWall ? 2 : 0;
        GetChicken()->pos.x = hitRightWall ? GetGamevars()->screenWidth - CHICKEN_SIZE : 0;
        GetChicken()->squash = 0.7f;
        generateSpikes(GetGamevars()->screenWidth, GetGamevars()->screenHeight, hitRightWall);
        if (GameLevel() > oldLevel) {
            SpawnPopup(
                (Vector2){ GetGamevars()->screenWidth / 2.0f,
                           GetGamevars()->screenHeight / 2.0f - 60 },
                TextFormat("LEVEL %d!", GameLevel()), MENU_GOLD);
        }
    }

    // Tilt the beak up while rising and down while falling
    float targetRotation = GetChicken()->velocityY * 2.5f;
    if (targetRotation < -25.0f) targetRotation = -25.0f;
    if (targetRotation > 45.0f) targetRotation = 45.0f;
    if (GetChicken()->direction == 2) targetRotation = -targetRotation;
    GetChicken()->rotation += (targetRotation - GetChicken()->rotation) * 0.25f;
    GetChicken()->squash *= 0.85f;
    if (GetChicken()->invulnerable > 0) GetChicken()->invulnerable--;

    Rectangle chickenRect = {
        GetChicken()->pos.x + 8, GetChicken()->pos.y + 8,
        CHICKEN_SIZE - (0.2f * CHICKEN_SIZE), CHICKEN_SIZE - (0.2f * CHICKEN_SIZE)
    };

    for (int i = 0; i < MAX_SPIKES && GetChicken()->invulnerable == 0; i++) {
        Rectangle leftSpikeRect  = { GetLspikes()[i].pos.x,
                                     GetLspikes()[i].pos.y + 20, 44, 24 };
        Rectangle rightSpikeRect = { GetRspikes()[i].pos.x - 44,
                                     GetRspikes()[i].pos.y + 20, 44, 24 };

        if ((GetLspikes()[i].active && CheckCollisionRecs(chickenRect, leftSpikeRect)) ||
            (GetRspikes()[i].active && CheckCollisionRecs(chickenRect, rightSpikeRect))) {
            KillChicken();
            return;
        }
    }

    Rectangle coinRect = { GetMaincoin()->pos.x + 12, GetMaincoin()->pos.y + 4, 40, 56 };
    if (CheckCollisionRecs(chickenRect, coinRect)) {
        Vector2 coinCenter = { GetMaincoin()->pos.x + 32, GetMaincoin()->pos.y + 32 };
        SpawnParticles(coinCenter, 16, YELLOW, 4.0f, 6.0f);
        SpawnPopup((Vector2){ coinCenter.x, coinCenter.y - 20 }, "+100", YELLOW);
        generateCoin(GetGamevars()->screenWidth, GetGamevars()->screenHeight);
        GetGamevars()->Coins += 1;
        GetGamevars()->score += 100;
        PlaySound(GetMaincoin()->sound);
    }
}

static void SelectMenuOption(int option) {
    if (option == 0) StartGame();
    else if (option == 1) GetMenuvars()->untermenu = 1;
    else if (option == 2) GetMenuvars()->quit = true;
}

void UpdateMenu(void) {
    if (GetMenuvars()->untermenu == 1) {
        UpdateShop(GetGamevars()->screenWidth);
        return;
    }

    if (BackPressed())
        GetMenuvars()->quit = true;   // Android back button leaves the game
    if (IsKeyPressed(KEY_M) || PointerPressedIn(MusicButtonRect())) {
        ToggleMusic();
        return;
    }
    if (IsKeyPressed(KEY_X))
        SelectMenuOption(GetMenuvars()->auswahl);
    else if (IsKeyPressed(KEY_UP))
        GetMenuvars()->auswahl =
            (GetMenuvars()->auswahl - 1 + MENU_OPTIONS) % MENU_OPTIONS;
    else if (IsKeyPressed(KEY_DOWN))
        GetMenuvars()->auswahl =
            (GetMenuvars()->auswahl + 1) % MENU_OPTIONS;
    else if (PrivacyOptionsRequired() &&
             PointerPressedIn(PrivacyButtonRect(GetGamevars()->screenWidth,
                                                GetGamevars()->screenHeight))) {
        ShowPrivacyOptions();
    } else if (LeaderboardAvailable() &&
               PointerPressedIn(LeaderboardButtonRect(GetGamevars()->screenWidth))) {
        ShowLeaderboard();
    } else {
        for (int i = 0; i < MENU_OPTIONS; i++) {
            if (!PointerPressedIn(MenuOptionRect(i, GetGamevars()->screenWidth)))
                continue;
            GetMenuvars()->auswahl = i;
            SelectMenuOption(i);
            break;
        }
    }
}

void UpdateDeath(void) {
    // The chicken tumbles off the screen before the death screen shows up
    GetChicken()->deathTimer++;
    if (GetChicken()->pos.y < GetGamevars()->screenHeight + CHICKEN_SIZE) {
        GetChicken()->velocityY += GetChicken()->gravity;
        GetChicken()->pos.y += GetChicken()->velocityY;
        GetChicken()->rotation += GetChicken()->direction == 0 ? 12.0f : -12.0f;
    }
    if (!DeathScreenActive()) return;

    int w = GetGamevars()->screenWidth;
    int h = GetGamevars()->screenHeight;
    if (IsKeyPressed(KEY_X) ||
        PointerPressedIn(DeathButtonRect(DEATH_BUTTON_RETRY, w, h, CanRevive()))) {
        StartGame();
    } else if (BackPressed() ||
               PointerPressedIn(DeathButtonRect(DEATH_BUTTON_MENU, w, h, CanRevive()))) {
        BackToMenu();
    } else if (CanRevive() && RewardedAdReady() &&
               PointerPressedIn(DeathButtonRect(DEATH_BUTTON_REVIVE, w, h, true))) {
        ShowRewardedAd(REWARD_REVIVE);
    }
}

void DrawFrame(void) {
    Camera2D camera = { 0 };
    camera.offset = UpdateShake();
    camera.zoom = 1.0f;

    BeginVirtualScreen();
    DrawBackground(GetGamevars()->screenWidth, GetGamevars()->screenHeight,
                   themes[PreviewedBackground()]);

    BeginMode2D(camera);
    if (!GetMenuvars()->enabled) {
        for (int i = 0; i < MAX_SPIKES; i++) {
            if (GetLspikes()[i].active)
                DrawTextureEx(GetLspikes()[i].spiketexture, GetLspikes()[i].pos,
                              0.0f, 1.0f, WHITE);
            if (GetRspikes()[i].active) {
                Vector2 rotatedPos = {
                    GetRspikes()[i].pos.x,
                    GetRspikes()[i].pos.y + SLOT_HEIGHT
                };
                DrawTextureEx(GetLspikes()[i].spiketexture, rotatedPos,
                              180.0f, 1.0f, WHITE);
            }
        }
        if (!GetChicken()->dead) DrawCoin();
        // Blink while invulnerable after a revive
        Color tint =
            (GetChicken()->invulnerable / 6) % 2 == 1
                ? Fade(WHITE, 0.35f)
                : WHITE;
        DrawChicken(CurrentCharacterTexture(GetChicken()->direction), tint);
    }
    EndMode2D();

    if (GetMenuvars()->enabled) {
        if (GetMenuvars()->untermenu == 0) {
            DrawMainMenu(
                GetGamevars()->screenWidth, GetGamevars()->screenHeight,
                GetMenuvars()->auswahl,
                CurrentCharacterTexture(0),
                GetGamevars()->highscore, GetGamevars()->Coins,
                PrivacyOptionsRequired(),
                MusicOn(),
                LeaderboardAvailable(),
                PlayerName(),
                SignInState());
        } else {
            DrawShop(GetGamevars()->screenWidth, GetGamevars()->screenHeight);
        }
    } else {
        DrawText(TextFormat("Score: %d", GetGamevars()->score), 8, 8, 20, WHITE);
        DrawText(TextFormat("Coins: %d", GetGamevars()->Coins), 8, 32, 20, YELLOW);
        DrawText(TextFormat("Level: %d", GameLevel()), 8, 56, 20, MENU_GOLD);
    }

    DrawEffects();

    if (!GetMenuvars()->enabled && GetChicken()->dead &&
        GetChicken()->deathTimer > DEATH_ANIMATION_FRAMES - DEATH_FADE_FRAMES) {
        float fade = (GetChicken()->deathTimer - (DEATH_ANIMATION_FRAMES - DEATH_FADE_FRAMES)) /
                     (float)DEATH_FADE_FRAMES;
        if (fade > 1.0f) fade = 1.0f;
        DrawDeathScreen(GetGamevars()->screenWidth, GetGamevars()->screenHeight,
                        GetGamevars()->score, GetGamevars()->highscore,
                        GetGamevars()->newHighscore, CanRevive(),
                        RewardedAdReady(), fade);
    }
    EndVirtualScreen();
}
