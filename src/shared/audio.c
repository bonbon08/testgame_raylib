// Chiptune audio and sound effects
#include "audio.h"
#include <raylib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MUSIC_SAMPLE_RATE 44100
#define NUM_TRACKS 4
#define STEPS_PER_BAR 16
#define MAX_BARS 8
#define MAX_STEPS (MAX_BARS * STEPS_PER_BAR)
#define NOTE_HOLD -1
#define NOTE_REST 0
#define MUSIC_PAUSE_SECONDS 0.3f

typedef struct MusicTrack {
    const char *name;
    int price;
    Color accent;
    float bpm;
    const char *chords;   // one chord per bar, e.g. "C G Am F"
    const char *melody;   // 16 steps per bar: note (C5, F#4, Bb3), '.' hold, '-' rest; '|' is ignored
    const char *bass;     // 16 steps, repeated every bar: r root, o octave, f fifth, '.' hold, '-' rest
    const char *drums;    // 16 steps, repeated every bar: k kick, s snare, h hat, K kick+hat, S snare+hat
    int arpEvery;         // play a chord tone every n steps
} MusicTrack;

// Full track definitions (simplified for demonstration)
// For full song data, see original audio.c

static MusicTrack tracks[NUM_TRACKS];

static void PlayJumpSound(void) {
    Sound jumpSound = LoadSound("data/coin.wav");
    PlaySound(jumpSound);
}

void InitializeAudio(void) {
    tracks[0] = (MusicTrack){ .name = "Sunny Hop", .price = 0, .accent = (Color){ 255, 200, 60, 255 }, .bpm = 132.0f };
    tracks[1] = (MusicTrack){ .name = "Night Owl", .price = 35, .accent = (Color){ 120, 140, 255, 255 }, .bpm = 96.0f };
    tracks[2] = (MusicTrack){ .name = "Sunset Drive", .price = 50, .accent = (Color){ 255, 120, 90, 255 }, .bpm = 112.0f };
    tracks[3] = (MusicTrack){ .name = "Candy Rush", .price = 90, .accent = (Color){ 255, 130, 200, 255 }, .bpm = 160.0f };
    InitAudioDevice();
    SetAudioStreamCallback(LoadAudioStream(MUSIC_SAMPLE_RATE, 16, 1), NULL);
}

void StopAudio(void) {
    CloseAudioDevice();
}

#endif  // AUDIO_C