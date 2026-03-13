/**
 * JNI: PKCS#11 wrapper - opensc-pkcs11 + libftreader-pcsc.
 * Prerequisites: PCSCServer running, FTReader_InitSocket.
 * Supports PIV and other OpenSC cards.
 */
#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <stdio.h>
#include <android/log.h>

#define LOG_TAG "PKCS11Demo"
/* Use INFO level so logs are visible with default logcat filter */
#define PKCS11_LOGD(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define PKCS11_FALSE 0
#define CKF_SERIAL_SESSION 4
#define CKF_RW_SESSION 2
#define CKU_USER 1

typedef unsigned long CK_ULONG;
typedef long CK_RV;
#define CKR_OK 0
#define CKR_CRYPTOKI_NOT_INITIALIZED 0x00000190UL

typedef struct { unsigned char major, minor; } CK_VERSION;
typedef CK_RV (*CK_C_Initialize)(void *initArgs);
typedef CK_RV (*CK_C_Finalize)(void *reserved);
typedef CK_RV (*CK_C_GetSlotList)(unsigned char tokenPresent, CK_ULONG *slotList, CK_ULONG *count);
typedef CK_RV (*CK_C_GetSlotInfo)(CK_ULONG slotID, void *pInfo);
typedef CK_RV (*CK_C_GetTokenInfo)(CK_ULONG slotID, void *pInfo);
typedef CK_RV (*CK_C_OpenSession)(CK_ULONG slotID, CK_ULONG flags, void *pApplication, void *notify, CK_ULONG *phSession);
typedef CK_RV (*CK_C_CloseSession)(CK_ULONG hSession);
typedef CK_RV (*CK_C_Login)(CK_ULONG hSession, CK_ULONG userType, unsigned char *pPin, CK_ULONG ulPinLen);
typedef CK_RV (*CK_C_Logout)(CK_ULONG hSession);
typedef CK_RV (*CK_C_FindObjectsInit)(CK_ULONG hSession, void *pTemplate, CK_ULONG ulCount);
typedef CK_RV (*CK_C_FindObjects)(CK_ULONG hSession, CK_ULONG *phObject, CK_ULONG ulMaxObjectCount, CK_ULONG *pulObjectCount);
typedef CK_RV (*CK_C_FindObjectsFinal)(CK_ULONG hSession);
typedef CK_RV (*CK_C_GetMechanismList)(CK_ULONG slotID, CK_ULONG *pMechanismList, CK_ULONG *pulCount);
typedef CK_RV (*CK_C_GetMechanismInfo)(CK_ULONG slotID, CK_ULONG type, void *pInfo);
typedef CK_RV (*CK_C_GetSessionInfo)(CK_ULONG hSession, void *pInfo);
typedef CK_RV (*CK_C_GetAttributeValue)(CK_ULONG hSession, CK_ULONG hObject, void *pTemplate, CK_ULONG ulCount);
typedef CK_RV (*CK_C_SignInit)(CK_ULONG hSession, void *pMechanism, CK_ULONG hKey);
typedef CK_RV (*CK_C_Sign)(CK_ULONG hSession, unsigned char *pData, CK_ULONG ulDataLen, unsigned char *pSignature, CK_ULONG *pulSignatureLen);
typedef CK_RV (*CK_C_DigestInit)(CK_ULONG hSession, void *pMechanism);
typedef CK_RV (*CK_C_Digest)(CK_ULONG hSession, unsigned char *pData, CK_ULONG ulDataLen, unsigned char *pDigest, CK_ULONG *pulDigestLen);
typedef CK_RV (*CK_C_VerifyInit)(CK_ULONG hSession, void *pMechanism, CK_ULONG hKey);
typedef CK_RV (*CK_C_Verify)(CK_ULONG hSession, unsigned char *pData, CK_ULONG ulDataLen, unsigned char *pSignature, CK_ULONG ulSignatureLen);
typedef CK_RV (*CK_C_DecryptInit)(CK_ULONG hSession, void *pMechanism, CK_ULONG hKey);
typedef CK_RV (*CK_C_Decrypt)(CK_ULONG hSession, unsigned char *pCipher, CK_ULONG ulCipherLen, unsigned char *pData, CK_ULONG *pulDataLen);
typedef CK_RV (*CK_C_EncryptInit)(CK_ULONG hSession, void *pMechanism, CK_ULONG hKey);
typedef CK_RV (*CK_C_Encrypt)(CK_ULONG hSession, unsigned char *pData, CK_ULONG ulDataLen, unsigned char *pCipher, CK_ULONG *pulCipherLen);
typedef CK_RV (*CK_C_GenerateRandom)(CK_ULONG hSession, unsigned char *pRandom, CK_ULONG ulRandomLen);

typedef struct {
    CK_VERSION version;
    CK_C_Initialize C_Initialize;
    CK_C_Finalize C_Finalize;
    void *C_GetInfo;
    void *C_GetFunctionList;
    CK_C_GetSlotList C_GetSlotList;
    CK_C_GetSlotInfo C_GetSlotInfo;
    CK_C_GetTokenInfo C_GetTokenInfo;
    CK_C_OpenSession C_OpenSession;
    CK_C_CloseSession C_CloseSession;
    CK_C_Login C_Login;
    CK_C_Logout C_Logout;
    CK_C_FindObjectsInit C_FindObjectsInit;
    CK_C_FindObjects C_FindObjects;
    CK_C_FindObjectsFinal C_FindObjectsFinal;
} CK_FUNCTION_LIST;

/* Optional APIs loaded via dlsym (OpenSC struct layout may differ) */
static CK_C_GetMechanismList g_C_GetMechanismList;
static CK_C_GetMechanismInfo g_C_GetMechanismInfo;
static CK_C_GetSessionInfo g_C_GetSessionInfo;
static CK_C_GetAttributeValue g_C_GetAttributeValue;
static CK_C_SignInit g_C_SignInit;
static CK_C_Sign g_C_Sign;
static CK_C_DigestInit g_C_DigestInit;
static CK_C_Digest g_C_Digest;
static CK_C_VerifyInit g_C_VerifyInit;
static CK_C_Verify g_C_Verify;
static CK_C_DecryptInit g_C_DecryptInit;
static CK_C_Decrypt g_C_Decrypt;
static CK_C_EncryptInit g_C_EncryptInit;
static CK_C_Encrypt g_C_Encrypt;
static CK_C_GenerateRandom g_C_GenerateRandom;

