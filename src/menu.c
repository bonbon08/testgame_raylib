#include "raylib.h"
#include <math.h>

#define MENU_OPTIONS 3
#define DEATH_BUTTON_RETRY 0
#define DEATH_BUTTON_MENU 1
#define DEATH_BUTTON_REVIVE 2

const char *menuOptions[MENU_OPTIONS] = { "START", "SHOP", "EXIT" };

const Color MENU_ORANGE = { 255, 140, 40, 255 };
const Color MENU_GOLD = { 255, 214, 64, 255 };
const Color MENU_GREEN = { 80, 190, 110, 255 };

void drawTextCentered(const char *text, int centerX, int y, int fontSize, Color color) {
    DrawText(text, centerX - MeasureText(text, fontSize) / 2, y, fontSize, color);
}

void drawTextShadowCentered(const char *text, int centerX, int y, int fontSize, Color color) {
    drawTextCentered(text, centerX + 4, y + 4, fontSize, Fade(BLACK, 0.35f * color.a / 255.0f));
    drawTextCentered(text, centerX, y, fontSize, color);
}

void drawButton(Rectangle button, const char *text, int fontSize, Color fill, float alpha) {
    DrawRectangleRounded(button, 1.0f, 12, Fade(fill, alpha));
    DrawRectangleRoundedLinesEx(button, 1.0f, 12, 3.0f, Fade(MENU_GOLD, alpha));
    drawTextCentered(text, (int)(button.x + button.width / 2), (int)(button.y + (button.height - fontSize) / 2), fontSize, Fade(WHITE, alpha));
}

Rectangle menuPanelRect(int screenWidth) {
    return (Rectangle){ screenWidth / 2 - 220, 315, 440, 290 };
}

// The tappable area of a main menu entry
Rectangle MenuOptionRect(int option, int screenWidth) {
    Rectangle panel = menuPanelRect(screenWidth);
    return (Rectangle){ screenWidth / 2 - 150, panel.y + 21 + option * 68, 300, 58 };
}

Rectangle MusicButtonRect() {
    return (Rectangle){ 24, 24, 190, 50 };
}

Rectangle PrivacyButtonRect(int screenWidth, int screenHeight) {
    return (Rectangle){ screenWidth - 170, screenHeight - 64, 150, 44 };
}

void DrawMainMenu(int screenWidth, int screenHeight, int selected, Texture2D chickenTexture, int highscore, int coins, bool showPrivacy,
                  bool musicOn) {
    int centerX = screenWidth / 2;
    float time = (float)GetTime();

    int titleY = 70 + (int)(6 * sinf(time * 2.0f));
    drawTextShadowCentered("JUMPER CHICKEN", centerX, titleY, 80, MENU_GOLD);

    float chickenBob = 10 * sinf(time * 3.0f);
    Rectangle source = { 0, 0, (float)chickenTexture.width, (float)chickenTexture.height };
    Rectangle dest = { (float)centerX, 235 + chickenBob, chickenTexture.width * 2.0f, chickenTexture.height * 2.0f };
    DrawTexturePro(chickenTexture, source, dest, (Vector2){ dest.width / 2, dest.height / 2 }, 8 * sinf(time * 3.0f), WHITE);

    Rectangle panel = menuPanelRect(screenWidth);
    DrawRectangleRounded(panel, 0.2f, 12, Fade(BLACK, 0.25f));

    for (int i = 0; i < MENU_OPTIONS; i++) {
        Rectangle option = MenuOptionRect(i, screenWidth);
        int textY = (int)option.y + 11;
        if (i == selected) {
            float pulse = 4 * sinf(time * 5.0f);
            Rectangle pill = { option.x - pulse, option.y - pulse / 2, option.width + 2 * pulse, option.height + pulse };
            DrawRectangleRounded(pill, 1.0f, 12, MENU_ORANGE);
            DrawRectangleRoundedLinesEx(pill, 1.0f, 12, 3.0f, MENU_GOLD);
            drawTextCentered(menuOptions[i], centerX, textY, 36, WHITE);
        } else {
            drawTextCentered(menuOptions[i], centerX, textY, 36, Fade(WHITE, 0.75f));
        }
    }

    drawTextCentered(TextFormat("Highscore: %d", highscore), centerX - 100, (int)(panel.y + panel.height) - 45, 24, WHITE);
    drawTextCentered(TextFormat("Coins: %d", coins), centerX + 110, (int)(panel.y + panel.height) - 45, 24, YELLOW);

    const char *hint = IS_MOBILE ? "Tap to choose   -   tap to jump" : "UP / DOWN to choose   -   X to confirm   -   M for music";
    drawTextShadowCentered(hint, centerX, screenHeight - 50, 20, WHITE);

    drawButton(MusicButtonRect(), musicOn ? "MUSIC: ON" : "MUSIC: OFF", 22, Fade(BLACK, musicOn ? 0.4f : 0.25f),
               musicOn ? 1.0f : 0.6f);
    if (showPrivacy) drawButton(PrivacyButtonRect(screenWidth, screenHeight), "PRIVACY", 20, Fade(BLACK, 0.4f), 1.0f);
}

