// Game loop declarations (platform-independent)
#ifndef GAME_H
#define GAME_H

#include <raylib.h>

// Called once per frame from platform main
void UpdateGameplay(void);
void UpdateMenu(void);
void UpdateDeath(void);
void DrawFrame(void);
void StartGame(void);
void BackToMenu(void);
void ApplyAdRewards(void);

// Called once at startup (Android: raylib writes into internal storage, so the
// save directory needs preparing there before anything reads or writes a save file)
void PrepareSaveDirectory(void);

#endif // GAME_H