/* CK_MECHANISM: type + pParameter (NULL for common mechs) */
typedef struct { CK_ULONG type; void *pParameter; CK_ULONG ulParameterLen; } CK_MECHANISM_DEMO;
/* CK_ATTRIBUTE for C_GetAttributeValue */
typedef struct { CK_ULONG type; void *pValue; CK_ULONG ulValueLen; } CK_ATTRIBUTE_DEMO;
/* CK_MECHANISM_INFO: ulMinKeySize, ulMaxKeySize, flags */
typedef struct { CK_ULONG ulMinKeySize; CK_ULONG ulMaxKeySize; CK_ULONG flags; } CK_MECHANISM_INFO_DEMO;
/* CK_SESSION_INFO: slotID, state, flags, ulDeviceError */
typedef struct { CK_ULONG slotID; CK_ULONG state; CK_ULONG flags; CK_ULONG ulDeviceError; } CK_SESSION_INFO_DEMO;
#define CKM_SHA256_RSA_PKCS 0x00000040UL
#define CKM_ECDSA 0x00001041UL
#define CKM_SHA256 0x00000250UL
#define CKM_RSA_PKCS 0x00000001UL
#define CKA_VALUE 0x00000011UL
#define CKR_BUFFER_TOO_SMALL 0x00000150UL

typedef CK_RV (*CK_C_GetFunctionList)(CK_FUNCTION_LIST **ppFunctionList);

/* PKCS#11 Demo shared state */
static void *g_handle = NULL;
static CK_FUNCTION_LIST *g_funcs = NULL;
static int g_initialized = 0;  /* C_Initialize was called and succeeded */

static const char *load_pkcs11_error(void) {
    const char *e = dlerror();
    return (e && e[0]) ? e : "libopensc-pkcs11.so not found";
}

/* Set OPENSC_DEBUG before C_Initialize so OpenSC reads it. Call from Java before any PKCS11 operation. */
JNIEXPORT void JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeSetOpenSCDebug(JNIEnv *env, jclass clazz, jint level) {
    (void)env;
    (void)clazz;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", level >= 0 ? level : 9);
    setenv("OPENSC_DEBUG", buf, 1);
    PKCS11_LOGD("nativeSetOpenSCDebug: OPENSC_DEBUG=%s", buf);
}

static int load_pkcs11(void) {
    if (g_handle && g_funcs) return 0;
    if (g_handle) { dlclose(g_handle); g_handle = NULL; }
    g_handle = dlopen("libopensc-pkcs11.so", RTLD_NOW | RTLD_LOCAL);
    if (!g_handle) return -1;
    CK_C_GetFunctionList C_GetFunctionList = (CK_C_GetFunctionList) dlsym(g_handle, "C_GetFunctionList");
    {
        Dl_info info;
        if (C_GetFunctionList && dladdr((void *)C_GetFunctionList, &info) && info.dli_fname)
            PKCS11_LOGD("load_pkcs11: lib from %s", info.dli_fname);
        else
            PKCS11_LOGD("load_pkcs11: handle=%p C_GetFunctionList=%p", (void *)g_handle, (void *)C_GetFunctionList);
    }
    if (!C_GetFunctionList) { dlclose(g_handle); g_handle = NULL; return -2; }
    CK_RV rv = C_GetFunctionList(&g_funcs);
    if (rv != CKR_OK || !g_funcs) { dlclose(g_handle); g_handle = NULL; return -3; }
    g_C_GetMechanismList = (CK_C_GetMechanismList) dlsym(g_handle, "C_GetMechanismList");
    g_C_GetMechanismInfo = (CK_C_GetMechanismInfo) dlsym(g_handle, "C_GetMechanismInfo");
    g_C_GetSessionInfo = (CK_C_GetSessionInfo) dlsym(g_handle, "C_GetSessionInfo");
    g_C_GetAttributeValue = (CK_C_GetAttributeValue) dlsym(g_handle, "C_GetAttributeValue");
    g_C_SignInit = (CK_C_SignInit) dlsym(g_handle, "C_SignInit");
    g_C_Sign = (CK_C_Sign) dlsym(g_handle, "C_Sign");
    g_C_DigestInit = (CK_C_DigestInit) dlsym(g_handle, "C_DigestInit");
    g_C_Digest = (CK_C_Digest) dlsym(g_handle, "C_Digest");
    g_C_VerifyInit = (CK_C_VerifyInit) dlsym(g_handle, "C_VerifyInit");
    g_C_Verify = (CK_C_Verify) dlsym(g_handle, "C_Verify");
    g_C_DecryptInit = (CK_C_DecryptInit) dlsym(g_handle, "C_DecryptInit");
    g_C_Decrypt = (CK_C_Decrypt) dlsym(g_handle, "C_Decrypt");
    g_C_EncryptInit = (CK_C_EncryptInit) dlsym(g_handle, "C_EncryptInit");
    g_C_Encrypt = (CK_C_Encrypt) dlsym(g_handle, "C_Encrypt");
    g_C_GenerateRandom = (CK_C_GenerateRandom) dlsym(g_handle, "C_GenerateRandom");
    return 0;
}

static void format_pkcs11_error(char *buf, size_t sz, const char *api, CK_RV rv) {
    if (rv == (CK_RV)CKR_CRYPTOKI_NOT_INITIALIZED)
        snprintf(buf, sz, "%s failed: 0x%08lX (CKR_CRYPTOKI_NOT_INITIALIZED - call C_Initialize first)", api, (unsigned long)rv);
    else
        snprintf(buf, sz, "%s failed: 0x%08lX", api, (unsigned long)rv);
}

