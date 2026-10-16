// Shop system (characters, backgrounds, music)
#ifndef SHOP_H
#define SHOP_H

#include <raylib.h>
#include <stdbool.h>

#define NUM_CHARACTERS 5
#define SHOP_ROWS 5
#define ROW_CHARACTERS 0
#define ROW_BACKGROUNDS 1
#define ROW_MUSIC 2
#define ROW_ABILITIES 3
#define ROW_HUT 4

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

// Abilities (see abilities.h for the ABILITY_* indices)
bool OwnsAbility(int ability);

// Chicken Coop: a one-off purchase that slowly earns coins in real time,
// even while the game isn't running
bool OwnsHut(void);
int LastHutEarnings(void);   // coins credited since the save was last loaded, 0 if none

#endif // SHOP_H