// Common input handling for Desktop and Android
// Build flag PLATFORM_ANDROID activates platform-specific behavior
#include "input.h"

#include <raylib.h>

#if defined(PLATFORM_ANDROID)
#define IS_MOBILE true
#else
#define IS_MOBILE false
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

static VirtualScreen Screen;

static void updateVirtualScale(void) {
    float scaleX = GetScreenWidth() / (float)Screen.width;
    float scaleY = GetScreenHeight() / (float)Screen.height;
    Screen.scale = scaleX < scaleY ? scaleX : scaleY;
    Screen.offset = (Vector2){
        (GetScreenWidth() - Screen.width * Screen.scale) / 2,
        (GetScreenHeight() - Screen.height * Screen.scale) / 2
    };
}

static void BeginVirtualScreenInternal(void) {
    updateVirtualScale();
    BeginTextureMode(Screen.target);
}

static void EndVirtualScreenInternal(void) {
    EndTextureMode();
    BeginDrawing();
    ClearBackground(BLACK);
    Rectangle source = { 0, 0, (float)Screen.width, -(float)Screen.height };
    Rectangle dest = {
        Screen.offset.x, Screen.offset.y,
        Screen.width * Screen.scale, Screen.height * Screen.scale
    };
    DrawTexturePro(Screen.target.texture, source, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);
    EndDrawing();
}

// InitVirtualScreen: called once at startup with final window dimensions
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

void CloseVirtualScreen(void) {
    UnloadRenderTexture(Screen.target);
}

// A click on desktop or a tap on the phone
bool IsMousePressed(void) {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// Pointer position in virtual screen coordinates
static Vector2 GetPointerPosition(void) {
#if defined(PLATFORM_ANDROID)
    Vector2 pos = GetTouchPosition(0);
#else
    Vector2 pos = GetMousePosition();
#endif
    return (Vector2){
        (pos.x - Screen.offset.x) / Screen.scale,
        (pos.y - Screen.offset.y) / Screen.scale
    };
}

bool IsPointerPressedIn(Rectangle area) {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
           CheckCollisionPointRec(GetPointerPosition(), area);
}

// BACKSPACE on desktop, the back button on Android
bool IsBackPressed(void) {
    return IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_BACK);
}

#endif /* INPUT_C */
