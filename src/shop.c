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

typedef struct Shop {
    int row;               // 0 = characters, 1 = backgrounds
    int col;
    int ownedCharacters;   // bitmask
    int ownedBackgrounds;  // bitmask
    int character;
    int background;
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
    return characterTextures[Shopvars.character][direction == 0 ? 0 : 1];
}

bool owns(int mask, int item) {
    return (mask >> item) & 1;
}

void LoadGame() {
    SaveData save;
    LoadSave(&save);

    Gamevars.highscore = save.highscore;
    Gamevars.Coins = save.coins;
    Shopvars.ownedCharacters = save.ownedCharacters;
    Shopvars.ownedBackgrounds = save.ownedBackgrounds;
    Shopvars.character = save.character;
    Shopvars.background = save.background;

    if (Shopvars.character < 0 || Shopvars.character >= NUM_CHARACTERS || !owns(Shopvars.ownedCharacters, Shopvars.character)) {
        Shopvars.character = 0;
    }
    if (Shopvars.background < 0 || Shopvars.background >= NUM_THEMES || !owns(Shopvars.ownedBackgrounds, Shopvars.background)) {
        Shopvars.background = 0;
    }
}

void SaveGame() {
    SaveData save = {
        Gamevars.highscore, Gamevars.Coins,
        Shopvars.ownedCharacters, Shopvars.ownedBackgrounds,
        Shopvars.character, Shopvars.background
    };
    if (!WriteSave(&save)) TraceLog(LOG_WARNING, "Could not write save file %s", SAVE_FILE);
}

int shopRowSize(int row) {
    return row == 0 ? NUM_CHARACTERS : NUM_THEMES;
}

Rectangle shopCardRect(int row, int col, int screenWidth) {
    float cardWidth = 170, cardHeight = 200, gap = 22;
    int size = shopRowSize(row);
    float totalWidth = size * cardWidth + (size - 1) * gap;
    float x = (screenWidth - totalWidth) / 2 + col * (cardWidth + gap);
    float y = row == 0 ? 150 : 420;
    return (Rectangle){ x, y, cardWidth, cardHeight };
}

void setShopMessage(const char *text, Color color) {
    snprintf(Shopvars.message, sizeof(Shopvars.message), "%s", text);
    Shopvars.messageColor = color;
    Shopvars.messageTimer = SHOP_MESSAGE_FRAMES;
}

// The background shown behind everything: while browsing backgrounds in the shop, the selected one is previewed
int PreviewedBackground() {
    if (Menuvars.enabled && Menuvars.untermenu == 1 && Shopvars.row == 1) return Shopvars.col;
    return Shopvars.background;
}

void UpdateShop(int screenWidth) {
    if (Shopvars.messageTimer > 0) Shopvars.messageTimer--;

    if (IsKeyPressed(KEY_BACKSPACE)) {
        Menuvars.untermenu = 0;
        return;
    }

    int size = shopRowSize(Shopvars.row);
    if (IsKeyPressed(KEY_LEFT)) Shopvars.col = (Shopvars.col - 1 + size) % size;
    if (IsKeyPressed(KEY_RIGHT)) Shopvars.col = (Shopvars.col + 1) % size;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {
        Shopvars.row = 1 - Shopvars.row;
        if (Shopvars.col >= shopRowSize(Shopvars.row)) Shopvars.col = shopRowSize(Shopvars.row) - 1;
    }

    if (!IsKeyPressed(KEY_X)) return;

    bool isCharacter = Shopvars.row == 0;
    int item = Shopvars.col;
    int *owned = isCharacter ? &Shopvars.ownedCharacters : &Shopvars.ownedBackgrounds;
    int *equipped = isCharacter ? &Shopvars.character : &Shopvars.background;
    const char *name = isCharacter ? characters[item].name : themes[item].name;
    int price = isCharacter ? characters[item].price : themes[item].price;

    if (owns(*owned, item)) {
        *equipped = item;
        setShopMessage(TextFormat("%s equipped", name), WHITE);
    } else if (Gamevars.Coins >= price) {
        Gamevars.Coins -= price;
        *owned |= 1 << item;
        *equipped = item;
        setShopMessage(TextFormat("Bought %s!", name), (Color){ 120, 230, 120, 255 });
        Rectangle card = shopCardRect(Shopvars.row, item, screenWidth);
        SpawnParticles((Vector2){ card.x + card.width / 2, card.y + card.height / 2 }, 40, YELLOW, 7.0f, 7.0f);
        PlaySound(maincoin.sound);
    } else {
        setShopMessage(TextFormat("Not enough coins - you need %d more", price - Gamevars.Coins), (Color){ 255, 110, 110, 255 });
        return;
    }
    SaveGame();
}

