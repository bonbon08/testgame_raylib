// Menu system (main menu and death screen)
#ifndef MENU_H
#define MENU_H

#include <raylib.h>

#define MENU_OPTIONS 3
#define DEATH_BUTTON_RETRY 0
#define DEATH_BUTTON_MENU 1
#define DEATH_BUTTON_REVIVE 2

// Shared UI colors (also used by shop.c)
extern const Color MENU_ORANGE;
extern const Color MENU_GOLD;
extern const Color MENU_GREEN;

// Shared text/button drawing helpers (also used by shop.c)
void drawTextCentered(const char *text, int centerX, int y, int fontSize, Color color);
void drawTextShadowCentered(const char *text, int centerX, int y, int fontSize, Color color);
void drawButton(Rectangle button, const char *text, int fontSize, Color fill, float alpha);

// Menu option rectangles
Rectangle MenuOptionRect(int option, int screenWidth);
Rectangle MusicButtonRect(void);
Rectangle LeaderboardButtonRect(int screenWidth);
Rectangle PrivacyButtonRect(int screenWidth, int screenHeight);
Rectangle CoopButtonRect(int screenWidth, int screenHeight);
Rectangle DeathButtonRect(int button, int screenWidth, int screenHeight, bool canRevive);

// Drawing functions
void DrawMainMenu(int screenWidth, int screenHeight, int selected, Texture2D chickenTexture,
                  int highscore, int coins, bool showPrivacy, bool musicOn,
                  bool showLeaderboard, const char *playerName, int signInState,
                  bool showCoop);
void DrawDeathScreen(int screenWidth, int screenHeight, int score, int highscore,
                     bool newHighscore, bool canRevive, bool reviveReady, float fade);

#endif // MENU_H