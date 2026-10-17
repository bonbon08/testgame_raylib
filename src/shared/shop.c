// Shop system: characters, backgrounds, music tracks, nest, hats, ability unlocks
#include "shop.h"
#include "objects.h"
#include "menu.h"
#include "input.h"
#include "services.h"
#include "background.h"
#include "audio.h"
#include "effects.h"
#include "filerw.h"

#include <raylib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define NUM_CHARACTERS 5
#define SHOP_MESSAGE_FRAMES 120

typedef struct CharacterItem {
    const char *name;
    int price;
    Color body;
    Color shade;
    Color dark;
} CharacterItem;

static CharacterItem characters[NUM_CHARACTERS] = {
    { "Chicken",  0,   { 255, 255, 255, 255 }, { 203, 219, 252, 255 }, { 155, 173, 183, 255 } },
    { "Bluebird", 25,  { 130, 200, 255, 255 }, { 90, 160, 235, 255 },  { 55, 110, 190, 255 } },
    { "Flamingo", 40,  { 255, 175, 210, 255 }, { 240, 135, 185, 255 }, { 195, 95, 145, 255 } },
    { "Golden",   80,  { 255, 226, 90, 255 },  { 240, 190, 55, 255 },  { 200, 140, 35, 255 } },
    { "Shadow",   150, { 95, 95, 120, 255 },   { 65, 65, 90, 255 },    { 40, 40, 58, 255 } },
};

typedef struct ShopItem {
    const char *name;
    int price;
} ShopItem;

#define NUM_HUT_ITEMS 1
static ShopItem hutItems[NUM_HUT_ITEMS] = {
    { "Nest", 200 },
};

// A coin every HUT_SECONDS_PER_COIN seconds of real time, capped at HUT_MAX_OFFLINE_SECONDS
#define HUT_SECONDS_PER_COIN 120
#define HUT_MAX_OFFLINE_SECONDS (24 * 60 * 60)

// Purely cosmetic headwear; HAT_NONE is always owned and equipped by default
#define HAT_NONE  0
#define HAT_TOP   1
#define HAT_PARTY 2
#define HAT_CAP   3
#define HAT_CROWN 4
#define NUM_HATS  5
static ShopItem hatItems[NUM_HATS] = {
    { "None",      0 },
    { "Top Hat",   60 },
    { "Party Hat", 40 },
    { "Cap",       30 },
    { "Crown",     150 },
};

// Unlocking an ability here just lets its pickup spawn in a level (abilities.c);
// it's not equipped and doesn't do anything by itself
static ShopItem abilityItems[NUM_ABILITIES] = {
    { "Shield", 500 },
    { "Magnet", 500 },
    { "Dash",   500 },
};

static const char *shopRowTitles[SHOP_ROWS] = {
    "CHARACTERS", "BACKGROUNDS", "MUSIC", "NEST", "HATS", "ABILITIES"
};

#define SHOP_PAGE_ITEMS 0
#define SHOP_PAGE_EXTRAS 1
#define SHOP_PAGES 2

typedef struct Shop {
    int page;
    int row;                  // slot within the current page (0-based)
    int col;
    int owned[SHOP_ROWS];     // bitmask per row: bit i set = item i bought
    int equipped[SHOP_ROWS];  // equipped item per row (only meaningful for rowHasEquip() rows)
    bool musicOn;
    long hutLastCollect;      // unix timestamp of the last idle-coin payout
    int lastHutEarnings;      // coins credited the last time LoadGame() ran, for a popup
    char message[64];
    Color messageColor;
    int messageTimer;
} Shop;

static Shop Shopvars;

static Texture2D characterTextures[NUM_CHARACTERS][2];   // [character][0 = facing right, 1 = facing left]

