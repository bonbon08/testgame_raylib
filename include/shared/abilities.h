// Gameplay abilities (shield, coin magnet, dash): collected as in-level pickups,
// then active for the rest of the run (lost again on death)
#ifndef ABILITIES_H
#define ABILITIES_H

#include <raylib.h>
#include <stdbool.h>

#define ABILITY_SHIELD 0
#define ABILITY_MAGNET 1
#define ABILITY_DASH   2
#define NUM_ABILITIES  3

// Called once when a run starts: clears owned abilities, resets charge/cooldown
// state, and spawns the first pickup
void ResetAbilities(void);

// Called once per gameplay frame: checks for pickup collection, reads input for
// the activatable abilities (shield, dash) and advances their cooldowns
void UpdateAbilities(void);

bool OwnsAbility(int ability);

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

// The floating pickup that grants an ability for the rest of the run, if any is
// still waiting to be found
void DrawAbilityPickup(void);

#endif // ABILITIES_H
