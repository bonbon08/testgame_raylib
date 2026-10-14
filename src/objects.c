#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "Filerw.c"

#define NUM_SPIKES 5
#define SLOT_HEIGHT 64
#define CHICKEN_SIZE 64

typedef struct Spike {
    Vector2 pos;
    bool active;
    Texture2D spiketexture;
} Spike;

typedef struct Coin {
    Vector2 pos;
    Texture2D texture;
    Sound sound;
} Coin;

typedef struct Player{
    Vector2 pos;
    int direction;
    float velocityY;
    float gravity;
    float jumpForce;
    float rotation;
    float squash;      // > 0 stretches the chicken (jump), < 0 flattens it (landing)
    int deathTimer;
    bool dead;
} Player;

typedef struct Gamelogik{
    int score;
    int highscore;
    int Coins;
    bool newHighscore;
    bool startgame;
    int frameCounter;
    int screenWidth;
    int screenHeight;
} Gamelogik ;

typedef struct Menu {
    int auswahl;
    bool enabled;
    int untermenu;     // 0 = main menu, 1 = shop
    bool quit;
} Menu ;


Spike lspikes[NUM_SPIKES];
Spike rspikes[NUM_SPIKES];
Coin maincoin;
Player Chicken;
Gamelogik Gamevars;
Menu Menuvars;

void Initgamevars() {
    Gamevars.score = 0;
    Gamevars.highscore = 0;
    Gamevars.Coins = 0;
    Gamevars.newHighscore = false;
    Gamevars.startgame = false;
    Gamevars.frameCounter = 0;
    Gamevars.screenWidth = 1080;
    Gamevars.screenHeight = 720;
}

void Initcolors() {
    Menuvars.enabled = true;
    Menuvars.quit = false;
    Menuvars.auswahl = 0;
    Menuvars.untermenu = 0;
}

void generateSpikes(int screenWidth, int screenHeight, bool activateLeft) {
    const int totalSlots = screenHeight / SLOT_HEIGHT;

    int *allSlots = (int *)malloc(totalSlots * sizeof(int));
    for (int i = 0; i < totalSlots; i++) {
        allSlots[i] = i;
    }

    for (int i = totalSlots - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = allSlots[i];
        allSlots[i] = allSlots[j];
        allSlots[j] = temp;
    }

    for (int i = 0; i < NUM_SPIKES; i++) {
        lspikes[i].pos = (Vector2){ 0, (float)(allSlots[i] * SLOT_HEIGHT) };
        lspikes[i].active = activateLeft;

        rspikes[i].pos = (Vector2){ (float)(screenWidth), (float)(allSlots[i] * SLOT_HEIGHT) };
        rspikes[i].active = !activateLeft;
    }

    free(allSlots);
}

Texture2D spikeLeftTex;
Texture2D spikeRightTex;

void InitSpikes(int screenWidth, int screenHeight) {
    Image spikel = LoadImage("data/nSpike.png");
    ImageResize(&spikel, SLOT_HEIGHT, SLOT_HEIGHT);
    spikeLeftTex = LoadTextureFromImage(spikel);
    UnloadImage(spikel);

    Image spiker = LoadImage("data/nSpike.png");
    ImageResize(&spiker, SLOT_HEIGHT, SLOT_HEIGHT);
    spikeRightTex = LoadTextureFromImage(spiker);
    UnloadImage(spiker);

    for (int i = 0; i < NUM_SPIKES; i++){
        lspikes[i].spiketexture = spikeLeftTex;
        rspikes[i].spiketexture = spikeRightTex;
    }

    generateSpikes(screenWidth, screenHeight, false);
}

void generateCoin(int screenWidth, int screenHeight) {
    int genwitdh = screenWidth - 128;
    int genheight = screenHeight - 64;
    int x = rand() % genwitdh;
    int y = rand() % genheight;
    maincoin.pos = (Vector2){(x+64), (y)};
}

void InitCoin() {
    Image CoinImage = LoadImage("data/coin.png");
    ImageResize(&CoinImage, 64, 64);
    maincoin.texture = LoadTextureFromImage(CoinImage);
    UnloadImage(CoinImage);
    maincoin.sound = LoadSound("data/coin.wav");
}

// The coin spins (squeezed horizontally) and floats up and down
void DrawCoin() {
    float time = (float)GetTime();
    float spin = fabsf(cosf(time * 3.0f));
    if (spin < 0.15f) spin = 0.15f;
    float bob = 6.0f * sinf(time * 4.0f);

    Rectangle source = { 0, 0, (float)maincoin.texture.width, (float)maincoin.texture.height };
    Rectangle dest = { maincoin.pos.x + 32, maincoin.pos.y + 32 + bob, 64 * spin, 64 };
    DrawTexturePro(maincoin.texture, source, dest, (Vector2){ dest.width / 2, 32 }, 0.0f, WHITE);
}

void Initplayer(int screenWidth, int screenHeight){
    Chicken.pos = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    Chicken.direction = 0;

    Chicken.velocityY = 0.1f;
    Chicken.gravity = 0.5f;
    Chicken.jumpForce = -12.0f;
    Chicken.rotation = 0.0f;
    Chicken.squash = 0.0f;
    Chicken.deathTimer = 0;
    Chicken.dead = false;
}

void DrawChicken(Texture2D texture) {
    float scaleX = 1.0f - 0.25f * Chicken.squash;
    float scaleY = 1.0f + 0.25f * Chicken.squash;

    Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest = { Chicken.pos.x + CHICKEN_SIZE / 2.0f, Chicken.pos.y + CHICKEN_SIZE / 2.0f,
                       CHICKEN_SIZE * scaleX, CHICKEN_SIZE * scaleY };
    DrawTexturePro(texture, source, dest, (Vector2){ dest.width / 2, dest.height / 2 }, Chicken.rotation, WHITE);
}