void InitCharacters(void) {
    for (int i = 0; i < NUM_CHARACTERS; i++) {
        Image image = LoadImage("data/Chicken.png");
        ImageColorReplace(&image, characters[0].body, characters[i].body);
        ImageColorReplace(&image, characters[0].shade, characters[i].shade);
        ImageColorReplace(&image, characters[0].dark, characters[i].dark);
        ImageResizeNN(&image, 64, 64);   // CHICKEN_SIZE
        characterTextures[i][0] = LoadTextureFromImage(image);
        ImageFlipHorizontal(&image);
        characterTextures[i][1] = LoadTextureFromImage(image);
        UnloadImage(image);
    }
}

void UnloadCharacters(void) {
    for (int i = 0; i < NUM_CHARACTERS; i++) {
        UnloadTexture(characterTextures[i][0]);
        UnloadTexture(characterTextures[i][1]);
    }
}

Texture2D CurrentCharacterTexture(int direction) {
    return characterTextures[Shopvars.equipped[ROW_CHARACTERS]][direction == 0 ? 0 : 1];
}

Color CurrentCharacterBodyColor(void) {
    return characters[Shopvars.equipped[ROW_CHARACTERS]].body;
}

Color CurrentCharacterShadeColor(void) {
    return characters[Shopvars.equipped[ROW_CHARACTERS]].shade;
}

static bool owns(int mask, int item) {
    return (mask >> item) & 1;
}

bool OwnsHut(void) {
    return owns(Shopvars.owned[ROW_HUT], 0);
}

int LastHutEarnings(void) {
    return Shopvars.lastHutEarnings;
}

bool AbilityUnlocked(int ability) {
    return owns(Shopvars.owned[ROW_ABILITIES], ability);
}

// Characters, backgrounds, music and hats can be equipped (one active item per
// row); the nest and ability unlocks are one-off purchases with no "equipped" state.
static bool rowHasEquip(int row) {
    return row == ROW_CHARACTERS || row == ROW_BACKGROUNDS || row == ROW_MUSIC || row == ROW_HATS;
}

// Vertical slot a row is drawn at on its shop page (both pages start back at the top)
static int rowSlot(int row) {
    if (row == ROW_HATS) return 0;
    if (row == ROW_HUT) return 1;
    if (row == ROW_ABILITIES) return 2;
    return row;   // ROW_CHARACTERS/BACKGROUNDS/MUSIC are already 0, 1, 2
}

static int rowsOnPage(int page) {
    (void)page;
    return 3;   // both pages hold exactly 3 rows now
}

static int pageRow(int page, int slot) {
    if (page == SHOP_PAGE_ITEMS) return slot;             // 0,1,2 -> CHARACTERS,BACKGROUNDS,MUSIC
    if (slot == 0) return ROW_HATS;
    return slot == 1 ? ROW_HUT : ROW_ABILITIES;
}

static int shopRowSize(int row) {
    if (row == ROW_CHARACTERS) return NUM_CHARACTERS;
    if (row == ROW_BACKGROUNDS) return NUM_THEMES;
    if (row == ROW_MUSIC) return NUM_TRACKS;
    if (row == ROW_HATS) return NUM_HATS;
    if (row == ROW_ABILITIES) return NUM_ABILITIES;
    return NUM_HUT_ITEMS;
}

static const char *shopItemName(int row, int item) {
    if (row == ROW_CHARACTERS) return characters[item].name;
    if (row == ROW_BACKGROUNDS) return themes[item].name;
    if (row == ROW_MUSIC) return tracks[item].name;
    if (row == ROW_HATS) return hatItems[item].name;
    if (row == ROW_ABILITIES) return abilityItems[item].name;
    return hutItems[item].name;
}

static int shopItemPrice(int row, int item) {
    if (row == ROW_CHARACTERS) return characters[item].price;
    if (row == ROW_BACKGROUNDS) return themes[item].price;
    if (row == ROW_MUSIC) return tracks[item].price;
    if (row == ROW_HATS) return hatItems[item].price;
    if (row == ROW_ABILITIES) return abilityItems[item].price;
    return hutItems[item].price;
}

static int validEquipped(int row, int item) {
    if (item < 0 || item >= shopRowSize(row) || !owns(Shopvars.owned[row], item)) return 0;
    return item;
}

