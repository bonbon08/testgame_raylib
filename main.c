#include "raylib.h"

#define NUM_TEXTURES 4
int main(void)
{
    const int screenWidth = 1080;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Jumper Chicken");
    Image chickenImage1r = LoadImage("Chicken.png");
    ImageResize(&chickenImage1r, 64, 64);
    Image chickenImage2r = LoadImage("Chicken.png");
    ImageResize(&chickenImage2r, 64, 64);
    ImageRotate(&chickenImage1r, -20);
    ImageRotate(&chickenImage2r, 20);


    Image chickenImage1l = LoadImage("Chicken.png");
    ImageResize(&chickenImage1l, 64, 64);
    Image chickenImage2l = LoadImage("Chicken.png");
    ImageResize(&chickenImage2l, 64, 64);
    ImageRotate(&chickenImage1l, -20);
    ImageRotate(&chickenImage2l, 20);
    ImageFlipHorizontal(&chickenImage1l);
    ImageFlipHorizontal(&chickenImage2l);

    Texture2D chickentextures[NUM_TEXTURES] = {0};
    chickentextures[0] = LoadTextureFromImage(chickenImage1r);
    chickentextures[1] = LoadTextureFromImage(chickenImage2r);
    chickentextures[2] = LoadTextureFromImage(chickenImage1l);
    chickentextures[3] = LoadTextureFromImage(chickenImage2l);
    UnloadImage(chickenImage1r); 
    UnloadImage(chickenImage2r); 
    UnloadImage(chickenImage1l); 
    UnloadImage(chickenImage2l); 



    Vector2 chickenPosition = { (float)screenWidth/2, (float)screenHeight/2 };
    int jumpFrames = 0;
    int chickenspr = 0;
    int chickendirection = 0;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {   
        if (IsKeyPressed(KEY_X) && chickenPosition.y>=0) {  
            jumpFrames = 12;
            if (chickenspr == 0) {
                chickenspr = 1;
            } else {
                chickenspr = 0;
            };
        }

        if (jumpFrames > 0) {
            chickenPosition.y -= 8.0f;
            jumpFrames--;
        } else {
            chickenPosition.y += 6.0f;
        }
        if (chickenPosition.x >= (screenWidth-64)) chickendirection = 2;
        if (chickenPosition.x <= 0) chickendirection = 0;
        if (chickendirection == 0) chickenPosition.x += 8;
        if (chickendirection == 2) chickenPosition.x -= 8;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTexture(chickentextures[chickenspr+chickendirection], 
                    (int)chickenPosition.x, 
                    (int)chickenPosition.y, 
                    WHITE);

        EndDrawing();
    }

    for (int i = 0; i < NUM_TEXTURES; i++) UnloadTexture(chickentextures[i]);
    CloseWindow();
    return 0;
}
