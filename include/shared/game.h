// Game loop declarations (platform-independent)
#ifndef GAME_H
#define GAME_H

#include <raylib.h>

// Game state accessors (defined in game.c / objects.c / shop.c)
extern int Shopvars_musicOn;   // placeholder; real accessor via shop.h

// Called once per frame from platform main
void UpdateGameplay(void);
void UpdateMenu(void);
void UpdateDeath(void);
void DrawFrame(void);
void StartGame(void);
void BackToMenu(void);
void ApplyAdRewards(void);

// Called once at startup
void InitGame(void);

// Called once at shutdown
void ShutdownGame(void);

#endif // GAME_H