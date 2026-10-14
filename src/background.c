#include "raylib.h"
#include <stdlib.h>
#include <math.h>

#define NUM_CLOUDS 7
#define NUM_HILLS 6
#define NUM_STARS 70
#define NUM_THEMES 4

typedef struct Cloud {
    Vector2 pos;
    float speed;
    float scale;
} Cloud;

typedef struct Hill {
    Vector2 center;
    float radius;
} Hill;

typedef struct Theme {
    const char *name;
    int price;
    Color skyTop;
    Color skyBottom;
    Color sun;
    Color cloud;
    Color farHill;
    Color nearHill;
    float sunY;
    bool night;
} Theme;

Theme themes[NUM_THEMES] = {
    { "Day",    0,   { 72, 152, 235, 255 }, { 186, 226, 255, 255 }, { 255, 228, 110, 255 },
      { 244, 249, 255, 255 }, { 132, 196, 214, 255 }, { 104, 178, 150, 255 }, 120, false },
    { "Sunset", 30,  { 64, 48, 120, 255 },  { 255, 160, 100, 255 }, { 255, 120, 60, 255 },
      { 255, 196, 190, 255 }, { 170, 90, 120, 255 },  { 110, 60, 100, 255 },  470, false },
    { "Night",  60,  { 8, 12, 36, 255 },    { 36, 54, 104, 255 },   { 236, 236, 214, 255 },
      { 60, 72, 120, 255 },   { 32, 48, 92, 255 },    { 20, 32, 66, 255 },    110, true },
    { "Candy",  120, { 255, 160, 215, 255 }, { 190, 235, 255, 255 }, { 255, 255, 255, 255 },
      { 255, 255, 255, 255 }, { 170, 235, 210, 255 }, { 255, 190, 225, 255 }, 120, false },
};

Cloud clouds[NUM_CLOUDS];
Hill farHills[NUM_HILLS];
Hill nearHills[NUM_HILLS];
Vector2 stars[NUM_STARS];

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

    for (int i = 0; i < NUM_STARS; i++) {
        stars[i] = (Vector2){ (float)(rand() % screenWidth), (float)(rand() % (screenHeight * 2 / 3)) };
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

void drawCloud(Cloud cloud, Color c) {
    float s = cloud.scale;
    DrawCircleV((Vector2){ cloud.pos.x, cloud.pos.y + 10 * s }, 34 * s, c);
    DrawCircleV((Vector2){ cloud.pos.x + 38 * s, cloud.pos.y - 8 * s }, 44 * s, c);
    DrawCircleV((Vector2){ cloud.pos.x + 82 * s, cloud.pos.y + 6 * s }, 36 * s, c);
    DrawRectangleRounded((Rectangle){ cloud.pos.x - 20 * s, cloud.pos.y + 6 * s, 124 * s, 38 * s }, 1.0f, 8, c);
}

void DrawBackground(int screenWidth, int screenHeight, Theme theme) {
    float time = (float)GetTime();
    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, theme.skyTop, theme.skyBottom);

    if (theme.night) {
        for (int i = 0; i < NUM_STARS; i++) {
            float twinkle = 0.5f + 0.5f * sinf(time * 2.0f + i * 1.7f);
            DrawRectangleV(stars[i], (Vector2){ 2 + (i % 3 == 0), 2 + (i % 3 == 0) }, Fade(WHITE, 0.3f + 0.7f * twinkle));
        }
    }

    Vector2 sun = { screenWidth - 150.0f, theme.sunY };
    float glow = 6.0f * sinf(time * 1.5f);
    float glowAlpha = theme.night ? 0.06f : 0.2f;   // the moon only glows faintly
    DrawCircleV(sun, 90 + glow, Fade(theme.sun, glowAlpha));
    DrawCircleV(sun, 66 + glow / 2, Fade(theme.sun, 2 * glowAlpha));
    DrawCircleV(sun, 48, theme.sun);
    if (theme.night) {
        // a few craters turn the sun into a moon
        Color crater = { 206, 206, 186, 255 };
        DrawCircleV((Vector2){ sun.x - 14, sun.y - 10 }, 10, crater);
        DrawCircleV((Vector2){ sun.x + 16, sun.y + 12 }, 7, crater);
        DrawCircleV((Vector2){ sun.x + 6, sun.y - 22 }, 5, crater);
    }

    for (int i = 0; i < NUM_CLOUDS; i++) drawCloud(clouds[i], theme.cloud);

    for (int i = 0; i < NUM_HILLS; i++) DrawCircleV(farHills[i].center, farHills[i].radius, theme.farHill);
    for (int i = 0; i < NUM_HILLS; i++) DrawCircleV(nearHills[i].center, nearHills[i].radius, theme.nearHill);
}

// Small static version of a theme, used for the shop cards
void DrawThemePreview(Theme theme, Rectangle area, int screenHeight) {
    BeginScissorMode((int)area.x, (int)area.y, (int)area.width, (int)area.height);
    DrawRectangleGradientV((int)area.x, (int)area.y, (int)area.width, (int)area.height, theme.skyTop, theme.skyBottom);
    if (theme.night) {
        for (int i = 0; i < 12; i++) {
            DrawRectangle((int)(area.x + (i * 37) % (int)area.width), (int)(area.y + (i * 23) % (int)(area.height / 2)), 2, 2, WHITE);
        }
    }
    Vector2 sun = { area.x + area.width * 0.75f, area.y + area.height * theme.sunY / screenHeight };
    DrawCircleV(sun, 16, Fade(theme.sun, 0.35f));
    DrawCircleV(sun, 10, theme.sun);
    DrawCircleV((Vector2){ area.x + area.width * 0.3f, area.y + area.height * 0.3f }, 9, theme.cloud);
    DrawCircleV((Vector2){ area.x + area.width * 0.3f + 12, area.y + area.height * 0.3f - 3 }, 11, theme.cloud);
    DrawCircleV((Vector2){ area.x + area.width * 0.2f, area.y + area.height + 30 }, 60, theme.farHill);
    DrawCircleV((Vector2){ area.x + area.width * 0.85f, area.y + area.height + 34 }, 58, theme.farHill);
    DrawCircleV((Vector2){ area.x + area.width * 0.55f, area.y + area.height + 40 }, 56, theme.nearHill);
    EndScissorMode();
}
