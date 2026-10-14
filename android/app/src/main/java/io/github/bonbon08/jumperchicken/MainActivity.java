package io.github.bonbon08.jumperchicken;

import android.app.NativeActivity;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;

import androidx.annotation.NonNull;

import com.google.android.gms.ads.AdError;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.FullScreenContentCallback;
import com.google.android.gms.ads.LoadAdError;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.rewarded.RewardedAd;
import com.google.android.gms.ads.rewarded.RewardedAdLoadCallback;
import com.google.android.gms.games.PlayGames;
import com.google.android.ump.ConsentInformation;
import com.google.android.ump.ConsentRequestParameters;
import com.google.android.ump.UserMessagingPlatform;

import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Hosts the raylib game (NativeActivity) and handles rewarded ads and the Play Games leaderboard.
 * The native code (src/ads.c) calls the public methods below from the game thread.
 */
public class MainActivity extends NativeActivity {
    private static final String TAG = "JumperChicken";
    private static final long RETRY_LOAD_DELAY_MS = 30_000;
    private static final int RC_LEADERBOARD = 9001;

    private final Handler mainHandler = new Handler(Looper.getMainLooper());
    private final AtomicBoolean mobileAdsStarted = new AtomicBoolean(false);
    private final AtomicInteger earnedRewards = new AtomicInteger(0);

    private ConsentInformation consentInformation;
    private RewardedAd rewardedAd;          // only touched on the main thread
    private boolean loadingAd = false;      // only touched on the main thread
    private volatile boolean adReady = false;

    private static final int SIGN_IN_IDLE = 0;
    private static final int SIGN_IN_RUNNING = 1;
    private static final int SIGN_IN_FAILED = 2;

