// Chiptune music synthesized at runtime (no music files needed) and the jump sound effect.
#include "audio.h"

#include <raylib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define STEPS_PER_BAR 16
#define MAX_BARS 8
#define MAX_STEPS (MAX_BARS * STEPS_PER_BAR)
#define NOTE_HOLD -1
#define NOTE_REST 0
// When the game loop stops (app in background, ad on screen) the music fades out after this time
#define MUSIC_PAUSE_SECONDS 0.3f

MusicTrack tracks[NUM_TRACKS] = {
    { "Sunny Hop", 0, { 255, 200, 60, 255 }, 132.0f, "C G Am F C G F G",
      "E5 . G5 . C6 . G5 . E5 . G5 . A5 . G5 . | D5 . G5 . B5 . G5 . D5 . . . B4 . D5 . | "
      "C5 . E5 . A5 . E5 . C5 . E5 . G5 . E5 . | F5 . . . A5 . . . C6 . A5 . F5 . . . | "
      "G5 . E5 . G5 . C6 . D6 . C6 . B5 . G5 . | B5 . . . G5 . D5 . G5 . B5 . D6 . . . | "
      "C6 . A5 . F5 . A5 . C6 . D6 . C6 . A5 . | G5 . . . . . . . D5 . E5 . F5 . G5 .",
      "r.r.o.r.r.r.o.r.", "K.h.S.h.K.K.S.h.", 2 },
    { "Night Owl", 35, { 120, 140, 255, 255 }, 96.0f, "Am F C G Am Dm E Am",
      "A4 . . . C5 . . . E5 . . . D5 . C5 . | C5 . . . . . . . A4 . . . F4 . . . | "
      "G4 . . . C5 . . . E5 . . . G5 . . . | D5 . . . . . . . B4 . . . - . . . | "
      "E5 . . . A5 . . . G5 . . . E5 . . . | F5 . . . E5 . D5 . A4 . . . D5 . . . | "
      "E5 . . . G#5 . . . B5 . . . G#5 . . . | A5 . . . . . . . - . . . E5 . C5 .",
      "r.......f.......", "k...h...s...h...", 4 },
    { "Sunset Drive", 50, { 255, 120, 90, 255 }, 112.0f, "Dm Bb F C Dm Bb Gm C",
      "D5 . . . F5 . A5 . . . G5 . F5 . E5 . | D5 . . . . . . . - . F5 . D5 . Bb4 . | "
      "C5 . . . F5 . . . A5 . . . C6 . . . | G5 . . . . . E5 . . . C5 . . . . . | "
      "A5 . . . G5 . F5 . . . E5 . F5 . . . | D5 . . . . . F5 . . . Bb5 . A5 . . . | "
      "G5 . . . Bb5 . . . D6 . . . C6 . Bb5 . | C6 . . . . . . . E5 . . . G5 . . .",
      "r.r.r.r.r.r.r.o.", "K.h.S.h.K.h.S.hh", 1 },
    { "Candy Rush", 90, { 255, 130, 200, 255 }, 160.0f, "G D Em C G D C D",
      "G5 B5 D6 B5 G5 B5 D6 B5 G5 . A5 . B5 . D6 . | F#5 A5 D6 A5 F#5 A5 D6 A5 F#5 . E5 . D5 . . . | "
      "E5 G5 B5 G5 E5 G5 B5 G5 E6 . D6 . B5 . G5 . | C6 . . . B5 . . . A5 . . . G5 . E5 . | "
      "D6 . B5 . G5 . B5 . D6 . E6 . D6 . B5 . | A5 . F#5 . D5 . F#5 . A5 . D6 . F#6 . . . | "
      "E6 . . . D6 . C6 . B5 . A5 . G5 . E5 . | F#5 . . . A5 . . . D6 . . . F#6 . . .",
      "r.o.r.o.r.o.r.o.", "KhhhShhhKhKhShhh", 1 },
};

