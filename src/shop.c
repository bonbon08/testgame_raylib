#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define NUM_CHARACTERS 5
#define SHOP_MESSAGE_FRAMES 120

typedef struct CharacterItem {
    const char *name;
    int price;
    Color body;    // replaces the white feathers of Chicken.png
    Color shade;   // replaces the light shading
    Color dark;    // replaces the dark outline of the feathers
} CharacterItem;

CharacterItem characters[NUM_CHARACTERS] = {
    { "Chicken",  0,   { 255, 255, 255, 255 }, { 203, 219, 252, 255 }, { 155, 173, 183, 255 } },
    { "Bluebird", 25,  { 130, 200, 255, 255 }, { 90, 160, 235, 255 },  { 55, 110, 190, 255 } },
    { "Flamingo", 40,  { 255, 175, 210, 255 }, { 240, 135, 185, 255 }, { 195, 95, 145, 255 } },
    { "Golden",   80,  { 255, 226, 90, 255 },  { 240, 190, 55, 255 },  { 200, 140, 35, 255 } },
    { "Shadow",   150, { 95, 95, 120, 255 },   { 65, 65, 90, 255 },    { 40, 40, 58, 255 } },
};

// [character][0 = facing right, 1 = facing left]
Texture2D characterTextures[NUM_CHARACTERS][2];

#define SHOP_ROWS 3
#define ROW_CHARACTERS 0
#define ROW_BACKGROUNDS 1
#define ROW_MUSIC 2

const char *shopRowTitles[SHOP_ROWS] = { "CHARACTERS", "BACKGROUNDS", "MUSIC" };

typedef struct Shop {
    int row;                  // ROW_CHARACTERS, ROW_BACKGROUNDS or ROW_MUSIC
    int col;
    int owned[SHOP_ROWS];     // bitmask per row: bit i set = item i bought
    int equipped[SHOP_ROWS];  // equipped item per row
    bool musicOn;
    char message[64];
    Color messageColor;
    int messageTimer;
} Shop;

Shop Shopvars;

void InitCharacters() {
    for (int i = 0; i < NUM_CHARACTERS; i++) {
        Image image = LoadImage("data/Chicken.png");
        ImageColorReplace(&image, characters[0].body, characters[i].body);
        ImageColorReplace(&image, characters[0].shade, characters[i].shade);
        ImageColorReplace(&image, characters[0].dark, characters[i].dark);
        ImageResizeNN(&image, CHICKEN_SIZE, CHICKEN_SIZE);
        characterTextures[i][0] = LoadTextureFromImage(image);
        ImageFlipHorizontal(&image);
        characterTextures[i][1] = LoadTextureFromImage(image);
        UnloadImage(image);
    }
}

void UnloadCharacters() {
    for (int i = 0; i < NUM_CHARACTERS; i++) {
        UnloadTexture(characterTextures[i][0]);
        UnloadTexture(characterTextures[i][1]);
    }
}

Texture2D CurrentCharacterTexture(int direction) {
    return characterTextures[Shopvars.equipped[ROW_CHARACTERS]][direction == 0 ? 0 : 1];
}

bool owns(int mask, int item) {
    return (mask >> item) & 1;
}

int shopRowSize(int row) {
    if (row == ROW_CHARACTERS) return NUM_CHARACTERS;
    if (row == ROW_BACKGROUNDS) return NUM_THEMES;
    return NUM_TRACKS;
}

const char *shopItemName(int row, int item) {
    if (row == ROW_CHARACTERS) return characters[item].name;
    if (row == ROW_BACKGROUNDS) return themes[item].name;
    return tracks[item].name;
}

int shopItemPrice(int row, int item) {
    if (row == ROW_CHARACTERS) return characters[item].price;
    if (row == ROW_BACKGROUNDS) return themes[item].price;
    return tracks[item].price;
}

int validEquipped(int row, int item) {
    if (item < 0 || item >= shopRowSize(row) || !owns(Shopvars.owned[row], item)) return 0;
    return item;
}

