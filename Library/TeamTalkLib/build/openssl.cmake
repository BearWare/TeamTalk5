# ACE is built using the static OpenSSL library from
# ${TTLIBS_ROOT}/openssl on Windows, macOS, Android and iOS.
#
# On Linux distributions we build ACE from shared library by
# linking libssl-dev package

if (MSVC)
  
  set (OPENSSL_LINK_FLAGS openssl-ssl openssl-crypto)
  
else()

  if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_library(OPENSSL_SSL_LIBRARY ssl)
    set (OPENSSL_LINK_FLAGS ${OPENSSL_SSL_LIBRARY})
    find_library(OPENSSL_CRYPTO_LIBRARY crypto)
    list (APPEND OPENSSL_LINK_FLAGS ${OPENSSL_CRYPTO_LIBRARY})
    # Include path OPENSSL_INCLUDE_DIR is automatically set by CMake.
  else()
    set (OPENSSL_INCLUDE_DIR ${TTLIBS_ROOT}/openssl/include)
    set (OPENSSL_STATIC_LIB ${TTLIBS_ROOT}/openssl/lib/libssl.a ${TTLIBS_ROOT}/openssl/lib/libcrypto.a)
    set (OPENSSL_LINK_FLAGS ${OPENSSL_STATIC_LIB})
  endif()
endif()
