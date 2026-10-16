// Purchasable gameplay abilities (double jump, shield, coin magnet, dash)
#ifndef ABILITIES_H
#define ABILITIES_H

#include <raylib.h>
#include <stdbool.h>

#define ABILITY_DOUBLE_JUMP 0
#define ABILITY_SHIELD      1
#define ABILITY_MAGNET      2
#define ABILITY_DASH        3
#define NUM_ABILITIES       4

// Called once when a run starts: resets charge/cooldown state
void ResetAbilities(void);

// Called once per gameplay frame: reads input for the activatable abilities
// (shield, dash) and advances their cooldowns
void UpdateAbilities(void);

// Called right when the jump input fires; gives Double Jump a chance to
// turn a quick second tap into a stronger boost
void ApplyJumpAbility(void);

// Pulls a coin towards the chicken when Coin Magnet is owned and in range;
// returns the (possibly adjusted) coin position
Vector2 ApplyCoinMagnet(Vector2 coinPos, Vector2 chickenCenter);

// Extra speed added on top of ChickenSpeed() while Dash is active
float DashSpeedBonus(void);

// True while Shield grants invulnerability
bool ShieldIsActive(void);

// On-screen ability buttons (desktop keys + touch rects); only drawn once owned
Rectangle ShieldButtonRect(int screenWidth, int screenHeight);
Rectangle DashButtonRect(int screenWidth, int screenHeight);
void DrawAbilityButtons(int screenWidth, int screenHeight);

#endif // ABILITIES_H
