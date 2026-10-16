// Background rendering
#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <raylib.h>
#include <stdbool.h>

#define NUM_THEMES 4
#define NUM_CLOUDS 7
#define NUM_HILLS 6
#define NUM_STARS 70

typedef struct {
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

typedef struct {
    Vector2 pos;
    float speed;
    float scale;
} Cloud;

typedef struct {
    Vector2 center;
    float radius;
} Hill;

extern Theme themes[NUM_THEMES];

void InitBackground(int screenWidth, int screenHeight);
void UpdateBackground(int screenWidth, int screenHeight);
void DrawBackground(int screenWidth, int screenHeight, Theme theme);
void DrawThemePreview(Theme theme, Rectangle area, int screenHeight);

#endif // BACKGROUND_H