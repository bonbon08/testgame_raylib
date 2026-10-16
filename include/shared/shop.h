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

// Shop state accessors
void LoadGame(void);
void SaveGame(void);
void UpdateShop(int screenWidth);
void DrawShop(int screenWidth, int screenHeight);
Texture2D CurrentCharacterTexture(int direction);
int PreviewedBackground(void);
int PreviewedTrack(void);
bool MusicAudible(void);

// Shop item info
int shopRowSize(int row);
const char *shopItemName(int row, int item);
int shopItemPrice(int row, int item);
bool owns(int mask, int item);

#endif // SHOP_H