# ACE is built using the static OpenSSL library from on Windows,
# macOS, Android and iOS.
#
# On Linux distributions we build ACE using OpenSSL shared library by
# linking libssl-dev package

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  find_library(OPENSSL_SSL_LIBRARY ssl)
  if (NOT OPENSSL_SSL_LIBRARY)
    message (FATAL_ERROR "OpenSSL SSL library not found")
  endif()
  set (OPENSSL_LINK_FLAGS ${OPENSSL_SSL_LIBRARY})

  find_library(OPENSSL_CRYPTO_LIBRARY crypto)
  if (NOT OPENSSL_CRYPTO_LIBRARY)
    message (FATAL_ERROR "OpenSSL Crypto library not found")
  endif()
  list (APPEND OPENSSL_LINK_FLAGS ${OPENSSL_CRYPTO_LIBRARY})

else()

  set (OPENSSL_LINK_FLAGS openssl-ssl openssl-crypto)

endif()
