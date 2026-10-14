package io.github.bonbon08.jumperchicken;

import android.app.Application;
import android.content.Context;
import android.util.Log;
import android.webkit.WebSettings;

import com.google.android.gms.games.PlayGamesSdk;

/**
 * Loads the WebView provider before the game activity exists. The ads and consent SDKs use a
 * WebView, and loading it for the first time adds asset paths to the process, which makes
 * Android relaunch a running activity (the game would restart a few seconds after launch).
 * No ads are initialized here; that still waits for the user's consent.
 * Also starts Google Play Games when a leaderboard is configured.
 */
public class JumperChickenApp extends Application {
    /** True when the Play Games IDs were set in the gradle properties. */
    public static boolean isPlayGamesConfigured(Context context) {
        return !context.getString(R.string.play_games_app_id).isEmpty()
                && !context.getString(R.string.leaderboard_id).isEmpty();
    }

    @Override
    public void onCreate() {
        super.onCreate();
        if (isPlayGamesConfigured(this)) PlayGamesSdk.initialize(this);
        try {
            WebSettings.getDefaultUserAgent(this);
        } catch (RuntimeException e) {
            Log.w("JumperChicken", "WebView not available: " + e.getMessage());
        }
    }
}
