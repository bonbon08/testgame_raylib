#include "raylib.h"
#include <stdbool.h>

#ifndef IS_MOBILE   // can be forced with -DIS_MOBILE=true, e.g. for store screenshots
#if defined(PLATFORM_ANDROID)
#define IS_MOBILE true
#else
#define IS_MOBILE false
#endif
#endif

#define MIN_VIRTUAL_WIDTH 960

// The game is drawn into a texture with a fixed height and then scaled to the real screen
typedef struct VirtualScreen {
    RenderTexture2D target;
    int width;
    int height;
    float scale;
    Vector2 offset;
} VirtualScreen;

VirtualScreen Screen;

void updateVirtualScale() {
    float scaleX = GetScreenWidth() / (float)Screen.width;
    float scaleY = GetScreenHeight() / (float)Screen.height;
    Screen.scale = scaleX < scaleY ? scaleX : scaleY;
    Screen.offset = (Vector2){ (GetScreenWidth() - Screen.width * Screen.scale) / 2,
                               (GetScreenHeight() - Screen.height * Screen.scale) / 2 };
}

// On phones the width grows with the display aspect ratio, so wide screens are filled without black bars
void InitVirtualScreen(int *width, int *height) {
    if (IS_MOBILE) {
        float aspect = GetScreenWidth() / (float)GetScreenHeight();
        *width = (int)(*height * aspect);
        if (*width < MIN_VIRTUAL_WIDTH) *width = MIN_VIRTUAL_WIDTH;
    }
    Screen.width = *width;
    Screen.height = *height;
    Screen.target = LoadRenderTexture(*width, *height);
    SetTextureFilter(Screen.target.texture, TEXTURE_FILTER_BILINEAR);
    updateVirtualScale();
}

void BeginVirtualScreen() {
    updateVirtualScale();
    BeginTextureMode(Screen.target);
}

void EndVirtualScreen() {
    EndTextureMode();
    BeginDrawing();
    ClearBackground(BLACK);
    Rectangle source = { 0, 0, (float)Screen.width, -(float)Screen.height };
    Rectangle dest = { Screen.offset.x, Screen.offset.y, Screen.width * Screen.scale, Screen.height * Screen.scale };
    DrawTexturePro(Screen.target.texture, source, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);
    EndDrawing();
}

void CloseVirtualScreen() {
    UnloadRenderTexture(Screen.target);
}

// A click on desktop or a tap on the phone
bool PointerPressed() {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// Pointer position in virtual screen coordinates
Vector2 PointerPosition() {
#if defined(PLATFORM_ANDROID)
    Vector2 pos = GetTouchPosition(0);
#else
    Vector2 pos = GetMousePosition();
#endif
    return (Vector2){ (pos.x - Screen.offset.x) / Screen.scale, (pos.y - Screen.offset.y) / Screen.scale };
}

bool PointerPressedIn(Rectangle area) {
    return PointerPressed() && CheckCollisionPointRec(PointerPosition(), area);
}

// BACKSPACE on desktop, the back button on Android
bool BackPressed() {
    return IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_BACK);
}
