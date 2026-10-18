// Input abstraction layer
#ifndef INPUT_H
#define INPUT_H

#include <raylib.h>
#include <stdbool.h>

#if defined(PLATFORM_ANDROID)
#define IS_MOBILE true
#else
#define IS_MOBILE false
#endif

// A click on desktop or a tap on the phone
bool PointerPressed(void);

// Pointer position in virtual screen coordinates
Vector2 PointerPosition(void);

// Pointer pressed in rectangle
bool PointerPressedIn(Rectangle area);

// Back button pressed (BACKSPACE on desktop, back button on Android)
bool BackPressed(void);

// Fast-fall held (desktop only: DOWN/S key or the right/side mouse button; always false on Android)
bool FastFallHeld(void);

// Virtual screen management
void InitVirtualScreen(int *width, int *height);
void BeginVirtualScreen(void);
void EndVirtualScreen(void);
void CloseVirtualScreen(void);

#endif // INPUT_H