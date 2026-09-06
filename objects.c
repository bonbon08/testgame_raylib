#include "raylib.h"
#include <stdlib.h>

#define NUM_TEXTURES 4
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
} Coin;

typedef struct Player{
    Vector2 pos;
    int spr;
    int direction;
    Texture2D textures[4];
    float velocityY;
    float gravity;
    float jumpForce;
    bool dead;
} Player;

typedef struct Gamelogik{
    int score;
    int highscore;
    int Coins;
    bool startgame;
    int frameCounter;
    int screenWidth;
    int screenHeight;
} Gamelogik ;

Spike lspikes[NUM_SPIKES];
Spike rspikes[NUM_SPIKES];
Coin maincoin;
Player Chicken;
Gamelogik Gamevars;

void Initgamevars() {
    Gamevars.score = 0;
    Gamevars.highscore = 0;
    Gamevars.Coins = 0;
    Gamevars.startgame = false;
    Gamevars.frameCounter = 0;
    Gamevars.screenWidth = 1080;
    Gamevars.screenHeight = 720;
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

// Globale Textur-Variablen (oder in main)
Texture2D spikeLeftTex;
Texture2D spikeRightTex;

void InitSpikes(int screenWidth, int screenHeight) {
    Image spikel = LoadImage("nSpike.png");
    ImageResize(&spikel, SLOT_HEIGHT, SLOT_HEIGHT);  
    spikeLeftTex = LoadTextureFromImage(spikel);
    UnloadImage(spikel);

    Image spiker = LoadImage("nSpike.png");
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
    Image CoinImage = LoadImage("coin.png");
    ImageResize(&CoinImage, 64, 64);
    maincoin.texture = LoadTextureFromImage(CoinImage);
}

void Initplayer(int screenWidth, int screenHeight){
    Image chickenImage1r = LoadImage("Chicken.png");
    ImageResize(&chickenImage1r, CHICKEN_SIZE, CHICKEN_SIZE);
    ImageRotate(&chickenImage1r, -20);
    Chicken.textures[0] = LoadTextureFromImage(chickenImage1r);
    UnloadImage(chickenImage1r); 

    Image chickenImage2r = LoadImage("Chicken.png");
    ImageResize(&chickenImage2r, CHICKEN_SIZE, CHICKEN_SIZE);
    ImageRotate(&chickenImage2r, 20);
    Chicken.textures[1] = LoadTextureFromImage(chickenImage2r);
    UnloadImage(chickenImage2r); 

    Image chickenImage1l = LoadImage("Chicken.png");
    ImageResize(&chickenImage1l, CHICKEN_SIZE, CHICKEN_SIZE);
    ImageRotate(&chickenImage1l, -20);
    ImageFlipHorizontal(&chickenImage1l);
    Chicken.textures[2] = LoadTextureFromImage(chickenImage1l);
    UnloadImage(chickenImage1l); 

    Image chickenImage2l = LoadImage("Chicken.png");
    ImageResize(&chickenImage2l, CHICKEN_SIZE, CHICKEN_SIZE);
    ImageRotate(&chickenImage2l, 20);
    ImageFlipHorizontal(&chickenImage2l);
    Chicken.textures[3] = LoadTextureFromImage(chickenImage2l);
    UnloadImage(chickenImage2l); 

    Chicken.pos = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    Chicken.spr = 0;
    Chicken.direction = 0; 

    Chicken.velocityY = 0.1f;
    Chicken.gravity = 0.5f;
    Chicken.jumpForce = -12.0f;
    Chicken.dead = false;
}