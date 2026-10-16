// Purchasable gameplay abilities (double jump, shield, coin magnet, dash)
#include "abilities.h"
#include "shop.h"
#include "objects.h"
#include "input.h"
#include "menu.h"

#include <raylib.h>
#include <math.h>

#define DOUBLE_JUMP_WINDOW_FRAMES 14   // tap jump again within this many frames for the boost
#define DOUBLE_JUMP_BONUS 1.35f

#define SHIELD_DURATION_FRAMES 90      // 1.5s of invulnerability at 60 fps
#define SHIELD_COOLDOWN_FRAMES 600     // 10s before it can be used again

#define DASH_DURATION_FRAMES 45        // 0.75s boosted
#define DASH_COOLDOWN_FRAMES 420       // 7s before it can be used again
#define DASH_SPEED_BONUS 9.0f

#define MAGNET_RADIUS 150.0f
#define MAGNET_PULL 0.12f

static int lastJumpFrame = -1000;
static int shieldCooldown = 0;
static int shieldFramesLeft = 0;
static int dashCooldown = 0;
static int dashFramesLeft = 0;

void ResetAbilities(void) {
    lastJumpFrame = -1000;
    shieldCooldown = 0;
    shieldFramesLeft = 0;
    dashCooldown = 0;
    dashFramesLeft = 0;
}

void ApplyJumpAbility(void) {
    int frame = GetGamevars()->frameCounter;
    if (OwnsAbility(ABILITY_DOUBLE_JUMP) && frame - lastJumpFrame <= DOUBLE_JUMP_WINDOW_FRAMES) {
        // A quick second tap chains into a stronger boost instead of just resetting the jump
        GetChicken()->velocityY = GetChicken()->jumpForce * DOUBLE_JUMP_BONUS;
    }
    lastJumpFrame = frame;
}

void UpdateAbilities(void) {
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