void LoadGame(void) {
    SaveData save;
    LoadSave(&save);

    GetGamevars()->highscore = save.highscore;
    GetGamevars()->Coins = save.coins;
    Shopvars.owned[ROW_CHARACTERS] = save.ownedCharacters;
    Shopvars.owned[ROW_BACKGROUNDS] = save.ownedBackgrounds;
    Shopvars.owned[ROW_MUSIC] = save.ownedMusic;
    Shopvars.owned[ROW_HUT] = save.hasHut ? 1 : 0;
    Shopvars.owned[ROW_HATS] = save.ownedHats;
    Shopvars.owned[ROW_ABILITIES] = save.unlockedAbilities;
    Shopvars.equipped[ROW_CHARACTERS] = validEquipped(ROW_CHARACTERS, save.character);
    Shopvars.equipped[ROW_BACKGROUNDS] = validEquipped(ROW_BACKGROUNDS, save.background);
    Shopvars.equipped[ROW_MUSIC] = validEquipped(ROW_MUSIC, save.music);
    Shopvars.equipped[ROW_HATS] = validEquipped(ROW_HATS, save.hat);
    Shopvars.musicOn = save.musicOn;
    Shopvars.hutLastCollect = save.hutLastCollect;

    Shopvars.lastHutEarnings = 0;
    if (OwnsHut()) {
        long now = (long)time(NULL);
        long elapsed = now - Shopvars.hutLastCollect;
        if (elapsed < 0) elapsed = 0;
        if (elapsed > HUT_MAX_OFFLINE_SECONDS) elapsed = HUT_MAX_OFFLINE_SECONDS;
        int earned = (int)(elapsed / HUT_SECONDS_PER_COIN);
        if (earned > 0) {
            GetGamevars()->Coins += earned;
            Shopvars.lastHutEarnings = earned;
        }
        Shopvars.hutLastCollect = now;
        SaveGame();
    }
}

void SaveGame(void) {
    SaveData save = {
        .highscore = GetGamevars()->highscore,
        .coins = GetGamevars()->Coins,
        .ownedCharacters = Shopvars.owned[ROW_CHARACTERS],
        .ownedBackgrounds = Shopvars.owned[ROW_BACKGROUNDS],
        .ownedMusic = Shopvars.owned[ROW_MUSIC],
        .character = Shopvars.equipped[ROW_CHARACTERS],
        .background = Shopvars.equipped[ROW_BACKGROUNDS],
        .music = Shopvars.equipped[ROW_MUSIC],
        .musicOn = Shopvars.musicOn,
        .hasHut = OwnsHut() ? 1 : 0,
        .hutLastCollect = Shopvars.hutLastCollect,
        .ownedHats = Shopvars.owned[ROW_HATS],
        .hat = Shopvars.equipped[ROW_HATS],
        .unlockedAbilities = Shopvars.owned[ROW_ABILITIES],
    };
    if (!WriteSave(&save)) TraceLog(LOG_WARNING, "Could not write save file data/save.txt");
}

static Rectangle shopCardRect(int row, int col, int screenWidth) {
    float cardWidth = 170, cardHeight = 132, gap = 22;
    int size = shopRowSize(row);
    float totalWidth = size * cardWidth + (size - 1) * gap;
    float x = (screenWidth - totalWidth) / 2 + col * (cardWidth + gap);
    float y = 150 + rowSlot(row) * 156;
    return (Rectangle){ x, y, cardWidth, cardHeight };
}

static void setShopMessage(const char *text, Color color) {
    snprintf(Shopvars.message, sizeof(Shopvars.message), "%s", text);
    Shopvars.messageColor = color;
    Shopvars.messageTimer = SHOP_MESSAGE_FRAMES;
}

static bool browsingShopRow(int row) {
    return GetMenuvars()->enabled && GetMenuvars()->untermenu == 1 &&
           pageRow(Shopvars.page, Shopvars.row) == row;
}

int PreviewedBackground(void) {
    return browsingShopRow(ROW_BACKGROUNDS) ? Shopvars.col : Shopvars.equipped[ROW_BACKGROUNDS];
}