Rectangle deathPanelRect(int screenWidth, int screenHeight, bool canRevive, float fade) {
    float width = canRevive ? 620 : 520;
    return (Rectangle){ screenWidth / 2 - width / 2, screenHeight / 2 - 180 + 30 * (1 - fade), width, 340 };
}

// The buttons on the death screen; the revive button only exists when an ad can be offered
Rectangle DeathButtonRect(int button, int screenWidth, int screenHeight, bool canRevive) {
    Rectangle panel = deathPanelRect(screenWidth, screenHeight, canRevive, 1.0f);
    int count = canRevive ? 3 : 2;
    float width = 170, gap = 20;
    float startX = screenWidth / 2 - (count * width + (count - 1) * gap) / 2;
    return (Rectangle){ startX + button * (width + gap), panel.y + panel.height - 82, width, 56 };
}

// fade goes from 0 to 1 while the screen appears
void DrawDeathScreen(int screenWidth, int screenHeight, int score, int highscore, bool newHighscore,
                     bool canRevive, bool reviveReady, float fade) {
    int centerX = screenWidth / 2;
    float time = (float)GetTime();

    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.25f * fade));

    Rectangle panel = deathPanelRect(screenWidth, screenHeight, canRevive, fade);
    DrawRectangleRounded(panel, 0.15f, 12, Fade(BLACK, 0.45f * fade));
    DrawRectangleRoundedLinesEx(panel, 0.15f, 12, 3.0f, Fade(MENU_GOLD, fade));

    int y = (int)panel.y + 26;
    drawTextShadowCentered("YOU DIED!", centerX, y, 64, Fade((Color){ 255, 90, 80, 255 }, fade));

    if (newHighscore) {
        float pulse = 0.6f + 0.4f * sinf(time * 8.0f);
        drawTextCentered("NEW HIGHSCORE!", centerX, y + 80, 30, Fade(MENU_GOLD, fade * pulse));
    }
    drawTextCentered(TextFormat("Score: %d", score), centerX, y + 124, 36, Fade(WHITE, fade));
    drawTextCentered(TextFormat("Highscore: %d", highscore), centerX, y + 168, 26, Fade(YELLOW, fade));

    float offsetY = 30 * (1 - fade);
    Rectangle retry = DeathButtonRect(DEATH_BUTTON_RETRY, screenWidth, screenHeight, canRevive);
    Rectangle menu = DeathButtonRect(DEATH_BUTTON_MENU, screenWidth, screenHeight, canRevive);
    retry.y += offsetY;
    menu.y += offsetY;
    drawButton(retry, "RETRY", 28, MENU_ORANGE, fade);
    drawButton(menu, "MENU", 28, Fade(BLACK, 0.5f), fade);
    if (canRevive) {
        Rectangle revive = DeathButtonRect(DEATH_BUTTON_REVIVE, screenWidth, screenHeight, canRevive);
        revive.y += offsetY;
        drawButton(revive, reviveReady ? "REVIVE (AD)" : "AD LOADING", 22, MENU_GREEN, fade * (reviveReady ? 1.0f : 0.5f));
    }

    if (!IS_MOBILE) {
        drawTextShadowCentered("X: retry   -   BACKSPACE: menu", centerX, (int)(panel.y + panel.height) + 20, 20, Fade(WHITE, 0.85f * fade));
    }
}
