# ACE is built using the static OpenSSL library from on Windows,
# macOS, Android and iOS.
#
# On Linux distributions we build ACE using OpenSSL shared library by
# linking libssl-dev package

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  find_library(OPENSSL_SSL_LIBRARY ssl)
  set (OPENSSL_LINK_FLAGS ${OPENSSL_SSL_LIBRARY})
  find_library(OPENSSL_CRYPTO_LIBRARY crypto)
  list (APPEND OPENSSL_LINK_FLAGS ${OPENSSL_CRYPTO_LIBRARY})
  # Include path OPENSSL_INCLUDE_DIR is automatically set by CMake.

else()

  set (OPENSSL_LINK_FLAGS openssl-ssl openssl-crypto)

endif()
