/**
 * PC/SC type definitions and constants
 * Compatible with pcsc-lite / OpenSC
 */

#ifndef __wintypes_h__
#define __wintypes_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef BYTE
typedef unsigned char BYTE;
#endif
typedef unsigned char UCHAR;
typedef unsigned char *PUCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef void *LPVOID;
typedef const void *LPCVOID;
typedef unsigned long DWORD;
typedef unsigned long *PDWORD;
typedef long LONG;
typedef const char *LPCSTR;
typedef const BYTE *LPCBYTE;
typedef BYTE *LPBYTE;
typedef DWORD *LPDWORD;
typedef char *LPSTR;
typedef LPSTR LPTSTR;
typedef LPCSTR LPCTSTR;

#define MAX_ATR_SIZE 33

/* Error codes - PC/SC standard */
#define SCARD_S_SUCCESS             ((LONG)0x00000000)
#define SCARD_F_INTERNAL_ERROR      ((LONG)0x80100001)
#define SCARD_E_CANCELLED           ((LONG)0x80100002)
#define SCARD_E_INVALID_HANDLE      ((LONG)0x80100003)
#define SCARD_E_INVALID_PARAMETER   ((LONG)0x80100004)
#define SCARD_E_INVALID_TARGET      ((LONG)0x80100005)
#define SCARD_E_NO_MEMORY           ((LONG)0x80100006)
#define SCARD_F_WAITED_TOO_LONG     ((LONG)0x80100007)
#define SCARD_E_INSUFFICIENT_BUFFER ((LONG)0x80100008)
#define SCARD_E_UNKNOWN_READER      ((LONG)0x80100009)
#define SCARD_E_TIMEOUT             ((LONG)0x8010000A)
#define SCARD_E_SHARING_VIOLATION   ((LONG)0x8010000B)
#define SCARD_E_NO_SMARTCARD        ((LONG)0x8010000C)
#define SCARD_E_UNKNOWN_CARD        ((LONG)0x8010000D)
#define SCARD_E_CANT_DISPOSE        ((LONG)0x8010000E)
#define SCARD_E_PROTO_MISMATCH      ((LONG)0x8010000F)
#define SCARD_E_NOT_READY           ((LONG)0x80100010)
#define SCARD_E_INVALID_VALUE       ((LONG)0x80100011)
#define SCARD_E_SYSTEM_CANCELLED    ((LONG)0x80100012)
#define SCARD_F_COMM_ERROR          ((LONG)0x80100013)
#define SCARD_F_UNKNOWN_ERROR       ((LONG)0x80100014)
#define SCARD_E_INVALID_ATR         ((LONG)0x80100015)
#define SCARD_E_NOT_TRANSACTED      ((LONG)0x80100016)
#define SCARD_E_READER_UNAVAILABLE  ((LONG)0x80100017)
#define SCARD_P_SHUTDOWN            ((LONG)0x80100018)
#define SCARD_E_PCI_TOO_SMALL       ((LONG)0x80100019)
#define SCARD_E_READER_UNSUPPORTED  ((LONG)0x8010001A)
#define SCARD_E_DUPLICATE_READER    ((LONG)0x8010001B)
#define SCARD_E_CARD_UNSUPPORTED    ((LONG)0x8010001C)
#define SCARD_E_NO_SERVICE          ((LONG)0x8010001D)
#define SCARD_E_SERVICE_STOPPED     ((LONG)0x8010001E)
#define SCARD_E_UNEXPECTED          ((LONG)0x8010001F)
#define SCARD_E_UNSUPPORTED_FEATURE ((LONG)0x8010001F)
#define SCARD_E_ICC_INSTALLATION    ((LONG)0x80100020)
#define SCARD_E_ICC_CREATEORDER     ((LONG)0x80100021)
#define SCARD_E_DIR_NOT_FOUND       ((LONG)0x80100023)
#define SCARD_E_FILE_NOT_FOUND      ((LONG)0x80100024)
#define SCARD_E_NO_READERS_AVAILABLE ((LONG)0x8010002E)
#define SCARD_E_COMM_DATA_LOST      ((LONG)0x8010002F)

#define SCARD_W_UNSUPPORTED_CARD    ((LONG)0x80100065)
#define SCARD_W_UNRESPONSIVE_CARD   ((LONG)0x80100066)
#define SCARD_W_UNPOWERED_CARD      ((LONG)0x80100067)
#define SCARD_W_RESET_CARD          ((LONG)0x80100068)
#define SCARD_W_REMOVED_CARD        ((LONG)0x80100069)
#define SCARD_W_SECURITY_VIOLATION  ((LONG)0x8010006A)
#define SCARD_W_WRONG_CHV           ((LONG)0x8010006B)
#define SCARD_W_CHV_BLOCKED         ((LONG)0x8010006C)

#define SCARD_AUTOALLOCATE ((DWORD)(-1))
#define SCARD_SCOPE_USER   0x0000
#define SCARD_SCOPE_TERMINAL 0x0001
#define SCARD_SCOPE_SYSTEM 0x0002

#define SCARD_PROTOCOL_UNDEFINED 0x0000
#define SCARD_PROTOCOL_T0        0x0001
#define SCARD_PROTOCOL_T1        0x0002
#define SCARD_PROTOCOL_RAW       0x0004
#define SCARD_PROTOCOL_ANY       (SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1)

#define SCARD_SHARE_EXCLUSIVE 0x0001
#define SCARD_SHARE_SHARED    0x0002
#define SCARD_SHARE_DIRECT    0x0003

#define SCARD_LEAVE_CARD    0x0000
#define SCARD_RESET_CARD    0x0001
#define SCARD_UNPOWER_CARD  0x0002
#define SCARD_EJECT_CARD    0x0003

#define SCARD_UNKNOWN     0x0001
#define SCARD_ABSENT      0x0002
#define SCARD_PRESENT     0x0004
#define SCARD_SWALLOWED   0x0008
#define SCARD_POWERED     0x0010
#define SCARD_NEGOTIABLE  0x0020
#define SCARD_SPECIFIC    0x0040

#define SCARD_STATE_UNAWARE    0x0000
#define SCARD_STATE_IGNORE     0x0001
#define SCARD_STATE_CHANGED    0x0002
#define SCARD_STATE_UNKNOWN    0x0004
#define SCARD_STATE_UNAVAILABLE 0x0008
#define SCARD_STATE_EMPTY      0x0010
#define SCARD_STATE_PRESENT    0x0020
#define SCARD_STATE_ATRMATCH   0x0040
#define SCARD_STATE_EXCLUSIVE  0x0080
#define SCARD_STATE_INUSE      0x0100
#define SCARD_STATE_MUTE       0x0200
#define SCARD_STATE_UNPOWERED  0x0400

#ifndef INFINITE
#define INFINITE 0xFFFFFFFF
#endif

#define MAX_BUFFER_SIZE         264
#define MAX_BUFFER_SIZE_EXTENDED (4 + 3 + (1<<16) + 3)

#ifdef __cplusplus
}
#endif

#endif
