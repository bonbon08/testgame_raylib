// Android entry point (NativeActivity) for Jumper Chicken
#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <android/native_app_glue.h>

#include "shared/game.h"
#include "shared/objects.h"
#include "shared/input.h"
#include "shared/services.h"
#include "shared/audio.h"
#include "shared/background.h"
#include "shared/effects.h"
#include "shared/menu.h"
#include "shared/shop.h"
#include "shared/filerw.h"

// Forward declaration from Android framework
extern void *GetAndroidApp(void);

// Called once per frame from platform main
static void RunGameLoop(struct android_app *app)
{
    int screenWidth = 1080, screenHeight = 720;

    GetGamevars()->screenWidth = screenWidth;
    GetGamevars()->screenHeight = screenHeight;

    InitWindow(0, 0, "Jumper Chicken");   // full screen on phones
    InitVirtualScreen(&screenWidth, &screenHeight);
    GetGamevars()->screenWidth = screenWidth;
    GetGamevars()->screenHeight = screenHeight;

    SetTargetFPS(60);
    srand((unsigned int)time(NULL));

    // Prepare internal storage path on Android
    if (chdir(GetAndroidApp()->activity->internalDataPath) != 0)
        TraceLog(LOG_WARNING, "Could not open internal storage");
    mkdir("data", 0700);

    LoadGame();
    InitAds();
    SubmitLeaderboardScore(GetGamevars()->highscore);
    Initplayer(screenWidth, screenHeight);
    InitSpikes(screenWidth, screenHeight);
    InitColors();
    InitCoin();
    InitCharacters();
    InitBackground(screenWidth, screenHeight);
    InitGameAudio(EquippedMusicTrack(), MusicOn());

    while (!WindowShouldClose() && !GetMenuvars()->quit)
    {
        UpdateBackground(screenWidth, screenHeight);
        UpdateEffects();
        ApplyAdRewards();
        UpdateGameAudio(PreviewedTrack(), MusicAudible());

        if (!GetChicken()->dead && GetGamevars()->startgame)
            UpdateGameplay();
        else if (GetMenuvars()->enabled)
            UpdateMenu();
        else
            UpdateDeath();

        DrawFrame();
    }

    SaveGame();

    CloseAds();
    CloseGameAudio();
    UnloadCharacters();
    UnloadTexture(*GetSpikeLeftTex());
    UnloadTexture(*GetSpikeRightTex());
    UnloadTexture(GetMaincoin()->texture);
    UnloadSound(GetMaincoin()->sound);
    CloseVirtualScreen();
    CloseAudioDevice();
    CloseWindow();
}

// NativeActivity entry point
void android_main(struct android_app *app)
{
    app->onAppCmd = NULL;
    app->onInputEvent = NULL;
    RunGameLoop(app);
}