int PreviewedTrack(void) {
    return browsingShopRow(ROW_MUSIC) ? Shopvars.col : Shopvars.equipped[ROW_MUSIC];
}

int EquippedMusicTrack(void) {
    return Shopvars.equipped[ROW_MUSIC];
}

bool MusicAudible(void) {
    return Shopvars.musicOn || browsingShopRow(ROW_MUSIC);
}

bool MusicOn(void) {
    return Shopvars.musicOn;
}

void ToggleMusic(void) {
    Shopvars.musicOn = !Shopvars.musicOn;
    SaveGame();
}

static Rectangle ShopBackButtonRect(void) {
    return (Rectangle){ 24, 24, 130, 52 };
}

static Rectangle ShopAdButtonRect(int screenWidth) {
    return (Rectangle){ screenWidth - 420, 24, 230, 52 };
}

static Rectangle ShopPageTabRect(int page, int screenWidth) {
    float width = 160, gap = 10;
    float startX = screenWidth / 2 - (SHOP_PAGES * width + (SHOP_PAGES - 1) * gap) / 2;
    return (Rectangle){ startX + page * (width + gap), 78, width, 40 };
}

static void shopActivate(int row, int item, int screenWidth) {
    const char *name = shopItemName(row, item);
    int price = shopItemPrice(row, item);
    bool alreadyOwned = owns(Shopvars.owned[row], item);

    if (alreadyOwned && rowHasEquip(row)) {
        Shopvars.equipped[row] = item;
        setShopMessage(TextFormat("%s equipped", name), WHITE);
    } else if (alreadyOwned) {
        setShopMessage(TextFormat("%s already owned", name), WHITE);
        return;
    } else if (GetGamevars()->Coins >= price) {
        GetGamevars()->Coins -= price;
        Shopvars.owned[row] |= 1 << item;
        if (rowHasEquip(row)) Shopvars.equipped[row] = item;
        if (row == ROW_HUT) Shopvars.hutLastCollect = (long)time(NULL);
        setShopMessage(TextFormat("Bought %s!", name), (Color){ 120, 230, 120, 255 });
        Rectangle card = shopCardRect(row, item, screenWidth);
        SpawnParticles((Vector2){ card.x + card.width / 2, card.y + card.height / 2 }, 40, YELLOW, 7.0f, 7.0f);
        PlaySound(GetMaincoin()->sound);
    } else {
        setShopMessage(TextFormat("Not enough coins - you need %d more", price - GetGamevars()->Coins),
                       (Color){ 255, 110, 110, 255 });
        return;
    }
    SaveGame();
}

void UpdateShop(int screenWidth) {
    if (Shopvars.messageTimer > 0) Shopvars.messageTimer--;

    if (BackPressed() || PointerPressedIn(ShopBackButtonRect())) {
        GetMenuvars()->untermenu = 0;
        return;
    }

    if (AdsSupported() && PointerPressedIn(ShopAdButtonRect(screenWidth))) {
        if (RewardedAdReady()) ShowRewardedAd(REWARD_COINS);
        else setShopMessage("No ad available right now - try again later", WHITE);
        return;
    }

    if (IsKeyPressed(KEY_TAB)) {
        Shopvars.page = (Shopvars.page + 1) % SHOP_PAGES;
        Shopvars.row = 0;
        Shopvars.col = 0;
    }
    for (int page = 0; page < SHOP_PAGES; page++) {
        if (PointerPressedIn(ShopPageTabRect(page, screenWidth)) && page != Shopvars.page) {
            Shopvars.page = page;
            Shopvars.row = 0;
            Shopvars.col = 0;
            return;
        }
    }

    int row = pageRow(Shopvars.page, Shopvars.row);
    int size = shopRowSize(row);
    if (IsKeyPressed(KEY_LEFT)) Shopvars.col = (Shopvars.col - 1 + size) % size;
    if (IsKeyPressed(KEY_RIGHT)) Shopvars.col = (Shopvars.col + 1) % size;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {
        int rows = rowsOnPage(Shopvars.page);
        int direction = IsKeyPressed(KEY_UP) ? rows - 1 : 1;
        Shopvars.row = (Shopvars.row + direction) % rows;
        int newRow = pageRow(Shopvars.page, Shopvars.row);
        if (Shopvars.col >= shopRowSize(newRow)) Shopvars.col = shopRowSize(newRow) - 1;
    }

    if (IsKeyPressed(KEY_X)) {
        shopActivate(pageRow(Shopvars.page, Shopvars.row), Shopvars.col, screenWidth);
        return;
    }

    if (!PointerPressed()) return;
    int rows = rowsOnPage(Shopvars.page);
    for (int slot = 0; slot < rows; slot++) {
        int absRow = pageRow(Shopvars.page, slot);
        for (int col = 0; col < shopRowSize(absRow); col++) {
            if (!PointerPressedIn(shopCardRect(absRow, col, screenWidth))) continue;
            if (Shopvars.row == slot && Shopvars.col == col) {
                shopActivate(absRow, col, screenWidth);
            } else {
                Shopvars.row = slot;
                Shopvars.col = col;
            }
            return;
        }
    }
}

