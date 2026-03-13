# opensc-pkcs11 Android Module

OpenSC PKCS#11 module integrated with libftreader-pcsc.

## Build Prerequisites

1. **Fetch OpenSC source**
   ```bash
   ./scripts/setup_opensc_android.sh
   # Or manually: git clone --depth 1 https://github.com/OpenSC/OpenSC.git thirdparty/opensc-pkcs11/opensc
   ```

2. **Prepare OpenSSL for Android**
   - Download from [Prebuilt-OpenSSL-for-Android](https://github.com/kibitzerCZ/Prebuilt-OpenSSL-for-Android/releases)
   - Place `libcrypto.a` and `libssl.a` in `libs/arm64-v8a/`, `libs/armeabi-v7a/`, `libs/x86/`, `libs/x86_64/`

3. **Build**
   ```bash
   ./gradlew :thirdparty:opensc-pkcs11:assembleDebug
   ```

## Runtime

- Load libftreader-pcsc first, call `FTReader_InitSocket`
- Then load opensc-pkcs11 (SCard* symbols resolved from libftreader-pcsc)
