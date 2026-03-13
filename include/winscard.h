/**
 * PC/SC winscard API - OpenSC compatible
 * Compatible with opensc-pkcs11.so (dlopen provider)
 */

#ifndef __winscard_h__
#define __winscard_h__

#include <stdint.h>
#include "wintypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PCSC_API
#define PCSC_API
#endif

/* Protocol Control Information (PCI) */
typedef struct _SCARD_IO_REQUEST {
    uint32_t dwProtocol;
    uint32_t cbPciLength;
} SCARD_IO_REQUEST, *PSCARD_IO_REQUEST, *LPSCARD_IO_REQUEST;
typedef const SCARD_IO_REQUEST *LPCSCARD_IO_REQUEST;

extern SCARD_IO_REQUEST g_rgSCardT0Pci, g_rgSCardT1Pci, g_rgSCardRawPci;

#define SCARD_PCI_T0  (&g_rgSCardT0Pci)
#define SCARD_PCI_T1  (&g_rgSCardT1Pci)
#define SCARD_PCI_RAW (&g_rgSCardRawPci)

typedef LONG SCARDCONTEXT;
typedef SCARDCONTEXT *LPSCARDCONTEXT;
typedef LONG SCARDHANDLE;
typedef SCARDHANDLE *LPSCARDHANDLE;

/* pcsc-lite compatible - no isConnected */
typedef struct {
    const char *szReader;
    const void *pvUserData;
    DWORD dwCurrentState;
    DWORD dwEventState;
    DWORD cbAtr;
    unsigned char rgbAtr[MAX_ATR_SIZE];
} SCARD_READERSTATE, *LPSCARD_READERSTATE;

/* Optional: init for socket transport (use before first SCard call) */
void FTReader_InitSocket(const char* host, int port);
void FTReader_Uninit(void);

/* Feitian private commands (optional, for device info / testing) */
void FtGetLibVersion(char *buffer);
LONG FtGetDevVer(SCARDCONTEXT hContext, char *firmwareRevision, char *hardwareRevision);
LONG FtGetSerialNum(SCARDCONTEXT hContext, unsigned int *length, char *buffer);
LONG FtGetAccessoryManufacturer(SCARDCONTEXT hContext, unsigned int *length, char *buffer);
LONG FtGetAccessoryModelName(SCARDCONTEXT hContext, unsigned int *length, char *buffer);
LONG FtGetReaderName(SCARDCONTEXT hContext, unsigned int *length, char *buffer);

/* PC/SC API */
PCSC_API LONG SCardEstablishContext(DWORD dwScope, LPCVOID pvReserved1, LPCVOID pvReserved2, LPSCARDCONTEXT phContext);
PCSC_API LONG SCardReleaseContext(SCARDCONTEXT hContext);
PCSC_API LONG SCardIsValidContext(SCARDCONTEXT hContext);
PCSC_API LONG SCardConnect(SCARDCONTEXT hContext, LPCSTR szReader, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol);
PCSC_API LONG SCardDisconnect(SCARDHANDLE hCard, DWORD dwDisposition);
PCSC_API LONG SCardBeginTransaction(SCARDHANDLE hCard);
PCSC_API LONG SCardEndTransaction(SCARDHANDLE hCard, DWORD dwDisposition);
PCSC_API LONG SCardStatus(SCARDHANDLE hCard, LPSTR mszReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen);
PCSC_API LONG SCardGetStatusChange(SCARDCONTEXT hContext, DWORD dwTimeout, LPSCARD_READERSTATE rgReaderStates, DWORD cReaders);
PCSC_API LONG SCardCancel(SCARDCONTEXT hContext);
PCSC_API LONG SCardControl(SCARDHANDLE hCard, DWORD dwControlCode, LPCVOID pbSendBuffer, DWORD cbSendLength, LPVOID pbRecvBuffer, DWORD cbRecvLength, LPDWORD lpBytesReturned);
PCSC_API LONG SCardTransmit(SCARDHANDLE hCard, const SCARD_IO_REQUEST *pioSendPci, LPCBYTE pbSendBuffer, DWORD cbSendLength, SCARD_IO_REQUEST *pioRecvPci, LPBYTE pbRecvBuffer, LPDWORD pcbRecvLength);
PCSC_API LONG SCardListReaderGroups(SCARDCONTEXT hContext, LPSTR mszGroups, LPDWORD pcchGroups);
PCSC_API LONG SCardListReaders(SCARDCONTEXT hContext, LPCSTR mszGroups, LPSTR mszReaders, LPDWORD pcchReaders);
PCSC_API LONG SCardReconnect(SCARDHANDLE hCard, DWORD dwShareMode, DWORD dwPreferredProtocols, DWORD dwInitialization, LPDWORD pdwActiveProtocol);
PCSC_API LONG SCardGetAttrib(SCARDHANDLE hCard, DWORD dwAttrId, LPBYTE pbAttr, LPDWORD pcbAttrLen);

#ifdef __cplusplus
}
#endif

#endif
