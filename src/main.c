#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "objects.c"
#include "input.c"
#include "services.c"
#include "audio.c"
#include "background.c"
#include "effects.c"
#include "menu.c"
#include "shop.c"

#if defined(PLATFORM_ANDROID)
#include <unistd.h>
#include <sys/stat.h>
#endif
#if defined(PLATFORM_WEB)
#include <emscripten.h>
#endif

#define DEATH_ANIMATION_FRAMES 70
#define DEATH_FADE_FRAMES 20
#define REVIVE_INVULNERABLE_FRAMES 120

// raylib redirects written files to the app's internal storage on Android,
// but rename() does not know about that, so we work directly inside that folder
void prepareSaveDirectory() {
#if defined(PLATFORM_ANDROID)
    if (chdir(GetAndroidApp()->activity->internalDataPath) != 0) TraceLog(LOG_WARNING, "Could not open internal storage");
    mkdir("data", 0700);
#elif defined(PLATFORM_WEB)
    mountSaveFolder();   // loads asynchronously, see gameFrame()
#else
    // Started from a file manager or the itch.io app the working directory can be anywhere,
    // but data/ always lies next to the executable
    ChangeDirectory(GetApplicationDirectory());
#endif
}

void StartGame() {
    generateCoin(Gamevars.screenWidth, Gamevars.screenHeight);
    Chicken.dead = false;
    Menuvars.enabled = false;
    Gamevars.score = 0;
    Gamevars.newHighscore = false;
    Gamevars.bounces = 0;
    Gamevars.usedRevive = false;
    Chicken.pos = (Vector2){ Gamevars.screenWidth/2.0f, Gamevars.screenHeight/2.0f };
    Chicken.velocityY = 0;
    Chicken.direction = 0;
    Chicken.rotation = 0;
    Chicken.squash = 0;
    Chicken.invulnerable = 0;
    Gamevars.startgame = true;
    generateSpikes(Gamevars.screenWidth, Gamevars.screenHeight, false);
}

void updateHighscore() {
    if (Gamevars.highscore < Gamevars.score) {
        Gamevars.highscore = Gamevars.score;
        Gamevars.newHighscore = true;
    }
}

void KillChicken() {
    Chicken.dead = true;
    Chicken.deathTimer = 0;
    Chicken.velocityY = -11.0f;   // a little hop before falling off the screen
    updateHighscore();
    SaveGame();
    SubmitLeaderboardScore(Gamevars.highscore);
    StartShake(10.0f, 20);
    Vector2 center = { Chicken.pos.x + CHICKEN_SIZE / 2.0f, Chicken.pos.y + CHICKEN_SIZE / 2.0f };
    SpawnParticles(center, 24, characters[Shopvars.equipped[ROW_CHARACTERS]].body, 6.0f, 7.0f);
    SpawnParticles(center, 8, characters[Shopvars.equipped[ROW_CHARACTERS]].shade, 5.0f, 6.0f);
}

// Continue the run after watching an ad: back to the middle, keep the score
void ReviveChicken() {
    Chicken.dead = false;
    Chicken.pos = (Vector2){ Gamevars.screenWidth/2.0f - CHICKEN_SIZE/2.0f, Gamevars.screenHeight/2.0f };
    Chicken.velocityY = Chicken.jumpForce * 0.6f;
    Chicken.rotation = 0;
    Chicken.squash = 1.0f;
    Chicken.invulnerable = REVIVE_INVULNERABLE_FRAMES;
    Gamevars.usedRevive = true;
    generateSpikes(Gamevars.screenWidth, Gamevars.screenHeight, Chicken.direction == 2);
    SpawnPopup((Vector2){ Gamevars.screenWidth / 2.0f, Gamevars.screenHeight / 2.0f - 40 }, "REVIVED!", (Color){ 120, 230, 120, 255 });
}

void BackToMenu() {
    Menuvars.enabled = true;
    Menuvars.untermenu = 0;
    Menuvars.auswahl = 0;
    Gamevars.startgame = false;
}

bool deathScreenActive() {
    return !Menuvars.enabled && Chicken.dead && Chicken.deathTimer > DEATH_ANIMATION_FRAMES;
}

bool canRevive() {
    return AdsSupported() && !Gamevars.usedRevive;
}