void LoadGame() {
    SaveData save;
    LoadSave(&save);

    Gamevars.highscore = save.highscore;
    Gamevars.Coins = save.coins;
    Shopvars.owned[ROW_CHARACTERS] = save.ownedCharacters;
    Shopvars.owned[ROW_BACKGROUNDS] = save.ownedBackgrounds;
    Shopvars.owned[ROW_MUSIC] = save.ownedMusic;
    Shopvars.equipped[ROW_CHARACTERS] = validEquipped(ROW_CHARACTERS, save.character);
    Shopvars.equipped[ROW_BACKGROUNDS] = validEquipped(ROW_BACKGROUNDS, save.background);
    Shopvars.equipped[ROW_MUSIC] = validEquipped(ROW_MUSIC, save.music);
    Shopvars.musicOn = save.musicOn;
}

void SaveGame() {
    SaveData save = {
        .highscore = Gamevars.highscore,
        .coins = Gamevars.Coins,
        .ownedCharacters = Shopvars.owned[ROW_CHARACTERS],
        .ownedBackgrounds = Shopvars.owned[ROW_BACKGROUNDS],
        .ownedMusic = Shopvars.owned[ROW_MUSIC],
        .character = Shopvars.equipped[ROW_CHARACTERS],
        .background = Shopvars.equipped[ROW_BACKGROUNDS],
        .music = Shopvars.equipped[ROW_MUSIC],
        .musicOn = Shopvars.musicOn,
    };
    if (!WriteSave(&save)) TraceLog(LOG_WARNING, "Could not write save file %s", SAVE_FILE);
}

Rectangle shopCardRect(int row, int col, int screenWidth) {
    float cardWidth = 170, cardHeight = 150, gap = 22;
    int size = shopRowSize(row);
    float totalWidth = size * cardWidth + (size - 1) * gap;
    float x = (screenWidth - totalWidth) / 2 + col * (cardWidth + gap);
    float y = 118 + row * 178;
    return (Rectangle){ x, y, cardWidth, cardHeight };
}

void setShopMessage(const char *text, Color color) {
    snprintf(Shopvars.message, sizeof(Shopvars.message), "%s", text);
    Shopvars.messageColor = color;
    Shopvars.messageTimer = SHOP_MESSAGE_FRAMES;
}

bool browsingShopRow(int row) {
    return Menuvars.enabled && Menuvars.untermenu == 1 && Shopvars.row == row;
}

// While browsing backgrounds or music in the shop, the selected one is previewed
int PreviewedBackground() {
    return browsingShopRow(ROW_BACKGROUNDS) ? Shopvars.col : Shopvars.equipped[ROW_BACKGROUNDS];
}

int PreviewedTrack() {
    return browsingShopRow(ROW_MUSIC) ? Shopvars.col : Shopvars.equipped[ROW_MUSIC];
}

// The shop preview plays even when music is switched off, so you can hear what you buy
bool MusicAudible() {
    return Shopvars.musicOn || browsingShopRow(ROW_MUSIC);
}

Rectangle ShopBackButtonRect() {
    return (Rectangle){ 24, 24, 130, 52 };
}

Rectangle ShopAdButtonRect(int screenWidth) {
    return (Rectangle){ screenWidth - 420, 24, 230, 52 };
}

void shopActivate(int row, int item, int screenWidth) {
    const char *name = shopItemName(row, item);
    int price = shopItemPrice(row, item);

    if (owns(Shopvars.owned[row], item)) {
        Shopvars.equipped[row] = item;
        setShopMessage(TextFormat("%s equipped", name), WHITE);
    } else if (Gamevars.Coins >= price) {
        Gamevars.Coins -= price;
        Shopvars.owned[row] |= 1 << item;
        Shopvars.equipped[row] = item;
        setShopMessage(TextFormat("Bought %s!", name), (Color){ 120, 230, 120, 255 });
        Rectangle card = shopCardRect(row, item, screenWidth);
        SpawnParticles((Vector2){ card.x + card.width / 2, card.y + card.height / 2 }, 40, YELLOW, 7.0f, 7.0f);
        PlaySound(maincoin.sound);
    } else {
        setShopMessage(TextFormat("Not enough coins - you need %d more", price - Gamevars.Coins), (Color){ 255, 110, 110, 255 });
        return;
    }
    SaveGame();
}

