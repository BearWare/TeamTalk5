project(LibVPX)

include(ExternalProject)

##################################################
# LibVPX
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_LIBVPX ${TOOLCHAIN_INSTALL_PREFIX}/libvpx)

if (MSVC)

  if (NOT CMAKE_VS_PLATFORM_NAME)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      set (CMAKE_VS_PLATFORM_NAME x64)
    else()
      set (CMAKE_VS_PLATFORM_NAME win32)
    endif()
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)

      find_path(YASM_EXE_DIRECTORY yasm.exe PATHS C:/tt5dist/yasm/x64)
      if (YASM_EXE_DIRECTORY-NOTFOUND)
        message(WARNING "yasm.exe is required to build libvpx. See https://yasm.tortall.net")
      else()
        message("Found YASM for libvpx here: ${YASM_EXE_DIRECTORY}")
      endif()

      file (TO_NATIVE_PATH ${YASM_EXE_DIRECTORY} YASM_EXE_DIRECTORY_NATIVE)
      set (LIBVPX_WIN_TARGET x86_64-win64-vs17)
      configure_file(${CMAKE_CURRENT_LIST_DIR}/build-libvpx-win.bat_sample ${CMAKE_CURRENT_BINARY_DIR}/build-libvpx-win64.bat @ONLY)

      ExternalProject_Add(libvpx-src
        GIT_REPOSITORY    https://github.com/webmproject/libvpx.git
        GIT_TAG           v1.15.2
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/libvpx
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ${CMAKE_CURRENT_BINARY_DIR}/build-libvpx-win64.bat
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}
        INSTALL_COMMAND   cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_headers.bat <INSTALL_DIR>
        COMMAND           cd <SOURCE_DIR>/build-${LIBVPX_WIN_TARGET}
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_libs.bat x64 <INSTALL_DIR>
        BUILD_IN_SOURCE   TRUE
        BUILD_BYPRODUCTS  <INSTALL_DIR>/${CMAKE_VS_PLATFORM_NAME}/Debug/vpxmtd.lib
                          <INSTALL_DIR>/${CMAKE_VS_PLATFORM_NAME}/Release/vpxmt.lib
        )
    else()

      find_path(YASM_EXE_DIRECTORY yasm.exe PATHS C:/tt5dist/yasm/win32)
      if (YASM_EXE_DIRECTORY-NOTFOUND)
        message(WARNING "yasm.exe is required to build libvpx. See https://yasm.tortall.net")
      else()
        message("Found YASM.exe for libvpx here: ${YASM_EXE_DIRECTORY}")
      endif()

      file (TO_NATIVE_PATH ${YASM_EXE_DIRECTORY} YASM_EXE_DIRECTORY_NATIVE)
      set (LIBVPX_WIN_TARGET x86-win32-vs17)
      configure_file(${CMAKE_CURRENT_LIST_DIR}/build-libvpx-win.bat_sample ${CMAKE_CURRENT_BINARY_DIR}/build-libvpx-win32.bat @ONLY)

      ExternalProject_Add(libvpx-src
        GIT_REPOSITORY    https://github.com/webmproject/libvpx.git
        GIT_TAG           v1.15.2
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/libvpx
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     ${CMAKE_CURRENT_BINARY_DIR}/build-libvpx-win32.bat
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}
        INSTALL_COMMAND   cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_headers.bat <INSTALL_DIR>
        COMMAND           cd <SOURCE_DIR>/build-${LIBVPX_WIN_TARGET}
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_libs.bat win32 <INSTALL_DIR>
        BUILD_IN_SOURCE   TRUE
        BUILD_BYPRODUCTS  <INSTALL_DIR>/${CMAKE_VS_PLATFORM_NAME}/Debug/vpxmtd.lib
                          <INSTALL_DIR>/${CMAKE_VS_PLATFORM_NAME}/Release/vpxmt.lib
        )
    endif()
    ExternalProject_Get_Property(libvpx-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  add_library(libvpx STATIC IMPORTED GLOBAL)
  target_include_directories (libvpx INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(libvpx PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/${CMAKE_VS_PLATFORM_NAME}/Debug/vpxmtd.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/${CMAKE_VS_PLATFORM_NAME}/Release/vpxmt.lib)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(libvpx libvpx-src)
  endif()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)

    ExternalProject_Add(libvpx-arm64-src
      GIT_REPOSITORY    https://github.com/webmproject/libvpx.git
      GIT_TAG           v1.15.2
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/libvpx-arm64
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/libvpx-arm64
      CONFIGURE_COMMAND <SOURCE_DIR>/configure --extra-cflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
      --prefix=<INSTALL_DIR> --target=arm64-darwin20-gcc
      --disable-examples --disable-docs --disable-unit-tests
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libvpx.a
      )
    ExternalProject_Get_Property(libvpx-arm64-src INSTALL_DIR)

    ExternalProject_Add(libvpx-intel-src
      GIT_REPOSITORY    https://github.com/webmproject/libvpx.git
      GIT_TAG           v1.15.2
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/libvpx-intel
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/libvpx-intel
      CONFIGURE_COMMAND <SOURCE_DIR>/configure --extra-cflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
      --prefix=<INSTALL_DIR> --target=x86_64-darwin20-gcc
      --disable-examples --disable-docs --disable-unit-tests
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libvpx.a
      )
    ExternalProject_Get_Property(libvpx-intel-src INSTALL_DIR)

    # Use 'lipo' to create universal binary
    file(MAKE_DIRECTORY ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}/lib)
    file(MAKE_DIRECTORY ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}/include)
    add_custom_command (OUTPUT ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}/lib/libvpx.a
      ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}/include/vpx/vpx_codec.h
      COMMAND lipo -create ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}-arm64/lib/libvpx.a ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}-intel/lib/libvpx.a -output ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}/lib/libvpx.a
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}-intel/include ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}/include
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating universal libvpx binaries"
      DEPENDS libvpx-intel-src libvpx-arm64-src)

    add_custom_target (libvpx-src DEPENDS ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}/lib/libvpx.a
      ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}/include/vpx/vpx_codec.h)
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX})
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  add_library(libvpx STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(libvpx libvpx-src)
  endif()
  target_include_directories (libvpx INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(libvpx PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libvpx.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  if ("armv7" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (LIBVPX_CPPFLAGS -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set (LIBVPX_TARGET armv7-darwin-gcc)
  elseif ("arm64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (LIBVPX_CPPFLAGS -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set (LIBVPX_TARGET arm64-darwin-gcc)
  elseif ("i386" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (LIBVPX_CPPFLAGS -mios-simulator-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set (LIBVPX_TARGET x86-iphonesimulator-gcc)
  elseif ("x86_64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (LIBVPX_CPPFLAGS -mios-simulator-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    set (LIBVPX_TARGET x86_64-iphonesimulator-gcc)
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(libvpx-src
      GIT_REPOSITORY    https://github.com/webmproject/libvpx.git
      GIT_TAG           v1.15.2
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/libvpx
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/libvpx
      CONFIGURE_COMMAND <SOURCE_DIR>/configure --extra-cflags=${LIBVPX_CPPFLAGS}
      --prefix=<INSTALL_DIR> --target=${LIBVPX_TARGET}
      --disable-examples --disable-docs --disable-unit-tests
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libvpx.a
      )
    ExternalProject_Get_Property(libvpx-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  add_library(libvpx STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(libvpx libvpx-src)
  endif()
  target_include_directories (libvpx INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(libvpx PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libvpx.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "armv7l")
    set (LIBVPX_TARGET --enable-pic)
  elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
    set (LIBVPX_TARGET --enable-pic)
  else()
    set (LIBVPX_TARGET --target=x86_64-linux-gcc)
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(libvpx-src
      GIT_REPOSITORY    https://github.com/webmproject/libvpx.git
      GIT_TAG           v1.15.2
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/libvpx
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/libvpx
      CONFIGURE_COMMAND <SOURCE_DIR>/configure
      --prefix=<INSTALL_DIR> ${LIBVPX_TARGET}
      --disable-examples --disable-docs --disable-unit-tests
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libvpx.a
      )
    ExternalProject_Get_Property(libvpx-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  add_library(libvpx STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(libvpx libvpx-src)
  endif()
  target_include_directories (libvpx INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(libvpx PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libvpx.a)
  if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    target_link_options(libvpx INTERFACE "-Wl,-Bsymbolic")
  endif()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  set (LIBVPX_EXPORT export PATH=${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin:$ENV{PATH})

  if ("armeabi-v7a" STREQUAL "${ANDROID_ABI}")
    list (APPEND LIBVPX_EXPORT && export CC=armv7a-linux-androideabi${ANDROID_PLATFORM_LEVEL}-clang)
    list (APPEND LIBVPX_EXPORT && export LD=armv7a-linux-androideabi${ANDROID_PLATFORM_LEVEL}-clang)
    list (APPEND LIBVPX_EXPORT && export CXX=armv7a-linux-androideabi${ANDROID_PLATFORM_LEVEL}-clang++)
    list (APPEND LIBVPX_EXPORT && export AS=armv7a-linux-androideabi${ANDROID_PLATFORM_LEVEL}-clang)
    list (APPEND LIBVPX_EXPORT && export STRIP=llvm-strip)
    set (LIBVPX_CFG --target=armv7-android-gcc)
  elseif ("arm64-v8a" STREQUAL "${ANDROID_ABI}")
    list (APPEND LIBVPX_EXPORT && export CC=aarch64-linux-android${ANDROID_PLATFORM_LEVEL}-clang)
    list (APPEND LIBVPX_EXPORT && export LD=aarch64-linux-android${ANDROID_PLATFORM_LEVEL}-clang)
    list (APPEND LIBVPX_EXPORT && export CXX=aarch64-linux-android${ANDROID_PLATFORM_LEVEL}-clang++)
    list (APPEND LIBVPX_EXPORT && export AS=aarch64-linux-android${ANDROID_PLATFORM_LEVEL}-clang)
    list (APPEND LIBVPX_EXPORT && export STRIP=llvm-strip)
    set (LIBVPX_CFG --target=arm64-android-gcc)
  elseif ("x86" STREQUAL "${ANDROID_ABI}")
    list (APPEND LIBVPX_EXPORT && export CC=i686-linux-android${ANDROID_PLATFORM_LEVEL}-clang)
    list (APPEND LIBVPX_EXPORT && export LD=i686-linux-android${ANDROID_PLATFORM_LEVEL}-clang)
    list (APPEND LIBVPX_EXPORT && export CXX=i686-linux-android${ANDROID_PLATFORM_LEVEL}-clang++)
    list (APPEND LIBVPX_EXPORT && export STRIP=llvm-strip)
    set (LIBVPX_CFG --target=x86-android-gcc --as=yasm)
  elseif ("x86_64" STREQUAL "${ANDROID_ABI}")
    list (APPEND LIBVPX_EXPORT && export CC=x86_64-linux-android${ANDROID_PLATFORM_LEVEL}-clang)
    list (APPEND LIBVPX_EXPORT && export LD=x86_64-linux-android${ANDROID_PLATFORM_LEVEL}-clang)
    list (APPEND LIBVPX_EXPORT && export CXX=x86_64-linux-android${ANDROID_PLATFORM_LEVEL}-clang++)
    list (APPEND LIBVPX_EXPORT && export STRIP=llvm-strip)
    set (LIBVPX_CFG --target=x86_64-android-gcc)
  else()
    message(FATAL_ERROR "Unknown Android architecture")
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(libvpx-src
      GIT_REPOSITORY    https://github.com/webmproject/libvpx.git
      GIT_TAG           v1.15.2
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/libvpx
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/libvpx
      CONFIGURE_COMMAND ${LIBVPX_EXPORT} && <SOURCE_DIR>/configure
      --prefix=<INSTALL_DIR> ${LIBVPX_CFG}
      --disable-examples --disable-docs --disable-unit-tests --enable-pic
      BUILD_COMMAND     ${LIBVPX_EXPORT} && make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   ${LIBVPX_EXPORT} && make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libvpx.a
      )
    ExternalProject_Get_Property(libvpx-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  add_library(libvpx STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(libvpx libvpx-src)
  endif()
  target_include_directories (libvpx INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(libvpx PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libvpx.a)
  if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    target_link_options(libvpx INTERFACE "-Wl,-Bsymbolic")
  endif()

endif()
