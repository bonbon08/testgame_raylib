// Local two-player co-op over Bluetooth (Android only; see MainActivity/CoopManager.java)
#ifndef COOP_H
#define COOP_H

#include <raylib.h>
#include <stdbool.h>

// Called once at startup/shutdown, alongside InitAds()/CloseAds()
void InitCoop(void);
void CloseCoop(void);

// True on a device with a Bluetooth radio (always false on desktop)
bool CoopSupported(void);

// True once the runtime Bluetooth permissions have been granted
bool CoopPermissionsGranted(void);
void CoopRequestPermissions(void);

// Starts hosting (makes the device discoverable, waits for a peer) or opens
// a picker of paired/nearby devices to join one of
void CoopStartHost(void);
void CoopStartJoin(void);

bool CoopConnected(void);
void CoopDisconnect(void);

// Call once per gameplay frame while connected: sends this device's chicken
// state to the peer (throttled internally) and refreshes the peer's last
// known state for PeerPosition()/PeerDirection()/PeerScore()
void UpdateCoop(void);

Vector2 PeerPosition(void);
int PeerDirection(void);
int PeerScore(void);

// The HOST/JOIN/DISCONNECT screen reached from the main menu's CO-OP button
void UpdateCoopMenu(int screenWidth, int screenHeight);
void DrawCoopMenu(int screenWidth, int screenHeight);

#endif // COOP_H
