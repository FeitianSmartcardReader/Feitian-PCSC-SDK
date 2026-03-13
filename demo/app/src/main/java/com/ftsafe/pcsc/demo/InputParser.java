package com.ftsafe.pcsc.demo;

/**
 * Parses user input for host and port. Used by ApiDemoActivity and SettingsActivity.
 */
public final class InputParser {

    private InputParser() {}

    /**
     * Parse port from string. Valid range is 1-65535.
     *
     * @param s       input string (trimmed)
     * @param fallback value when empty or invalid
     * @return parsed port or fallback
     */
    public static int parsePort(String s, int fallback) {
        if (s == null || s.trim().isEmpty()) return fallback;
        try {
            int p = Integer.parseInt(s.trim());
            return (p > 0 && p <= 65535) ? p : fallback;
        } catch (NumberFormatException e) {
            return fallback;
        }
    }

    /**
     * Parse host from string.
     *
     * @param s       input string (trimmed)
     * @param fallback value when empty
     * @return parsed host or fallback
     */
    public static String parseHost(String s, String fallback) {
        if (s == null || s.trim().isEmpty()) return fallback;
        return s.trim();
    }
}
