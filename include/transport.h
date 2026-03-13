/**
 * Transport abstraction - replaces commManager for socket mode
 * No JNI - used by ft_ccid and winscard
 */

#ifndef __transport_h__
#define __transport_h__

#include "wintypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_USB 0
#define TYPE_BT3 1
#define TYPE_BT4 2

typedef struct {
    int (*readData)(int slotIndex, int timeout, unsigned char* data, unsigned int* dataLen);
    int (*writeData)(int slotIndex, unsigned char* data, unsigned int dataLen);
    int (*readerFind)(void);
    int (*readerOpen)(const char* readerName, int* slotIndex);
    int (*readerClose)(void);
    int (*listReaders)(char* readers, LPDWORD readersLen);
    int (*getDeviceType)(void);
} FTReader_TransportOps;

void FTReader_SetTransport(FTReader_TransportOps* ops);

/* Compatibility API - same signatures as commManager */
void setSlotIndex(int slotIndex);
int getSlotIndex(void);
int getDeviceType(void);
void setConnecting(int isConnecting);

int readData(int slotIndex, int timeout, unsigned char* data, unsigned int* dataLen);
int writeData(int slotIndex, unsigned char* data, unsigned int dataLen);
int readerFind(void);
int readerOpen(unsigned char* readerName, int* slotIndex);
int readerClose(void);
int listReaders(unsigned char* readers, LPDWORD readersLen);
int getBleVersion(void);

#ifdef __cplusplus
}
#endif

#endif
