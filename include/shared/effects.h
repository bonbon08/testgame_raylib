// Particle effects and popups
#ifndef EFFECTS_H
#define EFFECTS_H

#include <raylib.h>

void SpawnParticles(Vector2 pos, int count, Color color, float speed, float size);
void SpawnPopup(Vector2 pos, const char *text, Color color);
void StartShake(float strength, int frames);
void UpdateEffects(void);
Vector2 UpdateShake(void);
void DrawEffects(void);

#endif // EFFECTS_H