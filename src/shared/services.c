// Services layer (Ads, Leaderboard, I/O)
#include "services.h"

#include <raylib.h>
#include <jni.h>
#include <android/native_app_glue.h>
#include <android/glue.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// Android-specific implementation
#ifdef PLATFORM_ANDROID

static struct android_app *GetAndroidApp(void) {
    return NULL;  // will be initialized by Android framework
}

static AdsBridge adsBridge = { 0 };

static bool clearJavaException() {
    if (!adsBridge.env) return false;
    JNIEnv *env = adsBridge.env;
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return true;
    }
    return false;
}

static void InitAds(void) {
    struct android_app *app = GetAndroidApp();
    adsBridge.vm = app->activity->vm;
    (*adsBridge.vm)->AttachCurrentThread(adsBridge.vm, &adsBridge.env, NULL);
    JNIEnv *env = adsBridge.env;
    jclass activityClass = (*env)->GetObjectClass(env, app->activity->clazz);
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
        TraceLog(LOG_WARNING, "ADS: MainActivity is missing a method");
        return;
    }
    adsBridge.ready = true;
}

static void CloseAds(void) {
    if (adsBridge.vm != NULL) (*adsBridge.vm)->DetachCurrentThread(adsBridge.vm);
    adsBridge.ready = false;
}

static bool callJavaBool(jmethodID method) {
    if (!adsBridge.ready) return false;
    jboolean result = (*adsBridge.env)->CallBooleanMethod(adsBridge.env, adsBridge.activity, method);
    return !clearJavaException() && result;
}

static void callJavaVoid(jmethodID method) {
    if (!adsBridge.ready) return;
    (*adsBridge.env)->CallVoidMethod(adsBridge.env, adsBridge.activity, method);
    clearJavaException();
}

bool AdsSupported(void) { return adsBridge.ready; }
bool RewardedAdReady(void) { return callJavaBool(adsBridge.isRewardedAdReady); }
bool PrivacyOptionsRequired(void) { return callJavaBool(adsBridge.isPrivacyOptionsRequired); }
void ShowPrivacyOptions(void) { callJavaVoid(adsBridge.showPrivacyOptions); }

void ShowRewardedAd(RewardType reward) {
    pendingReward = reward;
    callJavaVoid(adsBridge.showRewardedAd);
}

int ConsumeAdReward(void) {
    if (!adsBridge.ready) return 0;
    jint rewards = (*adsBridge.env)->CallIntMethod(adsBridge.env, adsBridge.activity, adsBridge.consumeReward);
    return clearJavaException() ? 0 : rewards;
}

bool LeaderboardAvailable(void) { return callJavaBool(adsBridge.isLeaderboardAvailable); }
void ShowLeaderboard(void) { callJavaVoid(adsBridge.showLeaderboard); }

int SignInState(void) {
    if (!adsBridge.ready) return 0;
    jint state = (*adsBridge.env)->CallIntMethod(adsBridge.env, adsBridge.activity, adsBridge.getSignInState);
    return clearJavaException() ? 0 : state;
}

void SubmitLeaderboardScore(int score) {
    if (!adsBridge.ready) return;
    (*adsBridge.env)->CallVoidMethod(adsBridge.env, adsBridge.activity, adsBridge.submitScore, (jlong)score);
    clearJavaException();
}

const char *PlayerName(void) {
    if (!adsBridge.ready || playerNameCheck++ % 60 != 0) return playerName;
    JNIEnv *env = adsBridge.env;
    jstring name = (jstring)(*env)->CallObjectMethod(env, adsBridge.activity, adsBridge.getPlayerName);
    if (clearJavaException() || name == NULL) return playerName;
    const char *chars = (*env)->GetStringUTFChars(env, name, NULL);
    snprintf(playerName, sizeof(playerName), "%s", chars);
    (*env)->ReleaseStringUTFChars(env, name, chars);
    (*env)->DeleteLocalRef(env, name);
    return playerName;
}

#else   // Non-Android (desktop) implementation

static bool adsBridgeReady = false;
static bool adsBridgeReady = false;

static void InitAds(void) {
    adsBridgeReady = true;
}

static void CloseAds(void) {
    adsBridgeReady = false;
}

static bool callJavaBool(jmethodID method) { return false; }
static void callJavaVoid(jmethodID method) { }

bool AdsSupported(void) { return adsBridgeReady; }
bool RewardedAdReady(void) { return false; }
bool PrivacyOptionsRequired(void) { return false; }
void ShowPrivacyOptions(void) { }
void ShowRewardedAd(RewardType reward) { (void)reward; }
int ConsumeAdReward(void) { return 0; }
bool LeaderboardAvailable(void) { return false; }
void ShowLeaderboard(void) { }
int SignInState(void) { return 0; }
void SubmitLeaderboardScore(int score) { (void)score; }
const char *PlayerName(void) { return ""; }

#endif  // PLATFORM_ANDROID

// RewardType enum and Reward handling
typedef enum {
    REWARD_NONE,
    REWARD_COINS,
    REWARD_REVIVE
} RewardType;

static RewardType pendingReward = REWARD_NONE;

static void ShowRewardedAd(RewardType reward) {
    pendingReward = reward;
    ShowPrivacyOptions();
}

int ConsumeAdReward(void) {
    if (!adsBridgeReady) return 0;
    // Simulate reward consumption
    return 1;  // return 1 reward for demo purposes
}

void InitializeAds(void) {
#ifdef PLATFORM_ANDROID
    InitAds();
#else
    adsBridgeReady = true;
#endif
}

void CloseAds(void) {
#ifdef PLATFORM_ANDROID
    CloseAds();
#else
    adsBridgeReady = false;
#endif
}

bool AdsSupported(void) { return adsBridgeReady; }
bool RewardedAdReady(void) { return false; }  // handled by ShowRewardedAd()
bool PrivacyOptionsRequired(void) { return false; }
void ShowPrivacyOptions(void) { ShowPrivacyOptions(); }
void ShowRewardedAd(RewardType reward) { ShowRewardedAd(reward); }
int ConsumeAdReward(void) { return ConsumeAdReward(); }

bool LeaderboardAvailable(void) { return false; }
void ShowLeaderboard(void) { }
int SignInState(void) { return 0; }
void SubmitLeaderboardScore(int score) { }
const char *PlayerName(void) { return ""; }

#endif  // SERVICES_C