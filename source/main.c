#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <switch.h>

#define NUM_TEXTURES 4
#define NUM_COINS 5

typedef struct Coin {
    Vector2 pos;
    bool active;
} Coin;

Coin coins[NUM_COINS];   
#define SLOT_HEIGHT 64 
#define CHICKEN_SIZE 64

int score = 0;
int highscore = 0;
int coinScore = 0;
bool startgame = false;

typedef struct Spike {
    Vector2 pos;
    bool active;
} Spike;

Spike lspikes[NUM_SPIKES];
Spike rspikes[NUM_SPIKES];

void generateSpikes(int screenWidth, int screenHeight, bool activateLeft) {
    const int totalSlots = screenHeight / SLOT_HEIGHT; 
    
    int *allSlots = (int *)malloc(totalSlots * sizeof(int));
    for (int i = 0; i < totalSlots; i++) {
        allSlots[i] = i;
    }
    
    for (int i = totalSlots - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = allSlots[i];
        allSlots[i] = allSlots[j];
        allSlots[j] = temp;
    }
    
    for (int i = 0; i < NUM_SPIKES; i++) {
        lspikes[i].pos = (Vector2){ 0, (float)(allSlots[i] * SLOT_HEIGHT) };
        lspikes[i].active = activateLeft;
        
        rspikes[i].pos = (Vector2){ (float)(screenWidth - 64), (float)(allSlots[i] * SLOT_HEIGHT) };
        rspikes[i].active = !activateLeft;
    }
    
    // Place coins between spike rows
    for (int i = 0; i < NUM_COINS; i++) {
        int coinSlot = rand() % totalSlots;
        coins[i].pos = (Vector2){ (float)(screenWidth + 64), (float)(allSlots[coinSlot] * SLOT_HEIGHT) };
        coins[i].active = true;
    }
    
    free(allSlots);
}

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    romfsInit();

    InitWindow(screenWidth, screenHeight, "Jumper Chicken");
    SetTargetFPS(60);
    srand(time(NULL));

    Image chickenImage1r = LoadImage("romfs:/resources/Chicken.png");
    ImageResize(&chickenImage1r, CHICKEN_SIZE, CHICKEN_SIZE);
    Image chickenImage2r = LoadImage("romfs:/resources/Chicken.png");
    ImageResize(&chickenImage2r, CHICKEN_SIZE, CHICKEN_SIZE);

    Image chickenImage1l = LoadImage("romfs:/resources/Chicken.png");
    ImageResize(&chickenImage1l, CHICKEN_SIZE, CHICKEN_SIZE);
    Image chickenImage2l = LoadImage("romfs:/resources/Chicken.png");
    ImageResize(&chickenImage2l, CHICKEN_SIZE, CHICKEN_SIZE);
    ImageFlipHorizontal(&chickenImage1l);
    ImageFlipHorizontal(&chickenImage2l);

    Texture2D chickentextures[NUM_TEXTURES] = {
        LoadTextureFromImage(chickenImage1r),
        LoadTextureFromImage(chickenImage2r),
        LoadTextureFromImage(chickenImage1l),
        LoadTextureFromImage(chickenImage2l)
    };

    UnloadImage(chickenImage1r); 
    UnloadImage(chickenImage2r); 
    UnloadImage(chickenImage1l); 
    UnloadImage(chickenImage2l); 

    Image spikel = LoadImage("romfs:/resources/nSpike.png");
    ImageResize(&spikel, SLOT_HEIGHT, SLOT_HEIGHT);  
    Texture2D spikeltexture = LoadTextureFromImage(spikel);
    UnloadImage(spikel);

    Image spiker = LoadImage("romfs:/resources/nSpike.png");
    ImageResize(&spiker, SLOT_HEIGHT, SLOT_HEIGHT);
    Texture2D spikertexture = LoadTextureFromImage(spiker);
    UnloadImage(spiker);

    Vector2 chickenPosition = { screenWidth/2.0f, screenHeight/2.0f };
    int chickenspr = 0;
    int chickendirection = 0; 
    float velocityY = 0.0f;
    const float gravity = 0.5f;
    const float jumpForce = -12.0f;
    bool dead = false;

    generateSpikes(screenWidth, screenHeight, false); 
    int frameCounter = 0;
    
    // Initialize coins
    for (int i = 0; i < NUM_COINS; i++) {
        coins[i].active = false;
    }

    while (!WindowShouldClose())
    {
        
        if (!dead && startgame) {
            score += 1;
            if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_F) || IsKeyPressed(KEY_Q)) velocityY = jumpForce;

            velocityY += gravity;
            chickenPosition.y += velocityY;

            if (chickenPosition.y > screenHeight - CHICKEN_SIZE) { chickenPosition.y = screenHeight - CHICKEN_SIZE; velocityY = 0; }
            if (chickenPosition.y < 0) { chickenPosition.y = 0; velocityY = 0; }

            if (chickendirection == 0) chickenPosition.x += 8;
            if (chickendirection == 2) chickenPosition.x -= 8;

            if (chickenPosition.x >= screenWidth - CHICKEN_SIZE) {
                chickendirection = 2; 
                chickenPosition.x = screenWidth - CHICKEN_SIZE;
                generateSpikes(screenWidth, screenHeight, true); 
            } else if (chickenPosition.x <= 0) {
                chickendirection = 0; 
                chickenPosition.x = 0;
                generateSpikes(screenWidth, screenHeight, false); 
            }

            frameCounter++;
            if (frameCounter % 20 == 0) chickenspr = 1 - chickenspr;

            Rectangle chickenRect = { chickenPosition.x + 8, chickenPosition.y + 8, CHICKEN_SIZE-(0.2*CHICKEN_SIZE), CHICKEN_SIZE-(0.2*CHICKEN_SIZE) };
            
            // Coin collection
            for (int i = 0; i < NUM_COINS; i++) {
                if (coins[i].active) {
                    Rectangle coinRect = { coins[i].pos.x - 8, coins[i].pos.y - 8, 16, 16 };
                    if (CheckCollisionRecs(chickenRect, coinRect)) {
                        coins[i].active = false;
                        coinScore++;
                    }
                }
            }
            
            for (int i = 0; i < NUM_SPIKES; i++) {
                Rectangle leftSpikeRect = { lspikes[i].pos.x, lspikes[i].pos.y + 20, 44, 24 };
                Rectangle rightSpikeRect = { rspikes[i].pos.x + 20, rspikes[i].pos.y + 20, 44, 24 };
                
                if ((lspikes[i].active && CheckCollisionRecs(chickenRect, leftSpikeRect)) ||
                    (rspikes[i].active && CheckCollisionRecs(chickenRect, rightSpikeRect))) {
                    dead = true;
                    break;
                }
            }
} else if (!startgame) { 
            if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_E)) {
                startgame = true;
            }
        } else {
            if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_E)) {
                dead = false;
                if (highscore<score) {highscore=score;}
                score = 0;
                chickenPosition = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
                velocityY = 0;
                chickenspr = 0;
                chickendirection = 0;
                generateSpikes(screenWidth, screenHeight, false);
            }
        }

        BeginDrawing();
        ClearBackground(BLUE);

        // Draw coins
        for (int i = 0; i < NUM_COINS; i++) {
            if (coins[i].active) {
                DrawRectangleV(coins[i].pos, (Vector2){ 16, 16 }, YELLOW);
            }
        }

        for (int i = 0; i < NUM_SPIKES; i++) {
            
            
            if (lspikes[i].active) DrawTextureEx(spikeltexture, lspikes[i].pos, 0.0f, 1.0f, WHITE);
            if (rspikes[i].active) DrawTextureEx(spikertexture, rspikes[i].pos, 0.0f, 1.0f, WHITE);
        }

        if (!dead && startgame) {
            DrawTexture(chickentextures[chickenspr + chickendirection],
                        (int)chickenPosition.x, (int)chickenPosition.y, WHITE);
        } else if (!startgame) {
            DrawText("PRESS A/B/X/Y TO START", screenWidth/4+60, (screenHeight/2)-30, 40, BLACK);
        } else {
            DrawText("YOU DIED! PRESS A/B/X/Y TO RESTART", screenWidth/4-80, (screenHeight/2)-30, 40, RED);
            DrawText(TextFormat("Highscore: %d", highscore), screenWidth/4+140, (screenHeight/2)+20, 40, RED);
            DrawText(TextFormat("Score: %d", score), screenWidth/4+140, (screenHeight/2)+70, 40, RED);
        }
        DrawText(TextFormat("Score: %d", score), 8, 8, 20, WHITE);
        EndDrawing();
    }

    for (int i = 0; i < NUM_TEXTURES; i++) UnloadTexture(chickentextures[i]);
    UnloadTexture(spikertexture);
    UnloadTexture(spikeltexture);
    CloseWindow();

    romfsExit();

    return 0;
}
