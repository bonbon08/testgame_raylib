// Twitch Plays mode (see twitchplays.h for the idea). This talks to Twitch's
// IRC-based chat server directly over a plain TCP socket - no API key needed,
// chat is read anonymously. Twitch IRC reference: https://dev.twitch.tv/docs/irc/
#include "twitchplays.h"
#include "objects.h"
#include "menu.h"
#include "input.h"

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET SocketHandle;
    #define SOCKET_INVALID INVALID_SOCKET
    #define CLOSESOCKET(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    typedef int SocketHandle;
    #define SOCKET_INVALID (-1)
    #define CLOSESOCKET(s) close(s)
#endif

#define TWITCH_IRC_HOST "irc.chat.twitch.tv"
#define TWITCH_IRC_PORT "6667"
#define CHANNEL_NAME_MAX 25          // Twitch login names are at most 25 characters
#define LINE_BUFFER_SIZE 1024
#define RECV_CHUNK_SIZE 2048
#define VOTE_WINDOW_FRAMES 30        // half a second at 60 fps: snappy, but still lets chat catch up
#define MAX_VOTERS_PER_WINDOW 2048   // as many distinct chatters per window as viewers who'd realistically vote at once

typedef enum VoteAction { VOTE_NONE = 0, VOTE_JUMP, VOTE_FALL } VoteAction;

static SocketHandle sock = SOCKET_INVALID;
static bool connected = false;
static bool wsaReady = false;
static char statusText[96] = "Not connected";
static char channelEdit[CHANNEL_NAME_MAX + 1] = "";

static char lineBuffer[LINE_BUFFER_SIZE];
static int lineLength = 0;

static char voters[MAX_VOTERS_PER_WINDOW][CHANNEL_NAME_MAX + 1];
static int voterCount = 0;
static int jumpVotes = 0;
static int fallVotes = 0;
static int windowFramesLeft = VOTE_WINDOW_FRAMES;

static int lastVoterCount = 0;
static bool jumpTriggeredThisFrame = false;
static bool fallHeldState = false;

// ---- Socket plumbing ----

static void setBlockingTimeout(SocketHandle s, int seconds) {
#if defined(_WIN32)
    DWORD timeout = (DWORD)seconds * 1000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
#else
    struct timeval tv = { seconds, 0 };
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
}

static void setNonBlocking(SocketHandle s) {
#if defined(_WIN32)
    u_long mode = 1;
    ioctlsocket(s, FIONBIO, &mode);
#else
    int flags = fcntl(s, F_GETFL, 0);
    fcntl(s, F_SETFL, flags | O_NONBLOCK);
#endif
}

static bool sendAll(SocketHandle s, const char *data) {
    size_t len = strlen(data);
    size_t sent = 0;
    while (sent < len) {
        int n = send(s, data + sent, (int)(len - sent), 0);
        if (n <= 0) return false;
        sent += (size_t)n;
    }
    return true;
}

void InitTwitchPlays(void) {
#if defined(_WIN32)
    WSADATA wsaData;
    wsaReady = WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
    if (!wsaReady) TraceLog(LOG_WARNING, "TWITCH: WSAStartup failed");
#else
    wsaReady = true;
#endif
}

void CloseTwitchPlays(void) {
    TwitchPlaysDisconnect();
#if defined(_WIN32)
    if (wsaReady) WSACleanup();
#endif
    wsaReady = false;
}

bool TwitchPlaysSupported(void) { return wsaReady; }
bool TwitchPlaysConnected(void) { return connected; }
const char *TwitchPlaysStatus(void) { return statusText; }

static void resetVoteWindow(void) {
    voterCount = 0;
    jumpVotes = 0;
    fallVotes = 0;
    windowFramesLeft = VOTE_WINDOW_FRAMES;
}

// Blocking by design (DNS + TCP handshake capped at a few seconds); only ever
// called from the menu button handler, never from the gameplay loop
void TwitchPlaysConnect(const char *channel) {
    TwitchPlaysDisconnect();
    if (!wsaReady || channel == NULL || channel[0] == '\0') return;

    while (*channel == '#') channel++;   // people paste the channel with or without the '#'

    struct addrinfo hints = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *addr = NULL;
    if (getaddrinfo(TWITCH_IRC_HOST, TWITCH_IRC_PORT, &hints, &addr) != 0 || addr == NULL) {
        snprintf(statusText, sizeof(statusText), "Could not resolve %s", TWITCH_IRC_HOST);
        return;
    }

    sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (sock == SOCKET_INVALID) {
        snprintf(statusText, sizeof(statusText), "Could not open a socket");
        freeaddrinfo(addr);
        return;
    }

    setBlockingTimeout(sock, 5);
    if (connect(sock, addr->ai_addr, (int)addr->ai_addrlen) != 0) {
        snprintf(statusText, sizeof(statusText), "Could not reach Twitch chat");
        CLOSESOCKET(sock);
        sock = SOCKET_INVALID;
        freeaddrinfo(addr);
        return;
    }
    freeaddrinfo(addr);

    // Anonymous read-only login: any nick starting with "justinfan" works with a dummy password
    char login[96];
    snprintf(login, sizeof(login), "PASS SCHMOOPIIE\r\nNICK justinfan%d\r\n", 10000 + rand() % 89999);
    char join[64];
    snprintf(join, sizeof(join), "JOIN #%s\r\n", channel);

    if (!sendAll(sock, login) || !sendAll(sock, join)) {
        snprintf(statusText, sizeof(statusText), "Connection dropped while joining");
        CLOSESOCKET(sock);
        sock = SOCKET_INVALID;
        return;
    }

    setNonBlocking(sock);
    lineLength = 0;
    resetVoteWindow();
    jumpTriggeredThisFrame = false;
    fallHeldState = false;
    connected = true;
    snprintf(statusText, sizeof(statusText), "Connected to #%s", channel);
}