// Little equalizer bars; they only dance while the track is playing
static void drawMusicIcon(Rectangle area, Color accent, bool playing) {
    float time = (float)GetTime();
    int bars = 5;
    float barWidth = 12, gap = 8;
    float startX = area.x + (area.width - (bars * barWidth + (bars - 1) * gap)) / 2;
    for (int i = 0; i < bars; i++) {
        float level = playing ? 0.35f + 0.65f * fabsf(sinf(time * (3.0f + i * 1.3f) + i))
                              : 0.25f + 0.12f * (i % 3);
        float height = area.height * level;
        DrawRectangleRounded((Rectangle){ startX + i * (barWidth + gap), area.y + area.height - height, barWidth, height },
                             0.5f, 4, accent);
    }
}

// Rotates a world-space point around `pivot` by `angleDeg`, matching how the
// chicken sprite itself is rotated (DrawTexturePro with the center as origin)
static Vector2 rotateAround(Vector2 point, Vector2 pivot, float angleDeg) {
    float rad = angleDeg * DEG2RAD;
    float s = sinf(rad), c = cosf(rad);
    float dx = point.x - pivot.x;
    float dy = point.y - pivot.y;
    return (Vector2){ pivot.x + dx * c - dy * s, pivot.y + dx * s + dy * c };
}

static void drawRotatedRect(Vector2 center, float halfWidth, float halfHeight,
                            Vector2 pivot, float angleDeg, Color color) {
    Vector2 p0 = rotateAround((Vector2){ center.x - halfWidth, center.y - halfHeight }, pivot, angleDeg);
    Vector2 p1 = rotateAround((Vector2){ center.x + halfWidth, center.y - halfHeight }, pivot, angleDeg);
    Vector2 p2 = rotateAround((Vector2){ center.x + halfWidth, center.y + halfHeight }, pivot, angleDeg);
    Vector2 p3 = rotateAround((Vector2){ center.x - halfWidth, center.y + halfHeight }, pivot, angleDeg);
    DrawTriangle(p0, p2, p1, color);
    DrawTriangle(p0, p3, p2, color);
}

