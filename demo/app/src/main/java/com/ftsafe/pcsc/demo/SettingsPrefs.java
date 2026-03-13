package com.ftsafe.pcsc.demo;

import android.content.Context;
import android.content.SharedPreferences;

import com.ftsafe.pcsc.demo.BuildConfig;
import com.ftsafe.pcsc.server.PCSCServerService;
import com.ftsafe.util.BleDebugConfig;

/**
 * Persisted settings for host, port, and BLE debug.
 */
public final class SettingsPrefs {
    private static final String PREFS = "pcsc_demo_prefs";
    private static final String KEY_HOST = "host";
    private static final String KEY_PORT = "port";
    private static final String KEY_BLE_CCID_DEBUG = "ble_ccid_debug";

    private SettingsPrefs() {}

    public static String getHost(Context ctx) {
        return getPrefs(ctx).getString(KEY_HOST, "127.0.0.1");
    }

    public static void setHost(Context ctx, String host) {
        getPrefs(ctx).edit().putString(KEY_HOST, host != null ? host : "127.0.0.1").apply();
    }

    public static int getPort(Context ctx) {
        return getPrefs(ctx).getInt(KEY_PORT, PCSCServerService.getDefaultPort());
    }

    public static void setPort(Context ctx, int port) {
        int validPort = (port > 0 && port <= 65535) ? port : PCSCServerService.getDefaultPort();
        getPrefs(ctx).edit().putInt(KEY_PORT, validPort).apply();
    }

    /** Whether BLE CCID traffic should be logged (for debugging). Default: true. */
    public static boolean getBleCcidDebug(Context ctx) {
        return getPrefs(ctx).getBoolean(KEY_BLE_CCID_DEBUG, true);
    }

    public static void setBleCcidDebug(Context ctx, boolean enabled) {
        getPrefs(ctx).edit().putBoolean(KEY_BLE_CCID_DEBUG, enabled).apply();
        BleDebugConfig.BLE_CCID_DEBUG = enabled;
    }

    /** Apply saved BLE debug setting to BleDebugConfig. Call at app startup. */
    public static void applyBleCcidDebug(Context ctx) {
        if (BuildConfig.DEBUG) {
            BleDebugConfig.BLE_CCID_DEBUG = getBleCcidDebug(ctx);
        } else {
            BleDebugConfig.BLE_CCID_DEBUG = false;  /* always off in release */
        }
    }

    private static SharedPreferences getPrefs(Context ctx) {
        return ctx.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
    }
}
