// Desktop platform layer
#ifndef PLATFORM_DESKTOP_H
#define PLATFORM_DESKTOP_H

#include <stdbool.h>

// Desktop platform initialization
void Desktop_Init();
void Desktop_Shutdown();

// Desktop-specific input handling
void Desktop_ProcessInput(void* arg);
void Desktop_HandleInputEvents(void* arg);

// Desktop-specific audio
void Desktop_InitAudio();
void Desktop_StopAudio();

// Desktop-specific ads (not supported)
void Desktop_InitAds();
void Desktop_ShowAd();
void Desktop_HideAd();

#endif // PLATFORM_DESKTOP_H