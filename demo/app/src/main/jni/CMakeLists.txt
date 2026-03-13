cmake_minimum_required(VERSION 3.10)
project(pcsc-demo)

add_library(pkcs11_test SHARED pkcs11_test_jni.c)
target_link_libraries(pkcs11_test log)
