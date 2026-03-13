/**
 * CCID protocol - types and declarations
 * Based on GNU CCID (http://pcsclite.alioth.debian.org/ccid.html)
 * Copyright (C) Feitian 2014
 */
#ifndef __CCID_H__
#define __CCID_H__

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "wintypes.h"
#include "winscard.h"
#include "transport.h"
#include "utils/proto-t1.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Version */
#define FT_ANDROID_SDK_VER_MAJOR     2
#define FT_ANDROID_SDK_VER_MINOR     0
#define FT_ANDROID_SDK_VER_PATCH     1
#define FT_ANDROID_SDK_VER_CODEFIX   7
#define FT_STR_HELPER(x) #x
#define FT_STR(x) FT_STR_HELPER(x)
#define FT_ANDROID_SDK_VER_STR FT_STR(FT_ANDROID_SDK_VER_MAJOR) "." FT_STR(FT_ANDROID_SDK_VER_MINOR) "." FT_STR(FT_ANDROID_SDK_VER_PATCH) "." FT_STR(FT_ANDROID_SDK_VER_CODEFIX)

#define FTREADER_PCSC_VER_MAJOR      3
#define FTREADER_PCSC_VER_MINOR      0
#define FTREADER_PCSC_VER_PATCH      1
#define FTREADER_PCSC_VER_STR        FT_STR(FTREADER_PCSC_VER_MAJOR) "." FT_STR(FTREADER_PCSC_VER_MINOR) "." FT_STR(FTREADER_PCSC_VER_PATCH)
#define FTREADER_PCSC_FULL_VER_STR   FTREADER_PCSC_VER_STR " (FT " FT_ANDROID_SDK_VER_STR ")"

typedef long RESPONSECODE;
#define CCID_DRIVER_MAX_READERS 4

typedef enum FTREADER_INTERNAL {
    FT_READER_DEFAULT = 0x00,
    FT_READER_UA = 0x01, FT_READER_UB = 0x02, FT_READER_UB_LT = 0x03,
    FT_READER_UC = 0x04, FT_READER_UC_LT = 0x05, FT_READER_UC_B = 0x06, FT_READER_UC_LT_B = 0x07,
    FT_READER_UM = 0x08, FT_READER_UD = 0x09, FT_READER_UD_LT = 0x0A,
} FTREADER_INTERNAL;

#define ReadPort ReadSerial
#define WritePort WriteSerial

#define LONG_TIMEOUT       (30 * 1000)
#define DEFAULT_TIMEOUT    (3 * 1000)
#define BLE_DEFAULT_TIMEOUT (5 * 1000)  /* BLE needs >=5s; used when TYPE_BT4 */
#define DEFAULT_COM_READ_TIMEOUT 30000

#define T_0   0
#define T_1   1
#define T_RAW 3

#define PROTOCOL_CCID   0
#define PROTOCOL_ICCD_A 1
#define PROTOCOL_ICCD_B 2

#define CCID_CLASS_AUTO_CONF_ATR    0x00000002
#define CCID_CLASS_AUTO_VOLTAGE     0x00000008
#define CCID_CLASS_AUTO_BAUD        0x00000020
#define CCID_CLASS_AUTO_PPS_PROP    0x00000040
#define CCID_CLASS_AUTO_PPS_CUR     0x00000080
#define CCID_CLASS_AUTO_IFSD        0x00000400
#define CCID_CLASS_CHARACTER        0x00000000
#define CCID_CLASS_TPDU             0x00010000
#define CCID_CLASS_SHORT_APDU       0x00020000
#define CCID_CLASS_EXTENDED_APDU    0x00040000
#define CCID_CLASS_EXCHANGE_MASK    0x00070000
#define CCID_CLASS_PIN_VERIFY       0x01
#define CCID_CLASS_PIN_MODIFY       0x02