void applyAdRewards() {
    if (pendingReward == REWARD_NONE || ConsumeAdReward() <= 0) return;

    if (pendingReward == REWARD_COINS) {
        Gamevars.Coins += AD_REWARD_COINS;
        SaveGame();
        setShopMessage(TextFormat("+%d coins - thanks for watching!", AD_REWARD_COINS), YELLOW);
        SpawnParticles((Vector2){ Gamevars.screenWidth - 80.0f, 58 }, 30, YELLOW, 6.0f, 7.0f);
        PlaySound(maincoin.sound);
    } else if (pendingReward == REWARD_REVIVE && deathScreenActive() && !Gamevars.usedRevive) {
        ReviveChicken();
    }
    pendingReward = REWARD_NONE;
}

void updateGameplay() {
    Gamevars.score += 1;
    if (BackPressed()) {
        // Leaving a run keeps the collected coins and the highscore
        updateHighscore();
        SaveGame();
        SubmitLeaderboardScore(Gamevars.highscore);
        BackToMenu();
        return;
    }
    if (IsKeyPressed(KEY_X) || IsKeyPressed(KEY_SPACE) || PointerPressed()) {
        Chicken.velocityY = Chicken.jumpForce;
        Chicken.squash = 1.0f;
        PlayJumpSound();
        SpawnParticles((Vector2){ Chicken.pos.x + CHICKEN_SIZE / 2.0f, Chicken.pos.y + CHICKEN_SIZE - 8 },
                       4, Fade(characters[Shopvars.equipped[ROW_CHARACTERS]].body, 0.8f), 2.5f, 6.0f);
    }

    Chicken.velocityY += Chicken.gravity;
    Chicken.pos.y += Chicken.velocityY;

    if (Chicken.pos.y > Gamevars.screenHeight - CHICKEN_SIZE) {
        if (Chicken.velocityY > 4.0f) Chicken.squash = -0.8f;
        Chicken.pos.y = Gamevars.screenHeight - CHICKEN_SIZE;
        Chicken.velocityY = 0;
    }
    if (Chicken.pos.y < 0) { Chicken.pos.y = 0; Chicken.velocityY = 0; }

    if (Chicken.direction == 0) Chicken.pos.x += ChickenSpeed();
    if (Chicken.direction == 2) Chicken.pos.x -= ChickenSpeed();

    bool hitRightWall = Chicken.pos.x >= Gamevars.screenWidth - CHICKEN_SIZE;
    if (hitRightWall || Chicken.pos.x <= 0) {
        int oldLevel = GameLevel();
        Gamevars.bounces++;
        Chicken.direction = hitRightWall ? 2 : 0;
        Chicken.pos.x = hitRightWall ? Gamevars.screenWidth - CHICKEN_SIZE : 0;
        Chicken.squash = 0.7f;
        generateSpikes(Gamevars.screenWidth, Gamevars.screenHeight, hitRightWall);
        if (GameLevel() > oldLevel) {
            SpawnPopup((Vector2){ Gamevars.screenWidth / 2.0f, Gamevars.screenHeight / 2.0f - 60 },
                       TextFormat("LEVEL %d!", GameLevel()), MENU_GOLD);
        }
    }

    // Tilt the beak up while rising and down while falling
    float targetRotation = Chicken.velocityY * 2.5f;
    if (targetRotation < -25.0f) targetRotation = -25.0f;
    if (targetRotation > 45.0f) targetRotation = 45.0f;
    if (Chicken.direction == 2) targetRotation = -targetRotation;
    Chicken.rotation += (targetRotation - Chicken.rotation) * 0.25f;
    Chicken.squash *= 0.85f;
    if (Chicken.invulnerable > 0) Chicken.invulnerable--;

    Rectangle chickenRect = { Chicken.pos.x + 8, Chicken.pos.y + 8, CHICKEN_SIZE-(0.2*CHICKEN_SIZE), CHICKEN_SIZE-(0.2*CHICKEN_SIZE) };

    for (int i = 0; i < MAX_SPIKES && Chicken.invulnerable == 0; i++) {
        Rectangle leftSpikeRect = { lspikes[i].pos.x, lspikes[i].pos.y + 20, 44, 24 };
        Rectangle rightSpikeRect = { rspikes[i].pos.x -44, rspikes[i].pos.y + 20, 44, 24 };

        if ((lspikes[i].active && CheckCollisionRecs(chickenRect, leftSpikeRect)) ||
            (rspikes[i].active && CheckCollisionRecs(chickenRect, rightSpikeRect))) {
            KillChicken();
            return;
        }
    }
    Rectangle coinRect = { maincoin.pos.x+12, maincoin.pos.y+4, 40, 56};
    if (CheckCollisionRecs(chickenRect, coinRect)) {
        Vector2 coinCenter = { maincoin.pos.x + 32, maincoin.pos.y + 32 };
        SpawnParticles(coinCenter, 16, YELLOW, 4.0f, 6.0f);
        SpawnPopup((Vector2){ coinCenter.x, coinCenter.y - 20 }, "+100", YELLOW);
        generateCoin(Gamevars.screenWidth, Gamevars.screenHeight);
        Gamevars.Coins += 1;
        Gamevars.score += 100;
        PlaySound(maincoin.sound);
    }
}

