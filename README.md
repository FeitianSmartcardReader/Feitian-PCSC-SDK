# Feitian PC/SC Reader SDK for Android (2026)

PC/SC winscard implementation for Feitian readers (BLE and USB), compatible with OpenSC and opensc-pkcs11.

**Version**: 3.0.3 (based on Feitian SDK 2.0.1.7)

## Overview

This SDK enables the use case:

```
OpenSC Java Provider → opensc-pkcs11.so → libftreader-pcsc.so → (socket) → PCSCServer → BLE/USB → C45C
```

- **libftreader-pcsc.so**: Native PC/SC winscard implementation (no JNI)
- **PCSCServer**: Android service bridging BLE and USB readers to the native library via local socket
- **BLE + USB**: BLE 4.x (BT4) and USB (OTG) support; BT3 is not supported

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│  Application (Java PKCS#11 / OpenSC)                             │
└────────────────────────────┬────────────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────────────┐
│  opensc-pkcs11.so  (dlopen provider)                             │
└────────────────────────────┬────────────────────────────────────┘
                             │ winscard API
┌────────────────────────────▼────────────────────────────────────┐
│  libftreader-pcsc.so  (PC/SC implementation)                     │
│  - transport_socket.c → connects to PCSCServer                  │
└────────────────────────────┬────────────────────────────────────┘
                             │ TCP localhost:35963
┌────────────────────────────▼────────────────────────────────────┐
│  PCSCServer (Java)                                                │
│  - SocketHandler: READ/WRITE/OPEN/CLOSE/LIST                     │
│  - CombinedTransport: BLE (BT4) + USB (OTG)                      │
└────────────────────────────┬────────────────────────────────────┘
                             │ BLE / USB
┌────────────────────────────▼────────────────────────────────────┐
│  Feitian C45C Reader (BLE or USB)                                │
└─────────────────────────────────────────────────────────────────┘
```

## Quick Start

### 1. Start PCSCServer

Before any PC/SC or PKCS#11 operations, start the PCSCServer:

```java
PCSCServerService.start(context, PCSCServerService.getDefaultPort());
```

Or use the demo app: tap **Start PCSCServer**.

### 2. Initialize Socket Transport

Call this **before** the first SCard* call (e.g. before OpenSC loads the provider):

```java
FTReaderPCSC.initSocket("127.0.0.1", 35963);
```

Or from C:

```c
FTReader_InitSocket("127.0.0.1", 35963);
```

### 3. Use PC/SC or PKCS#11

Configure opensc.conf to use libftreader-pcsc.so as the provider, then use OpenSC Java Provider or any PKCS#11 client as usual.

## Project Structure

```
android-new-sdk-2026/
├── sdk/                        # SDK modules
│   ├── ftreader-pcsc/          # Native libftreader-pcsc.so
│   │   ├── include/            # Public headers (ccid.h, transport.h, winscard.h, wintypes.h)
│   │   └── src/                # ccid.c, transport_socket.c, winscard.c, utils/, main/jni/, main/java/
│   ├── pcsc-server/            # Java BLE + USB bridge service
│   └── comm/                   # BLE/USB transport library
├── app/                        # Demo application
│   └── src/main/java/          # MainActivity, ApiDemoActivity, SettingsActivity, PKCS11TestActivity
├── thirdparty/                 # Third-party integration
│   └── opensc-pkcs11/          # OpenSC PKCS#11 module
├── docs/                       # QUICK_START.md
└── README.md
```

## Build

### Quick Build

```bash
# Build Debug Demo (SDK + opensc-pkcs11)
./gradlew :app:assembleDebug

# Install to device
./scripts/build_install_debug.sh

# Build full SDK package (ZIP + DMG)
./scripts/build_final_release.sh
```

### Prerequisites

- Android SDK (API 21+)
- Android NDK (28.x recommended)
- JDK 11+

### Build Commands

```bash
# Build SDK
./gradlew :sdk:ftreader-pcsc:assembleDebug
./gradlew :sdk:pcsc-server:assembleDebug

# Build demo app
./gradlew :app:assembleDebug

# Build opensc-pkcs11 (optional)
./gradlew :thirdparty:opensc-pkcs11:assembleDebug
```

### Native-Only Build (e.g. Linux/macOS)

```bash
cd sdk/ftreader-pcsc
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
```

## Configuration

| Environment Variable   | Description                    | Default |
|------------------------|--------------------------------|---------|
| `FTREADER_PCSC_HOST`   | PCSCServer host                | 127.0.0.1 |
| `FTREADER_PCSC_PORT`   | PCSCServer port                | 35963 |

## Integration with opensc-pkcs11

1. Configure `opensc.conf` to use libftreader-pcsc.so as the PC/SC provider.
2. Ensure PCSCServer is running before using the provider.
3. Call `FTReader_InitSocket("127.0.0.1", 35963)` before any SCard operations.
4. Use OpenSC Java Provider or PKCS#11 as normal.

### Option A: Desktop Cross-Debug

```bash
./scripts/build_opensc_pkcs11_test.sh
```

Dependencies: `autoconf automake libtool pkg-config libpcsclite-dev libssl-dev` (Ubuntu) or `brew install pcsc-lite openssl` (macOS).

Steps: Android device runs PCSCServer → `adb forward tcp:35963 tcp:35963` → `build/opensc-test/run_pkcs11_test.sh -O`

### Option B: Android-Side Integration

1. Run `./scripts/setup_opensc_android.sh` to fetch OpenSC source
2. Place OpenSSL prebuilt libs in `thirdparty/opensc-pkcs11/libs/<abi>/`
3. Build `./gradlew :app:assembleDebug`
4. Main screen → **PKCS#11 Integration Test** → Run test

## Reader Compatibility

- **BLE**: C4 (C45C), C6, and Corilus readers
- **USB**: Feitian USB readers (Vendor ID 0x096E only). Requires USB host (OTG) and user-granted permission.

See [docs/QUICK_START.md](docs/QUICK_START.md) §3 Reader Compatibility.

## Build Final SDK Package

```bash
./scripts/build_final_release.sh
```

Output: `build/Reader_SDK_YYYYMMDD/` (e.g. Reader_SDK_20250313) with docs, lib, demo, apk, thirdparty; plus `.zip` and `.dmg` (macOS).


## Documentation

| Document | Description |
|----------|-------------|
| [docs/QUICK_START.md](docs/QUICK_START.md) | Quick deployment, integration, API reference, compatibility, build, debugging |

## Changelog

### [3.0.3] - 2026-03

**Added**
- Release build script (`scripts/build_release.sh`) with organized output: apk/, aar/, include/, docs/
- Final SDK package script (`scripts/build_final_release.sh`) producing full distribution: docs, lib, demo, apk, thirdparty
- ProGuard rules to strip debug Log calls in release builds
- BLE CCID debug forced off in release (SettingsPrefs)

**Changed**
- Release build: minifyEnabled, shrinkResources for smaller APK
- PC/SC reader list format: added trailing null per spec (fixes OpenSC C_GetSlotList crash)
- CombinedTransport.toPcscFormat: UTF-8 encoding, proper multi-string termination

**Fixed**
- SIGSEGV in sc_ctx_detect_readers when parsing reader list (trailing null in PC/SC multi-string format)
- LOCK_COMM around SCardListReaders to avoid concurrent socket access

### [3.0.2] - 2026-03

**Added**
- USB support: PCSCServer supports both BLE and USB readers
- CombinedTransport: lists and opens BLE + USB readers
- Socket protocol: OPEN response includes device type (TYPE_USB/TYPE_BT4) for native ccid

### [3.0.1] - 2026-03

**Added**
- Version info: PC/SC SDK 3.0.1 (based on Feitian SDK 2.0.1.7)
- FtGetLibVersion returns full version string

### [3.0.0] - 2026-03

**Added**
- Initial PC/SC SDK for C45C BLE readers
- libftreader-pcsc.so with socket transport
- PCSCServer (BLE bridge) with BT4 support
- Demo app: Start/Stop PCSCServer, Init for PKCS#11, Get Device Info (private commands)
- Reader compatibility: C4, C6, Corilus
- BLE data fragmentation (20-byte MTU) handled by BT4
- Feitian private commands: FtGetDevVer, FtGetSerialNum, FtGetAccessoryManufacturer, FtGetReaderName, FtGetAccessoryModelName

## License

Based on Feitian SDK and GNU CCID. See source headers for details.
