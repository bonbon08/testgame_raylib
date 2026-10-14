package io.github.bonbon08.jumperchicken;

import android.app.Application;
import android.util.Log;
import android.webkit.WebSettings;

/**
 * Loads the WebView provider before the game activity exists. The ads and consent SDKs use a
 * WebView, and loading it for the first time adds asset paths to the process, which makes
 * Android relaunch a running activity (the game would restart a few seconds after launch).
 * No ads are initialized here; that still waits for the user's consent.
 */
public class JumperChickenApp extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        try {
            WebSettings.getDefaultUserAgent(this);
        } catch (RuntimeException e) {
            Log.w("JumperChicken", "WebView not available: " + e.getMessage());
        }
    }
}
