# opensc-pkcs11 for Android - integrates with libftreader-pcsc
# Prerequisite: run scripts/setup_opensc_android.sh
cmake_minimum_required(VERSION 3.10)
project(opensc-pkcs11)

set(OPENSC_SRC "${CMAKE_SOURCE_DIR}/opensc")
# Main project: sdk/ftreader-pcsc/include; SDK package demo: ../../../include (Reader_SDK/include)
set(FTREADER_INCLUDE "${CMAKE_SOURCE_DIR}/../../sdk/ftreader-pcsc/include")
if(NOT EXISTS "${FTREADER_INCLUDE}/winscard.h")
    set(FTREADER_INCLUDE "${CMAKE_SOURCE_DIR}/../../../include")
endif()
if(NOT EXISTS "${FTREADER_INCLUDE}/winscard.h")
    message(FATAL_ERROR "winscard.h not found. Tried:\n  ${CMAKE_SOURCE_DIR}/../../sdk/ftreader-pcsc/include\n  ${CMAKE_SOURCE_DIR}/../../../include")
endif()
set(OPENSSL_DIR "${CMAKE_SOURCE_DIR}/libs")
set(OPENSSL_INCLUDE "${OPENSSL_DIR}/include")

if(NOT EXISTS "${OPENSC_SRC}/src/pkcs11/pkcs11-global.c")
    message(FATAL_ERROR
        "OpenSC source not found. Run:\n"
        "  ./scripts/setup_opensc_android.sh\n"
        "  or: git clone https://github.com/OpenSC/OpenSC.git thirdparty/opensc-pkcs11/opensc")
endif()

if(NOT EXISTS "${OPENSSL_DIR}/${ANDROID_ABI}/libcrypto.a")
    message(FATAL_ERROR "OpenSSL not found. Run: ./scripts/setup_opensc_android.sh")
endif()
if(NOT EXISTS "${OPENSSL_INCLUDE}/openssl/opensslv.h")
    message(FATAL_ERROR "OpenSSL include not found. Run: ./scripts/setup_opensc_android.sh (requires network)")
endif()

add_definitions(
    -D__ANDROID__
    -DHAVE_CONFIG_H
    -DHAVE_BUILTIN_OVERFLOW
    -D_DEFAULT_SOURCE
    -DOPENSC_CONF_PATH=\"none\"
    -DDEFAULT_SM_MODULE_PATH=\"none\"
    -DDEFAULT_SM_MODULE=\"none\"
    -DSC_PKCS15_PROFILE_DIRECTORY=\"none\"
    -DFORCE_GDIS
)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/main/cpp
    ${OPENSC_SRC}/src
    ${FTREADER_INCLUDE}
    ${OPENSSL_INCLUDE}
)

# OpenSSL
add_library(crypto STATIC IMPORTED)
set_target_properties(crypto PROPERTIES
    IMPORTED_LOCATION ${OPENSSL_DIR}/${ANDROID_ABI}/libcrypto.a)
add_library(ssl STATIC IMPORTED)
set_target_properties(ssl PROPERTIES
    IMPORTED_LOCATION ${OPENSSL_DIR}/${ANDROID_ABI}/libssl.a)

# Collect libopensc sources (adapts to OpenSC version changes)
file(GLOB OPENSC_LIBOPENSC_SRCS "${OPENSC_SRC}/src/libopensc/*.c")
# Exclude OpenPACE (eac) dependent files - Android build does not include OpenPACE
list(FILTER OPENSC_LIBOPENSC_SRCS EXCLUDE REGEX ".*/card-npa\\.c$")
list(FILTER OPENSC_LIBOPENSC_SRCS EXCLUDE REGEX ".*/card-dtrust\\.c$")
file(GLOB OPENSC_PKCS15INIT_SRCS "${OPENSC_SRC}/src/pkcs15init/*.c")

# opensc-pkcs11 core
# SCard* symbols not linked here; resolved at runtime from pre-loaded libftreader-pcsc
add_library(opensc-pkcs11 SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/card_npa_dtrust_stub.c
    ${OPENSC_SRC}/src/common/compat_dummy.c
    ${OPENSC_SRC}/src/common/compat_strlcat.c
    ${OPENSC_SRC}/src/common/compat_strlcpy.c
    ${OPENSC_SRC}/src/common/compat_strnlen.c
    ${OPENSC_SRC}/src/common/compat_getpass.c
    ${OPENSC_SRC}/src/common/compat_getopt.c
    ${OPENSC_SRC}/src/common/compat_report_rangecheckfailure.c
    ${OPENSC_SRC}/src/common/compat___iob_func.c
    ${OPENSC_SRC}/src/common/compat_overflow.c
    ${OPENSC_SRC}/src/common/simclist.c
    ${OPENSC_SRC}/src/common/libscdl.c
    ${OPENSC_SRC}/src/scconf/scconf.c
    ${OPENSC_SRC}/src/scconf/parse.c
    ${OPENSC_SRC}/src/scconf/write.c
    ${OPENSC_SRC}/src/scconf/sclex.c
    ${OPENSC_SRC}/src/ui/strings.c
    ${OPENSC_SRC}/src/ui/notify.c
    ${OPENSC_PKCS15INIT_SRCS}
    ${OPENSC_SRC}/src/sm/sm-common.c
    ${OPENSC_SRC}/src/sm/sm-iso.c
    ${OPENSC_LIBOPENSC_SRCS}
    ${OPENSC_SRC}/src/pkcs11/pkcs11-global.c
    ${OPENSC_SRC}/src/pkcs11/pkcs11-session.c
    ${OPENSC_SRC}/src/pkcs11/pkcs11-object.c
    ${OPENSC_SRC}/src/pkcs11/misc.c
    ${OPENSC_SRC}/src/pkcs11/slot.c
    ${OPENSC_SRC}/src/pkcs11/mechanism.c
    ${OPENSC_SRC}/src/pkcs11/openssl.c
    ${OPENSC_SRC}/src/pkcs11/framework-pkcs15.c
    ${OPENSC_SRC}/src/pkcs11/framework-pkcs15init.c
    ${OPENSC_SRC}/src/pkcs11/debug.c
    ${OPENSC_SRC}/src/pkcs11/pkcs11-display.c
)

target_link_libraries(opensc-pkcs11 crypto ssl log z)
# Ensure Android logcat path: explicitly define __ANDROID__ and ANDROID
target_compile_definitions(opensc-pkcs11 PRIVATE __ANDROID__=1 ANDROID=1)
# Allow undefined SCard*; resolved at runtime from libftreader-pcsc (must load first)
set_target_properties(opensc-pkcs11 PROPERTIES
    LINK_FLAGS "-Wl,--allow-shlib-undefined"
)