static const char *ckr_name(CK_RV rv) {
    switch (rv) {
    case 0: return "CKR_OK";
    case 0x00000007UL: return "CKR_ARGUMENTS_BAD";
    case 0x00000030UL: return "CKR_DEVICE_ERROR";
    case 0x00000031UL: return "CKR_DEVICE_MEMORY";
    case 0x00000032UL: return "CKR_DEVICE_REMOVED";
    case 0x00000050UL: return "CKR_TOKEN_NOT_PRESENT";
    case 0x00000051UL: return "CKR_TOKEN_NOT_RECOGNIZED";
    case 0x000000A0UL: return "CKR_PIN_INCORRECT";
    case 0x000000A1UL: return "CKR_PIN_INVALID";
    case 0x000000A2UL: return "CKR_PIN_LEN_RANGE";
    case 0x000000A3UL: return "CKR_PIN_EXPIRED";
    case 0x000000A4UL: return "CKR_PIN_LOCKED";
    case 0x000000B0UL: return "CKR_USER_NOT_LOGGED_IN";
    case 0x00000100UL: return "CKR_CRYPTOKI_NOT_INITIALIZED";
    default: return NULL;
    }
}

/* ========== PKCS11Test.nativeRunTest ========== */
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Test_nativeRunTest(JNIEnv *env, jclass clazz) {
    (void)clazz;
    char buf[2048] = {0};
    if (load_pkcs11() != 0) {
        snprintf(buf, sizeof(buf),
            "Error: %s\n\nTo enable: uncomment opensc-pkcs11 in app/build.gradle, run setup_opensc_android.sh, rebuild.",
            load_pkcs11_error());
        return (*env)->NewStringUTF(env, buf);
    }
    CK_RV rv = g_funcs->C_Initialize(NULL);
    if (rv != CKR_OK) {
        format_pkcs11_error(buf, sizeof(buf), "C_Initialize", rv);
        return (*env)->NewStringUTF(env, buf);
    }
    g_initialized = 1;
    CK_ULONG count = 0;
    rv = g_funcs->C_GetSlotList(PKCS11_FALSE, NULL, &count);
    if (rv != CKR_OK) {
        format_pkcs11_error(buf, sizeof(buf), "C_GetSlotList", rv);
        g_funcs->C_Finalize(NULL);
        g_initialized = 0;
        return (*env)->NewStringUTF(env, buf);
    }
    size_t off = snprintf(buf, sizeof(buf), "C_Initialize: OK\nC_GetSlotList: OK\nSlot count: %lu\n", (unsigned long)count);
    if (count > 0 && count < 256) {
        CK_ULONG slots[256];
        rv = g_funcs->C_GetSlotList(PKCS11_FALSE, slots, &count);
        if (rv == CKR_OK) {
            off += snprintf(buf + off, sizeof(buf) - off, "Slots: ");
            for (CK_ULONG i = 0; i < count && off < sizeof(buf) - 32; i++)
                off += snprintf(buf + off, sizeof(buf) - off, "%lu ", (unsigned long)slots[i]);
            off += snprintf(buf + off, sizeof(buf) - off, "\n");
        }
    }
    g_funcs->C_Finalize(NULL);
    g_initialized = 0;
    return (*env)->NewStringUTF(env, buf);
}

/* ========== PKCS11Demo APIs ========== */
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeInitialize(JNIEnv *env, jclass clazz) {
    (void)clazz;
    char buf[512] = {0};
    if (load_pkcs11() != 0) {
        snprintf(buf, sizeof(buf), "Error: %s (uncomment opensc-pkcs11 in app/build.gradle)", load_pkcs11_error());
        return (*env)->NewStringUTF(env, buf);
    }
    CK_RV rv = g_funcs->C_Initialize(NULL);
    if (rv != CKR_OK)
        format_pkcs11_error(buf, sizeof(buf), "C_Initialize", rv);
    else {
        g_initialized = 1;
        snprintf(buf, sizeof(buf), "OK");
    }
    return (*env)->NewStringUTF(env, buf);
}

JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeFinalize(JNIEnv *env, jclass clazz) {
    (void)clazz;
    char buf[256] = {0};
    if (!g_funcs) { snprintf(buf, sizeof(buf), "Not initialized"); return (*env)->NewStringUTF(env, buf); }
    CK_RV rv = g_funcs->C_Finalize(NULL);
    g_initialized = 0;
    snprintf(buf, sizeof(buf), rv == CKR_OK ? "OK" : "C_Finalize failed: 0x%08lX", (unsigned long)rv);
    return (*env)->NewStringUTF(env, buf);
}

JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeGetSlotList(JNIEnv *env, jclass clazz) {
    (void)clazz;
    char buf[1024] = {0};
    if (!g_funcs) { snprintf(buf, sizeof(buf), "Load library first"); return (*env)->NewStringUTF(env, buf); }
    if (!g_initialized) { snprintf(buf, sizeof(buf), "Call C_Initialize first"); return (*env)->NewStringUTF(env, buf); }
    CK_ULONG count = 0;
    CK_RV rv = g_funcs->C_GetSlotList(PKCS11_FALSE, NULL, &count);
    if (rv != CKR_OK) {
        format_pkcs11_error(buf, sizeof(buf), "C_GetSlotList", rv);
        return (*env)->NewStringUTF(env, buf);
    }
    size_t off = snprintf(buf, sizeof(buf), "Slot count: %lu\n", (unsigned long)count);
    if (count > 0 && count < 64) {
        CK_ULONG slots[64];
        rv = g_funcs->C_GetSlotList(PKCS11_FALSE, slots, &count);
        if (rv == CKR_OK) {
            off += snprintf(buf + off, sizeof(buf) - off, "Slots: ");
            for (CK_ULONG i = 0; i < count && off < sizeof(buf) - 24; i++)
                off += snprintf(buf + off, sizeof(buf) - off, "%lu ", (unsigned long)slots[i]);
        }
    }
    return (*env)->NewStringUTF(env, buf);
}

/* CK_SLOT_INFO: slotDescription[64], manufacturerID[32], flags, hwVer, fwVer - standard layout */
typedef struct {
    unsigned char slotDescription[64];
    unsigned char manufacturerID[32];
    CK_ULONG flags;
    unsigned char hwMajor, hwMinor, fwMajor, fwMinor;
} CK_SLOT_INFO_DEMO;
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeGetSlotInfo(JNIEnv *env, jclass clazz, jlong slotId) {
    (void)clazz;
    char buf[512] = {0};
    if (!g_funcs) { snprintf(buf, sizeof(buf), "Load library first"); return (*env)->NewStringUTF(env, buf); }
    if (!g_initialized) { snprintf(buf, sizeof(buf), "Call C_Initialize first"); return (*env)->NewStringUTF(env, buf); }
    if (!g_funcs->C_GetSlotInfo) { snprintf(buf, sizeof(buf), "C_GetSlotInfo not available"); return (*env)->NewStringUTF(env, buf); }
    CK_SLOT_INFO_DEMO info;
    memset(&info, 0, sizeof(info));
    CK_RV rv = g_funcs->C_GetSlotInfo((CK_ULONG)slotId, &info);
    if (rv != CKR_OK) {
        format_pkcs11_error(buf, sizeof(buf), "C_GetSlotInfo", rv);
        return (*env)->NewStringUTF(env, buf);
    }
    info.slotDescription[63] = info.manufacturerID[31] = 0;
    snprintf(buf, sizeof(buf), "Slot %lu:\ndescription: %.64s\nmanufacturer: %.32s\nflags: 0x%lX",
             (unsigned long)slotId, info.slotDescription, info.manufacturerID, (unsigned long)info.flags);
    return (*env)->NewStringUTF(env, buf);
}

