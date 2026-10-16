// Services layer (Ads, Leaderboard, I/O)
#ifndef SERVICES_H
#define SERVICES_H

#include <stdbool.h>
#include <stdint.h>

// Reward types
typedef enum {
    REWARD_NONE,
    REWARD_COINS,
    REWARD_REVIVE
} RewardType;

#define AD_REWARD_COINS 10

// Service interfaces
void InitAds(void);
void CloseAds(void);
bool AdsSupported(void);
bool RewardedAdReady(void);
bool PrivacyOptionsRequired(void);
void ShowPrivacyOptions(void);
void ShowRewardedAd(RewardType reward);
int ConsumeAdReward(void);
bool LeaderboardAvailable(void);
void ShowLeaderboard(void);
int SignInState(void);
void SubmitLeaderboardScore(int score);
const char *PlayerName(void);

#endif // SERVICES_H