typedef struct ParsedTrack {
    int bars;
    int chordRoot[MAX_BARS];    // semitone 0..11
    bool chordMinor[MAX_BARS];
    signed char melody[MAX_STEPS];
} ParsedTrack;

typedef struct Voice {
    float phase;
    float freq;
    float env;
    int age;      // samples since the note started (for a short attack)
    bool gate;
} Voice;

typedef struct Synth {
    int track;
    int step;
    float stepPos;
    float gain;
    Voice lead, arp, bass;
    float kickTime, snareTime, hatTime;   // seconds since the drum hit, < 0 when silent
    float kickPhase;
    float hatPrev;
    float lowpass;
    unsigned int noise;
    unsigned int lastHeartbeat;
    float silentTime;
} Synth;

static ParsedTrack parsedTracks[NUM_TRACKS];
static Synth synth;
static AudioStream musicStream;
static Sound jumpSound;

// Written by the game thread, read by the audio thread
static volatile int requestedTrack = 0;
static volatile bool musicEnabled = true;
static volatile unsigned int musicHeartbeat = 0;

static int noteSemitone(char letter) {
    const int semitones[7] = { 9, 11, 0, 2, 4, 5, 7 };   // A B C D E F G
    return semitones[letter - 'A'];
}

// "C5" -> MIDI note number
static int parseNote(const char *token) {
    int semitone = noteSemitone(token[0]);
    int i = 1;
    if (token[i] == '#') { semitone++; i++; }
    else if (token[i] == 'b') { semitone--; i++; }
    int octave = atoi(token + i);
    return 12 * (octave + 1) + semitone;
}

static void parseTrack(const MusicTrack *track, ParsedTrack *parsed) {
    char buffer[1024];

    snprintf(buffer, sizeof(buffer), "%s", track->chords);
    parsed->bars = 0;
    for (char *chord = strtok(buffer, " "); chord != NULL && parsed->bars < MAX_BARS; chord = strtok(NULL, " ")) {
        int root = noteSemitone(chord[0]);
        int i = 1;
        if (chord[i] == '#') { root++; i++; }
        else if (chord[i] == 'b') { root--; i++; }
        parsed->chordRoot[parsed->bars] = (root + 12) % 12;
        parsed->chordMinor[parsed->bars] = chord[i] == 'm';
        parsed->bars++;
    }

    snprintf(buffer, sizeof(buffer), "%s", track->melody);
    int step = 0;
    for (char *token = strtok(buffer, " "); token != NULL; token = strtok(NULL, " ")) {
        if (token[0] == '|') continue;
        if (step >= MAX_STEPS) break;
        if (token[0] == '.') parsed->melody[step] = NOTE_HOLD;
        else if (token[0] == '-') parsed->melody[step] = NOTE_REST;
        else parsed->melody[step] = (signed char)parseNote(token);
        step++;
    }
    if (step != parsed->bars * STEPS_PER_BAR) {
        TraceLog(LOG_WARNING, "MUSIC: '%s' has %d melody steps, expected %d", track->name, step, parsed->bars * STEPS_PER_BAR);
    }
}

static float midiFrequency(int note) {
    return 440.0f * powf(2.0f, (note - 69) / 12.0f);
}

static void startVoice(Voice *voice, int note) {
    voice->freq = midiFrequency(note);
    voice->env = 1.0f;
    voice->age = 0;
    voice->gate = true;
}

