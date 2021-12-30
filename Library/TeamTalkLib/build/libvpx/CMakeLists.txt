cmake_minimum_required(VERSION 3.1)

project(LibVPX)

include(ExternalProject)

##################################################
# LibVPX
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_LIBVPX ${TOOLCHAIN_INSTALL_PREFIX}/libvpx)

if (MSVC)

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    find_program(CYGWIN_BASH_EXE_PATH bash.exe PATHS C:/cygwin64/bin)
    if (CYGWIN_BASH_EXE_PATH-NOTFOUND)
      message(WARNING "CYGWIN bash.exe is required to build libvpx")
    else()
      message("Found CYGWIN for libvpx here: ${CYGWIN_BASH_EXE_PATH}")
    endif()

    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      find_program(YASM_EXE_WIN64_PATH yasm.exe PATHS C:/tt5dist/yasm/x64)
      if (YASM_EXE_WIN64_PATH-NOTFOUND)
        message(WARNING "yasm.exe is required to build libvpx. See https://yasm.tortall.net")
      else()
        message("Found YASM for libvpx here: ${YASM_EXE_WIN64_PATH}")
      endif()

      set (CMAKE_MSVCIDE_RUN_PATH C:/tt5dist/yasm/x64)

      ExternalProject_Add(libvpx-src
        GIT_REPOSITORY    https://github.com/bear101/libvpx
        GIT_TAG           v1.9.0
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/libvpx
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     IF EXIST build-x64 rmdir /Q /S build-x64
        COMMAND           mkdir build-x64
        COMMAND           C:\\cygwin64\\bin\\bash.exe --login -c "cd $OLDPWD/build-x64 && ../configure --target=x86_64-win64-vs16 --disable-examples --disable-docs --enable-static-msvcrt --disable-unit-tests && make"
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}
        INSTALL_COMMAND   cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_headers.bat <INSTALL_DIR>
        COMMAND           cd <SOURCE_DIR>/build-x64
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_libs.bat x64 <INSTALL_DIR>
        BUILD_IN_SOURCE   TRUE
        )
    else()
      find_program(YASM_EXE_WIN32_PATH yasm.exe PATHS C:/tt5dist/yasm/win32)
      if (YASM_EXE_WIN32_PATH-NOTFOUND)
        message(WARNING "yasm.exe is required to build libvpx. See https://yasm.tortall.net")
      else()
        message("Found YASM for libvpx here: ${YASM_EXE_WIN32_PATH}")
      endif()

      set (CMAKE_MSVCIDE_RUN_PATH C:/tt5dist/yasm/win32)

      ExternalProject_Add(libvpx-src
        GIT_REPOSITORY    https://github.com/bear101/libvpx
        GIT_TAG           v1.9.0
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/libvpx
        CONFIGURE_COMMAND ""
        BUILD_COMMAND     IF EXIST build-win32 rmdir /Q /S build-win32
        COMMAND           mkdir build-win32
        COMMAND           C:\\cygwin64\\bin\\bash.exe --login -c "cd $OLDPWD/build-win32 && ../configure --target=x86-win32-vs16 --disable-examples --disable-docs --enable-static-msvcrt --disable-unit-tests && make"
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_LIBVPX}
        INSTALL_COMMAND   cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_headers.bat <INSTALL_DIR>
        COMMAND           cd <SOURCE_DIR>/build-win32
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_libs.bat win32 <INSTALL_DIR>
        BUILD_IN_SOURCE   TRUE
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
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/$(PlatformName)/Debug/vpxmtd.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/$(PlatformName)/Release/vpxmt.lib)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(libvpx libvpx-src)
  endif()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    
    ExternalProject_Add(libvpx-arm64-src
      GIT_REPOSITORY    https://github.com/bear101/libvpx
      GIT_TAG           v1.11.0
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
      GIT_REPOSITORY    https://github.com/bear101/libvpx
      GIT_TAG           v1.11.0
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
      GIT_REPOSITORY    https://github.com/bear101/libvpx
      GIT_TAG           v1.9.0
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

  if (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "armv7l")
    set (LIBVPX_TARGET --enable-pic)
  else()
    set (LIBVPX_TARGET --target=x86_64-linux-gcc)
  endif()
  
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(libvpx-src
      GIT_REPOSITORY    https://github.com/bear101/libvpx
      GIT_TAG           v1.9.0
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
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(libvpx PROPERTIES
      INTERFACE_LINK_OPTIONS "-Wl,-Bsymbolic"
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libvpx.a)
  else()
    target_include_directories (libvpx INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(libvpx PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libvpx.a)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      target_link_options(libvpx INTERFACE "-Wl,-Bsymbolic")
    endif()
  endif()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  set (LIBVPX_EXPORT export PATH=${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin:$ENV{PATH})
  
  if ("armeabi-v7a" STREQUAL "${ANDROID_ABI}")
    list (APPEND LIBVPX_EXPORT && export CROSS=arm-linux-androideabi-)
    list (APPEND LIBVPX_EXPORT && export CC=clang\ --target=armv7a-linux-android${ANDROID_PLATFORM_LEVEL})
    list (APPEND LIBVPX_EXPORT && export CXX=clang++\ --target=armv7a-linux-android${ANDROID_PLATFORM_LEVEL})
    set (LIBVPX_CFG --target=armv7-android-gcc)
  elseif ("arm64-v8a" STREQUAL "${ANDROID_ABI}")
    list (APPEND LIBVPX_EXPORT && export CROSS=aarch64-linux-android-)
    list (APPEND LIBVPX_EXPORT && export CC=clang\ --target=aarch64-linux-android${ANDROID_PLATFORM_LEVEL})
    list (APPEND LIBVPX_EXPORT && export CXX=clang++\ --target=aarch64-linux-android${ANDROID_PLATFORM_LEVEL})
    set (LIBVPX_CFG --target=arm64-android-gcc)
  elseif ("x86" STREQUAL "${ANDROID_ABI}")
    list (APPEND LIBVPX_EXPORT && export CROSS=i686-linux-android-)
    list (APPEND LIBVPX_EXPORT && export CC=clang\ --target=i686-linux-android${ANDROID_PLATFORM_LEVEL})
    list (APPEND LIBVPX_EXPORT && export CXX=clang++\ --target=i686-linux-android${ANDROID_PLATFORM_LEVEL})
    set (LIBVPX_CFG --target=x86-android-gcc)
  elseif ("x86_64" STREQUAL "${ANDROID_ABI}")
    list (APPEND LIBVPX_EXPORT && export CROSS=x86_64-linux-android-)
    list (APPEND LIBVPX_EXPORT && export CC=clang\ --target=x86_64-linux-android${ANDROID_PLATFORM_LEVEL})
    list (APPEND LIBVPX_EXPORT && export CXX=clang++\ --target=x86_64-linux-android${ANDROID_PLATFORM_LEVEL})
    set (LIBVPX_CFG --target=x86_64-android-gcc)
  else()
    message(FATAL_ERROR "Unknown Android architecture")
  endif()
  
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(libvpx-src
      GIT_REPOSITORY    https://github.com/bear101/libvpx
      GIT_TAG           v1.9.0
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
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(libvpx PROPERTIES
      INTERFACE_LINK_OPTIONS "-Wl,-Bsymbolic"
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libvpx.a)
  else()
    target_include_directories (libvpx INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(libvpx PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libvpx.a)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      target_link_options(libvpx INTERFACE "-Wl,-Bsymbolic")
    endif()
  endif()
  
endif()
