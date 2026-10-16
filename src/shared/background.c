// Background rendering: themes, clouds, hills, stars
#include "background.h"

#include <raylib.h>
#include <stdlib.h>
#include <math.h>

#define NUM_CLOUDS 7
#define NUM_HILLS 6
#define NUM_STARS 70

static Cloud clouds[NUM_CLOUDS];
static Hill farHills[NUM_HILLS];
static Hill nearHills[NUM_HILLS];
static Vector2 stars[NUM_STARS];

void InitBackground(int screenWidth, int screenHeight) {
    for (int i = 0; i < NUM_CLOUDS; i++) {
        clouds[i].scale = 0.6f + (rand() % 100) / 100.0f;
        clouds[i].speed = 0.2f + clouds[i].scale * 0.4f;
        clouds[i].pos.x = (float)(rand() % screenWidth);
        clouds[i].pos.y = (float)(rand() % (screenHeight / 2));
    }

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
            clouds[i].pos.x = -160.0f * clouds[i].scale;
            clouds[i].pos.y = (float)(rand() % (screenHeight / 2));
        }
    }
}

static void drawCloud(Cloud cloud, Color c) {
    float s = cloud.scale;
    DrawCircleV((Vector2){ cloud.pos.x, cloud.pos.y + 10 * s }, 34 * s, c);
    DrawCircleV((Vector2){ cloud.pos.x + 38 * s, cloud.pos.y - 8 * s }, 44 * s, c);
    DrawCircleV((Vector2){ cloud.pos.x + 82 * s, cloud.pos.y + 6 * s }, 36 * s, c);
    DrawRectangleRounded(
        (Rectangle){ cloud.pos.x - 20 * s, cloud.pos.y + 6 * s, 124 * s, 38 * s },
        1.0f, 8, c);
}

void DrawBackground(int screenWidth, int screenHeight, Theme theme) {
    float time = (float)GetTime();
    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, theme.skyTop, theme.skyBottom);

    if (theme.night) {
        for (int i = 0; i < NUM_STARS; i++) {
            float twinkle = 0.5f + 0.5f * sinf(time * 2.0f + i * 1.7f);
            DrawRectangleV(stars[i], (Vector2){ 2 + (i % 3 == 0), 2 + (i % 3 == 0) },
                           Fade(WHITE, 0.3f + 0.7f * twinkle));
        }
    }

    Vector2 sun = { screenWidth - 150.0f, theme.sunY };
    float glow = 6.0f * sinf(time * 1.5f);
    float glowAlpha = theme.night ? 0.06f : 0.2f;
    DrawCircleV(sun, 90 + glow, Fade(theme.sun, glowAlpha));
    DrawCircleV(sun, 66 + glow / 2, Fade(theme.sun, 2 * glowAlpha));
    DrawCircleV(sun, 48, theme.sun);

    if (theme.night) {
        Color crater = { 206, 206, 186, 255 };
        DrawCircleV((Vector2){ sun.x - 14, sun.y - 10 }, 10, crater);
        DrawCircleV((Vector2){ sun.x + 16, sun.y + 12 }, 7, crater);
        DrawCircleV((Vector2){ sun.x + 6, sun.y - 22 }, 5, crater);
    }

    for (int i = 0; i < NUM_CLOUDS; i++) drawCloud(clouds[i], theme.cloud);
    for (int i = 0; i < NUM_HILLS; i++) DrawCircleV(farHills[i].center, farHills[i].radius, theme.farHill);
    for (int i = 0; i < NUM_HILLS; i++) DrawCircleV(nearHills[i].center, nearHills[i].radius, theme.nearHill);
}

void DrawThemePreview(Theme theme, Rectangle area, int screenHeight) {
    BeginScissorMode((int)area.x, (int)area.y, (int)area.width, (int)area.height);
    DrawRectangleGradientV((int)area.x, (int)area.y, (int)area.width, (int)area.height,
                            theme.skyTop, theme.skyBottom);
    if (theme.night) {
        for (int i = 0; i < 12; i++) {
            DrawRectangle((int)(area.x + (i * 37) % (int)area.width),
                           (int)(area.y + (i * 23) % (int)(area.height / 2)), 2, 2, WHITE);
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

#endif // BACKGROUND_C