    private volatile boolean gamesSignedIn = false;
    private volatile int signInState = SIGN_IN_IDLE;
    private volatile String playerName = "";
    private long bestPendingScore = 0;      // only touched on the main thread

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        hideSystemBars();
        requestConsent();
        if (JumperChickenApp.isPlayGamesConfigured(this)) checkGamesSignIn();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) hideSystemBars();
    }

    private void hideSystemBars() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            WindowInsetsController controller = getWindow().getInsetsController();
            if (controller != null) {
                controller.hide(WindowInsets.Type.systemBars());
                controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else {
            getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                            | View.SYSTEM_UI_FLAG_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
        }
    }

    // GDPR: ask for consent (only shown where required, e.g. in the EU) before any ad is requested
    private void requestConsent() {
        ConsentRequestParameters params = new ConsentRequestParameters.Builder()
                .setTagForUnderAgeOfConsent(false)
                .build();
        consentInformation = UserMessagingPlatform.getConsentInformation(this);
        consentInformation.requestConsentInfoUpdate(this, params,
                () -> UserMessagingPlatform.loadAndShowConsentFormIfRequired(this, formError -> {
                    if (formError != null) Log.w(TAG, "Consent form: " + formError.getMessage());
                    startMobileAdsIfAllowed();
                }),
                requestError -> Log.w(TAG, "Consent update: " + requestError.getMessage()));

        // Consent from an earlier session can be used right away
        startMobileAdsIfAllowed();
    }

    private void startMobileAdsIfAllowed() {
        if (!consentInformation.canRequestAds() || mobileAdsStarted.getAndSet(true)) return;
        new Thread(() -> MobileAds.initialize(this, status -> mainHandler.post(this::loadRewardedAd))).start();
    }

    private void loadRewardedAd() {
        if (rewardedAd != null || loadingAd) return;
        loadingAd = true;
        RewardedAd.load(this, getString(R.string.admob_rewarded_id), new AdRequest.Builder().build(),
                new RewardedAdLoadCallback() {
                    @Override
                    public void onAdLoaded(@NonNull RewardedAd ad) {
                        rewardedAd = ad;
                        loadingAd = false;
                        adReady = true;
                    }

                    @Override
                    public void onAdFailedToLoad(@NonNull LoadAdError error) {
                        Log.w(TAG, "Rewarded ad failed to load: " + error.getMessage());
                        loadingAd = false;
                        mainHandler.postDelayed(MainActivity.this::loadRewardedAd, RETRY_LOAD_DELAY_MS);
                    }
                });
    }

    // ---- Called from native code (game thread) ----

    public boolean isRewardedAdReady() {
        return adReady;
    }

    public void showRewardedAd() {
        mainHandler.post(() -> {
            RewardedAd ad = rewardedAd;
            if (ad == null) {
                loadRewardedAd();
                return;
            }
            rewardedAd = null;
            adReady = false;
            ad.setFullScreenContentCallback(new FullScreenContentCallback() {
                @Override
                public void onAdDismissedFullScreenContent() {
                    hideSystemBars();
                    loadRewardedAd();
                }

                @Override
                public void onAdFailedToShowFullScreenContent(@NonNull AdError error) {
                    Log.w(TAG, "Rewarded ad failed to show: " + error.getMessage());
                    loadRewardedAd();
                }
            });
            ad.show(this, reward -> earnedRewards.incrementAndGet());
        });
    }

    /** Returns the rewards earned since the last call and resets the counter. */
    public int consumeReward() {
        return earnedRewards.getAndSet(0);
    }

    public boolean isPrivacyOptionsRequired() {
        return consentInformation != null
                && consentInformation.getPrivacyOptionsRequirementStatus()
                == ConsentInformation.PrivacyOptionsRequirementStatus.REQUIRED;
    }

    // ---- Google Play Games leaderboard ----

    private void checkGamesSignIn() {
        // Play Games signs players in automatically when they have used it before
        PlayGames.getGamesSignInClient(this).isAuthenticated().addOnCompleteListener(task ->
                onGamesSignIn(task.isSuccessful() && task.getResult().isAuthenticated(), false));
    }

    private void onGamesSignIn(boolean signedIn, boolean openLeaderboard) {
        gamesSignedIn = signedIn;
        // A failed silent check at startup is normal; only a failed tap is reported to the player
        signInState = signedIn || !openLeaderboard ? SIGN_IN_IDLE : SIGN_IN_FAILED;
        if (!signedIn) return;
        PlayGames.getPlayersClient(this).getCurrentPlayer()
                .addOnSuccessListener(player -> playerName = player.getDisplayName());
        if (bestPendingScore > 0) {
            submitScoreNow(bestPendingScore);
            bestPendingScore = 0;
        }
        if (openLeaderboard) openLeaderboard();
    }

    private void submitScoreNow(long score) {
        PlayGames.getLeaderboardsClient(this).submitScore(getString(R.string.leaderboard_id), score);
    }

    private void openLeaderboard() {
        PlayGames.getLeaderboardsClient(this).getLeaderboardIntent(getString(R.string.leaderboard_id))
                .addOnSuccessListener(intent -> startActivityForResult(intent, RC_LEADERBOARD))
                .addOnFailureListener(e -> Log.w(TAG, "Leaderboard: " + e.getMessage()));
    }

    public boolean isLeaderboardAvailable() {
        return JumperChickenApp.isPlayGamesConfigured(this);
    }

    /** The player's Play Games name, or "" when not signed in. */
    public String getPlayerName() {
        return playerName;
    }

    /** 0 = idle, 1 = signing in, 2 = the last sign-in attempt failed. */
    public int getSignInState() {
        return signInState;
    }

    /** Play Games keeps the best score, so sending the highscore again is harmless. */
    public void submitScore(long score) {
        if (!isLeaderboardAvailable() || score <= 0) return;
        mainHandler.post(() -> {
            if (gamesSignedIn) submitScoreNow(score);
            else bestPendingScore = Math.max(bestPendingScore, score);
        });
    }

    public void showLeaderboard() {
        if (!isLeaderboardAvailable()) return;
        mainHandler.post(() -> {
            if (gamesSignedIn) {
                openLeaderboard();
                return;
            }
            signInState = SIGN_IN_RUNNING;
            PlayGames.getGamesSignInClient(this).signIn().addOnCompleteListener(task ->
                    onGamesSignIn(task.isSuccessful() && task.getResult().isAuthenticated(), true));
        });
    }

    public void showPrivacyOptions() {
        mainHandler.post(() -> UserMessagingPlatform.showPrivacyOptionsForm(this, formError -> {
            if (formError != null) Log.w(TAG, "Privacy options: " + formError.getMessage());
            hideSystemBars();
        }));
    }
}
