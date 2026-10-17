// Gameplay abilities (shield, coin magnet, dash): collected as in-level pickups,
// then active for the rest of the run (lost again on death)
#include "abilities.h"
#include "objects.h"
#include "effects.h"
#include "input.h"
#include "menu.h"

#include <raylib.h>
#include <math.h>
#include <stdlib.h>

#define SHIELD_DURATION_FRAMES 90      // 1.5s of invulnerability at 60 fps
#define SHIELD_COOLDOWN_FRAMES 600     // 10s before it can be used again

#define DASH_DURATION_FRAMES 45        // 0.75s boosted
#define DASH_COOLDOWN_FRAMES 420       // 7s before it can be used again
#define DASH_SPEED_BONUS 9.0f

#define MAGNET_RADIUS 150.0f
#define MAGNET_PULL 0.12f

#define PICKUP_SIZE 48

static int shieldCooldown = 0;
static int shieldFramesLeft = 0;
static int dashCooldown = 0;
static int dashFramesLeft = 0;
static int ownedMask = 0;

static const char *abilityNames[NUM_ABILITIES] = { "SHIELD", "MAGNET", "DASH" };

typedef struct AbilityPickup {
    Vector2 pos;
    int ability;
    bool active;
} AbilityPickup;

static AbilityPickup pickup;

static Color abilityColor(int ability) {
    switch (ability) {
        case ABILITY_SHIELD: return (Color){ 90, 170, 255, 255 };
        case ABILITY_MAGNET: return (Color){ 230, 90, 90, 255 };
        default:             return (Color){ 255, 160, 60, 255 };   // ABILITY_DASH
    }
}

// Picks one of the abilities not yet owned this run and drops it somewhere on
// screen; once every ability has been found, no further pickup spawns
static void spawnAbilityPickup(void) {
    int remaining[NUM_ABILITIES];
    int count = 0;
    for (int i = 0; i < NUM_ABILITIES; i++)
        if (!((ownedMask >> i) & 1)) remaining[count++] = i;

    if (count == 0) {
        pickup.active = false;
        return;
    }

    pickup.ability = remaining[rand() % count];
    int genWidth = GetGamevars()->screenWidth - 2 * PICKUP_SIZE;
    int genHeight = GetGamevars()->screenHeight - 2 * PICKUP_SIZE;
    pickup.pos = (Vector2){
        (float)(rand() % genWidth + PICKUP_SIZE),
        (float)(rand() % genHeight + PICKUP_SIZE)
    };
    pickup.active = true;
}

void ResetAbilities(void) {
    shieldCooldown = 0;
    shieldFramesLeft = 0;
    dashCooldown = 0;
    dashFramesLeft = 0;
    ownedMask = 0;
    spawnAbilityPickup();
}

bool OwnsAbility(int ability) {
    return (ownedMask >> ability) & 1;
}

static void collectAbilityPickup(void) {
    if (!pickup.active) return;

    Rectangle pickupRect = { pickup.pos.x, pickup.pos.y, PICKUP_SIZE, PICKUP_SIZE };
    Rectangle chickenRect = { GetChicken()->pos.x, GetChicken()->pos.y, CHICKEN_SIZE, CHICKEN_SIZE };
    if (!CheckCollisionRecs(chickenRect, pickupRect)) return;

    ownedMask |= 1 << pickup.ability;
    SpawnParticles((Vector2){ pickup.pos.x + PICKUP_SIZE / 2.0f, pickup.pos.y + PICKUP_SIZE / 2.0f },
                  20, abilityColor(pickup.ability), 5.0f, 6.0f);
    SpawnPopup((Vector2){ pickup.pos.x + PICKUP_SIZE / 2.0f, pickup.pos.y - 10 },
              abilityNames[pickup.ability], abilityColor(pickup.ability));
    spawnAbilityPickup();
}

void UpdateAbilities(void) {
    collectAbilityPickup();

    if (shieldCooldown > 0) shieldCooldown--;
    if (shieldFramesLeft > 0) shieldFramesLeft--;
    if (dashCooldown > 0) dashCooldown--;
    if (dashFramesLeft > 0) dashFramesLeft--;

    int screenWidth = GetGamevars()->screenWidth;
    int screenHeight = GetGamevars()->screenHeight;

    if (OwnsAbility(ABILITY_SHIELD) && shieldCooldown == 0 &&
        (IsKeyPressed(KEY_C) || PointerPressedIn(ShieldButtonRect(screenWidth, screenHeight)))) {
        shieldFramesLeft = SHIELD_DURATION_FRAMES;
        shieldCooldown = SHIELD_COOLDOWN_FRAMES;
        if (GetChicken()->invulnerable < SHIELD_DURATION_FRAMES)
            GetChicken()->invulnerable = SHIELD_DURATION_FRAMES;
    }

    if (OwnsAbility(ABILITY_DASH) && dashCooldown == 0 &&
        (IsKeyPressed(KEY_V) || PointerPressedIn(DashButtonRect(screenWidth, screenHeight)))) {
        dashFramesLeft = DASH_DURATION_FRAMES;
        dashCooldown = DASH_COOLDOWN_FRAMES;
    }
}