void UpdateShop(int screenWidth) {
    if (Shopvars.messageTimer > 0) Shopvars.messageTimer--;

    if (BackPressed() || PointerPressedIn(ShopBackButtonRect())) {
        Menuvars.untermenu = 0;
        return;
    }

    if (AdsSupported() && PointerPressedIn(ShopAdButtonRect(screenWidth))) {
        if (RewardedAdReady()) ShowRewardedAd(REWARD_COINS);
        else setShopMessage("No ad available right now - try again later", WHITE);
        return;
    }

    int size = shopRowSize(Shopvars.row);
    if (IsKeyPressed(KEY_LEFT)) Shopvars.col = (Shopvars.col - 1 + size) % size;
    if (IsKeyPressed(KEY_RIGHT)) Shopvars.col = (Shopvars.col + 1) % size;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {
        int direction = IsKeyPressed(KEY_UP) ? SHOP_ROWS - 1 : 1;
        Shopvars.row = (Shopvars.row + direction) % SHOP_ROWS;
        if (Shopvars.col >= shopRowSize(Shopvars.row)) Shopvars.col = shopRowSize(Shopvars.row) - 1;
    }

    if (IsKeyPressed(KEY_X)) {
        shopActivate(Shopvars.row, Shopvars.col, screenWidth);
        return;
    }

    // Tapping a card selects it, tapping the selected card again buys / equips it
    if (!PointerPressed()) return;
    for (int row = 0; row < SHOP_ROWS; row++) {
        for (int col = 0; col < shopRowSize(row); col++) {
            if (!CheckCollisionPointRec(PointerPosition(), shopCardRect(row, col, screenWidth))) continue;
            if (Shopvars.row == row && Shopvars.col == col) {
                shopActivate(row, col, screenWidth);
            } else {
                Shopvars.row = row;
                Shopvars.col = col;
            }
            return;
        }
    }
}

// Little equalizer bars; they only dance while the track is playing
void drawMusicIcon(Rectangle area, Color accent, bool playing) {
    float time = (float)GetTime();
    int bars = 5;
    float barWidth = 12, gap = 8;
    float startX = area.x + (area.width - (bars * barWidth + (bars - 1) * gap)) / 2;
    for (int i = 0; i < bars; i++) {
        float level = playing ? 0.35f + 0.65f * fabsf(sinf(time * (3.0f + i * 1.3f) + i)) : 0.25f + 0.12f * (i % 3);
        float height = area.height * level;
        DrawRectangleRounded((Rectangle){ startX + i * (barWidth + gap), area.y + area.height - height, barWidth, height },
                             0.5f, 4, accent);
    }
}