static void triggerStep(void) {
    const MusicTrack *track = &tracks[synth.track];
    const ParsedTrack *parsed = &parsedTracks[synth.track];
    int bar = synth.step / STEPS_PER_BAR;
    int stepInBar = synth.step % STEPS_PER_BAR;
    int root = parsed->chordRoot[bar];
    int third = parsed->chordMinor[bar] ? 3 : 4;

    int note = parsed->melody[synth.step];
    if (note > 0) startVoice(&synth.lead, note);
    else if (note == NOTE_REST) synth.lead.gate = false;

    char bassStep = track->bass[stepInBar];
    int bassRoot = 36 + root;   // octave 2
    if (bassStep == 'r') startVoice(&synth.bass, bassRoot);
    else if (bassStep == 'o') startVoice(&synth.bass, bassRoot + 12);
    else if (bassStep == 'f') startVoice(&synth.bass, bassRoot + 7);
    else if (bassStep == '-') synth.bass.gate = false;

    if (track->arpEvery > 0 && stepInBar % track->arpEvery == 0) {
        const int chordTones[4] = { 0, third, 7, 12 };
        startVoice(&synth.arp, 60 + root + chordTones[(stepInBar / track->arpEvery) % 4]);
    }

    char drum = track->drums[stepInBar];
    if (drum == 'k' || drum == 'K') { synth.kickTime = 0; synth.kickPhase = 0; }
    if (drum == 's' || drum == 'S') synth.snareTime = 0;
    if (drum == 'h' || drum == 'K' || drum == 'S') synth.hatTime = 0;
}

static void resetSynth(int track) {
    synth.track = track;
    synth.step = 0;
    synth.stepPos = 0;
    synth.lead.gate = synth.bass.gate = synth.arp.gate = false;
    synth.lead.env = synth.bass.env = synth.arp.env = 0;
    synth.kickTime = synth.snareTime = synth.hatTime = -1;
    triggerStep();
}

static float noiseSample(void) {
    synth.noise ^= synth.noise << 13;
    synth.noise ^= synth.noise >> 17;
    synth.noise ^= synth.noise << 5;
    return (synth.noise / 4294967295.0f) * 2.0f - 1.0f;
}

static float squareWave(float phase, float duty) { return phase < duty ? 1.0f : -1.0f; }
static float triangleWave(float phase) { return 4.0f * fabsf(phase - 0.5f) - 1.0f; }

static float advance(Voice *voice) {
    voice->phase += voice->freq / MUSIC_SAMPLE_RATE;
    if (voice->phase >= 1.0f) voice->phase -= 1.0f;
    voice->age++;
    float attack = voice->age < 132 ? voice->age / 132.0f : 1.0f;   // 3 ms, avoids clicks
    return attack * voice->env;
}

static float drumSample(float *time, float decay) {
    if (*time < 0) return 0;
    float amplitude = expf(-*time * decay);
    *time += 1.0f / MUSIC_SAMPLE_RATE;
    if (amplitude < 0.001f) *time = -1;
    return amplitude;
}

static float nextSample(void) {
    float stepSamples = MUSIC_SAMPLE_RATE * 60.0f / tracks[synth.track].bpm / 4.0f;
    synth.stepPos += 1.0f;
    if (synth.stepPos >= stepSamples) {
        synth.stepPos -= stepSamples;
        synth.step = (synth.step + 1) % (parsedTracks[synth.track].bars * STEPS_PER_BAR);
        triggerStep();
    }

    // Envelopes: lead and bass sustain while the note is held, the arpeggio is plucked
    synth.lead.env = synth.lead.gate ? synth.lead.env + (0.6f - synth.lead.env) * 0.0004f : synth.lead.env * 0.9985f;
    synth.bass.env = synth.bass.gate ? synth.bass.env + (0.8f - synth.bass.env) * 0.0003f : synth.bass.env * 0.998f;
    synth.arp.env *= 0.9993f;

    float lead = squareWave(synth.lead.phase, 0.25f) * advance(&synth.lead);
    float arp = squareWave(synth.arp.phase, 0.125f) * advance(&synth.arp);
    float bass = triangleWave(synth.bass.phase) * advance(&synth.bass);

    float drums = 0;
    float kick = drumSample(&synth.kickTime, 18.0f);
    if (kick > 0) {
        float t = synth.kickTime;
        synth.kickPhase += (45.0f + 115.0f * expf(-t * 30.0f)) / MUSIC_SAMPLE_RATE;
        drums += sinf(2 * PI * synth.kickPhase) * kick * 0.55f;
    }
    float snare = drumSample(&synth.snareTime, 28.0f);
    if (snare > 0) drums += noiseSample() * snare * 0.22f + sinf(2 * PI * 185.0f * synth.snareTime) * snare * 0.12f;
    float hat = drumSample(&synth.hatTime, 90.0f);
    if (hat > 0) {
        float n = noiseSample();
        drums += (n - synth.hatPrev) * hat * 0.06f;
        synth.hatPrev = n;
    }

    float mix = lead * 0.13f + arp * 0.045f + bass * 0.2f + drums;
    synth.lowpass += (mix - synth.lowpass) * 0.45f;   // takes the harsh edge off the square waves
    float x = synth.lowpass;
    return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);   // soft clipping
}

