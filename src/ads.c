#include "raylib.h"
#include <stdbool.h>

// Rewarded video ads. On Android the calls go to MainActivity.java (Google AdMob),
// on desktop there are no ads and all functions do nothing.

typedef enum RewardType {
    REWARD_NONE,
    REWARD_COINS,
    REWARD_REVIVE
} RewardType;

#define AD_REWARD_COINS 10

RewardType pendingReward = REWARD_NONE;

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
    (*env)->DeleteLocalRef(env, activityClass);

    if (clearJavaException()) {
        TraceLog(LOG_WARNING, "ADS: MainActivity is missing an ads method");
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

#else

void InitAds() {}
void CloseAds() {}
bool AdsSupported() { return false; }
bool RewardedAdReady() { return false; }
bool PrivacyOptionsRequired() { return false; }
void ShowPrivacyOptions() {}
void ShowRewardedAd(RewardType reward) { (void)reward; }
int ConsumeAdReward() { return 0; }

#endif
