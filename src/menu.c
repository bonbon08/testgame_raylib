#include "raylib.h"
#include <math.h>

#define MENU_OPTIONS 2

const char *menuOptions[MENU_OPTIONS] = { "START", "EXIT" };

void drawTextCentered(const char *text, int centerX, int y, int fontSize, Color color) {
    DrawText(text, centerX - MeasureText(text, fontSize) / 2, y, fontSize, color);
}

void drawTextShadowCentered(const char *text, int centerX, int y, int fontSize, Color color) {
    drawTextCentered(text, centerX + 4, y + 4, fontSize, Fade(BLACK, 0.35f));
    drawTextCentered(text, centerX, y, fontSize, color);
}

void DrawMainMenu(int screenWidth, int screenHeight, int selected, Texture2D chickenTexture, int highscore, int coins) {
    int centerX = screenWidth / 2;
    float time = (float)GetTime();

    int titleY = 70 + (int)(6 * sinf(time * 2.0f));
    drawTextShadowCentered("JUMPER CHICKEN", centerX, titleY, 80, (Color){ 255, 214, 64, 255 });

    float chickenBob = 10 * sinf(time * 3.0f);
    DrawTextureEx(chickenTexture,
                  (Vector2){ centerX - chickenTexture.width, 175 + chickenBob },
                  0.0f, 2.0f, WHITE);

    Rectangle panel = { centerX - 220, 330, 440, 250 };
    DrawRectangleRounded(panel, 0.2f, 12, Fade(BLACK, 0.25f));

    for (int i = 0; i < MENU_OPTIONS; i++) {
        int optionY = (int)panel.y + 40 + i * 80;
        if (i == selected) {
            float pulse = 4 * sinf(time * 5.0f);
            Rectangle pill = { centerX - 150 - pulse, optionY - 12 - pulse / 2, 300 + 2 * pulse, 64 + pulse };
            DrawRectangleRounded(pill, 1.0f, 12, (Color){ 255, 140, 40, 255 });
            DrawRectangleRoundedLinesEx(pill, 1.0f, 12, 3.0f, (Color){ 255, 214, 64, 255 });
            drawTextCentered(menuOptions[i], centerX, optionY, 40, WHITE);
        } else {
            drawTextCentered(menuOptions[i], centerX, optionY, 40, Fade(WHITE, 0.75f));
        }
    }

    drawTextCentered(TextFormat("Highscore: %d", highscore), centerX - 100, (int)(panel.y + panel.height) - 50, 24, WHITE);
    drawTextCentered(TextFormat("Coins: %d", coins), centerX + 110, (int)(panel.y + panel.height) - 50, 24, YELLOW);

    drawTextShadowCentered("UP / DOWN to choose   -   X to confirm", centerX, screenHeight - 50, 20, WHITE);
}
