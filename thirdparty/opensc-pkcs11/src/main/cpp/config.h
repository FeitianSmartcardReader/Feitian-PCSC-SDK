/* Minimal config.h for OpenSC Android build */
#ifndef CONFIG_H
#define CONFIG_H

#define HAVE_CONFIG_H 1
#define HAVE_STDINT_H 1
#define PACKAGE "opensc"
#define PACKAGE_VERSION "0.25.0"
#define VERSION "0.25.0"

#define HAVE_STRNLEN 1
#define HAVE_STRLCAT 1
#define HAVE_PTHREAD 1
#define HAVE_STRLCPY 1
#define HAVE_GETPASS 0
#define HAVE_GETOPT 1
#define HAVE_READLINE 0
#define HAVE_OPENSSL 1
#define HAVE_LIBZ 1
#define HAVE_PCSC 1
#define ENABLE_PCSC 1
#define HAVE_WINSCARD_H 1
#define DEFAULT_PCSC_PROVIDER "libftreader-pcsc.so"
#define HAVE_UNISTD_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_ENDIAN_H 1
#define ENABLE_OPENSSL 1
#define ENABLE_ZLIB 1
#define ENABLE_READER_PCSC 1

#define OPENSC_VERSION_MAJOR 0
#define OPENSC_VERSION_MINOR 25
#define OPENSC_VS_FF_COMPANY_NAME "OpenSC Project"
#define OPENSC_VS_FF_PRODUCT_NAME "OpenSC PKCS#11"

#define OPENSC_CONF_PATH "none"
#define DEFAULT_SM_MODULE_PATH "none"
#define DEFAULT_SM_MODULE "none"
#define SC_PKCS15_PROFILE_DIRECTORY "none"

#endif