/* CCID message types (bMessageType, USB CCID Rev 1.1 §6.1) */
#define CCID_MSG_ICCPOWERON    0x62
#define CCID_MSG_ICCPOWEROFF   0x63
#define CCID_MSG_GETSLOTSTATUS 0x65
#define CCID_MSG_SETPARAMETERS 0x61
#define CCID_MSG_XFRBLOCK      0x6F
#define CCID_MSG_ESCAPE        0x6B

#define MAX_BUFFER_SIZE_ENC  1024
#define CMD_BUF_SIZE         MAX_BUFFER_SIZE_EXTENDED
#define CMD_BUF_SIZE_EACH    (MAX_BUFFER_SIZE_ENC + 10)

typedef enum {
    STATUS_NO_SUCH_DEVICE = 0xF9,
    STATUS_SUCCESS = 0xFA,
    STATUS_UNSUCCESSFUL = 0xFB,
    STATUS_COMM_ERROR = 0xFC,
    STATUS_DEVICE_PROTOCOL_ERROR = 0xFD,
    STATUS_COMM_NAK = 0xFE,
    STATUS_SECONDARY_SLOT = 0xFF
} status_t;

typedef struct {
    unsigned char real_bSeq;
    int readerID;
    unsigned int dwMaxCCIDMessageLength;
    int dwMaxIFSD;
    int dwFeatures;
    char bPINSupport;
    unsigned int wLcdLayout;
    int dwDefaultClock;
    unsigned int dwMaxDataRate;
    char bMaxSlotIndex;
    char bCurrentSlotIndex;
    unsigned int *arrayOfSupportedDataRates;
    unsigned int readTimeout;
    int cardProtocol;
    int bInterfaceProtocol;
    int dwSlotStatus;
    int bVoltageSupport;
} _ccid_descriptor;

#define CCID_ICC_PRESENT_ACTIVE   0x00
#define CCID_ICC_PRESENT_INACTIVE 0x01
#define CCID_ICC_ABSENT           0x02
#define CCID_ICC_STATUS_MASK      0x03
#define CCID_COMMAND_FAILED       0x40
#define CCID_TIME_EXTENSION       0x80

#define CCID_HEADER_LEN         10   /* All CCID bulk messages: bMessageType(1)+dwLength(4)+bSlot(1)+bSeq(1)+3 */
#define BSLOT_OFFSET            5
#define BSEQ_OFFSET             6
#define STATUS_OFFSET           7
#define ERROR_OFFSET            8
#define CHAIN_PARAMETER_OFFSET  9
#define SIZE_GET_SLOT_STATUS    10

#define dw2i(a, x) (unsigned int)((((((a[x+3]<<8)+a[x+2])<<8)+a[x+1])<<8)+a[x])

#define IFD_PARITY_ERROR 699
#define IFD_SUCCESS      0
#define IFD_NEGOTIATE_PTS1 1
#define IFD_NEGOTIATE_PTS2 2
#define IFD_NEGOTIATE_PTS3 4
#define IFD_POWER_UP      500
#define IFD_POWER_DOWN    501
#define IFD_RESET         502
#define IFD_ERROR_TAG     600
#define IFD_ERROR_SET_FAILURE 601
#define IFD_ERROR_VALUE_READ_ONLY 602
#define IFD_ERROR_PTS_FAILURE 605
#define IFD_ERROR_NOT_SUPPORTED 606
#define IFD_PROTOCOL_NOT_SUPPORTED 607
#define IFD_ERROR_POWER_ACTION 608
#define IFD_ERROR_SWALLOW 609
#define IFD_ERROR_EJECT 610
#define IFD_ERROR_CONFISCATE 611
#define IFD_COMMUNICATION_ERROR 612
#define IFD_RESPONSE_TIMEOUT 613
#define IFD_NOT_SUPPORTED 614
#define IFD_ICC_PRESENT 615
#define IFD_ICC_NOT_PRESENT 616
#define IFD_NO_SUCH_DEVICE 617
#define IFD_ERROR_INSUFFICIENT_BUFFER 618