// Simple procedural glyphs (no spare art budget for new sprites). `anchor` is
// the unrotated point the hat sits at (just above the head), `pivot` is what
// it rotates around (the chicken's own center) and `angleDeg` its rotation.
static void drawHat(int hat, Vector2 anchor, Vector2 pivot, float angleDeg, float scale) {
    switch (hat) {
        case HAT_TOP: {
            Color black = (Color){ 30, 30, 35, 255 };
            Color band = (Color){ 190, 40, 50, 255 };
            drawRotatedRect((Vector2){ anchor.x, anchor.y - 19 * scale }, 13 * scale, 15 * scale,
                            pivot, angleDeg, black);
            drawRotatedRect((Vector2){ anchor.x, anchor.y - 6 * scale }, 13 * scale, 4 * scale,
                            pivot, angleDeg, band);
            drawRotatedRect((Vector2){ anchor.x, anchor.y }, 24 * scale, 4 * scale,
                            pivot, angleDeg, black);
            break;
        }
        case HAT_PARTY: {
            Color cone = (Color){ 230, 90, 170, 255 };
            Vector2 tip = rotateAround((Vector2){ anchor.x, anchor.y - 38 * scale }, pivot, angleDeg);
            Vector2 baseL = rotateAround((Vector2){ anchor.x - 20 * scale, anchor.y }, pivot, angleDeg);
            Vector2 baseR = rotateAround((Vector2){ anchor.x + 20 * scale, anchor.y }, pivot, angleDeg);
            DrawTriangle(tip, baseL, baseR, cone);
            DrawCircleV(tip, 6 * scale, (Color){ 255, 230, 120, 255 });
            break;
        }
        case HAT_CAP: {
            Color capColor = (Color){ 70, 120, 200, 255 };
            Vector2 domeCenter = rotateAround((Vector2){ anchor.x, anchor.y - 8 * scale }, pivot, angleDeg);
            DrawCircleV(domeCenter, 14 * scale, capColor);
            Vector2 b0 = rotateAround((Vector2){ anchor.x + 8 * scale, anchor.y - 4 * scale }, pivot, angleDeg);
            Vector2 b1 = rotateAround((Vector2){ anchor.x + 32 * scale, anchor.y - 2 * scale }, pivot, angleDeg);
            Vector2 b2 = rotateAround((Vector2){ anchor.x + 8 * scale, anchor.y + 4 * scale }, pivot, angleDeg);
            DrawTriangle(b0, b1, b2, capColor);
            break;
        }
        default: {   // HAT_CROWN
            Color gold = (Color){ 255, 205, 60, 255 };
            Color jewel = (Color){ 210, 40, 60, 255 };
            drawRotatedRect((Vector2){ anchor.x, anchor.y - 2 * scale }, 18 * scale, 6 * scale,
                            pivot, angleDeg, gold);
            float tipOffsets[3] = { -14, 0, 14 };
            for (int i = 0; i < 3; i++) {
                Vector2 tip = rotateAround((Vector2){ anchor.x + tipOffsets[i] * scale, anchor.y - 20 * scale },
                                           pivot, angleDeg);
                Vector2 l = rotateAround((Vector2){ anchor.x + (tipOffsets[i] - 7) * scale, anchor.y - 4 * scale },
                                         pivot, angleDeg);
                Vector2 r = rotateAround((Vector2){ anchor.x + (tipOffsets[i] + 7) * scale, anchor.y - 4 * scale },
                                         pivot, angleDeg);
                DrawTriangle(l, tip, r, gold);
                DrawCircleV(tip, 3 * scale, jewel);
            }
            break;
        }
    }
}

// Drawn on top of the chicken during gameplay, right after DrawChicken()
void DrawEquippedHat(Vector2 chickenCenter, float rotation) {
    int hat = Shopvars.equipped[ROW_HATS];
    if (hat == HAT_NONE) return;
    Vector2 anchor = { chickenCenter.x, chickenCenter.y - 22.0f };
    drawHat(hat, anchor, chickenCenter, rotation, 1.0f);
}

// Simple glyphs matching the in-level pickup's look (abilities.c), so an
// unlocked ability is recognizable later when it shows up as a pickup
static void drawAbilityShopIcon(int ability, Vector2 center) {
    switch (ability) {
        case ABILITY_SHIELD:
            DrawPoly(center, 6, 22, 0, (Color){ 90, 170, 255, 255 });
            DrawPolyLinesEx(center, 6, 22, 0, 3.0f, WHITE);
            break;
        case ABILITY_MAGNET:
            DrawRing(center, 10, 20, 200, 340, 16, (Color){ 230, 90, 90, 255 });
            DrawRing(center, 10, 20, 20, 160, 16, (Color){ 230, 230, 230, 255 });
            break;
        default:   // ABILITY_DASH
            for (int i = 0; i < 3; i++) {
                DrawTriangle((Vector2){ center.x - 18 + i * 12, center.y - 14 },
                            (Vector2){ center.x - 18 + i * 12, center.y + 14 },
                            (Vector2){ center.x - 6 + i * 12, center.y }, (Color){ 255, 160, 60, 255 });
            }
            break;
    }
}