void TwitchPlaysDisconnect(void) {
    if (sock != SOCKET_INVALID) {
        CLOSESOCKET(sock);
        sock = SOCKET_INVALID;
    }
    if (connected) snprintf(statusText, sizeof(statusText), "Disconnected");
    connected = false;
    jumpTriggeredThisFrame = false;
    fallHeldState = false;
}

// ---- Chat parsing ----

static VoteAction parseCommand(const char *msg) {
    // Trim leading whitespace, then compare case-insensitively against the
    // handful of words chat actually needs to know
    while (*msg == ' ') msg++;
    if (msg[0] == '!') msg++;

    char word[16];
    size_t i = 0;
    while (msg[i] != '\0' && !isspace((unsigned char)msg[i]) && i < sizeof(word) - 1) {
        word[i] = (char)tolower((unsigned char)msg[i]);
        i++;
    }
    word[i] = '\0';

    if (strcmp(word, "jump") == 0 || strcmp(word, "up") == 0) return VOTE_JUMP;
    if (strcmp(word, "fall") == 0 || strcmp(word, "down") == 0 || strcmp(word, "drop") == 0) return VOTE_FALL;
    return VOTE_NONE;
}

static bool alreadyVoted(const char *user) {
    for (int i = 0; i < voterCount; i++)
        if (strcmp(voters[i], user) == 0) return true;
    return false;
}

static void castVote(const char *user, VoteAction action) {
    if (action == VOTE_NONE || voterCount >= MAX_VOTERS_PER_WINDOW) return;
    if (alreadyVoted(user)) return;   // one vote per chatter per window

    strncpy(voters[voterCount], user, CHANNEL_NAME_MAX);
    voters[voterCount][CHANNEL_NAME_MAX] = '\0';
    voterCount++;
    if (action == VOTE_JUMP) jumpVotes++;
    else fallVotes++;
}

// A line looks like ":nick!nick@nick.tmi.twitch.tv PRIVMSG #channel :message text"
static void handlePrivmsg(const char *line) {
    if (line[0] != ':') return;
    const char *bang = strchr(line, '!');
    if (bang == NULL) return;

    char user[CHANNEL_NAME_MAX + 1];
    size_t nameLen = (size_t)(bang - (line + 1));
    if (nameLen == 0 || nameLen > CHANNEL_NAME_MAX) return;
    memcpy(user, line + 1, nameLen);
    user[nameLen] = '\0';

    const char *privmsg = strstr(bang, " PRIVMSG #");
    if (privmsg == NULL) return;
    const char *msg = strstr(privmsg, " :");
    if (msg == NULL) return;
    msg += 2;

    castVote(user, parseCommand(msg));
}

static void handleLine(char *line) {
    // Strip the trailing \r left over from \r\n
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\r') line[len - 1] = '\0';

    if (strncmp(line, "PING", 4) == 0) {
        sendAll(sock, "PONG :tmi.twitch.tv\r\n");
        return;
    }
    if (strstr(line, "PRIVMSG") != NULL) handlePrivmsg(line);
}

static void pollSocket(void) {
    char chunk[RECV_CHUNK_SIZE];
    for (;;) {
        int n = recv(sock, chunk, sizeof(chunk), 0);
        if (n <= 0) break;   // no more data right now (or the connection is gone, see below)

        for (int i = 0; i < n; i++) {
            char c = chunk[i];
            if (c == '\n') {
                lineBuffer[lineLength < LINE_BUFFER_SIZE ? lineLength : LINE_BUFFER_SIZE - 1] = '\0';
                handleLine(lineBuffer);
                lineLength = 0;
            } else if (lineLength < LINE_BUFFER_SIZE - 1) {
                lineBuffer[lineLength++] = c;
            }
        }
    }

#if defined(_WIN32)
    int err = WSAGetLastError();
    bool wouldBlock = err == WSAEWOULDBLOCK;
#else
    bool wouldBlock = errno == EAGAIN || errno == EWOULDBLOCK;
#endif
    if (!wouldBlock) {
        // A hard recv() error (not just "nothing queued") means the connection died
        int probe = recv(sock, NULL, 0, 0);
        (void)probe;
    }
}

