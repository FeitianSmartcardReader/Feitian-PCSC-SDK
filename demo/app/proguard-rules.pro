# Strip Log calls in release
-assumenosideeffects class android.util.Log {
    public static int v(...);
    public static int d(...);
    public static int i(...);
    public static int w(...);
    public static int e(...);
}

# Keep app classes (no obfuscation needed for demo)
-keep class com.ftsafe.pcsc.demo.** { *; }

# Keep native methods for JNI
-keepclasseswithmembernames class * {
    native <methods>;
}
