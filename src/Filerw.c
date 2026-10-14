#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#if defined(PLATFORM_WEB)
// In the browser the save folder is kept in IndexedDB, see the helpers below
#define SAVE_FILE "save/save.txt"
#define SAVE_TMP_FILE "save/save.tmp"
#else
#define SAVE_FILE "data/save.txt"
#define SAVE_TMP_FILE "data/save.tmp"
#endif
#define LEGACY_HIGHSCORE_FILE "data/hscore.txt"
#define LEGACY_COINS_FILE "data/coins.txt"

#if defined(PLATFORM_WEB)
#include <emscripten.h>

// Mounts /save as IndexedDB-backed folder and starts loading it (asynchronous)
EM_JS(void, mountSaveFolder, (), {
    FS.mkdir('/save');
    FS.mount(IDBFS, {}, '/save');
    Module.saveFolderReady = 0;
    FS.syncfs(true, function (err) { Module.saveFolderReady = 1; });
});

EM_JS(int, saveFolderReady, (), { return Module.saveFolderReady; });

// Writes the in-memory save folder back to IndexedDB
EM_JS(void, flushSaveFolder, (), { FS.syncfs(false, function (err) {}); });
#endif

typedef struct SaveData {
    int highscore;
    int coins;
    int ownedCharacters;   // bitmask: bit i set = character i bought
    int ownedBackgrounds;  // bitmask: bit i set = background i bought
    int ownedMusic;        // bitmask: bit i set = music track i bought
    int character;
    int background;
    int music;
    int musicOn;
} SaveData;

void defaultSave(SaveData *save) {
    save->highscore = 0;
    save->coins = 0;
    save->ownedCharacters = 1;
    save->ownedBackgrounds = 1;
    save->ownedMusic = 1;
    save->character = 0;
    save->background = 0;
    save->music = 0;
    save->musicOn = 1;
}

void sanitizeSave(SaveData *save) {
    if (save->highscore < 0) save->highscore = 0;
    if (save->coins < 0) save->coins = 0;
    save->ownedCharacters |= 1;   // the default items are always owned
    save->ownedBackgrounds |= 1;
    save->ownedMusic |= 1;
    save->musicOn = save->musicOn != 0;
}

bool readLegacyInt(const char *filename, int *value) {
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
    int value;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (sscanf(line, "%31[^=]=%d", key, &value) != 2) continue;

        if (strcmp(key, "highscore") == 0) save->highscore = value;
        else if (strcmp(key, "coins") == 0) save->coins = value;
        else if (strcmp(key, "owned_characters") == 0) save->ownedCharacters = value;
        else if (strcmp(key, "owned_backgrounds") == 0) save->ownedBackgrounds = value;
        else if (strcmp(key, "character") == 0) save->character = value;
        else if (strcmp(key, "background") == 0) save->background = value;
        else if (strcmp(key, "owned_music") == 0) save->ownedMusic = value;
        else if (strcmp(key, "music") == 0) save->music = value;
        else if (strcmp(key, "music_on") == 0) save->musicOn = value;
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
                      "character=%d\nbackground=%d\nmusic=%d\nmusic_on=%d\n",
                      save->highscore, save->coins, save->ownedCharacters, save->ownedBackgrounds,
                      save->ownedMusic, save->character, save->background, save->music, save->musicOn) > 0;
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
