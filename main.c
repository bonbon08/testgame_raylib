#include "raylib.h"

int main(void)
{
    const int screenWidth = 1080;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Jumper Chicken");

    Image chickenImage = LoadImage("Chicken.png");
    ImageResize(&chickenImage, 64, 64);

    Texture2D chickenTexture = LoadTextureFromImage(chickenImage);
    UnloadImage(chickenImage); 

    Vector2 chickenPosition = { (float)screenWidth/2, (float)screenHeight/2 };
    int jumpFrames = 0;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {   
        if (IsKeyPressed(KEY_X)) {  
            jumpFrames = 12;
        }

        if (jumpFrames > 0) {
            chickenPosition.y -= 5.0f;
            jumpFrames--;
        } else {
            chickenPosition.y += 3.0f;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTexture(chickenTexture, 
                    (int)chickenPosition.x, 
                    (int)chickenPosition.y, 
                    WHITE);

        EndDrawing();
    }

    UnloadTexture(chickenTexture);
    CloseWindow();
    return 0;
}