// Runs on the audio thread
static void musicCallback(void *buffer, unsigned int frames) {
    short *out = (short *)buffer;

    unsigned int heartbeat = musicHeartbeat;
    if (heartbeat != synth.lastHeartbeat) {
        synth.lastHeartbeat = heartbeat;
        synth.silentTime = 0;
    } else {
        synth.silentTime += frames / (float)MUSIC_SAMPLE_RATE;
    }
    bool paused = synth.silentTime > MUSIC_PAUSE_SECONDS;

    for (unsigned int i = 0; i < frames; i++) {
        bool switching = requestedTrack != synth.track;
        float target = (musicEnabled && !paused && !switching) ? 1.0f : 0.0f;
        if (synth.gain < target) synth.gain = fminf(target, synth.gain + 0.0005f);
        else if (synth.gain > target) synth.gain = fmaxf(target, synth.gain - 0.0005f);

        if (synth.gain <= 0.0f) {
            if (switching) resetSynth(requestedTrack);
            out[i] = 0;
            continue;
        }
        out[i] = (short)(nextSample() * synth.gain * 0.8f * 32767.0f);
    }
}

// A short rising "bwip"
static Sound createJumpSound(void) {
    int frames = (int)(MUSIC_SAMPLE_RATE * 0.12f);
    short *samples = (short *)MemAlloc(frames * sizeof(short));
    float phase = 0;
    for (int i = 0; i < frames; i++) {
        float t = i / (float)MUSIC_SAMPLE_RATE;
        phase += (380.0f + 7000.0f * t) / MUSIC_SAMPLE_RATE;
        float attack = i < 100 ? i / 100.0f : 1.0f;
        samples[i] = (short)(squareWave(fmodf(phase, 1.0f), 0.5f) * expf(-t * 22.0f) * attack * 0.3f * 32767.0f);
    }
    Wave wave = { (unsigned int)frames, MUSIC_SAMPLE_RATE, 16, 1, samples };
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

void InitGameAudio(int track, bool enabled) {
    for (int i = 0; i < NUM_TRACKS; i++) parseTrack(&tracks[i], &parsedTracks[i]);
    synth.noise = 0x12345678u;
    requestedTrack = track;
    musicEnabled = enabled;
    resetSynth(track);

    musicStream = LoadAudioStream(MUSIC_SAMPLE_RATE, 16, 1);
    SetAudioStreamCallback(musicStream, musicCallback);
    PlayAudioStream(musicStream);

    jumpSound = createJumpSound();
    SetSoundVolume(jumpSound, 0.6f);
}

// Call once per frame: picks the track and tells the audio thread the game is still running
void UpdateGameAudio(int track, bool enabled) {
    requestedTrack = track;
    musicEnabled = enabled;
    musicHeartbeat++;
}

void PlayJumpSound(void) {
    PlaySound(jumpSound);
}

void CloseGameAudio(void) {
    StopAudioStream(musicStream);
    UnloadAudioStream(musicStream);
    UnloadSound(jumpSound);
}
