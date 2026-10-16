// Game objects (chicken, spikes, coin, difficulty)
#ifndef OBJECTS_H
#define OBJECTS_H

#include <raylib.h>
#include <stdbool.h>

#define MAX_SPIKES 7
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
    float squash;
    int deathTimer;
    int invulnerable;
    bool dead;
} Player;

typedef struct Gamelogik {
    int score;
    int highscore;
    int Coins;
    bool newHighscore;
    int bounces;
    bool usedRevive;
    bool startgame;
    int frameCounter;
    int screenWidth;
    int screenHeight;
} Gamelogik;

typedef struct Menu {
    int auswahl;
    bool enabled;
    int untermenu;
    bool quit;
} Menu;

// Global state accessors
Player *GetChicken(void);
Gamelogik *GetGamevars(void);
Menu *GetMenuvars(void);
Coin *GetMaincoin(void);
Spike *GetLspikes(void);
Spike *GetRspikes(void);
Texture2D *GetSpikeLeftTex(void);
Texture2D *GetSpikeRightTex(void);

// Initialization
void Initgamevars(void);
void Initplayer(int screenWidth, int screenHeight);
void InitSpikes(int screenWidth, int screenHeight);
void InitCoin(void);
void InitColors(void);

// Game logic
int GameLevel(void);
int SpikeCount(void);
float ChickenSpeed(void);

// Spawning
void generateSpikes(int screenWidth, int screenHeight, bool activateLeft);
void generateCoin(int screenWidth, int screenHeight);

// Rendering
void DrawCoin(void);
void DrawChicken(Texture2D texture, Color tint);

#endif // OBJECTS_H