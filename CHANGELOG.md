# Changelog

All notable changes to the Feitian PC/SC SDK for Android are documented in this file.

## [3.0.3] - 2026-03

### Added
- Release build script (`scripts/build_release.sh`) with organized output: apk/, aar/, include/, docs/
- Final SDK package script (`scripts/build_final_release.sh`) producing full distribution: docs, lib, demo, apk, thirdparty
- ProGuard rules to strip debug Log calls in release builds
- BLE CCID debug forced off in release (SettingsPrefs)

### Changed
- Release build: minifyEnabled, shrinkResources for smaller APK
- PC/SC reader list format: added trailing null per spec (fixes OpenSC C_GetSlotList crash)
- CombinedTransport.toPcscFormat: UTF-8 encoding, proper multi-string termination

### Fixed
- SIGSEGV in sc_ctx_detect_readers when parsing reader list (trailing null in PC/SC multi-string format)
- LOCK_COMM around SCardListReaders to avoid concurrent socket access

---

## [3.0.2] - 2026-03

### Added
- USB support: PCSCServer supports both BLE and USB readers
- CombinedTransport: lists and opens BLE + USB readers
- Socket protocol: OPEN response includes device type (TYPE_USB/TYPE_BT4) for native ccid

---

## [3.0.1] - 2026-03

### Added
- Version info: PC/SC SDK 3.0.1 (based on Feitian SDK 2.0.1.7)
- FtGetLibVersion returns full version string
- Changelog in README

---

## [3.0.0] - 2026-03

### Added
- Initial PC/SC SDK for C45C BLE readers
- libftreader-pcsc.so with socket transport
- PCSCServer (BLE bridge) with BT4 support
- Demo app: Start/Stop PCSCServer, Init for PKCS#11, Get Device Info (private commands)
- Reader compatibility: C4, C6, Corilus
- BLE data fragmentation (20-byte MTU) handled by BT4
- Feitian private commands: FtGetDevVer, FtGetSerialNum, FtGetAccessoryManufacturer, FtGetReaderName, FtGetAccessoryModelName
