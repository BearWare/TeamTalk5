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
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libcrypto.a <INSTALL_DIR>/lib/libssl.a
    )
  ExternalProject_Get_Property(openssl-src INSTALL_DIR)
  set (OPENSSL_DIR ${INSTALL_DIR})
  set (OPENSSL_DIR ${INSTALL_DIR} PARENT_SCOPE)
  
  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  file(MAKE_DIRECTORY ${INSTALL_DIR}/lib)

  add_library(openssl-crypto STATIC IMPORTED GLOBAL)
  add_dependencies(openssl-crypto openssl-src)
  target_include_directories (openssl-crypto INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET openssl-crypto PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libcrypto.a)

  add_library(openssl-ssl STATIC IMPORTED GLOBAL)
  add_dependencies(openssl-ssl openssl-src)
  target_include_directories (openssl-ssl INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET openssl-ssl PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libssl.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  if ("armv7" STREQUAL "${CMAKE_OSX_ARCHITECTURES}" OR "arm64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (OPENSSL_CFG_PREFIX export CROSS_TOP=${CMAKE_OSX_SYSROOT}/../.. && export CROSS_SDK=iPhoneOS.sdk)
    set (OPENSSL_CFG_TARGET iphoneos-cross)
    set (OPENSSL_CFG_POSTFIX -fembed-bitcode -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
  else()
    set (OPENSSL_CFG_PREFIX export CROSS_TOP=${CMAKE_OSX_SYSROOT}/../.. && export CROSS_SDK=iPhoneSimulator.sdk)
    set (OPENSSL_CFG_TARGET iossimulator-xcrun)
    set (OPENSSL_CFG_POSTFIX -mios-simulator-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()
  
  ExternalProject_Add(openssl-src
    GIT_REPOSITORY    https://github.com/bear101/openssl
    GIT_TAG           OpenSSL_1_1_1k
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CONFIGURE_COMMAND ${OPENSSL_CFG_PREFIX} && export CFLAGS=-arch\ ${CMAKE_OSX_ARCHITECTURES} && <SOURCE_DIR>/Configure --prefix=<INSTALL_DIR> ${OPENSSL_CFG_TARGET} no-shared no-async no-tests ${OPENSSL_CFG_POSTFIX}
    BUILD_COMMAND     ${OPENSSL_CFG_PREFIX} && make
    INSTALL_COMMAND   ${OPENSSL_CFG_PREFIX} && make install
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libcrypto.a <INSTALL_DIR>/lib/libssl.a
    )
  ExternalProject_Get_Property(openssl-src INSTALL_DIR)
  set (OPENSSL_DIR ${INSTALL_DIR})
  set (OPENSSL_DIR ${INSTALL_DIR} PARENT_SCOPE)
  
  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  file(MAKE_DIRECTORY ${INSTALL_DIR}/lib)

  add_library(openssl-crypto STATIC IMPORTED GLOBAL)
  add_dependencies(openssl-crypto openssl-src)
  target_include_directories (openssl-crypto INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET openssl-crypto PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libcrypto.a)

  add_library(openssl-ssl STATIC IMPORTED GLOBAL)
  add_dependencies(openssl-ssl openssl-src)
  target_include_directories (openssl-ssl INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET openssl-ssl PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libssl.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  ExternalProject_Add(openssl-src
    GIT_REPOSITORY    https://github.com/bear101/openssl
    GIT_TAG           OpenSSL_1_1_1k
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CONFIGURE_COMMAND <SOURCE_DIR>/Configure --prefix=<INSTALL_DIR> -DPIC -fPIC linux-x86_64
    BUILD_COMMAND     make
    INSTALL_COMMAND   make install
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libcrypto.a <INSTALL_DIR>/lib/libssl.a
    )
  ExternalProject_Get_Property(openssl-src INSTALL_DIR)
  set (OPENSSL_DIR ${INSTALL_DIR})
  set (OPENSSL_DIR ${INSTALL_DIR} PARENT_SCOPE)
  
  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  file(MAKE_DIRECTORY ${INSTALL_DIR}/lib)

  add_library(openssl-crypto STATIC IMPORTED GLOBAL)
  add_dependencies(openssl-crypto openssl-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(openssl-crypto PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      INTERFACE_COMPILE_OPTIONS -pthread
      INTERFACE_LINK_OPTIONS -pthread
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libcrypto.a)
  else()
    target_include_directories (openssl-crypto INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(openssl-crypto PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libcrypto.a)
  endif()

  add_library(openssl-ssl STATIC IMPORTED GLOBAL)
  add_dependencies(openssl-ssl openssl-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(openssl-ssl PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      INTERFACE_COMPILE_OPTIONS -pthread
      INTERFACE_LINK_OPTIONS -pthread
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libssl.a)
  else()
    target_include_directories (openssl-ssl INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(openssl-ssl PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libssl.a)
    target_compile_options (openssl-ssl INTERFACE -pthread)
    target_link_options(openssl-ssl INTERFACE -pthread)
  endif()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  if ("armeabi-v7a" STREQUAL "${ANDROID_ABI}")
    set (OPENSSL_CFG_TARGET android-arm)
  elseif ("arm64-v8a" STREQUAL "${ANDROID_ABI}")
    set (OPENSSL_CFG_TARGET android-arm64)
  elseif ("x86" STREQUAL "${ANDROID_ABI}")
    set (OPENSSL_CFG_TARGET android-x86)
  elseif ("x86_64" STREQUAL "${ANDROID_ABI}")
    set (OPENSSL_CFG_TARGET android-x86_64)
  else()
    message(WARNING "Unknown Android architecture")
  endif()

  set (OPENSSL_EXPORT export PATH=${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin:$ENV{PATH} && export ANDROID_NDK_HOME=${CMAKE_ANDROID_NDK})
    
  ExternalProject_Add(openssl-src
    GIT_REPOSITORY    https://github.com/bear101/openssl
    GIT_TAG           OpenSSL_1_1_1k
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CONFIGURE_COMMAND ${OPENSSL_EXPORT} && <SOURCE_DIR>/Configure --prefix=<INSTALL_DIR> -DPIC -fPIC no-shared
                      ${OPENSSL_CFG_TARGET} -D__ANDROID_API__=${ANDROID_PLATFORM_LEVEL}
    BUILD_COMMAND     ${OPENSSL_EXPORT} && make
    INSTALL_COMMAND   ${OPENSSL_EXPORT} && make install
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libcrypto.a <INSTALL_DIR>/lib/libssl.a
    )
  ExternalProject_Get_Property(openssl-src INSTALL_DIR)
  set (OPENSSL_DIR ${INSTALL_DIR})
  set (OPENSSL_DIR ${INSTALL_DIR} PARENT_SCOPE)
  
  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  file(MAKE_DIRECTORY ${INSTALL_DIR}/lib)

  add_library(openssl-crypto STATIC IMPORTED GLOBAL)
  add_dependencies(openssl-crypto openssl-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(openssl-crypto PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      INTERFACE_COMPILE_OPTIONS -pthread
      INTERFACE_LINK_OPTIONS -pthread
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libcrypto.a)
  else()
    target_include_directories (openssl-crypto INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(openssl-crypto PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libcrypto.a)
  endif()

  add_library(openssl-ssl STATIC IMPORTED GLOBAL)
  add_dependencies(openssl-ssl openssl-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(openssl-ssl PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      INTERFACE_COMPILE_OPTIONS -pthread
      INTERFACE_LINK_OPTIONS -pthread
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libssl.a)
  else()
    target_include_directories (openssl-ssl INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(openssl-ssl PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libssl.a)
    target_compile_options (openssl-ssl INTERFACE -pthread)
    target_link_options(openssl-ssl INTERFACE -pthread)
  endif()
  
endif()