void UpdateTwitchPlays(void) {
    jumpTriggeredThisFrame = false;
    if (!connected) return;

    pollSocket();

    if (--windowFramesLeft > 0) return;

    lastVoterCount = voterCount;
    if (jumpVotes > 0 || fallVotes > 0) {
        if (jumpVotes >= fallVotes) {
            jumpTriggeredThisFrame = true;
            fallHeldState = false;
        } else {
            fallHeldState = true;
        }
    } else {
        fallHeldState = false;
    }
    resetVoteWindow();
}

bool TwitchJumpTriggered(void) { return jumpTriggeredThisFrame; }
bool TwitchFastFallHeld(void) { return fallHeldState; }
int TwitchLastVoterCount(void) { return lastVoterCount; }

// ---- Menu screen ----

static Rectangle TwitchBackButtonRect(void) {
    return (Rectangle){ 24, 24, 130, 52 };
}

static Rectangle TwitchChannelFieldRect(int screenWidth, int screenHeight) {
    return (Rectangle){ screenWidth / 2.0f - 220, screenHeight / 2.0f - 90, 440, 60 };
}

static Rectangle TwitchConnectButtonRect(int screenWidth, int screenHeight) {
    return (Rectangle){ screenWidth / 2.0f - 100, screenHeight / 2.0f - 10, 200, 60 };
}

static Rectangle TwitchDisconnectButtonRect(int screenWidth, int screenHeight) {
    return (Rectangle){ screenWidth / 2.0f - 100, screenHeight / 2.0f - 10, 200, 60 };
}

void UpdateTwitchPlaysMenu(int screenWidth, int screenHeight) {
    if (BackPressed() || PointerPressedIn(TwitchBackButtonRect())) {
        GetMenuvars()->untermenu = 0;
        return;
    }
    if (!TwitchPlaysSupported()) return;

    if (connected) {
        if (PointerPressedIn(TwitchDisconnectButtonRect(screenWidth, screenHeight)))
            TwitchPlaysDisconnect();
        return;
    }

    // Simple text entry: letters/digits/underscore only (valid Twitch login characters)
    int key = GetCharPressed();
    while (key > 0) {
        size_t len = strlen(channelEdit);
        if (len < CHANNEL_NAME_MAX &&
            (isalnum(key) || key == '_')) {
            channelEdit[len] = (char)tolower(key);
            channelEdit[len + 1] = '\0';
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        size_t len = strlen(channelEdit);
        if (len > 0) channelEdit[len - 1] = '\0';
    }

    if ((IsKeyPressed(KEY_ENTER) ||
         PointerPressedIn(TwitchConnectButtonRect(screenWidth, screenHeight))) &&
        channelEdit[0] != '\0') {
        TwitchPlaysConnect(channelEdit);
    }
}

void DrawTwitchPlaysMenu(int screenWidth, int screenHeight) {
    int centerX = screenWidth / 2;

    drawTextShadowCentered("TWITCH PLAYS", centerX, 20, 56, MENU_GOLD);
    drawButton(TwitchBackButtonRect(), "BACK", 26, Fade(BLACK, 0.4f), 1.0f);

    if (!TwitchPlaysSupported()) {
        drawTextCentered("Networking not available", centerX, screenHeight / 2 - 12, 24, WHITE);
        return;
    }

    if (connected) {
        drawTextCentered(TwitchPlaysStatus(), centerX, screenHeight / 2 - 100, 28,
                         (Color){ 120, 230, 120, 255 });
        drawTextCentered(TextFormat("%d chatters voted last round", TwitchLastVoterCount()),
                         centerX, screenHeight / 2 - 60, 22, WHITE);
        drawButton(TwitchDisconnectButtonRect(screenWidth, screenHeight), "DISCONNECT", 22,
                   (Color){ 255, 110, 110, 255 }, 1.0f);
        return;
    }

    drawTextCentered("Enter your Twitch channel name", centerX, screenHeight / 2 - 140, 22,
                     Fade(WHITE, 0.85f));

    Rectangle field = TwitchChannelFieldRect(screenWidth, screenHeight);
    DrawRectangleRounded(field, 0.3f, 8, Fade(BLACK, 0.4f));
    DrawRectangleRoundedLinesEx(field, 0.3f, 8, 2.0f, MENU_GOLD);
    const char *shown = channelEdit[0] != '\0' ? channelEdit : "channel name...";
    Color textColor = channelEdit[0] != '\0' ? WHITE : Fade(WHITE, 0.4f);
    drawTextCentered(shown, (int)(field.x + field.width / 2), (int)(field.y + 16), 26, textColor);

    drawButton(TwitchConnectButtonRect(screenWidth, screenHeight), "CONNECT", 26, MENU_GREEN, 1.0f);
    drawTextCentered(TwitchPlaysStatus(), centerX, screenHeight / 2 + 90, 18, Fade(WHITE, 0.75f));
    drawTextCentered("Chat votes \"jump\" or \"fall\" every half second to fly the chicken together",
                     centerX, screenHeight / 2 + 120, 16, Fade(WHITE, 0.7f));
}
