// "Twitch Plays" mode: the chicken is controlled by chat votes instead of a
// single player's input, so an unlimited number of stream viewers (chat has
// no real participant cap - channels with thousands of people chatting at
// once are common) can play together. Works identically on desktop and
// Android since it only needs a plain TCP connection to Twitch's chat
// server, no dedicated game server of our own.
#ifndef TWITCHPLAYS_H
#define TWITCHPLAYS_H

#include <stdbool.h>

// Called once at startup/shutdown, alongside InitCoop()/CloseCoop()
void InitTwitchPlays(void);
void CloseTwitchPlays(void);

// True when sockets could be set up on this platform (always true in practice;
// mirrors CoopSupported()'s pattern so the menu can hide the button otherwise)
bool TwitchPlaysSupported(void);

bool TwitchPlaysConnected(void);
// Connecting blocks the caller briefly (DNS + TCP handshake, a few seconds at
// worst with the internal timeout) - call this from the menu button handler,
// never from the gameplay loop
void TwitchPlaysConnect(const char *channel);
void TwitchPlaysDisconnect(void);

// Call once per gameplay frame while connected: reads chat, tallies votes in
// short windows and refreshes TwitchJumpTriggered()/TwitchFastFallHeld()
void UpdateTwitchPlays(void);

// True for exactly one frame when a vote window just closed with "jump" ahead
bool TwitchJumpTriggered(void);
// True while the most recently closed window favored "fall"
bool TwitchFastFallHeld(void);

// Distinct chatters who voted in the last completed window (for the UI)
int TwitchLastVoterCount(void);
// Short human-readable connection status for the UI
const char *TwitchPlaysStatus(void);

// The HOST-less "connect to a channel" screen reached from the main menu's
// TWITCH PLAY button
void UpdateTwitchPlaysMenu(int screenWidth, int screenHeight);
void DrawTwitchPlaysMenu(int screenWidth, int screenHeight);

#endif // TWITCHPLAYS_H
