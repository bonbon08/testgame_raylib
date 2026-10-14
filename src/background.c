#include "raylib.h"
#include <stdlib.h>
#include <math.h>

#define NUM_CLOUDS 7
#define NUM_HILLS 6

typedef struct Cloud {
    Vector2 pos;
    float speed;
    float scale;
} Cloud;

typedef struct Hill {
    Vector2 center;
    float radius;
} Hill;

Cloud clouds[NUM_CLOUDS];
Hill farHills[NUM_HILLS];
Hill nearHills[NUM_HILLS];

const Color SKY_TOP = { 72, 152, 235, 255 };
const Color SKY_BOTTOM = { 186, 226, 255, 255 };
const Color FAR_HILL_COLOR = { 132, 196, 214, 255 };
const Color NEAR_HILL_COLOR = { 104, 178, 150, 255 };

void resetCloud(Cloud *cloud, int screenWidth, int screenHeight, bool randomX) {
    cloud->scale = 0.6f + (rand() % 100) / 100.0f;
    cloud->speed = 0.2f + cloud->scale * 0.4f;
    cloud->pos.x = randomX ? (float)(rand() % screenWidth) : -160.0f * cloud->scale;
    cloud->pos.y = (float)(rand() % (screenHeight / 2));
}

void InitBackground(int screenWidth, int screenHeight) {
    for (int i = 0; i < NUM_CLOUDS; i++) resetCloud(&clouds[i], screenWidth, screenHeight, true);

    float spacing = screenWidth / (float)(NUM_HILLS - 1);
    for (int i = 0; i < NUM_HILLS; i++) {
        farHills[i].radius = 220.0f + rand() % 80;
        farHills[i].center = (Vector2){ i * spacing, screenHeight + 90.0f };
        nearHills[i].radius = 180.0f + rand() % 60;
        nearHills[i].center = (Vector2){ i * spacing + spacing / 2.0f, screenHeight + 110.0f };
    }
}

void UpdateBackground(int screenWidth, int screenHeight) {
    for (int i = 0; i < NUM_CLOUDS; i++) {
        clouds[i].pos.x += clouds[i].speed;
        if (clouds[i].pos.x > screenWidth + 160.0f * clouds[i].scale) {
            resetCloud(&clouds[i], screenWidth, screenHeight, false);
        }
    }
}

void drawCloud(Cloud cloud) {
    float s = cloud.scale;
    Color c = { 244, 249, 255, 255 };
    DrawCircleV((Vector2){ cloud.pos.x, cloud.pos.y + 10 * s }, 34 * s, c);
    DrawCircleV((Vector2){ cloud.pos.x + 38 * s, cloud.pos.y - 8 * s }, 44 * s, c);
    DrawCircleV((Vector2){ cloud.pos.x + 82 * s, cloud.pos.y + 6 * s }, 36 * s, c);
    DrawRectangleRounded((Rectangle){ cloud.pos.x - 20 * s, cloud.pos.y + 6 * s, 124 * s, 38 * s }, 1.0f, 8, c);
}

void DrawBackground(int screenWidth, int screenHeight) {
    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, SKY_TOP, SKY_BOTTOM);

    Vector2 sun = { screenWidth - 150.0f, 120.0f };
    float glow = 6.0f * sinf((float)GetTime() * 1.5f);
    DrawCircleV(sun, 90 + glow, Fade((Color){ 255, 240, 170, 255 }, 0.25f));
    DrawCircleV(sun, 66 + glow / 2, Fade((Color){ 255, 236, 150, 255 }, 0.45f));
    DrawCircleV(sun, 48, (Color){ 255, 228, 110, 255 });

    for (int i = 0; i < NUM_CLOUDS; i++) drawCloud(clouds[i]);

    for (int i = 0; i < NUM_HILLS; i++) DrawCircleV(farHills[i].center, farHills[i].radius, FAR_HILL_COLOR);
    for (int i = 0; i < NUM_HILLS; i++) DrawCircleV(nearHills[i].center, nearHills[i].radius, NEAR_HILL_COLOR);
}
