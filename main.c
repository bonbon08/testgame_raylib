#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include "objects.c"


int main(void)
{
    Initgamevars();
    InitWindow(Gamevars.screenWidth, Gamevars.screenHeight, "Jumper Chicken");
    SetTargetFPS(60);
    srand(time(NULL));

    Initplayer(Gamevars.screenWidth, Gamevars.screenHeight);
    InitSpikes(Gamevars.screenWidth, Gamevars.screenHeight); 
    InitCoin();

    while (!WindowShouldClose())
    {
        if (!Chicken.dead && Gamevars.startgame) {
            Gamevars.score += 1;
            if (IsKeyPressed(KEY_X)) Chicken.velocityY = Chicken.jumpForce;

            Chicken.velocityY += Chicken.gravity;
            Chicken.pos.y += Chicken.velocityY;

            if (Chicken.pos.y > Gamevars.screenHeight - CHICKEN_SIZE) { Chicken.pos.y = Gamevars.screenHeight - CHICKEN_SIZE; Chicken.velocityY = 0; }
            if (Chicken.pos.y < 0) { Chicken.pos.y = 0; Chicken.velocityY = 0; }

            if (Chicken.direction == 0) Chicken.pos.x += 8;
            if (Chicken.direction == 2) Chicken.pos.x -= 8;

            if (Chicken.pos.x >= Gamevars.screenWidth - CHICKEN_SIZE) {
                Chicken.direction = 2; 
                Chicken.pos.x = Gamevars.screenWidth - CHICKEN_SIZE;
                generateSpikes(Gamevars.screenWidth, Gamevars.screenHeight, true); 
            } else if (Chicken.pos.x <= 0) {
                Chicken.direction = 0; 
                Chicken.pos.x = 0;
                generateSpikes(Gamevars.screenWidth, Gamevars.screenHeight, false); 
            }

            Gamevars.frameCounter++;
            if (Gamevars.frameCounter % 20 == 0) Chicken.spr = 1 - Chicken.spr;

            Rectangle chickenRect = { Chicken.pos.x + 8, Chicken.pos.y + 8, CHICKEN_SIZE-(0.2*CHICKEN_SIZE), CHICKEN_SIZE-(0.2*CHICKEN_SIZE) };
            
            for (int i = 0; i < NUM_SPIKES; i++) {
                Rectangle leftSpikeRect = { lspikes[i].pos.x, lspikes[i].pos.y + 20, 44, 24 };
                Rectangle rightSpikeRect = { rspikes[i].pos.x + 20, rspikes[i].pos.y + 20, 44, 24 };
                
                if ((lspikes[i].active && CheckCollisionRecs(chickenRect, leftSpikeRect)) ||
                    (rspikes[i].active && CheckCollisionRecs(chickenRect, rightSpikeRect))) {
                    Chicken.dead = true;
                    break;
                }
            }
            Rectangle coinRect = { maincoin.pos.x+12, maincoin.pos.y+4, 40, 56};
            if (CheckCollisionRecs(chickenRect, coinRect)) {
                generateCoin(Gamevars.screenWidth, Gamevars.screenHeight);
                Gamevars.Coins += 1;
                Gamevars.score += 100;
            }
        } else if (!Gamevars.startgame) { 
            if (IsKeyPressed(KEY_X)) {
                Gamevars.startgame = true;
                generateCoin(Gamevars.screenWidth, Gamevars.screenHeight);

            }
        }else {
            if (IsKeyPressed(KEY_X)) {
                Chicken.dead = false;
                if (Gamevars.highscore<Gamevars.score) {Gamevars.highscore=Gamevars.score;}
                Gamevars.score = 0;
                Chicken.pos = (Vector2){ Gamevars.screenWidth/2.0f, Gamevars.screenHeight/2.0f };
                Chicken.velocityY = 0;
                Chicken.spr = 0;
                Chicken.direction = 0;
                generateSpikes(Gamevars.screenWidth, Gamevars.screenHeight, false);
            }
        }

        BeginDrawing();
        ClearBackground(BLUE);

        for (int i = 0; i < NUM_SPIKES; i++) {
            
            
            if (lspikes[i].active) DrawTextureEx(lspikes[i].spiketexture, lspikes[i].pos, 0.0f, 1.0f, WHITE);
            if (rspikes[i].active) DrawTextureEx(lspikes[i].spiketexture, rspikes[i].pos, 0.0f, 1.0f, WHITE);
        }

        if (!Chicken.dead && Gamevars.startgame) {
            DrawTextureEx(maincoin.texture, maincoin.pos, 0.0f, 1.0f, WHITE);
            DrawTexture(Chicken.textures[Chicken.spr + Chicken.direction],
                        (int)Chicken.pos.x, (int)Chicken.pos.y, WHITE);
        } else if (!Gamevars.startgame) {
            DrawText("PRESS X TO TART", Gamevars.screenWidth/4+60, (Gamevars.screenHeight/2)-30, 40, BLACK);
        } else {
            DrawText("YOU DIED! PRESS X TO RESTART", Gamevars.screenWidth/4-80, (Gamevars.screenHeight/2)-30, 40, RED);
            DrawText(TextFormat("Highscore: %d", Gamevars.highscore), Gamevars.screenWidth/4+140, (Gamevars.screenHeight/2)+20, 40, RED);
            DrawText(TextFormat("Score: %d", Gamevars.score), Gamevars.screenWidth/4+140, (Gamevars.screenHeight/2)+70, 40, RED);
        }
        DrawText(TextFormat("Score: %d", Gamevars.score), 8, 8, 20, WHITE);
        DrawText(TextFormat("Coins: %d", Gamevars.Coins), 8, 32, 20, YELLOW);
        EndDrawing();
    }

    for (int i = 0; i < NUM_TEXTURES; i++) UnloadTexture(Chicken.textures[i]);
    for (int i = 0; i < NUM_SPIKES; i++) UnloadTexture(lspikes[i].spiketexture);
    for (int i = 0; i < NUM_SPIKES; i++) UnloadTexture(rspikes[i].spiketexture);
    CloseWindow();
    
    return 0;
}