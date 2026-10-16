// Menu system (main menu and death screen)
#ifndef MENU_H
#define MENU_H

#include <raylib.h>

#define MENU_OPTIONS 3
#define DEATH_BUTTON_RETRY 0
#define DEATH_BUTTON_MENU 1
#define DEATH_BUTTON_REVIVE 2

// Menu option rectangles
Rectangle MenuOptionRect(int option, int screenWidth);
Rectangle MusicButtonRect(void);
Rectangle LeaderboardButtonRect(int screenWidth);
Rectangle PrivacyButtonRect(int screenWidth, int screenHeight);
Rectangle DeathButtonRect(int button, int screenWidth, int screenHeight, bool canRevive);

// Drawing functions
void DrawMainMenu(int screenWidth, int screenHeight, int selected, Texture2D chickenTexture,
                  int highscore, int coins, bool showPrivacy, bool musicOn,
                  bool showLeaderboard, const char *playerName, int signInState);
void DrawDeathScreen(int screenWidth, int screenHeight, int score, int highscore,
                     bool newHighscore, bool canRevive, bool reviveReady, float fade);

#endif // MENU_H