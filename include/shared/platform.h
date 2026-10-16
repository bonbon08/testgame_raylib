// Platform abstraction layer
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>

// Platform detection
#define IS_DESKTOP
#define IS_ANDROID

// Platform-specific initialization
void InitializePlatform();
void ShutdownPlatform();

// Screen dimensions (platform-dependent)
typedef struct {
    int width;
    int height;
} ScreenDimensions;

// Platform-specific input handling
void ProcessInput(void* arg);
void HandleInputEvents(void* arg);

// Platform-specific audio
void InitializeAudio();
void StopAudio();

// Platform-specific ads
void InitializeAds();
void RequestAds();
void ShowAd();
void HideAd();

#endif // PLATFORM_H
