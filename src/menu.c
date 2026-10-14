#include "raylib.h"
#include <math.h>

#define MENU_OPTIONS 3

const char *menuOptions[MENU_OPTIONS] = { "START", "SHOP", "EXIT" };

const Color MENU_ORANGE = { 255, 140, 40, 255 };
const Color MENU_GOLD = { 255, 214, 64, 255 };

void drawTextCentered(const char *text, int centerX, int y, int fontSize, Color color) {
    DrawText(text, centerX - MeasureText(text, fontSize) / 2, y, fontSize, color);
}

void drawTextShadowCentered(const char *text, int centerX, int y, int fontSize, Color color) {
    drawTextCentered(text, centerX + 4, y + 4, fontSize, Fade(BLACK, 0.35f * color.a / 255.0f));
    drawTextCentered(text, centerX, y, fontSize, color);
}

void DrawMainMenu(int screenWidth, int screenHeight, int selected, Texture2D chickenTexture, int highscore, int coins) {
    int centerX = screenWidth / 2;
    float time = (float)GetTime();

    int titleY = 70 + (int)(6 * sinf(time * 2.0f));
    drawTextShadowCentered("JUMPER CHICKEN", centerX, titleY, 80, MENU_GOLD);

    float chickenBob = 10 * sinf(time * 3.0f);
    Rectangle source = { 0, 0, (float)chickenTexture.width, (float)chickenTexture.height };
    Rectangle dest = { (float)centerX, 235 + chickenBob, chickenTexture.width * 2.0f, chickenTexture.height * 2.0f };
    DrawTexturePro(chickenTexture, source, dest, (Vector2){ dest.width / 2, dest.height / 2 }, 8 * sinf(time * 3.0f), WHITE);

    Rectangle panel = { centerX - 220, 315, 440, 290 };
    DrawRectangleRounded(panel, 0.2f, 12, Fade(BLACK, 0.25f));

    for (int i = 0; i < MENU_OPTIONS; i++) {
        int optionY = (int)panel.y + 32 + i * 68;
        if (i == selected) {
            float pulse = 4 * sinf(time * 5.0f);
            Rectangle pill = { centerX - 150 - pulse, optionY - 11 - pulse / 2, 300 + 2 * pulse, 58 + pulse };
            DrawRectangleRounded(pill, 1.0f, 12, MENU_ORANGE);
            DrawRectangleRoundedLinesEx(pill, 1.0f, 12, 3.0f, MENU_GOLD);
            drawTextCentered(menuOptions[i], centerX, optionY, 36, WHITE);
        } else {
            drawTextCentered(menuOptions[i], centerX, optionY, 36, Fade(WHITE, 0.75f));
        }
    }

    drawTextCentered(TextFormat("Highscore: %d", highscore), centerX - 100, (int)(panel.y + panel.height) - 45, 24, WHITE);
    drawTextCentered(TextFormat("Coins: %d", coins), centerX + 110, (int)(panel.y + panel.height) - 45, 24, YELLOW);

    drawTextShadowCentered("UP / DOWN to choose   -   X to confirm", centerX, screenHeight - 50, 20, WHITE);
}

// fade goes from 0 to 1 while the screen appears
void DrawDeathScreen(int screenWidth, int screenHeight, int score, int highscore, bool newHighscore, float fade) {
    int centerX = screenWidth / 2;
    float time = (float)GetTime();

    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.25f * fade));

    Rectangle panel = { centerX - 260, screenHeight / 2 - 170 + 30 * (1 - fade), 520, 320 };
    DrawRectangleRounded(panel, 0.15f, 12, Fade(BLACK, 0.45f * fade));
    DrawRectangleRoundedLinesEx(panel, 0.15f, 12, 3.0f, Fade(MENU_GOLD, fade));

    int y = (int)panel.y + 30;
    drawTextShadowCentered("YOU DIED!", centerX, y, 64, Fade((Color){ 255, 90, 80, 255 }, fade));

    if (newHighscore) {
        float pulse = 0.6f + 0.4f * sinf(time * 8.0f);
        drawTextCentered("NEW HIGHSCORE!", centerX, y + 85, 30, Fade(MENU_GOLD, fade * pulse));
    }
    drawTextCentered(TextFormat("Score: %d", score), centerX, y + 135, 36, Fade(WHITE, fade));
    drawTextCentered(TextFormat("Highscore: %d", highscore), centerX, y + 180, 26, Fade(YELLOW, fade));

    drawTextCentered("X: retry   -   BACKSPACE: menu", centerX, y + 240, 22, Fade(WHITE, 0.85f * fade));
}
