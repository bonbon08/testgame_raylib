// Android entry point for Jumper Chicken.
// raylib's own android_main() (in rcore_android.c) sets up the NativeActivity
// glue and then calls this regular main(), exactly like the desktop build.
#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

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

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Initgamevars();
    InitWindow(0, 0, "Jumper Chicken");   // full screen on phones
    InitVirtualScreen(&GetGamevars()->screenWidth, &GetGamevars()->screenHeight);
    SetTargetFPS(60);
    srand((unsigned int)time(NULL));

    PrepareSaveDirectory();
    LoadGame();
    InitAds();
    SubmitLeaderboardScore(GetGamevars()->highscore);
    Initplayer(GetGamevars()->screenWidth, GetGamevars()->screenHeight);
    InitSpikes(GetGamevars()->screenWidth, GetGamevars()->screenHeight);
    InitColors();
    InitCoin();
    InitCharacters();
    InitBackground(GetGamevars()->screenWidth, GetGamevars()->screenHeight);
    InitGameAudio(EquippedMusicTrack(), MusicOn());

    while (!WindowShouldClose() && !GetMenuvars()->quit)
    {
        UpdateBackground(GetGamevars()->screenWidth, GetGamevars()->screenHeight);
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

    return 0;
}
