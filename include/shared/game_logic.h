// Core game logic – shared between Desktop and Android
#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdint.h>
#include <stdbool.h>

// Game state
typedef struct GameState {
    bool isRunning;
    int score;
    int coins;
    int highScore;
    int bounces;
    int usedRevive;
    int invulnerable;
    bool dead;
    bool quit;
    int frameCounter;
    int screenWidth;
    int screenHeight;
} GameState;

// Player state
typedef struct Player {
    int direction;
    float velocityY;
    float gravity;
    float jumpForce;
    float squash;
    int health;
    bool dead;
} Player;

// Object types
typedef enum {
    OBJ_CHICKEN,
    OBJ_BIRD,
    OBJ_FLAMINGO,
    OBJ_GOLD,
    OBJ_SHADOW,
    OBJ_MUSIC,
    OBJ_DARK
} ObjectType;

// Main game module
typedef struct GameModule {
    GameState state;
    Player player;
    ObjectType currentObject;
    int objectIndex;
    bool isMenu;
    bool isShop;
    bool isDead;
    bool isQuit;
    bool isPlaying;
    bool isLoadingAd;
    
    // Function pointers (called by main loop)
    void* onUpdate(void* arg);
    void* onDraw(void* arg);
    void* onInput(void* arg);
    void* onAudio(void* arg);
    void* onShop(void* arg);
    void* onMenu(void* arg);
    void* onLeaderboard(void* arg);
    void* onConsent(void* arg);
    void* onAds(void* arg);
    void* onSave(void* arg);
    void* onLoad(void* arg);
} GameModule;

// Global game instance (initialized in main)
GameModule g_game;

#endif // GAME_LOGIC_H
