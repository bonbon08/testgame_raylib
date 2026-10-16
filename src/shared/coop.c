// Local two-player co-op over Bluetooth (Android only; see MainActivity/CoopManager.java)
#include "coop.h"
#include "objects.h"
#include "menu.h"
#include "input.h"

#include <raylib.h>
#include <stdbool.h>

#define COOP_SEND_EVERY_N_FRAMES 4   // ~15 times a second at 60 fps, plenty for a ghost chicken

static Vector2 peerPos = { 0, 0 };
static int peerDirection = 0;
static int peerScore = 0;

#if defined(PLATFORM_ANDROID)
#include <jni.h>
#include <android_native_app_glue.h>

// Provided by raylib (rcore_android.c); not declared in raylib.h
struct android_app *GetAndroidApp(void);

typedef struct CoopBridge {
    JavaVM *vm;
    JNIEnv *env;
    jobject activity;
    jmethodID hasBluetooth;
    jmethodID hasPermissions;
    jmethodID requestPermissions;
    jmethodID startHost;
    jmethodID startJoin;
    jmethodID isConnected;
    jmethodID send;
    jmethodID getPeerX;
    jmethodID getPeerY;
    jmethodID getPeerDirection;
    jmethodID getPeerScore;
    jmethodID disconnect;
    bool ready;
} CoopBridge;

static CoopBridge coopBridge = { 0 };
static int sendCounter = 0;

static bool clearJavaException(void) {
    if (!(*coopBridge.env)->ExceptionCheck(coopBridge.env)) return false;
    (*coopBridge.env)->ExceptionDescribe(coopBridge.env);
    (*coopBridge.env)->ExceptionClear(coopBridge.env);
    return true;
}

void InitCoop(void) {
    struct android_app *app = GetAndroidApp();
    coopBridge.vm = app->activity->vm;
    if ((*coopBridge.vm)->AttachCurrentThread(coopBridge.vm, &coopBridge.env, NULL) != JNI_OK) {
        TraceLog(LOG_WARNING, "COOP: Could not attach to the Java VM");
        return;
    }

    JNIEnv *env = coopBridge.env;
    coopBridge.activity = app->activity->clazz;
    jclass activityClass = (*env)->GetObjectClass(env, coopBridge.activity);
    coopBridge.hasBluetooth = (*env)->GetMethodID(env, activityClass, "coopHasBluetooth", "()Z");
    coopBridge.hasPermissions = (*env)->GetMethodID(env, activityClass, "coopHasPermissions", "()Z");
    coopBridge.requestPermissions = (*env)->GetMethodID(env, activityClass, "coopRequestPermissions", "()V");
    coopBridge.startHost = (*env)->GetMethodID(env, activityClass, "coopStartHost", "()V");
    coopBridge.startJoin = (*env)->GetMethodID(env, activityClass, "coopStartJoin", "()V");
    coopBridge.isConnected = (*env)->GetMethodID(env, activityClass, "coopIsConnected", "()Z");
    coopBridge.send = (*env)->GetMethodID(env, activityClass, "coopSend", "(FFII)V");
    coopBridge.getPeerX = (*env)->GetMethodID(env, activityClass, "coopGetPeerX", "()F");
    coopBridge.getPeerY = (*env)->GetMethodID(env, activityClass, "coopGetPeerY", "()F");
    coopBridge.getPeerDirection = (*env)->GetMethodID(env, activityClass, "coopGetPeerDirection", "()I");
    coopBridge.getPeerScore = (*env)->GetMethodID(env, activityClass, "coopGetPeerScore", "()I");
    coopBridge.disconnect = (*env)->GetMethodID(env, activityClass, "coopDisconnect", "()V");
    (*env)->DeleteLocalRef(env, activityClass);

    if (clearJavaException()) {
        TraceLog(LOG_WARNING, "COOP: MainActivity is missing a method");
        return;
    }
    coopBridge.ready = true;
}

void CloseCoop(void) {
    if (coopBridge.ready) (*coopBridge.env)->CallVoidMethod(coopBridge.env, coopBridge.activity, coopBridge.disconnect);
    if (coopBridge.vm != NULL) (*coopBridge.vm)->DetachCurrentThread(coopBridge.vm);
    coopBridge.ready = false;
}

static bool callBool(jmethodID method) {
    if (!coopBridge.ready) return false;
    jboolean result = (*coopBridge.env)->CallBooleanMethod(coopBridge.env, coopBridge.activity, method);
    return !clearJavaException() && result;
}

static void callVoid(jmethodID method) {
    if (!coopBridge.ready) return;
    (*coopBridge.env)->CallVoidMethod(coopBridge.env, coopBridge.activity, method);
    clearJavaException();
}

bool CoopSupported(void) { return callBool(coopBridge.hasBluetooth); }
bool CoopPermissionsGranted(void) { return callBool(coopBridge.hasPermissions); }
void CoopRequestPermissions(void) { callVoid(coopBridge.requestPermissions); }
void CoopStartHost(void) { callVoid(coopBridge.startHost); }
void CoopStartJoin(void) { callVoid(coopBridge.startJoin); }
bool CoopConnected(void) { return callBool(coopBridge.isConnected); }
void CoopDisconnect(void) { callVoid(coopBridge.disconnect); }

