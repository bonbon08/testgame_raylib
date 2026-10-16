// Android platform layer
#ifndef PLATFORM_ANDROID_H
#define PLATFORM_ANDROID_H

#include <stdbool.h>

// Android platform initialization
void Android_Init();
void Android_Shutdown();

// Android-specific input handling
void Android_ProcessInput(void* arg);
void Android_HandleInputEvents(void* arg);

// Android-specific audio
void Android_InitAudio();
void Android_StopAudio();

// Android-specific ads
void Android_InitAds();
void Android_ShowAd();
void Android_HideAd();

#endif // PLATFORM_ANDROID_H