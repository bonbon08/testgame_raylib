#include "raylib.h"
#include <stdlib.h>

#define NUM_TEXTURES 4
#define NUM_SPIKES 5   
#define SLOT_HEIGHT 64 

typedef struct Spike {
    Vector2 pos;
    bool active;
} Spike;

typedef struct Coin {
    Vector2 pos;
} Coin;
Coin maincoin;
Spike lspikes[NUM_SPIKES];
Spike rspikes[NUM_SPIKES];

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
        
        rspikes[i].pos = (Vector2){ (float)(screenWidth - 64), (float)(allSlots[i] * SLOT_HEIGHT) };
        rspikes[i].active = !activateLeft;
    }
    
    free(allSlots);
}

void generateCoin(int screenWidth, int screenHeight) {
    int genwitdh = screenWidth - 128;
    int genheight = screenHeight - 64;
    int x = rand() % genwitdh;
    int y = rand() % genheight;
    maincoin.pos = (Vector2){(x+64), (y)};
}