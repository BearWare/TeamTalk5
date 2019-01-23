if (MSVC)
    set (OPENSSL_INCLUDE_DIR ${TTLIBS_ROOT}/openssl/include)
    set (OPENSSL_STATIC_LIB ${TTLIBS_ROOT}/openssl/libssl.lib ${TTLIBS_ROOT}/openssl/libcrypto.lib crypt32.lib)
    set (OPENSSL_LINK_FLAGS ${OPENSSL_STATIC_LIB})
else()

  # ACE is built using the static OpenSSL we by default we 
  # include this path although the shared libssl.so and libcrypto.so
  # may use a different version of OpenSSL
  set (OPENSSL_INCLUDE_DIR ${TTLIBS_ROOT}/openssl/include)
  set (OPENSSL_STATIC_LIB ${TTLIBS_ROOT}/openssl/lib/libssl.a ${TTLIBS_ROOT}/openssl/lib/libcrypto.a)

  if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    # On Ubuntu we link using libssl-dev package
    find_library(OPENSSL_SSL_LIBRARY ssl)
    set (OPENSSL_LINK_FLAGS ${OPENSSL_SSL_LIBRARY})
    find_library(OPENSSL_CRYPTO_LIBRARY crypto)
    list (APPEND OPENSSL_LINK_FLAGS ${OPENSSL_CRYPTO_LIBRARY})
  else() # (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (OPENSSL_LINK_FLAGS ${OPENSSL_STATIC_LIB})
    set (OPENSSL_INCLUDE_DIR ${TTLIBS_ROOT}/openssl/include)
  endif()
endif()
