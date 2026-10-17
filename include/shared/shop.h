// Shop system (characters, backgrounds, music, hats, nest, abilities)
#ifndef SHOP_H
#define SHOP_H

#include "abilities.h"

#include <raylib.h>
#include <stdbool.h>

#define NUM_CHARACTERS 5
#define SHOP_ROWS 6
#define ROW_CHARACTERS 0
#define ROW_BACKGROUNDS 1
#define ROW_MUSIC 2
#define ROW_HUT 3
#define ROW_HATS 4
#define ROW_ABILITIES 5

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

// Nest: a one-off purchase that slowly earns coins in real time, even while the
// game isn't running. Not to be confused with the CO-OP multiplayer mode (coop.h) -
// this is just a passive income building.
bool OwnsHut(void);
int LastHutEarnings(void);   // coins credited since the save was last loaded, 0 if none

// Hats: purely cosmetic headwear, equip one at a time like characters/backgrounds/music.
// Drawn on top of the chicken during gameplay.
void DrawEquippedHat(Vector2 chickenCenter, float rotation);

// Abilities (see abilities.h for the ABILITY_* indices): a one-off unlock per
// ability, bought here before its pickup can ever spawn in a level.
bool AbilityUnlocked(int ability);

#endif // SHOP_H
