// Audio layer (Chiptune synthesis)
#ifndef AUDIO_H
#define AUDIO_H

#include <raylib.h>
#include <stdbool.h>

#define NUM_TRACKS 4
#define MUSIC_SAMPLE_RATE 44100

typedef struct MusicTrack {
    const char *name;
    int price;
    Color accent;
    float bpm;
    const char *chords;
    const char *melody;
    const char *bass;
    const char *drums;
    int arpEvery;
} MusicTrack;

extern MusicTrack tracks[NUM_TRACKS];

void InitGameAudio(int track, bool enabled);
void UpdateGameAudio(int track, bool enabled);
void PlayJumpSound(void);
void CloseGameAudio(void);

#endif // AUDIO_H