#define PPS_OK 0
#define PPS_ICC_ERROR 1
#define PPS_HANDSAKE_ERROR 2
#define PPS_PROTOCOL_ERROR 3
#define PPS_MAX_LENGTH 6
#define PPS_HAS_PPS1(block)	((block[1] & 0x10) == 0x10)
#define PPS_HAS_PPS2(block)	((block[1] & 0x20) == 0x20)
#define PPS_HAS_PPS3(block)	((block[1] & 0x40) == 0x40)

#define ATR_OK        0
#define ATR_NOT_FOUND 1
#define ATR_MALFORMED 2
#define ATR_IO_ERROR  3
#define ATR_MAX_SIZE 33
#define ATR_MAX_HISTORICAL 15
#define ATR_MAX_PROTOCOLS 7
#define ATR_MAX_IB 4
#define ATR_CONVENTION_DIRECT 0
#define ATR_CONVENTION_INVERSE 1
#define ATR_PROTOCOL_TYPE_T0 0
#define ATR_PROTOCOL_TYPE_T1 1
#define ATR_PROTOCOL_TYPE_T2 2
#define ATR_PROTOCOL_TYPE_T3 3
#define ATR_PROTOCOL_TYPE_T14 14
#define ATR_INTERFACE_BYTE_TA 0
#define ATR_INTERFACE_BYTE_TB 1
#define ATR_INTERFACE_BYTE_TC 2
#define ATR_INTERFACE_BYTE_TD 3
#define ATR_PARAMETER_F 0
#define ATR_PARAMETER_D 1
#define ATR_PARAMETER_I 2
#define ATR_PARAMETER_P 3
#define ATR_PARAMETER_N 4
#define ATR_INTEGER_VALUE_FI 0
#define ATR_INTEGER_VALUE_DI 1
#define ATR_INTEGER_VALUE_II 2
#define ATR_INTEGER_VALUE_PI1 3
#define ATR_INTEGER_VALUE_N 4
#define ATR_INTEGER_VALUE_PI2 5

#define ATR_DEFAULT_D 1
#define ATR_DEFAULT_N 0
#define ATR_DEFAULT_P 5
#define ATR_DEFAULT_I 50
#define ATR_DEFAULT_F 372

typedef struct {
    unsigned length;
    BYTE TS;
    BYTE T0;
    struct { BYTE value; bool present; } ib[ATR_MAX_PROTOCOLS][ATR_MAX_IB], TCK;
    unsigned pn;
    BYTE hb[ATR_MAX_HISTORICAL];
    unsigned hbn;
} ATR_t;

typedef struct CCID_DESC {
    int nATRLength;
    UCHAR pcATRBuffer[MAX_ATR_SIZE];
    UCHAR bPowerFlags;
    t1_state_t t1;
    char *readerName;
} CcidDesc;

#define POWERFLAGS_RAZ 0x00
#define MASK_POWERFLAGS_PUP 0x01
#define MASK_POWERFLAGS_PDWN 0x02
#define VOLTAGE_AUTO 0
#define VOLTAGE_5V 1
#define VOLTAGE_3V 2
#define VOLTAGE_1_8V 3

extern RESPONSECODE CCID_Transmit(unsigned int reader_index, unsigned int tx_length,
    const unsigned char tx_buffer[], unsigned short rx_length, unsigned char bBWI);
extern RESPONSECODE CCID_Receive(unsigned int reader_index, unsigned int *rx_length,
    unsigned char rx_buffer[], unsigned char *chain_parameter);
extern int isCharLevel(int reader_index);
extern RESPONSECODE CmdPowerOn(unsigned int reader_index, unsigned int *nlength,
    unsigned char buffer[], int voltage);
extern RESPONSECODE CmdPowerOff(unsigned int reader_index);
extern RESPONSECODE SetParameters(unsigned int reader_index, char protocol,
    unsigned int length, unsigned char buffer[]);
extern _ccid_descriptor *get_ccid_descriptor(unsigned int reader_index);
extern CcidDesc *get_ccid_slot(unsigned int reader_index);
extern RESPONSECODE CmdXfrBlock(unsigned int reader_index, unsigned int tx_length,
    unsigned char tx_buffer[], unsigned int *rx_length, unsigned char rx_buffer[], int protocol);