/* CK_TOKEN_INFO: label[32], manufacturerID[32], model[16], serialNumber[16], ... */
typedef struct {
    unsigned char label[32];
    unsigned char manufacturerID[32];
    unsigned char model[16];
    unsigned char serialNumber[16];
    CK_ULONG flags;
} CK_TOKEN_INFO_DEMO;
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeGetTokenInfo(JNIEnv *env, jclass clazz, jlong slotId) {
    (void)clazz;
    char buf[640] = {0};
    if (!g_funcs) { snprintf(buf, sizeof(buf), "Load library first"); return (*env)->NewStringUTF(env, buf); }
    if (!g_initialized) { snprintf(buf, sizeof(buf), "Call C_Initialize first"); return (*env)->NewStringUTF(env, buf); }
    if (!g_funcs->C_GetTokenInfo) { snprintf(buf, sizeof(buf), "C_GetTokenInfo not available"); return (*env)->NewStringUTF(env, buf); }
    CK_TOKEN_INFO_DEMO info;
    memset(&info, 0, sizeof(info));
    CK_RV rv = g_funcs->C_GetTokenInfo((CK_ULONG)slotId, &info);
    if (rv != CKR_OK) {
        format_pkcs11_error(buf, sizeof(buf), "C_GetTokenInfo", rv);
        return (*env)->NewStringUTF(env, buf);
    }
    info.label[31] = info.manufacturerID[31] = info.model[15] = info.serialNumber[15] = 0;
    snprintf(buf, sizeof(buf),
             "Token slot %lu:\nlabel: %.32s\nmanufacturer: %.32s\nmodel: %.16s\nserial: %.16s\nflags: 0x%lX",
             (unsigned long)slotId, info.label, info.manufacturerID, info.model, info.serialNumber, (unsigned long)info.flags);
    return (*env)->NewStringUTF(env, buf);
}

JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeOpenSession(JNIEnv *env, jclass clazz, jlong slotId) {
    (void)clazz;
    char buf[384] = {0};
    if (!g_funcs || !g_initialized) {
        snprintf(buf, sizeof(buf), "FAIL:0x0 (not initialized - call C_Initialize first)");
        return (*env)->NewStringUTF(env, buf);
    }
    if (!g_funcs->C_OpenSession) {
        snprintf(buf, sizeof(buf), "FAIL:0x0 (C_OpenSession not available)");
        return (*env)->NewStringUTF(env, buf);
    }
    CK_ULONG hSession = 0;
    CK_ULONG flags = CKF_SERIAL_SESSION | CKF_RW_SESSION;
    PKCS11_LOGD("C_OpenSession: slotID=0x%lX (%lu), flags=0x%lX (%lu), phSession=%p",
                (unsigned long)slotId, (unsigned long)slotId, (unsigned long)flags, (unsigned long)flags, (void *)&hSession);
    CK_RV rv = g_funcs->C_OpenSession((CK_ULONG)slotId, flags, NULL, NULL, &hSession);
    PKCS11_LOGD("C_OpenSession: rv=0x%08lX (%s), hSession=0x%lX",
                (unsigned long)rv, ckr_name(rv) ? ckr_name(rv) : "unknown", (unsigned long)hSession);
    if (rv == CKR_OK) {
        snprintf(buf, sizeof(buf), "OK:%lu", (unsigned long)hSession);
        return (*env)->NewStringUTF(env, buf);
    }
    const char *name = ckr_name(rv);
    if (name)
        snprintf(buf, sizeof(buf), "FAIL:0x%08lX (%s)", (unsigned long)rv, name);
    else
        snprintf(buf, sizeof(buf), "FAIL:0x%08lX", (unsigned long)rv);
    return (*env)->NewStringUTF(env, buf);
}

JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeLogin(JNIEnv *env, jclass clazz, jlong hSession, jstring pin) {
    (void)clazz;
    char buf[256] = {0};
    if (!g_funcs || !g_funcs->C_Login) { snprintf(buf, sizeof(buf), "C_Login not available"); return (*env)->NewStringUTF(env, buf); }
    const char *pinUtf = pin ? (*env)->GetStringUTFChars(env, pin, NULL) : NULL;
    CK_ULONG pinLen = pinUtf ? (CK_ULONG)strlen(pinUtf) : 0;
    CK_RV rv = g_funcs->C_Login((CK_ULONG)hSession, CKU_USER, (unsigned char*)(pinUtf ? pinUtf : ""), pinLen);
    if (pinUtf) (*env)->ReleaseStringUTFChars(env, pin, pinUtf);
    if (rv != CKR_OK) format_pkcs11_error(buf, sizeof(buf), "C_Login", rv);
    else snprintf(buf, sizeof(buf), "OK");
    return (*env)->NewStringUTF(env, buf);
}

JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeFindObjects(JNIEnv *env, jclass clazz, jlong hSession) {
    (void)clazz;
    char buf[2048] = {0};
    if (!g_funcs || !g_funcs->C_FindObjectsInit || !g_funcs->C_FindObjects || !g_funcs->C_FindObjectsFinal) {
        snprintf(buf, sizeof(buf), "C_FindObjects* not available");
        return (*env)->NewStringUTF(env, buf);
    }
    CK_RV rv = g_funcs->C_FindObjectsInit((CK_ULONG)hSession, NULL, 0);
    if (rv != CKR_OK) {
        format_pkcs11_error(buf, sizeof(buf), "C_FindObjectsInit", rv);
        return (*env)->NewStringUTF(env, buf);
    }
    CK_ULONG objs[64], count = 0;
    rv = g_funcs->C_FindObjects((CK_ULONG)hSession, objs, 64, &count);
    g_funcs->C_FindObjectsFinal((CK_ULONG)hSession);
    if (rv != CKR_OK) {
        format_pkcs11_error(buf, sizeof(buf), "C_FindObjects", rv);
        return (*env)->NewStringUTF(env, buf);
    }
    size_t off = snprintf(buf, sizeof(buf), "Object count: %lu\n", (unsigned long)count);
    for (CK_ULONG i = 0; i < count && i < 32 && off < sizeof(buf) - 24; i++)
        off += snprintf(buf + off, sizeof(buf) - off, "  handle[%lu]=0x%lX\n", (unsigned long)i, (unsigned long)objs[i]);
    return (*env)->NewStringUTF(env, buf);
}

JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeLogout(JNIEnv *env, jclass clazz, jlong hSession) {
    (void)clazz;
    char buf[256] = {0};
    if (!g_funcs || !g_funcs->C_Logout) { snprintf(buf, sizeof(buf), "C_Logout not available"); return (*env)->NewStringUTF(env, buf); }
    CK_RV rv = g_funcs->C_Logout((CK_ULONG)hSession);
    if (rv != CKR_OK) format_pkcs11_error(buf, sizeof(buf), "C_Logout", rv);
    else snprintf(buf, sizeof(buf), "OK");
    return (*env)->NewStringUTF(env, buf);
}

JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeCloseSession(JNIEnv *env, jclass clazz, jlong hSession) {
    (void)clazz;
    char buf[256] = {0};
    if (!g_funcs || !g_funcs->C_CloseSession) { snprintf(buf, sizeof(buf), "C_CloseSession not available"); return (*env)->NewStringUTF(env, buf); }
    CK_RV rv = g_funcs->C_CloseSession((CK_ULONG)hSession);
    if (rv != CKR_OK) format_pkcs11_error(buf, sizeof(buf), "C_CloseSession", rv);
    else snprintf(buf, sizeof(buf), "OK");
    return (*env)->NewStringUTF(env, buf);
}

/* C_GetMechanismList(slotId) */
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeGetMechanismList(JNIEnv *env, jclass clazz, jlong slotId) {
    (void)clazz;
    char buf[1024] = {0};
    if (!g_funcs || !g_C_GetMechanismList) { snprintf(buf, sizeof(buf), "C_GetMechanismList not available"); return (*env)->NewStringUTF(env, buf); }
    if (!g_initialized) { snprintf(buf, sizeof(buf), "Call C_Initialize first"); return (*env)->NewStringUTF(env, buf); }
    CK_ULONG count = 0;
    CK_RV rv = g_C_GetMechanismList((CK_ULONG)slotId, NULL, &count);
    if (rv != CKR_OK) {
        format_pkcs11_error(buf, sizeof(buf), "C_GetMechanismList", rv);
        return (*env)->NewStringUTF(env, buf);
    }
    size_t off = snprintf(buf, sizeof(buf), "Mechanism count: %lu\n", (unsigned long)count);
    if (count > 0 && count < 64) {
        CK_ULONG mechs[64];
        rv = g_C_GetMechanismList((CK_ULONG)slotId, mechs, &count);
        if (rv == CKR_OK) {
            for (CK_ULONG i = 0; i < count && off < sizeof(buf) - 24; i++)
                off += snprintf(buf + off, sizeof(buf) - off, "  0x%08lX\n", (unsigned long)mechs[i]);
        }
    }
    return (*env)->NewStringUTF(env, buf);
}

/* C_SignInit + C_Sign: dataHex = hex string of data to sign, mechType = e.g. CKM_SHA256_RSA_PKCS (0x46) or CKM_ECDSA (0x1041) */
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeSign(JNIEnv *env, jclass clazz, jlong hSession, jlong keyHandle, jstring dataHex, jlong mechType) {
    (void)clazz;
    char buf[2048] = {0};
    size_t off = 0;
    if (!g_C_SignInit || !g_C_Sign) { snprintf(buf, sizeof(buf), "C_SignInit/C_Sign not available"); return (*env)->NewStringUTF(env, buf); }
    const char *hex = dataHex ? (*env)->GetStringUTFChars(env, dataHex, NULL) : NULL;
    if (!hex || hex[0] == '\0') { if (hex) (*env)->ReleaseStringUTFChars(env, dataHex, hex); snprintf(buf, sizeof(buf), "dataHex required"); return (*env)->NewStringUTF(env, buf); }
    size_t hexLen = strlen(hex);
    if (hexLen % 2 != 0) { (*env)->ReleaseStringUTFChars(env, dataHex, hex); snprintf(buf, sizeof(buf), "dataHex must have even length"); return (*env)->NewStringUTF(env, buf); }
    size_t dataLen = hexLen / 2;
    unsigned char *data = (unsigned char *)malloc(dataLen);
    if (!data) { (*env)->ReleaseStringUTFChars(env, dataHex, hex); snprintf(buf, sizeof(buf), "malloc failed"); return (*env)->NewStringUTF(env, buf); }
    for (size_t i = 0; i < dataLen; i++) {
        int a, b;
        if (sscanf(hex + i*2, "%1x%1x", &a, &b) != 2) { free(data); (*env)->ReleaseStringUTFChars(env, dataHex, hex); snprintf(buf, sizeof(buf), "invalid hex"); return (*env)->NewStringUTF(env, buf); }
        data[i] = (unsigned char)((a<<4)|b);
    }
    (*env)->ReleaseStringUTFChars(env, dataHex, hex);
    CK_MECHANISM_DEMO mech = { (CK_ULONG)mechType, NULL, 0 };
    CK_RV rv = g_C_SignInit((CK_ULONG)hSession, &mech, (CK_ULONG)keyHandle);
    if (rv != CKR_OK) {
        free(data);
        format_pkcs11_error(buf, sizeof(buf), "C_SignInit", rv);
        return (*env)->NewStringUTF(env, buf);
    }
    CK_ULONG sigLen = 0;
    rv = g_C_Sign((CK_ULONG)hSession, data, (CK_ULONG)dataLen, NULL, &sigLen);
    if (rv != CKR_OK && rv != (CK_RV)CKR_BUFFER_TOO_SMALL) {
        free(data);
        format_pkcs11_error(buf, sizeof(buf), "C_Sign", rv);
        return (*env)->NewStringUTF(env, buf);
    }
    if (sigLen == 0 || sigLen > 4096) { free(data); snprintf(buf, sizeof(buf), "C_Sign invalid sigLen=%lu", (unsigned long)sigLen); return (*env)->NewStringUTF(env, buf); }
    unsigned char *sig = (unsigned char *)malloc(sigLen);
    if (!sig) { free(data); snprintf(buf, sizeof(buf), "malloc sig failed"); return (*env)->NewStringUTF(env, buf); }
    rv = g_C_SignInit((CK_ULONG)hSession, &mech, (CK_ULONG)keyHandle);
    if (rv != CKR_OK) { free(data); free(sig); format_pkcs11_error(buf, sizeof(buf), "C_SignInit", rv); return (*env)->NewStringUTF(env, buf); }
    rv = g_C_Sign((CK_ULONG)hSession, data, (CK_ULONG)dataLen, sig, &sigLen);
    free(data);
    if (rv != CKR_OK) { free(sig); format_pkcs11_error(buf, sizeof(buf), "C_Sign", rv); return (*env)->NewStringUTF(env, buf); }
    for (CK_ULONG i = 0; i < sigLen && off < sizeof(buf) - 4; i++) off += snprintf(buf + off, sizeof(buf) - off, "%02x", sig[i]);
    free(sig);
    return (*env)->NewStringUTF(env, buf);
}