void UpdateCoop(void) {
    if (!coopBridge.ready || !CoopConnected()) return;

    JNIEnv *env = coopBridge.env;
    peerPos.x = (*env)->CallFloatMethod(env, coopBridge.activity, coopBridge.getPeerX);
    peerPos.y = (*env)->CallFloatMethod(env, coopBridge.activity, coopBridge.getPeerY);
    peerDirection = (*env)->CallIntMethod(env, coopBridge.activity, coopBridge.getPeerDirection);
    peerScore = (*env)->CallIntMethod(env, coopBridge.activity, coopBridge.getPeerScore);
    clearJavaException();

    if (++sendCounter < COOP_SEND_EVERY_N_FRAMES) return;
    sendCounter = 0;

    (*env)->CallVoidMethod(env, coopBridge.activity, coopBridge.send,
                           GetChicken()->pos.x, GetChicken()->pos.y,
                           GetChicken()->direction, GetGamevars()->score);
    clearJavaException();
}

#else   // Non-Android (desktop): co-op needs a Bluetooth radio this build doesn't have

void InitCoop(void) { }
void CloseCoop(void) { }
bool CoopSupported(void) { return false; }
bool CoopPermissionsGranted(void) { return false; }
void CoopRequestPermissions(void) { }
void CoopStartHost(void) { }
void CoopStartJoin(void) { }
bool CoopConnected(void) { return false; }
void CoopDisconnect(void) { }
void UpdateCoop(void) { }

#endif

Vector2 PeerPosition(void) { return peerPos; }
int PeerDirection(void) { return peerDirection; }
int PeerScore(void) { return peerScore; }

// ---- HOST/JOIN/DISCONNECT screen (platform-generic: desktop just shows "not available") ----

static Rectangle CoopBackButtonRect(void) {
    return (Rectangle){ 24, 24, 130, 52 };
}

static Rectangle CoopHostButtonRect(int screenWidth, int screenHeight) {
    return (Rectangle){ screenWidth / 2.0f - 210, screenHeight / 2.0f - 30, 200, 70 };
}

static Rectangle CoopJoinButtonRect(int screenWidth, int screenHeight) {
    return (Rectangle){ screenWidth / 2.0f + 10, screenHeight / 2.0f - 30, 200, 70 };
}

static Rectangle CoopDisconnectButtonRect(int screenWidth, int screenHeight) {
    return (Rectangle){ screenWidth / 2.0f - 100, screenHeight / 2.0f - 30, 200, 70 };
}

static Rectangle CoopPermissionButtonRect(int screenWidth, int screenHeight) {
    return (Rectangle){ screenWidth / 2.0f - 140, screenHeight / 2.0f - 30, 280, 60 };
}

void UpdateCoopMenu(int screenWidth, int screenHeight) {
    UpdateCoop();   // keeps the peer score fresh while this screen is open

    if (BackPressed() || PointerPressedIn(CoopBackButtonRect())) {
        GetMenuvars()->untermenu = 0;
        return;
    }
    if (!CoopSupported()) return;

    if (!CoopPermissionsGranted()) {
        if (PointerPressedIn(CoopPermissionButtonRect(screenWidth, screenHeight)))
            CoopRequestPermissions();
        return;
    }

    if (CoopConnected()) {
        if (PointerPressedIn(CoopDisconnectButtonRect(screenWidth, screenHeight)))
            CoopDisconnect();
        return;
    }

    if (PointerPressedIn(CoopHostButtonRect(screenWidth, screenHeight))) CoopStartHost();
    if (PointerPressedIn(CoopJoinButtonRect(screenWidth, screenHeight))) CoopStartJoin();
}

void DrawCoopMenu(int screenWidth, int screenHeight) {
    int centerX = screenWidth / 2;

    drawTextShadowCentered("CO-OP", centerX, 20, 56, MENU_GOLD);
    drawButton(CoopBackButtonRect(), "BACK", 26, Fade(BLACK, 0.4f), 1.0f);

    if (!CoopSupported()) {
        drawTextCentered("No Bluetooth on this device", centerX, screenHeight / 2 - 12, 24, WHITE);
        return;
    }

    if (!CoopPermissionsGranted()) {
        drawButton(CoopPermissionButtonRect(screenWidth, screenHeight), "GRANT BLUETOOTH ACCESS", 20,
                   MENU_GREEN, 1.0f);
        drawTextCentered("Needed to find and connect to the other phone", centerX,
                         screenHeight / 2 + 50, 18, Fade(WHITE, 0.8f));
        return;
    }

    if (CoopConnected()) {
        drawTextCentered("Connected!", centerX, screenHeight / 2 - 100, 32,
                         (Color){ 120, 230, 120, 255 });
        drawTextCentered(TextFormat("Peer score: %d", PeerScore()), centerX, screenHeight / 2 - 60, 22, WHITE);
        drawButton(CoopDisconnectButtonRect(screenWidth, screenHeight), "DISCONNECT", 22,
                   (Color){ 255, 110, 110, 255 }, 1.0f);
        return;
    }

    drawButton(CoopHostButtonRect(screenWidth, screenHeight), "HOST", 28, MENU_ORANGE, 1.0f);
    drawButton(CoopJoinButtonRect(screenWidth, screenHeight), "JOIN", 28, MENU_GREEN, 1.0f);
    drawTextCentered("One phone HOSTs, the other JOINs - both show up side by side once connected",
                     centerX, screenHeight / 2 + 60, 18, Fade(WHITE, 0.8f));
}
