// Desktop entry point for Jumper Chicken
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

#if !defined(PLATFORM_ANDROID)
int main(void)
{
    Initgamevars();
    InitWindow(1080, 720, "Jumper Chicken");
    InitVirtualScreen(&GetGamevars()->screenWidth, &GetGamevars()->screenHeight);
    InitAudioDevice();
    SetTargetFPS(60);
    srand((unsigned int)time(NULL));

    PrepareSaveDirectory();
    LoadSave(NULL);   // load via shop.LoadGame
    InitAds();
    SubmitLeaderboardScore(GetGamevars()->highscore);
    Initplayer(GetGamevars()->screenWidth, GetGamevars()->screenHeight);
    InitSpikes(GetGamevars()->screenWidth, GetGamevars()->screenHeight);
    InitColors();
    InitCoin();
    InitCharacters();
    InitBackground(GetGamevars()->screenWidth, GetGamevars()->screenHeight);
    InitGameAudio(GetShopEquippedMusic(), GetShopMusicOn());

    while (!WindowShouldClose() && !GetMenuvars()->quit)
    {
        UpdateBackground(GetGamevars()->screenWidth, GetGamevars()->screenHeight);
        UpdateEffects();
        ApplyAdRewards();
        UpdateGameAudio(GetShopPreviewedTrack(), MusicAudible());

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
#endif /* !PLATFORM_ANDROID */