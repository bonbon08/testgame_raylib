// Gameplay abilities (shield, coin magnet, dash): collected as in-level pickups.
// Each pickup activates instantly and runs for a fixed duration, then it's
// gone again - no permanent upgrade, no button to press. Only abilities
// unlocked in the shop (shop.c, ROW_ABILITIES) ever spawn as pickups.
#include "abilities.h"
#include "objects.h"
#include "effects.h"
#include "shop.h"
#include "menu.h"

#include <raylib.h>
#include <math.h>
#include <stdlib.h>

#define SHIELD_DURATION_FRAMES 600     // 10s of invulnerability at 60 fps

#define DASH_DURATION_FRAMES 30        // 0.5s brief speed burst
#define DASH_GRACE_FRAMES 180          // 3s invulnerable right after, so the
                                        // speed boost doesn't dump you into a spike
#define DASH_SPEED_BONUS 9.0f

#define MAGNET_DURATION_FRAMES 1200    // 20s, starts the moment it's picked up
#define MAGNET_RADIUS 150.0f
#define MAGNET_PULL 0.12f

#define PICKUP_SIZE 48
#define PICKUP_RESPAWN_FRAMES (25 * 60)   // pickups are rare: ~25s between them

static int shieldFramesLeft = 0;
static int dashFramesLeft = 0;
static int magnetFramesLeft = 0;
static int pickupCooldown = 0;   // frames until the next pickup is allowed to spawn

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

// Drops a random *unlocked* ability pickup somewhere on screen. If nothing
// has been unlocked in the shop yet, nothing spawns at all.
static void spawnAbilityPickup(void) {
    int unlocked[NUM_ABILITIES];
    int count = 0;
    for (int i = 0; i < NUM_ABILITIES; i++)
        if (AbilityUnlocked(i)) unlocked[count++] = i;

    if (count == 0) {
        pickup.active = false;
        return;
    }

    pickup.ability = unlocked[rand() % count];
    int genWidth = GetGamevars()->screenWidth - 2 * PICKUP_SIZE;
    int genHeight = GetGamevars()->screenHeight - 2 * PICKUP_SIZE;
    pickup.pos = (Vector2){
        (float)(rand() % genWidth + PICKUP_SIZE),
        (float)(rand() % genHeight + PICKUP_SIZE)
    };
    pickup.active = true;
}

void ResetAbilities(void) {
    shieldFramesLeft = 0;
    dashFramesLeft = 0;
    magnetFramesLeft = 0;
    pickup.active = false;
    pickupCooldown = PICKUP_RESPAWN_FRAMES;
}

static void collectAbilityPickup(void) {
    if (!pickup.active) return;

    Rectangle pickupRect = { pickup.pos.x, pickup.pos.y, PICKUP_SIZE, PICKUP_SIZE };
    Rectangle chickenRect = { GetChicken()->pos.x, GetChicken()->pos.y, CHICKEN_SIZE, CHICKEN_SIZE };
    if (!CheckCollisionRecs(chickenRect, pickupRect)) return;

    switch (pickup.ability) {
        case ABILITY_SHIELD:
            shieldFramesLeft = SHIELD_DURATION_FRAMES;
            if (GetChicken()->invulnerable < SHIELD_DURATION_FRAMES)
                GetChicken()->invulnerable = SHIELD_DURATION_FRAMES;
            break;
        case ABILITY_DASH:
            dashFramesLeft = DASH_DURATION_FRAMES;
            break;
        default:   // ABILITY_MAGNET
            magnetFramesLeft = MAGNET_DURATION_FRAMES;
            break;
    }
    SpawnParticles((Vector2){ pickup.pos.x + PICKUP_SIZE / 2.0f, pickup.pos.y + PICKUP_SIZE / 2.0f },
                  20, abilityColor(pickup.ability), 5.0f, 6.0f);
    SpawnPopup((Vector2){ pickup.pos.x + PICKUP_SIZE / 2.0f, pickup.pos.y - 10 },
              abilityNames[pickup.ability], abilityColor(pickup.ability));
    pickup.active = false;
    pickupCooldown = PICKUP_RESPAWN_FRAMES;
}

void UpdateAbilities(void) {
    collectAbilityPickup();

    if (!pickup.active) {
        if (pickupCooldown > 0) pickupCooldown--;
        else spawnAbilityPickup();
    }

    if (shieldFramesLeft > 0) shieldFramesLeft--;

    if (dashFramesLeft > 0) {
        dashFramesLeft--;
        if (dashFramesLeft == 0 && GetChicken()->invulnerable < DASH_GRACE_FRAMES)
            GetChicken()->invulnerable = DASH_GRACE_FRAMES;
    }

    if (magnetFramesLeft > 0) magnetFramesLeft--;
}

Vector2 ApplyCoinMagnet(Vector2 coinPos, Vector2 chickenCenter) {
    if (magnetFramesLeft <= 0) return coinPos;

    float dx = chickenCenter.x - coinPos.x;
    float dy = chickenCenter.y - coinPos.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist > MAGNET_RADIUS || dist < 1.0f) return coinPos;

    return (Vector2){ coinPos.x + dx * MAGNET_PULL, coinPos.y + dy * MAGNET_PULL };
}

float DashSpeedBonus(void) {
    return dashFramesLeft > 0 ? DASH_SPEED_BONUS : 0.0f;
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