/* C_GetMechanismInfo(slotId, mechType) */
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeGetMechanismInfo(JNIEnv *env, jclass clazz, jlong slotId, jlong mechType) {
    (void)clazz;
    char buf[256] = {0};
    if (!g_C_GetMechanismInfo) { snprintf(buf, sizeof(buf), "C_GetMechanismInfo not available"); return (*env)->NewStringUTF(env, buf); }
    if (!g_initialized) { snprintf(buf, sizeof(buf), "Call C_Initialize first"); return (*env)->NewStringUTF(env, buf); }
    CK_MECHANISM_INFO_DEMO info;
    memset(&info, 0, sizeof(info));
    CK_RV rv = g_C_GetMechanismInfo((CK_ULONG)slotId, (CK_ULONG)mechType, &info);
    if (rv != CKR_OK) {
        format_pkcs11_error(buf, sizeof(buf), "C_GetMechanismInfo", rv);
        return (*env)->NewStringUTF(env, buf);
    }
    snprintf(buf, sizeof(buf), "Mech 0x%08lX: minKey=%lu maxKey=%lu flags=0x%lX",
             (unsigned long)mechType, (unsigned long)info.ulMinKeySize, (unsigned long)info.ulMaxKeySize, (unsigned long)info.flags);
    return (*env)->NewStringUTF(env, buf);
}

/* C_GetSessionInfo(hSession) */
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeGetSessionInfo(JNIEnv *env, jclass clazz, jlong hSession) {
    (void)clazz;
    char buf[256] = {0};
    if (!g_C_GetSessionInfo) { snprintf(buf, sizeof(buf), "C_GetSessionInfo not available"); return (*env)->NewStringUTF(env, buf); }
    CK_SESSION_INFO_DEMO info;
    memset(&info, 0, sizeof(info));
    CK_RV rv = g_C_GetSessionInfo((CK_ULONG)hSession, &info);
    if (rv != CKR_OK) {
        format_pkcs11_error(buf, sizeof(buf), "C_GetSessionInfo", rv);
        return (*env)->NewStringUTF(env, buf);
    }
    snprintf(buf, sizeof(buf), "slotID=%lu state=%lu flags=0x%lX", (unsigned long)info.slotID, (unsigned long)info.state, (unsigned long)info.flags);
    return (*env)->NewStringUTF(env, buf);
}

/* C_GetAttributeValue(hSession, hObject, attrType) - returns hex of CKA_VALUE etc */
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeGetAttributeValue(JNIEnv *env, jclass clazz, jlong hSession, jlong hObject, jlong attrType) {
    (void)clazz;
    char buf[8192] = {0};
    size_t off = 0;
    if (!g_C_GetAttributeValue) { snprintf(buf, sizeof(buf), "C_GetAttributeValue not available"); return (*env)->NewStringUTF(env, buf); }
    CK_ATTRIBUTE_DEMO attr;
    attr.type = (CK_ULONG)attrType;
    attr.pValue = NULL;
    attr.ulValueLen = 0;
    CK_RV rv = g_C_GetAttributeValue((CK_ULONG)hSession, (CK_ULONG)hObject, &attr, 1);
    if (rv != CKR_OK && rv != (CK_RV)CKR_BUFFER_TOO_SMALL) {
        format_pkcs11_error(buf, sizeof(buf), "C_GetAttributeValue", rv);
        return (*env)->NewStringUTF(env, buf);
    }
    if (attr.ulValueLen == 0 || attr.ulValueLen > 65536) {
        snprintf(buf, sizeof(buf), "attrLen=%lu (empty or too large)", (unsigned long)attr.ulValueLen);
        return (*env)->NewStringUTF(env, buf);
    }
    unsigned char *val = (unsigned char *)malloc(attr.ulValueLen);
    if (!val) { snprintf(buf, sizeof(buf), "malloc failed"); return (*env)->NewStringUTF(env, buf); }
    attr.pValue = val;
    rv = g_C_GetAttributeValue((CK_ULONG)hSession, (CK_ULONG)hObject, &attr, 1);
    if (rv != CKR_OK) {
        free(val);
        format_pkcs11_error(buf, sizeof(buf), "C_GetAttributeValue", rv);
        return (*env)->NewStringUTF(env, buf);
    }
    off = snprintf(buf, sizeof(buf), "len=%lu hex=", (unsigned long)attr.ulValueLen);
    for (CK_ULONG i = 0; i < attr.ulValueLen && off < sizeof(buf) - 4; i++)
        off += snprintf(buf + off, sizeof(buf) - off, "%02x", val[i]);
    free(val);
    return (*env)->NewStringUTF(env, buf);
}

