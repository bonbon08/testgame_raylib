// Particle effects, popups and screen shake
#include "effects.h"

#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAX_PARTICLES 160
#define MAX_POPUPS 8
#define POPUP_FRAMES 50

typedef struct Particle {
    Vector2 pos;
    Vector2 vel;
    float life;
    float maxLife;
    float size;
    Color color;
} Particle;

typedef struct Popup {
    Vector2 pos;
    float life;
    char text[16];
    Color color;
} Popup;

static Particle particles[MAX_PARTICLES];
static Popup popups[MAX_POPUPS];
static float shakeStrength = 0.0f;
static int shakeFrames = 0;

static float randomFloat(float min, float max) {
    return min + (max - min) * (rand() / (float)RAND_MAX);
}

void SpawnParticles(Vector2 pos, int count, Color color, float speed, float size) {
    for (int i = 0; i < MAX_PARTICLES && count > 0; i++) {
        if (particles[i].life > 0) continue;

        float angle = randomFloat(0, 2 * PI);
        float s = randomFloat(0.3f * speed, speed);
        particles[i].pos = pos;
        particles[i].vel = (Vector2){ cosf(angle) * s, sinf(angle) * s - 1.0f };
        particles[i].maxLife = randomFloat(25, 45);
        particles[i].life = particles[i].maxLife;
        particles[i].size = randomFloat(size * 0.5f, size);
        particles[i].color = color;
        count--;
    }
}

void SpawnPopup(Vector2 pos, const char *text, Color color) {
    for (int i = 0; i < MAX_POPUPS; i++) {
        if (popups[i].life > 0) continue;
        popups[i].pos = pos;
        popups[i].life = POPUP_FRAMES;
        popups[i].color = color;
        snprintf(popups[i].text, sizeof(popups[i].text), "%s", text);
        return;
    }
}

void StartShake(float strength, int frames) {
    if (strength < shakeStrength) return;
    shakeStrength = strength;
    shakeFrames = frames;
}

void UpdateEffects(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life <= 0) continue;
        particles[i].pos.x += particles[i].vel.x;
        particles[i].pos.y += particles[i].vel.y;
        particles[i].vel.x *= 0.97f;
        particles[i].vel.y += 0.15f;
        particles[i].life -= 1;
    }
    for (int i = 0; i < MAX_POPUPS; i++) {
        if (popups[i].life <= 0) continue;
        popups[i].pos.y -= 1.2f;
        popups[i].life -= 1;
    }
}

// Call once per frame; returns the camera offset for the current shake
Vector2 UpdateShake(void) {
    if (shakeFrames <= 0) {
        shakeStrength = 0.0f;
        return (Vector2){ 0, 0 };
    }
    shakeFrames--;
    Vector2 offset = { randomFloat(-shakeStrength, shakeStrength),
                       randomFloat(-shakeStrength, shakeStrength) };
    shakeStrength *= 0.9f;
    return offset;
}

void DrawEffects(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life <= 0) continue;
        float alpha = particles[i].life / particles[i].maxLife;
        float size = particles[i].size;
        DrawRectangleV(
            (Vector2){ particles[i].pos.x - size / 2, particles[i].pos.y - size / 2 },
            (Vector2){ size, size },
            Fade(particles[i].color, alpha)
        );
    }
    for (int i = 0; i < MAX_POPUPS; i++) {
        if (popups[i].life <= 0) continue;
        float alpha = popups[i].life / (float)POPUP_FRAMES;
        int x = (int)popups[i].pos.x - MeasureText(popups[i].text, 28) / 2;
        DrawText(popups[i].text, x + 2, (int)popups[i].pos.y + 2, 28,
                 Fade(BLACK, 0.4f * alpha));
        DrawText(popups[i].text, x, (int)popups[i].pos.y, 28,
                 Fade(popups[i].color, alpha));
    }
}

#endif /* EFFECTS_C */