void selectMenuOption(int option) {
    if (option == 0) StartGame();
    else if (option == 1) Menuvars.untermenu = 1;
    else if (option == 2) Menuvars.quit = true;
}

void updateMenu() {
    if (Menuvars.untermenu == 1) {
        UpdateShop(Gamevars.screenWidth);
        return;
    }

    if (IsKeyPressed(KEY_BACK)) Menuvars.quit = true;   // Android back button leaves the game
    if (IsKeyPressed(KEY_M) || PointerPressedIn(MusicButtonRect())) {
        Shopvars.musicOn = !Shopvars.musicOn;
        SaveGame();
        return;
    }
    if (IsKeyPressed(KEY_X)) selectMenuOption(Menuvars.auswahl);
    else if (IsKeyPressed(KEY_UP)) Menuvars.auswahl = (Menuvars.auswahl - 1 + MenuOptionCount()) % MenuOptionCount();
    else if (IsKeyPressed(KEY_DOWN)) Menuvars.auswahl = (Menuvars.auswahl + 1) % MenuOptionCount();
    else if (PrivacyOptionsRequired() && PointerPressedIn(PrivacyButtonRect(Gamevars.screenWidth, Gamevars.screenHeight))) {
        ShowPrivacyOptions();
    } else if (LeaderboardAvailable() && PointerPressedIn(LeaderboardButtonRect(Gamevars.screenWidth))) {
        ShowLeaderboard();   // signs the player in first if needed
    } else {
        for (int i = 0; i < MenuOptionCount(); i++) {
            if (!PointerPressedIn(MenuOptionRect(i, Gamevars.screenWidth))) continue;
            Menuvars.auswahl = i;
            selectMenuOption(i);
            break;
        }
    }
}

void updateDeath() {
    // The chicken tumbles off the screen before the death screen shows up
    Chicken.deathTimer++;
    if (Chicken.pos.y < Gamevars.screenHeight + CHICKEN_SIZE) {
        Chicken.velocityY += Chicken.gravity;
        Chicken.pos.y += Chicken.velocityY;
        Chicken.rotation += Chicken.direction == 0 ? 12.0f : -12.0f;
    }
    if (!deathScreenActive()) return;

    int w = Gamevars.screenWidth, h = Gamevars.screenHeight;
    if (IsKeyPressed(KEY_X) || PointerPressedIn(DeathButtonRect(DEATH_BUTTON_RETRY, w, h, canRevive()))) {
        StartGame();
    } else if (BackPressed() || PointerPressedIn(DeathButtonRect(DEATH_BUTTON_MENU, w, h, canRevive()))) {
        BackToMenu();
    } else if (canRevive() && RewardedAdReady() && PointerPressedIn(DeathButtonRect(DEATH_BUTTON_REVIVE, w, h, true))) {
        ShowRewardedAd(REWARD_REVIVE);
    }
}

