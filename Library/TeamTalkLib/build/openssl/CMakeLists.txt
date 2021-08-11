cmake_minimum_required(VERSION 3.1)

project(OpenSSL)

include(ExternalProject)

##################################################
# OpenSSL
##################################################
if (MSVC)

  if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    ExternalProject_Add(openssl-win64
      GIT_REPOSITORY    https://github.com/bear101/openssl
      GIT_TAG           2c3f861718
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      CONFIGURE_COMMAND ""
      BUILD_IN_SOURCE   TRUE
      BUILD_COMMAND     build_win64.bat
      INSTALL_COMMAND   ""
      )
    ExternalProject_Get_Property(openssl-win64 SOURCE_DIR)
    set (OPENSSL_DIR ${SOURCE_DIR})
    set (OPENSSL_DIR ${SOURCE_DIR} PARENT_SCOPE)
    set (OPENSSL_DEPEND openssl-win64)
  else()
    ExternalProject_Add(openssl-win32
      GIT_REPOSITORY    https://github.com/bear101/openssl
      GIT_TAG           2c3f861718
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      CONFIGURE_COMMAND ""
      BUILD_IN_SOURCE   TRUE
      BUILD_COMMAND     build_win32.bat
      INSTALL_COMMAND   ""
      )
    ExternalProject_Get_Property(openssl-win32 SOURCE_DIR)
    set (OPENSSL_DIR ${SOURCE_DIR})
    set (OPENSSL_DIR ${SOURCE_DIR} PARENT_SCOPE)
    set (OPENSSL_DEPEND openssl-win32)
  endif()

  file(MAKE_DIRECTORY ${OPENSSL_DIR}/include)

  add_library(openssl-crypto STATIC IMPORTED GLOBAL)
  add_dependencies(openssl-crypto ${OPENSSL_DEPEND})
  target_include_directories (openssl-crypto INTERFACE ${OPENSSL_DIR}/include)
  target_link_libraries(openssl-crypto INTERFACE crypt32.lib)
  set_property(TARGET openssl-crypto PROPERTY IMPORTED_LOCATION ${OPENSSL_DIR}/lib/$(PlatformName)/lib/libcrypto.lib)

  add_library(openssl-ssl STATIC IMPORTED GLOBAL)
  add_dependencies(openssl-ssl ${OPENSSL_DEPEND})
  target_include_directories (openssl-ssl INTERFACE ${OPENSSL_DIR}/include)
  target_link_libraries(openssl-ssl INTERFACE crypt32.lib)
  set_property(TARGET openssl-ssl PROPERTY IMPORTED_LOCATION ${OPENSSL_DIR}/lib/$(PlatformName)/lib/libssl.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    message(FATAL_ERROR "No deployment target specified")
  endif()
    
  ExternalProject_Add(openssl-src
    GIT_REPOSITORY    https://github.com/bear101/openssl
    GIT_TAG           OpenSSL_1_1_1k
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CONFIGURE_COMMAND <SOURCE_DIR>/Configure --prefix=<INSTALL_DIR> darwin64-x86_64-cc -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
    BUILD_COMMAND     make
    INSTALL_COMMAND   make install
    )
  ExternalProject_Get_Property(openssl-src INSTALL_DIR)
  set (OPENSSL_DIR ${INSTALL_DIR})
  set (OPENSSL_DIR ${INSTALL_DIR} PARENT_SCOPE)
  
  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  file(MAKE_DIRECTORY ${INSTALL_DIR}/lib)

  add_library(openssl-crypto STATIC IMPORTED GLOBAL)
  add_dependencies(openssl-crypto openssl-src)
  target_include_directories (openssl-crypto INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET openssl-crypto PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libcrypto.lib)

  add_library(openssl-ssl STATIC IMPORTED GLOBAL)
  add_dependencies(openssl-ssl openssl-src)
  target_include_directories (openssl-ssl INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET openssl-ssl PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libssl.lib)
  
endif()