Vector2 ApplyCoinMagnet(Vector2 coinPos, Vector2 chickenCenter) {
    if (!OwnsAbility(ABILITY_MAGNET)) return coinPos;

    float dx = chickenCenter.x - coinPos.x;
    float dy = chickenCenter.y - coinPos.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist > MAGNET_RADIUS || dist < 1.0f) return coinPos;

    return (Vector2){ coinPos.x + dx * MAGNET_PULL, coinPos.y + dy * MAGNET_PULL };
}

float DashSpeedBonus(void) {
    return dashFramesLeft > 0 ? DASH_SPEED_BONUS : 0.0f;
}

bool ShieldIsActive(void) {
    return shieldFramesLeft > 0;
}

Rectangle ShieldButtonRect(int screenWidth, int screenHeight) {
    return (Rectangle){ screenWidth - 170.0f, screenHeight - 170.0f, 64, 64 };
}

Rectangle DashButtonRect(int screenWidth, int screenHeight) {
    return (Rectangle){ screenWidth - 90.0f, screenHeight - 170.0f, 64, 64 };
}

static void drawAbilityButton(Rectangle rect, const char *label, Color color,
                              int cooldown, int maxCooldown, bool active) {
    float fraction = maxCooldown > 0 ? (float)cooldown / maxCooldown : 0.0f;
    DrawRectangleRounded(rect, 0.3f, 8, Fade(BLACK, 0.45f));
    DrawRectangleRounded(rect, 0.3f, 8, Fade(color, active ? 0.9f : (fraction > 0.0f ? 0.25f : 0.7f)));
    DrawRectangleRoundedLinesEx(rect, 0.3f, 8, 2.0f, Fade(WHITE, 0.8f));
    drawTextCentered(label, (int)(rect.x + rect.width / 2), (int)(rect.y + rect.height / 2 - 10), 18, WHITE);
    if (fraction > 0.0f) {
        int secondsLeft = cooldown / 60 + 1;
        drawTextCentered(TextFormat("%d", secondsLeft),
                         (int)(rect.x + rect.width / 2), (int)(rect.y + rect.height / 2 + 10), 16,
                         Fade(WHITE, 0.85f));
    }
}

void DrawAbilityButtons(int screenWidth, int screenHeight) {
    if (OwnsAbility(ABILITY_SHIELD)) {
        drawAbilityButton(ShieldButtonRect(screenWidth, screenHeight), "SHLD",
                          (Color){ 90, 170, 255, 255 }, shieldCooldown, SHIELD_COOLDOWN_FRAMES,
                          ShieldIsActive());
    }
    if (OwnsAbility(ABILITY_DASH)) {
        drawAbilityButton(DashButtonRect(screenWidth, screenHeight), "DASH",
                          (Color){ 255, 160, 60, 255 }, dashCooldown, DASH_COOLDOWN_FRAMES,
                          dashFramesLeft > 0);
    }
}

// Simple glyphs for the pickup (no spare art budget for new sprites)
static void drawAbilityGlyph(int ability, Vector2 center) {
    switch (ability) {
        case ABILITY_SHIELD:
            DrawPoly(center, 6, 20, 0, abilityColor(ABILITY_SHIELD));
            DrawPolyLinesEx(center, 6, 20, 0, 3.0f, WHITE);
            break;
        case ABILITY_MAGNET:
            DrawRing(center, 9, 18, 200, 340, 16, abilityColor(ABILITY_MAGNET));
            DrawRing(center, 9, 18, 20, 160, 16, (Color){ 230, 230, 230, 255 });
            break;
        default:   // ABILITY_DASH
            for (int i = 0; i < 3; i++) {
                DrawTriangle((Vector2){ center.x - 16 + i * 11, center.y - 12 },
                            (Vector2){ center.x - 16 + i * 11, center.y + 12 },
                            (Vector2){ center.x - 5 + i * 11, center.y }, abilityColor(ABILITY_DASH));
            }
            break;
    }
}

// Floats and glows gently so it reads as a collectible rather than part of the scenery
void DrawAbilityPickup(void) {
    if (!pickup.active) return;

    float time = (float)GetTime();
    float bob = 6.0f * sinf(time * 4.0f);
    Vector2 center = { pickup.pos.x + PICKUP_SIZE / 2.0f, pickup.pos.y + PICKUP_SIZE / 2.0f + bob };

    float glowPulse = 0.5f + 0.3f * sinf(time * 5.0f);
    DrawCircleV(center, PICKUP_SIZE / 2.0f + 6, Fade(abilityColor(pickup.ability), glowPulse * 0.35f));
    DrawCircleV(center, PICKUP_SIZE / 2.0f, Fade(BLACK, 0.45f));
    drawAbilityGlyph(pickup.ability, center);
    drawTextCentered(abilityNames[pickup.ability], (int)center.x, (int)(center.y + PICKUP_SIZE / 2.0f + 6),
                     14, Fade(WHITE, 0.85f));
}
