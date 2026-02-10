#include "raylib.h"

int main(void)
{
    const int screenWidth = 1080;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Jumper Chicken");

    Vector2 chickenPosition = { (float)screenWidth/2, (float)screenHeight/2 };
    int Jumpframes = 0;
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {   
        if (IsKeyDown(KEY_X)) {
            Jumpframes = 10;
        };

        if (Jumpframes >= 1) {
            chickenPosition.y -= 5.0f;
            Jumpframes -= 1;
        } else {
            chickenPosition.y += 5.0f;
        };
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawCircleV(chickenPosition, 30.0f, MAROON);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
