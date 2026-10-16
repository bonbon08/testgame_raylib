// Shop system (characters, backgrounds, music)
#ifndef SHOP_H
#define SHOP_H

#include <raylib.h>
#include <stdbool.h>

#define NUM_CHARACTERS 5
#define SHOP_ROWS 3
#define ROW_CHARACTERS 0
#define ROW_BACKGROUNDS 1
#define ROW_MUSIC 2

// Called once at startup/shutdown (needs a GL context, so after InitWindow)
void InitCharacters(void);
void UnloadCharacters(void);

// Shop state accessors
void LoadGame(void);
void SaveGame(void);
void UpdateShop(int screenWidth);
void DrawShop(int screenWidth, int screenHeight);
Texture2D CurrentCharacterTexture(int direction);
Color CurrentCharacterBodyColor(void);
Color CurrentCharacterShadeColor(void);
int EquippedMusicTrack(void);
int PreviewedBackground(void);
int PreviewedTrack(void);
bool MusicAudible(void);
bool MusicOn(void);
void ToggleMusic(void);

#endif // SHOP_H