void drawShopCard(int row, int col, int screenWidth, int screenHeight) {
    bool selected = Shopvars.row == row && Shopvars.col == col;
    bool isCharacter = row == 0;
    int owned = isCharacter ? Shopvars.ownedCharacters : Shopvars.ownedBackgrounds;
    int equipped = isCharacter ? Shopvars.character : Shopvars.background;
    const char *name = isCharacter ? characters[col].name : themes[col].name;
    int price = isCharacter ? characters[col].price : themes[col].price;
    float time = (float)GetTime();

    Rectangle card = shopCardRect(row, col, screenWidth);
    if (selected) card.y -= 6;
    DrawRectangleRounded(card, 0.12f, 10, Fade(BLACK, selected ? 0.4f : 0.28f));

    if (isCharacter) {
        Texture2D texture = characterTextures[col][0];
        float wobble = selected ? 8 * sinf(time * 4.0f) : 0.0f;
        Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
        Rectangle dest = { card.x + card.width / 2, card.y + 64, texture.width * 1.6f, texture.height * 1.6f };
        DrawTexturePro(texture, source, dest, (Vector2){ dest.width / 2, dest.height / 2 }, wobble, WHITE);
    } else {
        Rectangle preview = { card.x + 14, card.y + 14, card.width - 28, 100 };
        DrawThemePreview(themes[col], preview, screenHeight);
        DrawRectangleLinesEx(preview, 2.0f, Fade(WHITE, 0.6f));
    }

    drawTextCentered(name, (int)(card.x + card.width / 2), (int)card.y + 128, 22, WHITE);

    int statusY = (int)card.y + 162;
    int centerX = (int)(card.x + card.width / 2);
    if (equipped == col) {
        drawTextCentered("EQUIPPED", centerX, statusY, 20, (Color){ 120, 230, 120, 255 });
    } else if (owns(owned, col)) {
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

    drawTextShadowCentered("SHOP", centerX, 28, 60, MENU_GOLD);

    const char *coinsText = TextFormat("%d", Gamevars.Coins);
    int coinsWidth = MeasureText(coinsText, 32);
    DrawRectangleRounded((Rectangle){ screenWidth - coinsWidth - 100, 32, coinsWidth + 76, 52 }, 1.0f, 10, Fade(BLACK, 0.3f));
    DrawTextureEx(maincoin.texture, (Vector2){ screenWidth - coinsWidth - 88, 38 }, 0.0f, 0.62f, WHITE);
    DrawText(coinsText, screenWidth - coinsWidth - 42, 43, 32, YELLOW);

    drawTextShadowCentered("CHARACTERS", centerX, 110, 26, WHITE);
    for (int i = 0; i < NUM_CHARACTERS; i++) drawShopCard(0, i, screenWidth, screenHeight);

    drawTextShadowCentered("BACKGROUNDS", centerX, 380, 26, WHITE);
    for (int i = 0; i < NUM_THEMES; i++) drawShopCard(1, i, screenWidth, screenHeight);

    if (Shopvars.messageTimer > 0) {
        float alpha = Shopvars.messageTimer > 20 ? 1.0f : Shopvars.messageTimer / 20.0f;
        drawTextShadowCentered(Shopvars.message, centerX, 636, 24, Fade(Shopvars.messageColor, alpha));
    }

    drawTextShadowCentered("ARROWS to choose   -   X to buy / equip   -   BACKSPACE to go back", centerX, screenHeight - 38, 20, WHITE);
}
