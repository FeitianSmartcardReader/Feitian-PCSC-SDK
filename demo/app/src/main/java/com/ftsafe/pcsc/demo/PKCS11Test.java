package com.ftsafe.pcsc.demo;

/**
 * PKCS#11 integration test - loads opensc-pkcs11 via dlopen and calls C_Initialize, C_GetSlotList.
 * Prerequisites: PCSCServer running, FTReaderPCSC.initSocket(), libftreader-pcsc loaded.
 */
public class PKCS11Test {
    static {
        System.loadLibrary("pkcs11_test");
    }
    public static native String nativeRunTest();
}