void drawFrame() {
    Camera2D camera = { 0 };
    camera.offset = UpdateShake();
    camera.zoom = 1.0f;

    BeginVirtualScreen();
    DrawBackground(Gamevars.screenWidth, Gamevars.screenHeight, themes[PreviewedBackground()]);

    BeginMode2D(camera);
    if (!Menuvars.enabled) {
        for (int i = 0; i < MAX_SPIKES; i++) {
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
        // Blink while invulnerable after a revive
        Color tint = (Chicken.invulnerable / 6) % 2 == 1 ? Fade(WHITE, 0.35f) : WHITE;
        DrawChicken(CurrentCharacterTexture(Chicken.direction), tint);
    }
    EndMode2D();

    if (Menuvars.enabled) {
        if (Menuvars.untermenu == 0) {
            DrawMainMenu(Gamevars.screenWidth, Gamevars.screenHeight, Menuvars.auswahl,
                         CurrentCharacterTexture(0), Gamevars.highscore, Gamevars.Coins, PrivacyOptionsRequired(),
                         Shopvars.musicOn, LeaderboardAvailable(), PlayerName(), SignInState());
        } else {
            DrawShop(Gamevars.screenWidth, Gamevars.screenHeight);
        }
    } else {
        DrawText(TextFormat("Score: %d", Gamevars.score), 8, 8, 20, WHITE);
        DrawText(TextFormat("Coins: %d", Gamevars.Coins), 8, 32, 20, YELLOW);
        DrawText(TextFormat("Level: %d", GameLevel()), 8, 56, 20, MENU_GOLD);
    }

    DrawEffects();

    if (!Menuvars.enabled && Chicken.dead && Chicken.deathTimer > DEATH_ANIMATION_FRAMES - DEATH_FADE_FRAMES) {
        float fade = (Chicken.deathTimer - (DEATH_ANIMATION_FRAMES - DEATH_FADE_FRAMES)) / (float)DEATH_FADE_FRAMES;
        if (fade > 1.0f) fade = 1.0f;
        DrawDeathScreen(Gamevars.screenWidth, Gamevars.screenHeight, Gamevars.score,
                        Gamevars.highscore, Gamevars.newHighscore, canRevive(), RewardedAdReady(), fade);
    }
    EndVirtualScreen();
}

bool saveLoaded = false;

// Everything that depends on the save file
void loadSaveAndServices() {
    LoadGame();
    InitAds();
    SubmitLeaderboardScore(Gamevars.highscore);   // uploads a highscore from before the leaderboard existed
    InitGameAudio(Shopvars.equipped[ROW_MUSIC], Shopvars.musicOn);
    saveLoaded = true;
}

// One frame of the game: update and draw
void gameFrame() {
#if defined(PLATFORM_WEB)
    // In the browser the save folder arrives asynchronously from IndexedDB
    if (!saveLoaded) {
        if (!saveFolderReady()) {
            BeginDrawing();
            ClearBackground((Color){ 11, 22, 48, 255 });
            DrawText("Loading...", 20, 20, 20, MENU_GOLD);
            EndDrawing();
            return;
        }
        loadSaveAndServices();
    }
#endif
    UpdateBackground(Gamevars.screenWidth, Gamevars.screenHeight);
    UpdateEffects();
    applyAdRewards();
    UpdateGameAudio(PreviewedTrack(), MusicAudible());

    if (!Chicken.dead && Gamevars.startgame) updateGameplay();
    else if (Menuvars.enabled) updateMenu();
    else updateDeath();

    drawFrame();
}

int main(void)
{
    Initgamevars();
    if (IS_MOBILE) InitWindow(0, 0, "Jumper Chicken");   // full screen on phones
    else InitWindow(Gamevars.screenWidth, Gamevars.screenHeight, "Jumper Chicken");
    InitVirtualScreen(&Gamevars.screenWidth, &Gamevars.screenHeight);
    InitAudioDevice();
#if !defined(PLATFORM_WEB)
    SetTargetFPS(60);   // in the browser emscripten_set_main_loop() sets the pace
#endif
    srand(time(NULL));

    prepareSaveDirectory();
#if !defined(PLATFORM_WEB)
    if (!IS_MOBILE) {
        Image icon = LoadImage("data/Chicken.png");
        SetWindowIcon(icon);
        UnloadImage(icon);
    }
#endif
    Initplayer(Gamevars.screenWidth, Gamevars.screenHeight);
    InitSpikes(Gamevars.screenWidth, Gamevars.screenHeight);
    Initcolors();
    InitCoin();
    InitCharacters();
    InitBackground(Gamevars.screenWidth, Gamevars.screenHeight);
#if !defined(PLATFORM_WEB)
    loadSaveAndServices();
#endif

#if defined(PLATFORM_WEB)
    // The browser calls gameFrame 60 times a second; this call never returns
    emscripten_set_main_loop(gameFrame, 60, 1);
#else
    while (!WindowShouldClose() && !Menuvars.quit) gameFrame();
#endif

    SaveGame();

    CloseAds();
    CloseGameAudio();
    UnloadCharacters();
    UnloadTexture(spikeLeftTex);
    UnloadTexture(spikeRightTex);
    UnloadTexture(maincoin.texture);
    UnloadSound(maincoin.sound);
    CloseVirtualScreen();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
