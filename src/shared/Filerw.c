// Save game file handling (key=value text file with atomic write)
#include "filerw.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SAVE_FILE "data/save.txt"
#define SAVE_TMP_FILE "data/save.tmp"
#define LEGACY_HIGHSCORE_FILE "data/hscore.txt"
#define LEGACY_COINS_FILE "data/coins.txt"

static void defaultSave(SaveData *save) {
    save->highscore = 0;
    save->coins = 0;
    save->ownedCharacters = 1;
    save->ownedBackgrounds = 1;
    save->ownedMusic = 1;
    save->character = 0;
    save->background = 0;
    save->music = 0;
    save->musicOn = 1;
    save->hasHut = 0;
    save->hutLastCollect = 0;
    save->ownedHats = 1;
    save->hat = 0;
    save->unlockedAbilities = 0;
}

static void sanitizeSave(SaveData *save) {
    if (save->highscore < 0) save->highscore = 0;
    if (save->coins < 0) save->coins = 0;
    save->ownedCharacters |= 1;   // default items are always owned
    save->ownedBackgrounds |= 1;
    save->ownedMusic |= 1;
    save->ownedHats |= 1;
    save->musicOn = save->musicOn != 0;
    save->hasHut = save->hasHut != 0;
    if (save->hutLastCollect < 0) save->hutLastCollect = 0;
}

static bool readLegacyInt(const char *filename, int *value) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) return false;
    bool ok = fscanf(fp, "%d", value) == 1;
    fclose(fp);
    return ok;
}

// Returns false when no save file exists yet (defaults or legacy values are used then)
bool LoadSave(SaveData *save) {
    defaultSave(save);

    FILE *fp = fopen(SAVE_FILE, "r");
    if (fp == NULL) {
        // Older versions stored highscore and coins in two separate files
        int value;
        if (readLegacyInt(LEGACY_HIGHSCORE_FILE, &value)) save->highscore = value;
        if (readLegacyInt(LEGACY_COINS_FILE, &value)) save->coins = value;
        sanitizeSave(save);
        return false;
    }

    char line[128];
    char key[32];
    long value;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (sscanf(line, "%31[^=]=%ld", key, &value) != 2) continue;

        if (strcmp(key, "highscore") == 0) save->highscore = (int)value;
        else if (strcmp(key, "coins") == 0) save->coins = (int)value;
        else if (strcmp(key, "owned_characters") == 0) save->ownedCharacters = (int)value;
        else if (strcmp(key, "owned_backgrounds") == 0) save->ownedBackgrounds = (int)value;
        else if (strcmp(key, "character") == 0) save->character = (int)value;
        else if (strcmp(key, "background") == 0) save->background = (int)value;
        else if (strcmp(key, "owned_music") == 0) save->ownedMusic = (int)value;
        else if (strcmp(key, "music") == 0) save->music = (int)value;
        else if (strcmp(key, "music_on") == 0) save->musicOn = (int)value;
        else if (strcmp(key, "has_hut") == 0) save->hasHut = (int)value;
        else if (strcmp(key, "hut_last_collect") == 0) save->hutLastCollect = value;
        else if (strcmp(key, "owned_hats") == 0) save->ownedHats = (int)value;
        else if (strcmp(key, "hat") == 0) save->hat = (int)value;
        else if (strcmp(key, "unlocked_abilities") == 0) save->unlockedAbilities = (int)value;
    }
    fclose(fp);

    sanitizeSave(save);
    return true;
}

// Writes to a temporary file first, so a crash while saving never destroys the old save
bool WriteSave(const SaveData *save) {
    FILE *fp = fopen(SAVE_TMP_FILE, "w");
    if (fp == NULL) return false;

    bool ok = fprintf(fp,
        "highscore=%d\ncoins=%d\nowned_characters=%d\nowned_backgrounds=%d\nowned_music=%d\n"
        "character=%d\nbackground=%d\nmusic=%d\nmusic_on=%d\n"
        "has_hut=%d\nhut_last_collect=%ld\nowned_hats=%d\nhat=%d\nunlocked_abilities=%d\n",
        save->highscore, save->coins,
        save->ownedCharacters, save->ownedBackgrounds, save->ownedMusic,
        save->character, save->background, save->music, save->musicOn,
        save->hasHut, save->hutLastCollect, save->ownedHats, save->hat, save->unlockedAbilities) > 0;
    if (fclose(fp) != 0) ok = false;

    if (!ok) {
        remove(SAVE_TMP_FILE);
        return false;
    }

    if (rename(SAVE_TMP_FILE, SAVE_FILE) != 0) {
        // Windows refuses to rename onto an existing file
        remove(SAVE_FILE);
        if (rename(SAVE_TMP_FILE, SAVE_FILE) != 0) return false;
    }
    return true;
}
