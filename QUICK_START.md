# Feitian PC/SC SDK — Quick Start & Reference

Quick deployment, integration, API reference, compatibility, and debugging guide.

# Demo App Source

Demo uses only UI + JNI code. SDK libraries are pre-built AARs in libs/.

To build:

1. Ensure Android SDK and NDK are installed
2. For PKCS#11: run `./scripts/setup_opensc_android.sh` first (OpenSSL)
3. Run: `./gradlew :app:assembleDebug`
4. Or open in Android Studio

---

## 1. Quick Integration (5 Minutes)

### 1.1 Add Dependencies

In your app's `build.gradle`:

```gradle
dependencies {
    implementation project(':sdk:pcsc-server')
    implementation project(':sdk:ftreader-pcsc')
}
```

In `settings.gradle`:

```gradle
include ':sdk:pcsc-server', ':sdk:ftreader-pcsc'
```

### 1.2 Add Permissions

In `AndroidManifest.xml`:

```xml
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
<uses-permission android:name="android.permission.BLUETOOTH_SCAN" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
<uses-feature android:name="android.hardware.usb.host" android:required="false" />
```

**Runtime**: Request `BLUETOOTH_CONNECT`, `BLUETOOTH_SCAN` (Android 12+), or `ACCESS_FINE_LOCATION` (Android 6–11) before first BLE scan.

### 1.3 Minimal Code

```java
import com.ftsafe.ftreader.pcsc.FTReaderPCSC;
import com.ftsafe.pcsc.server.PCSCServerService;

// 1. Start PCSCServer (e.g. in Application.onCreate)
PCSCServerService.start(context, PCSCServerService.getDefaultPort());

// 2. Initialize socket (before any SCard* or PKCS#11 call)
FTReaderPCSC.initSocket("127.0.0.1", PCSCServerService.getDefaultPort());

// 3. Use PC/SC or PKCS#11
String readers = FTReaderPCSC.nativeListReaders();
```

### 1.4 Checklist

- [ ] Dependencies added
- [ ] Permissions declared and requested at runtime
- [ ] PCSCServer started before reader operations
- [ ] FTReaderPCSC.initSocket called before first SCard* / PKCS#11 call
- [ ] BLE reader paired or USB reader connected with permission granted

### 1.5 Common Errors

| Error | Cause | Fix |
|-------|-------|-----|
| "Start server first" | initSocket/ListReaders before server | Call `PCSCServerService.start(context, port)` first |
| "No readers found" | No BLE paired / USB not connected | Pair BLE, connect USB and grant permission |
| "Native lib load failed" | ftreader-pcsc not linked | Add `implementation project(':sdk:ftreader-pcsc')` |
| BLE permission denied | User denied | Request BLUETOOTH_CONNECT, BLUETOOTH_SCAN (12+), ACCESS_FINE_LOCATION (6–11) |

---

## 2. Build

### 2.1 Quick Build

```bash
./gradlew :app:assembleDebug
./scripts/build_install_debug.sh
./scripts/build_final_release.sh
```

### 2.2 Prerequisites

- JDK 11+
- Android SDK (API 34)
- Android NDK 28.x

### 2.3 Output Locations

| Target | Output |
|--------|--------|
| Debug APK | `app/build/outputs/apk/debug/app-debug.apk` |
| Release APK | `app/build/outputs/apk/release/app-release.apk` |
| SDK package | `build/Reader_SDK_YYYYMMDD/` (ZIP + DMG) |

### 2.4 BLE CCID Debug

Enable "BLE CCID debug log" in Settings to see BLE send/recv hex dumps. Default: off in release.

---

## 3. Reader Compatibility

### 3.1 BLE Readers

| Reader | UUID Pattern | Status |
|--------|--------------|--------|
| C4 (C45C) | `46540001-0002-00c4-0000-465453414645` | ✓ |
| C6 | `46540001-0002-00c6-0000-465453414645` | ✓ |
| C4 New / Corilus | `46540001-0002-00c4-0000-000000000001` | ✓ |

Pair reader in system Bluetooth settings before use.

### 3.2 USB Readers

| Vendor ID | Description |
|-----------|-------------|
| 0x096E | Feitian (only supported VID) |

Requires USB host (OTG) and user-granted permission.

### 3.3 Android Support

