// Game objects: chicken, spikes, coin, difficulty curve
#include "objects.h"

#include <raylib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#define MAX_SPIKES 7    // never more spikes on one wall, so there is always a way through
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

typedef struct Player {
    Vector2 pos;
    int direction;
    float velocityY;
    float gravity;
    float jumpForce;
    float rotation;
    float squash;      // > 0 stretches the chicken (jump), < 0 flattens it (landing)
    int deathTimer;
    int invulnerable;  // frames left without spike collisions (after a revive)
    bool dead;
} Player;

typedef struct Gamelogik {
    int score;
    int highscore;
    int Coins;
    bool newHighscore;
    int bounces;       // wall touches in this run, drives the difficulty
    bool usedRevive;   // a run can only be continued once
    bool startgame;
    int frameCounter;
    int screenWidth;
    int screenHeight;
} Gamelogik;

typedef struct Menu {
    int auswahl;
    bool enabled;
    int untermenu;     // 0 = main menu, 1 = shop
    bool quit;
} Menu;

// Global state (shared with main loop)
static Spike lspikes[MAX_SPIKES];
static Spike rspikes[MAX_SPIKES];
static Coin maincoin;
static Player Chicken;
static Gamelogik Gamevars;
static Menu Menuvars;

static Texture2D spikeLeftTex;
static Texture2D spikeRightTex;

// ---- Public accessors ----
Player *GetChicken(void)      { return &Chicken; }
Gamelogik *GetGamevars(void)  { return &Gamevars; }
Menu *GetMenuvars(void)       { return &Menuvars; }
Coin *GetMaincoin(void)       { return &maincoin; }
Spike *GetLspikes(void)       { return lspikes; }
Spike *GetRspikes(void)       { return rspikes; }

Texture2D *GetSpikeLeftTex(void)  { return &spikeLeftTex; }
Texture2D *GetSpikeRightTex(void) { return &spikeRightTex; }

// ---- Difficulty ----
#define BOUNCES_PER_LEVEL 3
#define START_SPIKES 4
#define START_SPEED 8.0f
#define SPEED_PER_BOUNCE 0.3f
#define MAX_SPEED 15.0f

int GameLevel(void) {
    return 1 + Gamevars.bounces / BOUNCES_PER_LEVEL;
}

int SpikeCount(void) {
    int count = START_SPIKES + GameLevel() - 1;
    return count < MAX_SPIKES ? count : MAX_SPIKES;
}

float ChickenSpeed(void) {
    float speed = START_SPEED + Gamevars.bounces * SPEED_PER_BOUNCE;
    return speed < MAX_SPEED ? speed : MAX_SPEED;
}

void generateSpikes(int screenWidth, int screenHeight, bool activateLeft) {
    const int totalSlots = screenHeight / SLOT_HEIGHT;

    int *allSlots = (int *)malloc(totalSlots * sizeof(int));
    for (int i = 0; i < totalSlots; i++) allSlots[i] = i;

    for (int i = totalSlots - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = allSlots[i];
        allSlots[i] = allSlots[j];
        allSlots[j] = temp;
    }

    int count = SpikeCount();
    for (int i = 0; i < MAX_SPIKES; i++) {
        lspikes[i].pos = (Vector2){ 0, (float)(allSlots[i] * SLOT_HEIGHT) };
        lspikes[i].active = activateLeft && i < count;

        rspikes[i].pos = (Vector2){ (float)screenWidth, (float)(allSlots[i] * SLOT_HEIGHT) };
        rspikes[i].active = !activateLeft && i < count;
    }

    free(allSlots);
}

void InitSpikes(int screenWidth, int screenHeight) {
    Image spikel = LoadImage("data/nSpike.png");
    ImageResize(&spikel, SLOT_HEIGHT, SLOT_HEIGHT);
    spikeLeftTex = LoadTextureFromImage(spikel);
    UnloadImage(spikel);

    Image spiker = LoadImage("data/nSpike.png");
    ImageResize(&spiker, SLOT_HEIGHT, SLOT_HEIGHT);
    spikeRightTex = LoadTextureFromImage(spiker);
    UnloadImage(spiker);

    for (int i = 0; i < MAX_SPIKES; i++) {
        lspikes[i].spiketexture = spikeLeftTex;
        rspikes[i].spiketexture = spikeRightTex;
    }

    generateSpikes(screenWidth, screenHeight, false);
}

void generateCoin(int screenWidth, int screenHeight) {
    int genwidth = screenWidth - 128;
    int genheight = screenHeight - 64;
    int x = rand() % genwidth;
    int y = rand() % genheight;
    maincoin.pos = (Vector2){(float)(x + 64), (float)y};
}

void InitCoin(void) {
    Image CoinImage = LoadImage("data/coin.png");
    ImageResize(&CoinImage, 64, 64);
    maincoin.texture = LoadTextureFromImage(CoinImage);
    UnloadImage(CoinImage);
    maincoin.sound = LoadSound("data/coin.wav");
}

// The coin spins (squeezed horizontally) and floats up and down
void DrawCoin(void) {
    float time = (float)GetTime();
    float spin = fabsf(cosf(time * 3.0f));
    if (spin < 0.15f) spin = 0.15f;
    float bob = 6.0f * sinf(time * 4.0f);

    Rectangle source = { 0, 0, (float)maincoin.texture.width, (float)maincoin.texture.height };
    Rectangle dest = { maincoin.pos.x + 32, maincoin.pos.y + 32 + bob, 64 * spin, 64 };
    DrawTexturePro(maincoin.texture, source, dest,
                   (Vector2){ dest.width / 2, 32 }, 0.0f, WHITE);
}

void Initplayer(int screenWidth, int screenHeight) {
    Chicken.pos = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    Chicken.direction = 0;
    Chicken.velocityY = 0.1f;
    Chicken.gravity = 0.5f;
    Chicken.jumpForce = -12.0f;
    Chicken.rotation = 0.0f;
    Chicken.squash = 0.0f;
    Chicken.deathTimer = 0;
    Chicken.invulnerable = 0;
    Chicken.dead = false;
}

void Initgamevars(void) {
    Gamevars.score = 0;
    Gamevars.highscore = 0;
    Gamevars.Coins = 0;
    Gamevars.newHighscore = false;
    Gamevars.bounces = 0;
    Gamevars.usedRevive = false;
    Gamevars.startgame = false;
    Gamevars.frameCounter = 0;
    Gamevars.screenWidth = 1080;
    Gamevars.screenHeight = 720;
}

void Initcolors(void) {
    Menuvars.enabled = true;
    Menuvars.quit = false;
    Menuvars.auswahl = 0;
    Menuvars.untermenu = 0;
}

void DrawChicken(Texture2D texture, Color tint) {
    float scaleX = 1.0f - 0.25f * Chicken.squash;
    float scaleY = 1.0f + 0.25f * Chicken.squash;

    Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest = {
        Chicken.pos.x + CHICKEN_SIZE / 2.0f,
        Chicken.pos.y + CHICKEN_SIZE / 2.0f,
        CHICKEN_SIZE * scaleX, CHICKEN_SIZE * scaleY
    };
    DrawTexturePro(texture, source, dest,
                   (Vector2){ dest.width / 2, dest.height / 2 },
                   Chicken.rotation, tint);
}

#endif /* OBJECTS_C */
