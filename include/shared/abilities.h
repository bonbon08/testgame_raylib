// Gameplay abilities (shield, coin magnet, dash): collected as in-level pickups.
// Each pickup activates instantly and runs for a fixed duration, then it's
// gone again. Only abilities unlocked in the shop ever spawn (see shop.h).
#ifndef ABILITIES_H
#define ABILITIES_H

#include <raylib.h>
#include <stdbool.h>

#define ABILITY_SHIELD 0
#define ABILITY_MAGNET 1
#define ABILITY_DASH   2
#define NUM_ABILITIES  3

// Called once when a run starts: clears all charges and the pickup timer
void ResetAbilities(void);

// Called once per gameplay frame: checks for pickup collection and advances
// each ability's active timer
void UpdateAbilities(void);

// Pulls a coin towards the chicken while Coin Magnet is active and in range;
// returns the (possibly adjusted) coin position
Vector2 ApplyCoinMagnet(Vector2 coinPos, Vector2 chickenCenter);

// Extra speed added on top of ChickenSpeed() while Dash is active
float DashSpeedBonus(void);

// The floating pickup that grants an ability for a short while, if any is
// currently waiting to be found
void DrawAbilityPickup(void);

#endif // ABILITIES_H