/* hex2bytes helper */
static int hex2bytes(const char *hex, unsigned char *out, size_t maxLen) {
    size_t len = strlen(hex);
    if (len % 2 != 0) return -1;
    size_t n = len / 2;
    if (n > maxLen) return -1;
    for (size_t i = 0; i < n; i++) {
        int a, b;
        if (sscanf(hex + i*2, "%1x%1x", &a, &b) != 2) return -1;
        out[i] = (unsigned char)((a<<4)|b);
    }
    return (int)n;
}

/* C_Digest(hSession, dataHex, mechType) - mechType e.g. CKM_SHA256 0x250 */
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeDigest(JNIEnv *env, jclass clazz, jlong hSession, jstring dataHex, jlong mechType) {
    (void)clazz;
    char buf[256] = {0};
    size_t off = 0;
    if (!g_C_DigestInit || !g_C_Digest) { snprintf(buf, sizeof(buf), "C_Digest not available"); return (*env)->NewStringUTF(env, buf); }
    const char *hex = dataHex ? (*env)->GetStringUTFChars(env, dataHex, NULL) : NULL;
    if (!hex || hex[0] == '\0') { if (hex) (*env)->ReleaseStringUTFChars(env, dataHex, hex); snprintf(buf, sizeof(buf), "dataHex required"); return (*env)->NewStringUTF(env, buf); }
    unsigned char data[4096];
    int dataLen = hex2bytes(hex, data, sizeof(data));
    (*env)->ReleaseStringUTFChars(env, dataHex, hex);
    if (dataLen < 0) { snprintf(buf, sizeof(buf), "invalid hex"); return (*env)->NewStringUTF(env, buf); }
    CK_MECHANISM_DEMO mech = { (CK_ULONG)mechType, NULL, 0 };
    CK_RV rv = g_C_DigestInit((CK_ULONG)hSession, &mech);
    if (rv != CKR_OK) { format_pkcs11_error(buf, sizeof(buf), "C_DigestInit", rv); return (*env)->NewStringUTF(env, buf); }
    CK_ULONG digestLen = 64; /* SHA512 max */
    unsigned char digest[64];
    rv = g_C_Digest((CK_ULONG)hSession, data, (CK_ULONG)dataLen, digest, &digestLen);
    if (rv != CKR_OK) { format_pkcs11_error(buf, sizeof(buf), "C_Digest", rv); return (*env)->NewStringUTF(env, buf); }
    for (CK_ULONG i = 0; i < digestLen && off < sizeof(buf) - 4; i++) off += snprintf(buf + off, sizeof(buf) - off, "%02x", digest[i]);
    return (*env)->NewStringUTF(env, buf);
}

/* C_Verify(hSession, keyHandle, dataHex, sigHex, mechType) */
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeVerify(JNIEnv *env, jclass clazz, jlong hSession, jlong keyHandle, jstring dataHex, jstring sigHex, jlong mechType) {
    (void)clazz;
    char buf[256] = {0};
    if (!g_C_VerifyInit || !g_C_Verify) { snprintf(buf, sizeof(buf), "C_Verify not available"); return (*env)->NewStringUTF(env, buf); }
    const char *hexD = dataHex ? (*env)->GetStringUTFChars(env, dataHex, NULL) : NULL;
    const char *hexS = sigHex ? (*env)->GetStringUTFChars(env, sigHex, NULL) : NULL;
    if (!hexD || !hexS) { if (hexD) (*env)->ReleaseStringUTFChars(env, dataHex, hexD); if (hexS) (*env)->ReleaseStringUTFChars(env, sigHex, hexS); snprintf(buf, sizeof(buf), "dataHex and sigHex required"); return (*env)->NewStringUTF(env, buf); }
    unsigned char data[4096], sig[512];
    int dataLen = hex2bytes(hexD, data, sizeof(data));
    int sigLen = hex2bytes(hexS, sig, sizeof(sig));
    (*env)->ReleaseStringUTFChars(env, dataHex, hexD);
    (*env)->ReleaseStringUTFChars(env, sigHex, hexS);
    if (dataLen < 0 || sigLen < 0) { snprintf(buf, sizeof(buf), "invalid hex"); return (*env)->NewStringUTF(env, buf); }
    CK_MECHANISM_DEMO mech = { (CK_ULONG)mechType, NULL, 0 };
    CK_RV rv = g_C_VerifyInit((CK_ULONG)hSession, &mech, (CK_ULONG)keyHandle);
    if (rv != CKR_OK) { format_pkcs11_error(buf, sizeof(buf), "C_VerifyInit", rv); return (*env)->NewStringUTF(env, buf); }
    rv = g_C_Verify((CK_ULONG)hSession, data, (CK_ULONG)dataLen, sig, (CK_ULONG)sigLen);
    snprintf(buf, sizeof(buf), rv == CKR_OK ? "OK (signature valid)" : "Verify failed: 0x%08lX", (unsigned long)rv);
    return (*env)->NewStringUTF(env, buf);
}