static void drawShopCard(int row, int col, int screenWidth, int screenHeight) {
    bool selected = pageRow(Shopvars.page, Shopvars.row) == row && Shopvars.col == col;
    int price = shopItemPrice(row, col);
    float time = (float)GetTime();

    Rectangle card = shopCardRect(row, col, screenWidth);
    if (selected) card.y -= 6;
    DrawRectangleRounded(card, 0.12f, 10, Fade(BLACK, selected ? 0.4f : 0.28f));

    if (row == ROW_CHARACTERS) {
        Texture2D texture = characterTextures[col][0];
        float wobble = selected ? 8 * sinf(time * 4.0f) : 0.0f;
        Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
        Rectangle dest = { card.x + card.width / 2, card.y + 50,
                           texture.width * 1.3f, texture.height * 1.3f };
        DrawTexturePro(texture, source, dest,
                       (Vector2){ dest.width / 2, dest.height / 2 }, wobble, WHITE);
    } else if (row == ROW_BACKGROUNDS) {
        Rectangle preview = { card.x + 14, card.y + 12, card.width - 28, 74 };
        DrawThemePreview(themes[col], preview, screenHeight);
        DrawRectangleLinesEx(preview, 2.0f, Fade(WHITE, 0.6f));
    } else if (row == ROW_MUSIC) {
        bool playing = PreviewedTrack() == col && MusicAudible();
        drawMusicIcon((Rectangle){ card.x + 14, card.y + 16, card.width - 28, 66 }, tracks[col].accent, playing);
    } else if (row == ROW_HATS) {
        Vector2 previewAnchor = { card.x + card.width / 2, card.y + 78 };
        if (col == HAT_NONE)
            DrawCircleLines((int)previewAnchor.x, (int)(previewAnchor.y - 10), 18, Fade(WHITE, 0.3f));
        else
            drawHat(col, previewAnchor, previewAnchor, 0.0f, 1.3f);
    } else if (row == ROW_ABILITIES) {
        drawAbilityShopIcon(col, (Vector2){ card.x + card.width / 2, card.y + 48 });
    } else {   // ROW_HUT
        Rectangle hutArea = { card.x + card.width / 2 - 34, card.y + 12, 68, 64 };
        DrawTriangle((Vector2){ hutArea.x + hutArea.width / 2, hutArea.y },
                    (Vector2){ hutArea.x - 6, hutArea.y + 30 },
                    (Vector2){ hutArea.x + hutArea.width + 6, hutArea.y + 30 }, (Color){ 180, 110, 70, 255 });
        DrawRectangle((int)hutArea.x, (int)hutArea.y + 28, (int)hutArea.width, (int)(hutArea.height - 28),
                     (Color){ 220, 170, 120, 255 });
        DrawRectangle((int)(hutArea.x + hutArea.width / 2 - 8), (int)(hutArea.y + hutArea.height - 24), 16, 20,
                     (Color){ 120, 80, 50, 255 });
        if (OwnsHut()) {
            drawTextCentered("+1 coin / 2 min", (int)(card.x + card.width / 2), (int)card.y + 94, 14,
                             Fade(YELLOW, 0.9f));
        }
    }

    int centerX = (int)(card.x + card.width / 2);
    if (row != ROW_HUT || !OwnsHut())
        drawTextCentered(shopItemName(row, col), centerX, (int)card.y + 94, 20, WHITE);

    int statusY = (int)card.y + 120;
    if (rowHasEquip(row) && Shopvars.equipped[row] == col) {
        drawTextCentered("EQUIPPED", centerX, statusY, 20,
                         (Color){ 120, 230, 120, 255 });
    } else if (owns(Shopvars.owned[row], col)) {
        drawTextCentered("OWNED", centerX, statusY, 20, Fade(WHITE, 0.8f));
    } else {
        Color priceColor = GetGamevars()->Coins >= price ? YELLOW : (Color){ 255, 110, 110, 255 };
        const char *priceText = TextFormat("%d", price);
        int textWidth = MeasureText(priceText, 22);
        int startX = centerX - (textWidth + 26) / 2;
        DrawTextureEx(GetMaincoin()->texture,
                      (Vector2){ (float)startX, (float)statusY - 2 },
                      0.0f, 0.34f, WHITE);
        DrawText(priceText, startX + 26, statusY, 22, priceColor);
    }

    if (selected) {
        float pulse = 2 * sinf(time * 5.0f);
        Rectangle border = { card.x - 3 - pulse, card.y - 3 - pulse,
                             card.width + 6 + 2 * pulse, card.height + 6 + 2 * pulse };
        DrawRectangleRoundedLinesEx(border, 0.12f, 10, 4.0f, MENU_ORANGE);
    }
}