extern int ATR_InitFromArray(ATR_t *atr, const BYTE atr_buffer[ATR_MAX_SIZE], unsigned length);
extern int ATR_GetIntegerValue(ATR_t *atr, int name, BYTE *value);
extern int ATR_GetDefaultProtocol(ATR_t *atr, int *protocol);
extern int ATR_GetConvention(ATR_t *atr, int *convention);
extern int ATR_GetParameter(ATR_t *atr, int name, double *parameter);
extern RESPONSECODE Scrd_Negotiate(unsigned int reader_index, int type);
extern RESPONSECODE CmdGetSlotStatus(unsigned int reader_index, unsigned char buffer[]);
extern RESPONSECODE CmdGetSerialNum(unsigned int reader_index, unsigned int *pnlength, unsigned char buffer[]);
extern RESPONSECODE CmdWriteFlash(unsigned int reader_index, unsigned char bOffset, unsigned char blength, unsigned char buffer[]);
extern RESPONSECODE CmdReadFlash(unsigned int reader_index, unsigned char bOffset, unsigned char *blength, unsigned char buffer[]);
extern RESPONSECODE CmdGetDevInfo(unsigned int reader_index, unsigned int *pnlength, unsigned char buffer[]);
extern RESPONSECODE CmdGetDevVer(unsigned int reader_index, char *firmwareRevision, char *hardwareRevision);
extern RESPONSECODE CmdEscape(unsigned int reader_index, const unsigned char TxBuffer[], unsigned int TxLength,
    unsigned char RxBuffer[], unsigned int *RxLength);
extern RESPONSECODE CmdGenerateDeviceUID(unsigned int reader_index, unsigned int pnlength, unsigned char buffer[]);
extern RESPONSECODE CmdGetDeviceUID(unsigned int reader_index, unsigned int *pnlength, unsigned char buffer[]);
extern RESPONSECODE CmdGetCCIDMaxBlockLength(int reader_index, int *maxLength);
extern RESPONSECODE CmdEscapeDeviceUID(unsigned int reader_index, unsigned int pnlength, unsigned char buffer[]);
extern RESPONSECODE cmdAutoTurnOffReader(bool isOpen);
extern RESPONSECODE CmdGetDeviceHID(unsigned int reader_index, unsigned int *pnlength, unsigned char buffer[]);
extern RESPONSECODE CmdGetAccessoryModeNumber(unsigned int reader_index, unsigned int *pnlength, unsigned char buffer[]);
extern RESPONSECODE CmdGetAccessoryManufacturer(unsigned int reader_index, unsigned int *pnlength, char buffer[]);
extern RESPONSECODE CmdGetReaderBluetoothID(unsigned int reader_index, unsigned int *pnlength, unsigned char buffer[]);
extern RESPONSECODE CmdGetReaderName(unsigned int reader_index, unsigned int *pnlength, unsigned char buffer[]);
extern RESPONSECODE CmdAnalysis(unsigned int reader_index, unsigned int tx_length, unsigned char tx_buffer[],
    unsigned int *rx_length, unsigned char rx_buffer[], int protocol);

extern void ccid_error(int error, const char *file, int line, const char *function);
extern void ccid_log_comm_error(const char *func, int line, const char *ctx, const unsigned char *buf, unsigned int len);

#ifdef DEBUG
#define CCID_LOG_ERR(ctx, buf, len) ccid_log_comm_error(__func__, __LINE__, ctx, (const unsigned char *)(buf), (unsigned int)(len))
#else
#define CCID_LOG_ERR(ctx, buf, len) ((void)0)
#endif

/* Shared state (defined in ccid.c) */
extern bool gIsOpen;
extern unsigned int gDevType;
extern unsigned int gIsReadData;

#ifdef __cplusplus
}
#endif

#endif /* __CCID_H__ */
