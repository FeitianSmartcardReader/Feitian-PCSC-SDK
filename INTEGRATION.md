# Feitian PC/SC SDK Integration

**Version**: 3.0.3

## Package Contents

| Directory | Description |
|-----------|-------------|
| **docs/** | Documentation |
| **lib/** | AAR libraries (ftreader-ble-ccid-core, ftreader-comm, ftreader-pcsc-server, ftreader-pcsc) |
| **demo/** | Demo app source (buildable Android project) |
| **apk/** | Prebuilt demo APK |
| **thirdparty/** | OpenSC PKCS#11 (optional) |
| **include/** | C headers for native integration |

## Quick Start

1. **Use prebuilt APK**: Install apk/FTReaderPCSC-Demo-3.0.3.apk
2. **Use AARs**: Add lib/*.aar to your app (see QUICK_START.md)
3. **Build from source**: cd demo && ./gradlew :app:assembleDebug
