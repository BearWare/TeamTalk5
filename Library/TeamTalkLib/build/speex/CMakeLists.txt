project(Speex)

include(ExternalProject)

##################################################
# Speex
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_SPEEX ${TOOLCHAIN_INSTALL_PREFIX}/speex)

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

      ExternalProject_Add(speex-src
        GIT_REPOSITORY    https://github.com/xiph/speex.git
        GIT_TAG           Speex-1.2.1
        UPDATE_COMMAND    ""
        PATCH_COMMAND     ${CMAKE_COMMAND} -DPATCH_FILE=${CMAKE_CURRENT_LIST_DIR}/0001-VS2015-proj.patch -DSOURCE_DIR=<SOURCE_DIR> -P ${CMAKE_CURRENT_LIST_DIR}/../apply_patch.cmake
        COMMAND           ${CMAKE_COMMAND} -DPATCH_FILE=${CMAKE_CURRENT_LIST_DIR}/0002-Renamed-solution-file-from-x86-to-Win32.patch -DSOURCE_DIR=<SOURCE_DIR> -P ${CMAKE_CURRENT_LIST_DIR}/../apply_patch.cmake
        COMMAND           ${CMAKE_COMMAND} -DPATCH_FILE=${CMAKE_CURRENT_LIST_DIR}/0003-Upgrade-to-VS2019-and-Windows-10-SDK.patch -DSOURCE_DIR=<SOURCE_DIR> -P ${CMAKE_CURRENT_LIST_DIR}/../apply_patch.cmake
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/speex
        CONFIGURE_COMMAND ""
        BUILD_IN_SOURCE   TRUE
        BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 win32\\VS2015\\libspeex\\libspeex.sln -target:libspeex /property:Configuration=Debug
        COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 win32\\VS2015\\libspeex\\libspeex.sln -target:libspeex /property:Configuration=Release_SSE2
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_SPEEX}
        INSTALL_COMMAND   cd <SOURCE_DIR>/include
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_headers.bat <INSTALL_DIR>
        COMMAND           cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_libs.bat lib <INSTALL_DIR>
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/libspeexd.lib
                          <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/libspeex_sse2.lib
        )
      ExternalProject_Get_Property(speex-src INSTALL_DIR)
    else()

      ExternalProject_Add(speex-src
        GIT_REPOSITORY    https://github.com/xiph/speex.git
        GIT_TAG           Speex-1.2.1
        UPDATE_COMMAND    ""
        PATCH_COMMAND     ${CMAKE_COMMAND} -DPATCH_FILE=${CMAKE_CURRENT_LIST_DIR}/0001-VS2015-proj.patch -DSOURCE_DIR=<SOURCE_DIR> -P ${CMAKE_CURRENT_LIST_DIR}/../apply_patch.cmake
        COMMAND           ${CMAKE_COMMAND} -DPATCH_FILE=${CMAKE_CURRENT_LIST_DIR}/0002-Renamed-solution-file-from-x86-to-Win32.patch -DSOURCE_DIR=<SOURCE_DIR> -P ${CMAKE_CURRENT_LIST_DIR}/../apply_patch.cmake
        COMMAND           ${CMAKE_COMMAND} -DPATCH_FILE=${CMAKE_CURRENT_LIST_DIR}/0003-Upgrade-to-VS2019-and-Windows-10-SDK.patch -DSOURCE_DIR=<SOURCE_DIR> -P ${CMAKE_CURRENT_LIST_DIR}/../apply_patch.cmake
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/speex
        CONFIGURE_COMMAND ""
        BUILD_IN_SOURCE   TRUE
        BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 win32\\VS2015\\libspeex\\libspeex.sln -target:libspeex /property:Configuration=Debug
        COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 win32\\VS2015\\libspeex\\libspeex.sln -target:libspeex /property:Configuration=Release_SSE2
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_SPEEX}
        INSTALL_COMMAND   cd <SOURCE_DIR>/include
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_headers.bat <INSTALL_DIR>
        COMMAND           cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_libs.bat lib <INSTALL_DIR>
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/libspeexd.lib
                          <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/libspeex_sse2.lib
        )
      ExternalProject_Get_Property(speex-src INSTALL_DIR)
    endif()
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_SPEEX})
  endif()

  add_library(speex STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(speex speex-src)
  endif()
  target_include_directories (speex INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(speex PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/${CMAKE_VS_PLATFORM_NAME}/libspeexd.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/${CMAKE_VS_PLATFORM_NAME}/libspeex_sse2.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(speex-src
      GIT_REPOSITORY    https://github.com/xiph/speex.git
      GIT_TAG           Speex-1.2.1
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/speex
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_SPEEX}
      CONFIGURE_COMMAND <SOURCE_DIR>/autogen.sh
      COMMAND           <SOURCE_DIR>/configure CFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}\ -arch\ x86_64\ -arch\ arm64
                        CPPFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}\ -arch\ x86_64\ -arch\ arm64 --prefix=<INSTALL_DIR>
                        --enable-shared=no --enable-static=yes --disable-binaries
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libspeex.a
      )
    ExternalProject_Get_Property(speex-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_SPEEX})
  endif()

  add_library(speex STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(speex speex-src)
  endif()
  target_include_directories (speex INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(speex PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libspeex.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  if ("armv7" STREQUAL "${CMAKE_OSX_ARCHITECTURES}" OR "arm64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (SPEEX_GEN_FLAGS --host=arm-apple-darwin)
    set (SPEEX_CFG_FLAGS --host=arm-apple-darwin --enable-shared=no --enable-static=yes --disable-binaries --enable-fixed-point)

    set (SPEEX_CFLAGS " -arch ${CMAKE_OSX_ARCHITECTURES} -isysroot ${CMAKE_OSX_SYSROOT} -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -fembed-bitcode")
  elseif ("i386" STREQUAL "${CMAKE_OSX_ARCHITECTURES}" OR "x86_64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (SPEEX_GEN_FLAGS --host=${CMAKE_OSX_ARCHITECTURES})
    set (SPEEX_CFG_FLAGS --host=${CMAKE_OSX_ARCHITECTURES} --enable-shared=no --enable-static=yes --disable-binaries)
    set (SPEEX_CFLAGS " -arch ${CMAKE_OSX_ARCHITECTURES} -isysroot ${CMAKE_OSX_SYSROOT} -mios-simulator-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -fembed-bitcode")
  else()
    message(FATAL_ERROR "What iOS architecture is this?")
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(speex-src
      GIT_REPOSITORY    https://github.com/xiph/speex.git
      GIT_TAG           Speex-1.2.1
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/speex
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_SPEEX}
      CONFIGURE_COMMAND export CFLAGS=${SPEEX_CFLAGS} CPPFLAGS=${SPEEX_CFLAGS} LDFLAGS=${SPEEX_CFLAGS} && <SOURCE_DIR>/autogen.sh ${SPEEX_GEN_FLAGS}
      COMMAND           export CFLAGS=${SPEEX_CFLAGS} CPPFLAGS=${SPEEX_CFLAGS} LDFLAGS=${SPEEX_CFLAGS} && <SOURCE_DIR>/configure ${SPEEX_CFG_FLAGS} --prefix=<INSTALL_DIR>
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libspeex.a
      )
    ExternalProject_Get_Property(speex-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_SPEEX})
  endif()

  add_library(speex STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(speex speex-src)
  endif()
  target_include_directories (speex INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(speex PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libspeex.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(speex-src
      GIT_REPOSITORY    https://github.com/xiph/speex.git
      GIT_TAG           Speex-1.2.1
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/speex
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_SPEEX}
      CONFIGURE_COMMAND <SOURCE_DIR>/autogen.sh
      COMMAND           <SOURCE_DIR>/configure --with-pic --prefix=<INSTALL_DIR>
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libspeex.a
      )
    ExternalProject_Get_Property(speex-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_SPEEX})
  endif()

  add_library(speex STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(speex speex-src)
  endif()
  target_include_directories (speex INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(speex PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libspeex.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  set (SPEEX_EXPORT export PATH=${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin:$ENV{PATH})

  if ("armeabi-v7a" STREQUAL "${ANDROID_ABI}")
    set (SPEEX_GEN_FLAGS --host=arm-linux-androideabi)
    set (SPEEX_CFG_FLAGS --host=arm-linux-androideabi --enable-fixed-point CC=clang\ --target=armv7a-linux-android${ANDROID_PLATFORM_LEVEL})
  elseif ("arm64-v8a" STREQUAL "${ANDROID_ABI}")
    set (SPEEX_GEN_FLAGS --host=aarch64-linux-android)
    set (SPEEX_CFG_FLAGS --host=aarch64-linux-android --enable-fixed-point CC=clang\ --target=aarch64-linux-android${ANDROID_PLATFORM_LEVEL})
  elseif ("x86" STREQUAL "${ANDROID_ABI}")
    set (SPEEX_GEN_FLAGS --host=i686-linux-android)
    set (SPEEX_CFG_FLAGS --host=i686-linux-android CC=clang\ --target=i686-linux-android${ANDROID_PLATFORM_LEVEL})
  elseif ("x86_64" STREQUAL "${ANDROID_ABI}")
    set (SPEEX_GEN_FLAGS --host=x86_64-linux-android)
    set (SPEEX_CFG_FLAGS --host=x86_64-linux-android CC=clang\ --target=x86_64-linux-android${ANDROID_PLATFORM_LEVEL})
  else()
    message(FATAL_ERROR "What iOS architecture is this?")
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(speex-src
      GIT_REPOSITORY    https://github.com/xiph/speex.git
      GIT_TAG           Speex-1.2.1
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/speex
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_SPEEX}
      CONFIGURE_COMMAND ${SPEEX_EXPORT} && <SOURCE_DIR>/autogen.sh ${SPEEX_GEN_FLAGS}
      COMMAND           ${SPEEX_EXPORT} && <SOURCE_DIR>/configure ${SPEEX_CFG_FLAGS}
      --prefix=<INSTALL_DIR> --disable-binaries --with-pic
      --enable-shared=no --enable-static=yes
      BUILD_COMMAND     ${SPEEX_EXPORT} && make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   ${SPEEX_EXPORT} && make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libspeex.a
      )
    ExternalProject_Get_Property(speex-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_SPEEX})
  endif()

  add_library(speex STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(speex speex-src)
  endif()
  target_include_directories (speex INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(speex PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libspeex.a)

endif()