/* C_Decrypt(hSession, keyHandle, cipherHex, mechType) */
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeDecrypt(JNIEnv *env, jclass clazz, jlong hSession, jlong keyHandle, jstring cipherHex, jlong mechType) {
    (void)clazz;
    char buf[4096] = {0};
    size_t off = 0;
    if (!g_C_DecryptInit || !g_C_Decrypt) { snprintf(buf, sizeof(buf), "C_Decrypt not available"); return (*env)->NewStringUTF(env, buf); }
    const char *hex = cipherHex ? (*env)->GetStringUTFChars(env, cipherHex, NULL) : NULL;
    if (!hex || hex[0] == '\0') { if (hex) (*env)->ReleaseStringUTFChars(env, cipherHex, hex); snprintf(buf, sizeof(buf), "cipherHex required"); return (*env)->NewStringUTF(env, buf); }
    unsigned char cipher[2048];
    int cipherLen = hex2bytes(hex, cipher, sizeof(cipher));
    (*env)->ReleaseStringUTFChars(env, cipherHex, hex);
    if (cipherLen < 0) { snprintf(buf, sizeof(buf), "invalid hex"); return (*env)->NewStringUTF(env, buf); }
    CK_MECHANISM_DEMO mech = { (CK_ULONG)mechType, NULL, 0 };
    CK_RV rv = g_C_DecryptInit((CK_ULONG)hSession, &mech, (CK_ULONG)keyHandle);
    if (rv != CKR_OK) { format_pkcs11_error(buf, sizeof(buf), "C_DecryptInit", rv); return (*env)->NewStringUTF(env, buf); }
    CK_ULONG dataLen = 0;
    rv = g_C_Decrypt((CK_ULONG)hSession, cipher, (CK_ULONG)cipherLen, NULL, &dataLen);
    if (rv != CKR_OK && rv != (CK_RV)CKR_BUFFER_TOO_SMALL) { format_pkcs11_error(buf, sizeof(buf), "C_Decrypt", rv); return (*env)->NewStringUTF(env, buf); }
    if (dataLen == 0 || dataLen > 2048) { snprintf(buf, sizeof(buf), "invalid dataLen=%lu", (unsigned long)dataLen); return (*env)->NewStringUTF(env, buf); }
    unsigned char *data = (unsigned char *)malloc(dataLen);
    if (!data) { snprintf(buf, sizeof(buf), "malloc failed"); return (*env)->NewStringUTF(env, buf); }
    rv = g_C_DecryptInit((CK_ULONG)hSession, &mech, (CK_ULONG)keyHandle);
    if (rv != CKR_OK) { free(data); format_pkcs11_error(buf, sizeof(buf), "C_DecryptInit", rv); return (*env)->NewStringUTF(env, buf); }
    rv = g_C_Decrypt((CK_ULONG)hSession, cipher, (CK_ULONG)cipherLen, data, &dataLen);
    if (rv != CKR_OK) { free(data); format_pkcs11_error(buf, sizeof(buf), "C_Decrypt", rv); return (*env)->NewStringUTF(env, buf); }
    for (CK_ULONG i = 0; i < dataLen && off < sizeof(buf) - 4; i++) off += snprintf(buf + off, sizeof(buf) - off, "%02x", data[i]);
    free(data);
    return (*env)->NewStringUTF(env, buf);
}

/* C_Encrypt(hSession, keyHandle, plainHex, mechType) */
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeEncrypt(JNIEnv *env, jclass clazz, jlong hSession, jlong keyHandle, jstring plainHex, jlong mechType) {
    (void)clazz;
    char buf[4096] = {0};
    size_t off = 0;
    if (!g_C_EncryptInit || !g_C_Encrypt) { snprintf(buf, sizeof(buf), "C_Encrypt not available"); return (*env)->NewStringUTF(env, buf); }
    const char *hex = plainHex ? (*env)->GetStringUTFChars(env, plainHex, NULL) : NULL;
    if (!hex || hex[0] == '\0') { if (hex) (*env)->ReleaseStringUTFChars(env, plainHex, hex); snprintf(buf, sizeof(buf), "plainHex required"); return (*env)->NewStringUTF(env, buf); }
    unsigned char plain[2048];
    int plainLen = hex2bytes(hex, plain, sizeof(plain));
    (*env)->ReleaseStringUTFChars(env, plainHex, hex);
    if (plainLen < 0) { snprintf(buf, sizeof(buf), "invalid hex"); return (*env)->NewStringUTF(env, buf); }
    CK_MECHANISM_DEMO mech = { (CK_ULONG)mechType, NULL, 0 };
    CK_RV rv = g_C_EncryptInit((CK_ULONG)hSession, &mech, (CK_ULONG)keyHandle);
    if (rv != CKR_OK) { format_pkcs11_error(buf, sizeof(buf), "C_EncryptInit", rv); return (*env)->NewStringUTF(env, buf); }
    CK_ULONG cipherLen = 0;
    rv = g_C_Encrypt((CK_ULONG)hSession, plain, (CK_ULONG)plainLen, NULL, &cipherLen);
    if (rv != CKR_OK && rv != (CK_RV)CKR_BUFFER_TOO_SMALL) { format_pkcs11_error(buf, sizeof(buf), "C_Encrypt", rv); return (*env)->NewStringUTF(env, buf); }
    if (cipherLen == 0 || cipherLen > 2048) { snprintf(buf, sizeof(buf), "invalid cipherLen=%lu", (unsigned long)cipherLen); return (*env)->NewStringUTF(env, buf); }
    unsigned char *cipher = (unsigned char *)malloc(cipherLen);
    if (!cipher) { snprintf(buf, sizeof(buf), "malloc failed"); return (*env)->NewStringUTF(env, buf); }
    rv = g_C_EncryptInit((CK_ULONG)hSession, &mech, (CK_ULONG)keyHandle);
    if (rv != CKR_OK) { free(cipher); format_pkcs11_error(buf, sizeof(buf), "C_EncryptInit", rv); return (*env)->NewStringUTF(env, buf); }
    rv = g_C_Encrypt((CK_ULONG)hSession, plain, (CK_ULONG)plainLen, cipher, &cipherLen);
    if (rv != CKR_OK) { free(cipher); format_pkcs11_error(buf, sizeof(buf), "C_Encrypt", rv); return (*env)->NewStringUTF(env, buf); }
    for (CK_ULONG i = 0; i < cipherLen && off < sizeof(buf) - 4; i++) off += snprintf(buf + off, sizeof(buf) - off, "%02x", cipher[i]);
    free(cipher);
    return (*env)->NewStringUTF(env, buf);
}

/* C_GenerateRandom(hSession, len) */
JNIEXPORT jstring JNICALL
Java_com_ftsafe_pcsc_demo_PKCS11Demo_nativeGenerateRandom(JNIEnv *env, jclass clazz, jlong hSession, jint len) {
    (void)clazz;
    char buf[512] = {0};
    size_t off = 0;
    if (!g_C_GenerateRandom) { snprintf(buf, sizeof(buf), "C_GenerateRandom not available"); return (*env)->NewStringUTF(env, buf); }
    if (len <= 0 || len > 256) { snprintf(buf, sizeof(buf), "len must be 1..256"); return (*env)->NewStringUTF(env, buf); }
    unsigned char rnd[256];
    CK_RV rv = g_C_GenerateRandom((CK_ULONG)hSession, rnd, (CK_ULONG)len);
    if (rv != CKR_OK) { format_pkcs11_error(buf, sizeof(buf), "C_GenerateRandom", rv); return (*env)->NewStringUTF(env, buf); }
    for (int i = 0; i < len && off < sizeof(buf) - 4; i++) off += snprintf(buf + off, sizeof(buf) - off, "%02x", rnd[i]);
    return (*env)->NewStringUTF(env, buf);
}