| API Level | Notes |
|-----------|-------|
| 21–22 | minSdk 21 |
| 23–30 | BLE scan requires ACCESS_FINE_LOCATION |
| 31+ | BLE requires BLUETOOTH_SCAN, BLUETOOTH_CONNECT |

**targetSdk**: 34

### 3.4 ABIs

arm64-v8a, armeabi-v7a, x86_64, x86.

---

## 4. API Reference

### 4.1 Java

**PCSCServerService**

```java
PCSCServerService.start(context, port);
PCSCServerService.stop();
PCSCServerService.isRunning();
PCSCServerService.getDefaultPort();  // 35963
```

**FTReaderPCSC**

```java
FTReaderPCSC.initSocket("127.0.0.1", 35963);
FTReaderPCSC.uninit();
```

### 4.2 C (winscard)

```c
void FTReader_InitSocket(const char* host, int port);
void FTReader_Uninit(void);
```

Call `FTReader_InitSocket` before first `SCard*` call. Full winscard API: `SCardEstablishContext`, `SCardListReaders`, `SCardConnect`, `SCardTransmit`, etc. See `winscard.h`.

### 4.3 Environment

| Variable | Description | Default |
|----------|-------------|---------|
| FTREADER_PCSC_HOST | PCSCServer host | 127.0.0.1 |
| FTREADER_PCSC_PORT | PCSCServer port | 35963 |

---

## 5. SDK Structure

### 5.1 Package Layout (build_final_release.sh)

```
Reader_SDK_YYYYMMDD/
├── docs/
├── lib/          # AAR: ftreader-pcsc, pcsc-server, comm, ble-ccid-core
├── include/      # ccid.h, transport.h, winscard.h, wintypes.h
├── demo/
├── apk/
├── thirdparty/   # opensc-pkcs11
├── README.md
└── CHANGELOG.md
```

### 5.2 Module Dependencies

```
Your App
  ├── pcsc-server (BLE/USB bridge)
  │     ├── comm
  │     └── ble-ccid-core
  └── ftreader-pcsc (native PC/SC)
```

---

## 6. OpenSC PKCS#11 Integration

### 6.1 Android Demo

1. Start Demo App → Start Server → Connect BLE/USB reader → Insert PIV card
2. Main screen → OpenSC-PKCS11 Test → PIV Demo
3. Run C_Initialize → C_GetSlotList → C_OpenSession → C_Login → C_FindObjects

### 6.2 Desktop Cross-Debug

```bash
./scripts/build_opensc_pkcs11_test.sh
# Android: Start Server, connect reader, insert card
adb forward tcp:35963 tcp:35963
build/opensc-test/run_pkcs11_test.sh -O
```

**Dependencies**: `autoconf automake libtool pkg-config libpcsclite-dev libssl-dev` (Ubuntu) or `brew install pcsc-lite openssl` (macOS).

### 6.3 Android-Side Build (opensc-pkcs11.so)

```bash
./scripts/setup_opensc_android.sh
# Place OpenSSL prebuilt libs in thirdparty/opensc-pkcs11/libs/<abi>/
./gradlew :thirdparty:opensc-pkcs11:assembleDebug
```

### 6.4 PIV 6983 Hint

If `C_OpenSession` fails with 6983 (Conditions not satisfied), use full flow: OpenSession → Login with PIN → FindObjects. Ensure card present and PIN correct.

---

## 7. Debugging

### 7.1 Logcat Tags

| Tag | Component |
|-----|-----------|
| PCSCServer | Server start/stop |
| SocketHandler | READ, WRITE, OPEN, CLOSE, LIST |
| CombinedTransport | readerFind, readerOpen, listReaders |
| BleTransport | BLE discovery |
| UsbTransport | USB discovery |

**Regex filter**: `PCSCServer|SocketHandler|CombinedTransport|BleTransport|UsbTransport|ApiDemo|MainActivity`

---

## 8. Project Structure

```
android-new-sdk-2026/
├── sdk/ftreader-pcsc/   # Native PC/SC (winscard, CCID, socket)
├── sdk/pcsc-server/     # BLE + USB bridge
├── sdk/comm/            # BLE/USB transport
├── app/                 # Demo app
├── thirdparty/opensc-pkcs11/
├── docs/
└── scripts/
```