static void drawShopPageTab(int page, int screenWidth) {
    Rectangle rect = ShopPageTabRect(page, screenWidth);
    bool active = Shopvars.page == page;
    const char *label = page == SHOP_PAGE_ITEMS ? "ITEMS" : "EXTRAS";
    DrawRectangleRounded(rect, 0.4f, 8, Fade(active ? MENU_ORANGE : BLACK, active ? 0.9f : 0.35f));
    drawTextCentered(label, (int)(rect.x + rect.width / 2), (int)(rect.y + 10), 18, WHITE);
}

void DrawShop(int screenWidth, int screenHeight) {
    int centerX = screenWidth / 2;

    drawTextShadowCentered("SHOP", centerX, 20, 56, MENU_GOLD);
    drawButton(ShopBackButtonRect(), "BACK", 26, Fade(BLACK, 0.4f), 1.0f);
    if (AdsSupported()) {
        drawButton(ShopAdButtonRect(screenWidth), TextFormat("AD: +%d COINS", AD_REWARD_COINS), 22,
                   MENU_GREEN, RewardedAdReady() ? 1.0f : 0.5f);
    }

    const char *coinsText = TextFormat("%d", GetGamevars()->Coins);
    int coinsWidth = MeasureText(coinsText, 32);
    DrawRectangleRounded((Rectangle){ screenWidth - coinsWidth - 100, 24, coinsWidth + 76, 52 },
                         1.0f, 10, Fade(BLACK, 0.3f));
    DrawTextureEx(GetMaincoin()->texture,
                  (Vector2){ screenWidth - coinsWidth - 88, 30 },
                  0.0f, 0.62f, WHITE);
    DrawText(coinsText, screenWidth - coinsWidth - 42, 35, 32, YELLOW);

    for (int page = 0; page < SHOP_PAGES; page++) drawShopPageTab(page, screenWidth);

    int rows = rowsOnPage(Shopvars.page);
    for (int slot = 0; slot < rows; slot++) {
        int row = pageRow(Shopvars.page, slot);
        drawTextShadowCentered(shopRowTitles[row], centerX,
                               (int)shopCardRect(row, 0, screenWidth).y - 28, 22, WHITE);
        for (int col = 0; col < shopRowSize(row); col++)
            drawShopCard(row, col, screenWidth, screenHeight);
    }

    if (Shopvars.messageTimer > 0) {
        float alpha = Shopvars.messageTimer > 20 ? 1.0f : Shopvars.messageTimer / 20.0f;
        drawTextShadowCentered(Shopvars.message, centerX, 608, 24,
                               Fade(Shopvars.messageColor, alpha));
    }

    const char *hint = IS_MOBILE ? "Tap a card to choose   -   tap it again to buy / equip   -   tap ITEMS/EXTRAS to switch page"
                                 : "ARROWS to choose   -   X to buy / equip   -   TAB to switch page   -   BACKSPACE to go back";
    drawTextShadowCentered(hint, centerX, screenHeight - 34, 20, WHITE);
}
