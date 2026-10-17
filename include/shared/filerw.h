// Save game file handling
#ifndef FILERW_H
#define FILERW_H

#include <stdbool.h>

typedef struct SaveData {
    int highscore;
    int coins;
    int ownedCharacters;
    int ownedBackgrounds;
    int ownedMusic;
    int character;
    int background;
    int music;
    int musicOn;
    int hasHut;
    long hutLastCollect;   // unix timestamp of the last idle-coin payout, 0 = never
    int ownedHats;
    int hat;
    int unlockedAbilities;
} SaveData;

bool LoadSave(SaveData *save);
bool WriteSave(const SaveData *save);

#endif // FILERW_H