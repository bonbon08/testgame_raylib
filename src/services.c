#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

// Android services: rewarded video ads (Google AdMob) and the leaderboard (Google Play Games).
// On Android the calls go to MainActivity.java, on desktop all functions do nothing.

typedef enum RewardType {
    REWARD_NONE,
    REWARD_COINS,
    REWARD_REVIVE
} RewardType;

#define AD_REWARD_COINS 10

RewardType pendingReward = REWARD_NONE;
char playerName[64] = "";
int playerNameCheck = 0;

#define SIGN_IN_IDLE 0
#define SIGN_IN_RUNNING 1
#define SIGN_IN_FAILED 2

#if defined(PLATFORM_ANDROID)
#include <jni.h>
#include <android_native_app_glue.h>

struct android_app *GetAndroidApp(void);

typedef struct AdsBridge {
    JavaVM *vm;
    JNIEnv *env;
    jobject activity;
    jmethodID isRewardedAdReady;
    jmethodID showRewardedAd;
    jmethodID consumeReward;
    jmethodID isPrivacyOptionsRequired;
    jmethodID showPrivacyOptions;
    jmethodID isLeaderboardAvailable;
    jmethodID getPlayerName;
    jmethodID getSignInState;
    jmethodID submitScore;
    jmethodID showLeaderboard;
    bool ready;
} AdsBridge;

AdsBridge adsBridge = { 0 };

bool clearJavaException() {
    if (!(*adsBridge.env)->ExceptionCheck(adsBridge.env)) return false;
    (*adsBridge.env)->ExceptionDescribe(adsBridge.env);
    (*adsBridge.env)->ExceptionClear(adsBridge.env);
    return true;
}

void InitAds() {
    struct android_app *app = GetAndroidApp();
    adsBridge.vm = app->activity->vm;
    if ((*adsBridge.vm)->AttachCurrentThread(adsBridge.vm, &adsBridge.env, NULL) != JNI_OK) {
        TraceLog(LOG_WARNING, "ADS: Could not attach to the Java VM");
        return;
    }

    JNIEnv *env = adsBridge.env;
    adsBridge.activity = app->activity->clazz;   // the MainActivity instance
    jclass activityClass = (*env)->GetObjectClass(env, adsBridge.activity);
    adsBridge.isRewardedAdReady = (*env)->GetMethodID(env, activityClass, "isRewardedAdReady", "()Z");
    adsBridge.showRewardedAd = (*env)->GetMethodID(env, activityClass, "showRewardedAd", "()V");
    adsBridge.consumeReward = (*env)->GetMethodID(env, activityClass, "consumeReward", "()I");
    adsBridge.isPrivacyOptionsRequired = (*env)->GetMethodID(env, activityClass, "isPrivacyOptionsRequired", "()Z");
    adsBridge.showPrivacyOptions = (*env)->GetMethodID(env, activityClass, "showPrivacyOptions", "()V");
    adsBridge.isLeaderboardAvailable = (*env)->GetMethodID(env, activityClass, "isLeaderboardAvailable", "()Z");
    adsBridge.getPlayerName = (*env)->GetMethodID(env, activityClass, "getPlayerName", "()Ljava/lang/String;");
    adsBridge.getSignInState = (*env)->GetMethodID(env, activityClass, "getSignInState", "()I");
    adsBridge.submitScore = (*env)->GetMethodID(env, activityClass, "submitScore", "(J)V");
    adsBridge.showLeaderboard = (*env)->GetMethodID(env, activityClass, "showLeaderboard", "()V");
    (*env)->DeleteLocalRef(env, activityClass);

    if (clearJavaException()) {
        TraceLog(LOG_WARNING, "SERVICES: MainActivity is missing a method");
        return;
    }
    adsBridge.ready = true;
}

void CloseAds() {
    if (adsBridge.vm != NULL) (*adsBridge.vm)->DetachCurrentThread(adsBridge.vm);
    adsBridge.ready = false;
}

bool callJavaBool(jmethodID method) {
    if (!adsBridge.ready) return false;
    jboolean result = (*adsBridge.env)->CallBooleanMethod(adsBridge.env, adsBridge.activity, method);
    return !clearJavaException() && result;
}

void callJavaVoid(jmethodID method) {
    if (!adsBridge.ready) return;
    (*adsBridge.env)->CallVoidMethod(adsBridge.env, adsBridge.activity, method);
    clearJavaException();
}

bool AdsSupported() { return adsBridge.ready; }
bool RewardedAdReady() { return callJavaBool(adsBridge.isRewardedAdReady); }
bool PrivacyOptionsRequired() { return callJavaBool(adsBridge.isPrivacyOptionsRequired); }
void ShowPrivacyOptions() { callJavaVoid(adsBridge.showPrivacyOptions); }

void ShowRewardedAd(RewardType reward) {
    pendingReward = reward;
    callJavaVoid(adsBridge.showRewardedAd);
}

// Returns how many rewards were earned since the last call (normally 0 or 1)
int ConsumeAdReward() {
    if (!adsBridge.ready) return 0;
    jint rewards = (*adsBridge.env)->CallIntMethod(adsBridge.env, adsBridge.activity, adsBridge.consumeReward);
    return clearJavaException() ? 0 : rewards;
}

bool LeaderboardAvailable() { return callJavaBool(adsBridge.isLeaderboardAvailable); }
void ShowLeaderboard() { callJavaVoid(adsBridge.showLeaderboard); }

int SignInState() {
    if (!adsBridge.ready) return SIGN_IN_IDLE;
    jint state = (*adsBridge.env)->CallIntMethod(adsBridge.env, adsBridge.activity, adsBridge.getSignInState);
    return clearJavaException() ? SIGN_IN_IDLE : state;
}

void SubmitLeaderboardScore(int score) {
    if (!adsBridge.ready) return;
    (*adsBridge.env)->CallVoidMethod(adsBridge.env, adsBridge.activity, adsBridge.submitScore, (jlong)score);
    clearJavaException();
}

// The Google Play Games name, or "" when not signed in. Asks Java about once a second.
const char *PlayerName() {
    if (!adsBridge.ready || playerNameCheck++ % 60 != 0) return playerName;
    JNIEnv *env = adsBridge.env;
    jstring name = (jstring)(*env)->CallObjectMethod(env, adsBridge.activity, adsBridge.getPlayerName);
    if (clearJavaException() || name == NULL) return playerName;
    const char *chars = (*env)->GetStringUTFChars(env, name, NULL);
    snprintf(playerName, sizeof(playerName), "%s", chars);
    (*env)->ReleaseStringUTFChars(env, name, chars);
    (*env)->DeleteLocalRef(env, name);   // this thread never returns to Java, so free it ourselves
    return playerName;
}

#else

bool LeaderboardAvailable() { return false; }
void ShowLeaderboard() {}
int SignInState() { return SIGN_IN_IDLE; }
void SubmitLeaderboardScore(int score) { (void)score; }
const char *PlayerName() { return playerName; }
void InitAds() {}
void CloseAds() {}
bool AdsSupported() { return false; }
bool RewardedAdReady() { return false; }
bool PrivacyOptionsRequired() { return false; }
void ShowPrivacyOptions() {}
void ShowRewardedAd(RewardType reward) { (void)reward; }
int ConsumeAdReward() { return 0; }

#endif