void drawShopCard(int row, int col, int screenWidth, int screenHeight) {
    bool selected = Shopvars.row == row && Shopvars.col == col;
    int price = shopItemPrice(row, col);
    float time = (float)GetTime();

    Rectangle card = shopCardRect(row, col, screenWidth);
    if (selected) card.y -= 6;
    DrawRectangleRounded(card, 0.12f, 10, Fade(BLACK, selected ? 0.4f : 0.28f));

    if (row == ROW_CHARACTERS) {
        Texture2D texture = characterTextures[col][0];
        float wobble = selected ? 8 * sinf(time * 4.0f) : 0.0f;
        Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
        Rectangle dest = { card.x + card.width / 2, card.y + 50, texture.width * 1.3f, texture.height * 1.3f };
        DrawTexturePro(texture, source, dest, (Vector2){ dest.width / 2, dest.height / 2 }, wobble, WHITE);
    } else if (row == ROW_BACKGROUNDS) {
        Rectangle preview = { card.x + 14, card.y + 12, card.width - 28, 74 };
        DrawThemePreview(themes[col], preview, screenHeight);
        DrawRectangleLinesEx(preview, 2.0f, Fade(WHITE, 0.6f));
    } else {
        bool playing = PreviewedTrack() == col && MusicAudible();
        drawMusicIcon((Rectangle){ card.x + 14, card.y + 16, card.width - 28, 66 }, tracks[col].accent, playing);
    }

    int centerX = (int)(card.x + card.width / 2);
    drawTextCentered(shopItemName(row, col), centerX, (int)card.y + 94, 20, WHITE);

    int statusY = (int)card.y + 120;
    if (Shopvars.equipped[row] == col) {
        drawTextCentered("EQUIPPED", centerX, statusY, 20, (Color){ 120, 230, 120, 255 });
    } else if (owns(Shopvars.owned[row], col)) {
        drawTextCentered("OWNED", centerX, statusY, 20, Fade(WHITE, 0.8f));
    } else {
        Color priceColor = Gamevars.Coins >= price ? YELLOW : (Color){ 255, 110, 110, 255 };
        const char *priceText = TextFormat("%d", price);
        int textWidth = MeasureText(priceText, 22);
        int startX = centerX - (textWidth + 26) / 2;
        DrawTextureEx(maincoin.texture, (Vector2){ (float)startX, (float)statusY - 2 }, 0.0f, 0.34f, WHITE);
        DrawText(priceText, startX + 26, statusY, 22, priceColor);
    }

    if (selected) {
        float pulse = 2 * sinf(time * 5.0f);
        Rectangle border = { card.x - 3 - pulse, card.y - 3 - pulse, card.width + 6 + 2 * pulse, card.height + 6 + 2 * pulse };
        DrawRectangleRoundedLinesEx(border, 0.12f, 10, 4.0f, MENU_ORANGE);
    }
}

void DrawShop(int screenWidth, int screenHeight) {
    int centerX = screenWidth / 2;

    drawTextShadowCentered("SHOP", centerX, 20, 56, MENU_GOLD);
    drawButton(ShopBackButtonRect(), "BACK", 26, Fade(BLACK, 0.4f), 1.0f);
    if (AdsSupported()) {
        drawButton(ShopAdButtonRect(screenWidth), TextFormat("AD: +%d COINS", AD_REWARD_COINS), 22,
                   MENU_GREEN, RewardedAdReady() ? 1.0f : 0.5f);
    }

    const char *coinsText = TextFormat("%d", Gamevars.Coins);
    int coinsWidth = MeasureText(coinsText, 32);
    DrawRectangleRounded((Rectangle){ screenWidth - coinsWidth - 100, 24, coinsWidth + 76, 52 }, 1.0f, 10, Fade(BLACK, 0.3f));
    DrawTextureEx(maincoin.texture, (Vector2){ screenWidth - coinsWidth - 88, 30 }, 0.0f, 0.62f, WHITE);
    DrawText(coinsText, screenWidth - coinsWidth - 42, 35, 32, YELLOW);

    for (int row = 0; row < SHOP_ROWS; row++) {
        drawTextShadowCentered(shopRowTitles[row], centerX, (int)shopCardRect(row, 0, screenWidth).y - 28, 22, WHITE);
        for (int col = 0; col < shopRowSize(row); col++) drawShopCard(row, col, screenWidth, screenHeight);
    }

    if (Shopvars.messageTimer > 0) {
        float alpha = Shopvars.messageTimer > 20 ? 1.0f : Shopvars.messageTimer / 20.0f;
        drawTextShadowCentered(Shopvars.message, centerX, 640, 24, Fade(Shopvars.messageColor, alpha));
    }

    const char *hint = IS_MOBILE ? "Tap a card to choose   -   tap it again to buy / equip"
                                 : "ARROWS to choose   -   X to buy / equip   -   BACKSPACE to go back";
    drawTextShadowCentered(hint, centerX, screenHeight - 34, 20, WHITE);
}
