package com.ftsafe.pcsc.demo;

/**
 * PKCS#11 JNI wrapper for Demo step-by-step testing.
 * Prerequisites: PCSCServer running, FTReader_InitSocket, C_Initialize.
 * Supports OpenSC PIV and similar cards.
 */
public class PKCS11Demo {
    static {
        System.loadLibrary("pkcs11_test");
    }

    /** 调试时设置 OPENSC_DEBUG，需在 C_Initialize 之前调用。level 通常为 9。 */
    public static native void nativeSetOpenSCDebug(int level);
    public static native String nativeInitialize();
    public static native String nativeFinalize();
    public static native String nativeGetSlotList();
    public static native String nativeGetSlotInfo(long slotId);
    public static native String nativeGetTokenInfo(long slotId);
    /** @return "OK:&lt;handle&gt;" on success, "FAIL:0x... (CKR_xxx)" on error */
    public static native String nativeOpenSession(long slotId);
    public static native String nativeLogin(long hSession, String pin);
    public static native String nativeFindObjects(long hSession);
    public static native String nativeLogout(long hSession);
    public static native String nativeCloseSession(long hSession);
    public static native String nativeGetMechanismList(long slotId);
    public static native String nativeGetMechanismInfo(long slotId, long mechType);
    public static native String nativeGetSessionInfo(long hSession);
    public static native String nativeGetAttributeValue(long hSession, long hObject, long attrType);
    /** @param mechType e.g. 0x40 (CKM_SHA256_RSA_PKCS) or 0x1041 (CKM_ECDSA) */
    public static native String nativeSign(long hSession, long keyHandle, String dataHex, long mechType);
    public static native String nativeDigest(long hSession, String dataHex, long mechType);
    public static native String nativeVerify(long hSession, long keyHandle, String dataHex, String sigHex, long mechType);
    public static native String nativeDecrypt(long hSession, long keyHandle, String cipherHex, long mechType);
    public static native String nativeEncrypt(long hSession, long keyHandle, String plainHex, long mechType);
    public static native String nativeGenerateRandom(long hSession, int len);
}
