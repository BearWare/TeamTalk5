project(OpenSSL)

include(ExternalProject)

##################################################
# OpenSSL
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_OPENSSL ${TOOLCHAIN_INSTALL_PREFIX}/openssl)

if (MSVC)

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      ExternalProject_Add(openssl-src
        GIT_REPOSITORY    https://github.com/openssl/openssl.git
        GIT_TAG           openssl-3.6.0
        GIT_SHALLOW       TRUE
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/openssl
        CONFIGURE_COMMAND ""
        CONFIGURE_COMMAND perl <SOURCE_DIR>\\Configure no-shared no-asm --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR>\\ssl VC-WIN64A
        BUILD_COMMAND     set CL=/MP
        COMMAND           nmake
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}
        INSTALL_COMMAND   nmake install
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libcrypto.lib <INSTALL_DIR>/lib/libssl.lib
        )
    else()
      ExternalProject_Add(openssl-src
        GIT_REPOSITORY    https://github.com/openssl/openssl.git
        GIT_TAG           openssl-3.6.0
        GIT_SHALLOW       TRUE
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/openssl
        CONFIGURE_COMMAND perl <SOURCE_DIR>\\Configure no-shared no-asm --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR>\\ssl VC-WIN32
        BUILD_COMMAND     set CL=/MP
        COMMAND           nmake
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}
        INSTALL_COMMAND   nmake install
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libcrypto.lib <INSTALL_DIR>/lib/libssl.lib
        )
    endif()
    ExternalProject_Get_Property(openssl-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  set (OPENSSL_DIR ${INSTALL_DIR} PARENT_SCOPE)

  add_library(openssl-crypto STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(openssl-crypto openssl-src)
  endif()
  target_include_directories (openssl-crypto INTERFACE ${INSTALL_DIR}/include)
  target_link_libraries(openssl-crypto INTERFACE crypt32.lib)
  set_property(TARGET openssl-crypto PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libcrypto.lib)

  add_library(openssl-ssl STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(openssl-ssl openssl-src)
  endif()
  target_include_directories (openssl-ssl INTERFACE ${INSTALL_DIR}/include)
  target_link_libraries(openssl-ssl INTERFACE crypt32.lib)
  set_property(TARGET openssl-ssl PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libssl.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    message(FATAL_ERROR "No deployment target specified")
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(openssl-intel-src
      GIT_REPOSITORY    https://github.com/openssl/openssl.git
      GIT_TAG           openssl-3.6.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/openssl-intel
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}-intel
      CONFIGURE_COMMAND <SOURCE_DIR>/Configure --prefix=<INSTALL_DIR> darwin64-x86_64-cc -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install_sw install_ssldirs
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libcrypto.a <INSTALL_DIR>/lib/libssl.a
      )
    ExternalProject_Get_Property(openssl-intel-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/lib)

    ExternalProject_Add(openssl-arm64-src
      GIT_REPOSITORY    https://github.com/openssl/openssl.git
      GIT_TAG           openssl-3.6.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/openssl-arm64
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}-arm64
      CONFIGURE_COMMAND <SOURCE_DIR>/Configure --prefix=<INSTALL_DIR> darwin64-arm64-cc -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install_sw install_ssldirs
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libcrypto.a <INSTALL_DIR>/lib/libssl.a
      )
    ExternalProject_Get_Property(openssl-arm64-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/lib)

    # Use 'lipo' to create universal binary
    file(MAKE_DIRECTORY ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/lib)
    file(MAKE_DIRECTORY ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/include)
    add_custom_command (OUTPUT ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/lib/libcrypto.a
                               ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/lib/libssl.a
                               ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/include/openssl/opensslconf.h
      COMMAND lipo -create ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}-arm64/lib/libcrypto.a ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}-intel/lib/libcrypto.a -output ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/lib/libcrypto.a
      COMMAND lipo -create ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}-arm64/lib/libssl.a ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}-intel/lib/libssl.a -output ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/lib/libssl.a
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}-intel/include ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/include
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}-intel/ssl ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/ssl
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}-intel/lib/pkgconfig ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/lib/pkgconfig
      COMMAND sed -i "" -e "s/openssl-intel/openssl/" ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/lib/pkgconfig/libcrypto.pc
      COMMAND sed -i "" -e "s/openssl-intel/openssl/" ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/lib/pkgconfig/libssl.pc
      COMMAND sed -i "" -e "s/openssl-intel/openssl/" ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/lib/pkgconfig/openssl.pc
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating universal libcrypto.a" VERBATIM
      DEPENDS openssl-intel-src openssl-arm64-src)

    add_custom_target (openssl-src DEPENDS ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/lib/libcrypto.a ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/lib/libssl.a ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}/include/openssl/opensslconf.h)
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL})
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  set (OPENSSL_DIR ${INSTALL_DIR} PARENT_SCOPE)

  add_library(openssl-crypto STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(openssl-crypto openssl-src)
  endif()
  target_include_directories (openssl-crypto INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET openssl-crypto PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libcrypto.a)

  add_library(openssl-ssl STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(openssl-ssl openssl-src)
  endif()
  target_include_directories (openssl-ssl INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET openssl-ssl PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libssl.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  if ("armv7" STREQUAL "${CMAKE_OSX_ARCHITECTURES}" OR "arm64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (OPENSSL_CFG_PREFIX export CROSS_TOP=${CMAKE_OSX_SYSROOT}/../.. && export CROSS_SDK=iPhoneOS.sdk)
    set (OPENSSL_CFG_TARGET iphoneos-cross)
    set (OPENSSL_CFG_POSTFIX -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
  else()
    set (OPENSSL_CFG_PREFIX export CROSS_TOP=${CMAKE_OSX_SYSROOT}/../.. && export CROSS_SDK=iPhoneSimulator.sdk)
    set (OPENSSL_CFG_TARGET iossimulator-xcrun)
    set (OPENSSL_CFG_POSTFIX -mios-simulator-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(openssl-src
      GIT_REPOSITORY    https://github.com/openssl/openssl.git
      GIT_TAG           openssl-3.6.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/openssl
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}
      CONFIGURE_COMMAND ${OPENSSL_CFG_PREFIX} && export CFLAGS=-arch\ ${CMAKE_OSX_ARCHITECTURES} && <SOURCE_DIR>/Configure --prefix=<INSTALL_DIR> ${OPENSSL_CFG_TARGET} no-shared no-async no-tests ${OPENSSL_CFG_POSTFIX}
      BUILD_COMMAND     ${OPENSSL_CFG_PREFIX} && make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   ${OPENSSL_CFG_PREFIX} && make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libcrypto.a <INSTALL_DIR>/lib/libssl.a
      )
    ExternalProject_Get_Property(openssl-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/lib)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  set (OPENSSL_DIR ${INSTALL_DIR} PARENT_SCOPE)

  add_library(openssl-crypto STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(openssl-crypto openssl-src)
  endif()
  target_include_directories (openssl-crypto INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET openssl-crypto PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libcrypto.a)

  add_library(openssl-ssl STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(openssl-ssl openssl-src)
  endif()
  target_include_directories (openssl-ssl INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET openssl-ssl PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libssl.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(openssl-src
      GIT_REPOSITORY    https://github.com/openssl/openssl.git
      GIT_TAG           openssl-3.6.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/openssl
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}
      CONFIGURE_COMMAND <SOURCE_DIR>/Configure --prefix=<INSTALL_DIR> -DPIC -fPIC linux-x86_64
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libcrypto.a <INSTALL_DIR>/lib/libssl.a
      )
    ExternalProject_Get_Property(openssl-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/lib)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  set (OPENSSL_DIR ${INSTALL_DIR} PARENT_SCOPE)

  add_library(openssl-crypto STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(openssl-crypto openssl-src)
  endif()
  target_include_directories (openssl-crypto INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(openssl-crypto PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libcrypto.a)

  add_library(openssl-ssl STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(openssl-ssl openssl-src)
  endif()
  target_include_directories (openssl-ssl INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(openssl-ssl PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libssl.a)
  target_compile_options (openssl-ssl INTERFACE -pthread)
  target_link_options(openssl-ssl INTERFACE -pthread)

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

  set (OPENSSL_EXPORT export PATH=${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin:$ENV{PATH} && export ANDROID_NDK_ROOT=${CMAKE_ANDROID_NDK})

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(openssl-src
      GIT_REPOSITORY    https://github.com/openssl/openssl.git
      GIT_TAG           openssl-3.6.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/openssl
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL}
      CONFIGURE_COMMAND ${OPENSSL_EXPORT} && <SOURCE_DIR>/Configure --prefix=<INSTALL_DIR> -DPIC -fPIC no-shared
      ${OPENSSL_CFG_TARGET} -D__ANDROID_API__=${ANDROID_PLATFORM_LEVEL}
      BUILD_COMMAND     ${OPENSSL_EXPORT} && make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   ${OPENSSL_EXPORT} && make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libcrypto.a <INSTALL_DIR>/lib/libssl.a
      )
    ExternalProject_Get_Property(openssl-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/lib)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OPENSSL})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  set (OPENSSL_DIR ${INSTALL_DIR} PARENT_SCOPE)

  add_library(openssl-crypto STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(openssl-crypto openssl-src)
  endif()
  target_include_directories (openssl-crypto INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(openssl-crypto PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libcrypto.a)

  add_library(openssl-ssl STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(openssl-ssl openssl-src)
  endif()
  target_include_directories (openssl-ssl INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(openssl-ssl PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libssl.a)
  target_compile_options (openssl-ssl INTERFACE -pthread)
  target_link_options(openssl-ssl INTERFACE -pthread)

endif()
