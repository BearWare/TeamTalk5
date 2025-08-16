project(SpeexDSP)

include(ExternalProject)

##################################################
# SpeexDSP
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_SPEEXDSP ${TOOLCHAIN_INSTALL_PREFIX}/speexdsp)

if (MSVC)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)

      if (NOT CMAKE_VS_PLATFORM_NAME)
        set (CMAKE_VS_PLATFORM_NAME x64)
      endif()

      ExternalProject_Add(speexdsp-src
        GIT_REPOSITORY    https://github.com/bear101/speexdsp.git
        GIT_TAG           2907b00
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/speexdsp
        CONFIGURE_COMMAND ""
        BUILD_IN_SOURCE   TRUE
        BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 win32\\VS2015\\libspeexdsp.sln -target:libspeexdsp /property:Configuration=Debug
        COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 win32\\VS2015\\libspeexdsp.sln -target:libspeexdsp /property:Configuration=Release_SSE
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_SPEEXDSP}
        INSTALL_COMMAND   cd <SOURCE_DIR>/include
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_headers.bat <INSTALL_DIR>
        COMMAND           cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_libs.bat lib <INSTALL_DIR>
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/libspeexdspd.lib
                          <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/libspeexdsp_sse.lib
        )
      ExternalProject_Get_Property(speexdsp-src INSTALL_DIR)
    else()

      if (NOT CMAKE_VS_PLATFORM_NAME)
        set (CMAKE_VS_PLATFORM_NAME win32)
      endif()

      ExternalProject_Add(speexdsp-src
        GIT_REPOSITORY    https://github.com/bear101/speexdsp.git
        GIT_TAG           2907b00
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/speexdsp
        CONFIGURE_COMMAND ""
        BUILD_IN_SOURCE   TRUE
        BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 win32\\VS2015\\libspeexdsp.sln -target:libspeexdsp /property:Configuration=Debug
        COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 win32\\VS2015\\libspeexdsp.sln -target:libspeexdsp /property:Configuration=Release_SSE
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_SPEEXDSP}
        INSTALL_COMMAND   cd <SOURCE_DIR>/include
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_headers.bat <INSTALL_DIR>
        COMMAND           cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_libs.bat lib <INSTALL_DIR>
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/libspeexdspd.lib
                          <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/libspeexdsp_sse.lib
        )
      ExternalProject_Get_Property(speexdsp-src INSTALL_DIR)
    endif()
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_SPEEXDSP})
  endif()

  add_library(speexdsp STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(speexdsp speexdsp-src)
  endif()
  target_include_directories (speexdsp INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(speexdsp PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/${CMAKE_VS_PLATFORM_NAME}/libspeexdspd.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/${CMAKE_VS_PLATFORM_NAME}/libspeexdsp_sse.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(speexdsp-src
      GIT_REPOSITORY    https://github.com/xiph/speexdsp.git
      GIT_TAG           SpeexDSP-1.2.1
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/speexdsp
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_SPEEXDSP}
      CONFIGURE_COMMAND <SOURCE_DIR>/autogen.sh
      COMMAND           <SOURCE_DIR>/configure CFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}\ -arch\ x86_64\ -arch\ arm64
                        CPPFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}\ -arch\ x86_64\ -arch\ arm64 --prefix=<INSTALL_DIR>
                        --enable-shared=no --enable-static=yes --disable-examples
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libspeexdsp.a
      )
    ExternalProject_Get_Property(speexdsp-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_SPEEXDSP})
  endif()

  add_library(speexdsp STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(speexdsp speexdsp-src)
  endif()
  target_include_directories (speexdsp INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(speexdsp PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libspeexdsp.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  if ("armv7" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (SPEEXDSP_GEN_FLAGS --host=arm-apple-darwin)
    set (SPEEXDSP_CFG_FLAGS --host=arm-apple-darwin --enable-shared=no --enable-static=yes --enable-fixed-point --enable-neon --disable-examples)
    set (SPEEXDSP_CFLAGS "-arch ${CMAKE_OSX_ARCHITECTURES} -isysroot ${CMAKE_OSX_SYSROOT} -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -fembed-bitcode")
  elseif ("arm64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (SPEEXDSP_GEN_FLAGS --host=arm-apple-darwin)
    set (SPEEXDSP_CFG_FLAGS --host=arm-apple-darwin --enable-shared=no --enable-static=yes --disable-neon --disable-examples)
    set (SPEEXDSP_CFLAGS "-arch ${CMAKE_OSX_ARCHITECTURES} -isysroot ${CMAKE_OSX_SYSROOT} -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -fembed-bitcode")
  elseif ("i386" STREQUAL "${CMAKE_OSX_ARCHITECTURES}" OR "x86_64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (SPEEXDSP_GEN_FLAGS --host=${CMAKE_OSX_ARCHITECTURES})
    set (SPEEXDSP_CFG_FLAGS --host=${CMAKE_OSX_ARCHITECTURES} --enable-shared=no --enable-static=yes)
    set (SPEEXDSP_CFLAGS "-arch ${CMAKE_OSX_ARCHITECTURES} -isysroot ${CMAKE_OSX_SYSROOT} -mios-simulator-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(speexdsp-src
      GIT_REPOSITORY    https://github.com/xiph/speexdsp.git
      GIT_TAG           SpeexDSP-1.2.1
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/speexdsp
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_SPEEXDSP}
      CONFIGURE_COMMAND export CFLAGS=${SPEEXDSP_CFLAGS} LDFLAGS=${SPEEXDSP_CFLAGS} && <SOURCE_DIR>/autogen.sh ${SPEEXDSP_GEN_FLAGS}
      COMMAND           export CFLAGS=${SPEEXDSP_CFLAGS} LDFLAGS=${SPEEXDSP_CFLAGS} && <SOURCE_DIR>/configure ${SPEEXDSP_CFG_FLAGS} --prefix=<INSTALL_DIR>
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libspeexdsp.a
      )
    ExternalProject_Get_Property(speexdsp-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_SPEEXDSP})
  endif()

  add_library(speexdsp STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(speexdsp speexdsp-src)
  endif()
  target_include_directories (speexdsp INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(speexdsp PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libspeexdsp.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(speexdsp-src
      GIT_REPOSITORY    https://github.com/xiph/speexdsp.git
      GIT_TAG           SpeexDSP-1.2.1
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/speexdsp
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_SPEEXDSP}
      CONFIGURE_COMMAND <SOURCE_DIR>/autogen.sh
      COMMAND           <SOURCE_DIR>/configure --with-pic --prefix=<INSTALL_DIR>
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libspeexdsp.a
      )
    ExternalProject_Get_Property(speexdsp-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_SPEEXDSP})
  endif()

  add_library(speexdsp STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(speexdsp speexdsp-src)
  endif()
  target_include_directories (speexdsp INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(speexdsp PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libspeexdsp.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  set (SPEEXDSP_EXPORT export PATH=${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin:$ENV{PATH})

  if ("armeabi-v7a" STREQUAL "${ANDROID_ABI}")
    set (SPEEXDSP_GEN_FLAGS --host=arm-linux-androideabi)
    set (SPEEXDSP_CFG_FLAGS --host=arm-linux-androideabi --enable-fixed-point
      CC=clang\ --target=armv7a-linux-android${ANDROID_PLATFORM_LEVEL}\ -mfloat-abi=softfp\ -mfpu=neon
      --enable-neon)
  elseif ("arm64-v8a" STREQUAL "${ANDROID_ABI}")
    set (SPEEXDSP_GEN_FLAGS --host=aarch64-linux-android)
    set (SPEEXDSP_CFG_FLAGS --host=aarch64-linux-android --enable-fixed-point
      CC=clang\ --target=aarch64-linux-android${ANDROID_PLATFORM_LEVEL}
      --disable-neon)
  elseif ("x86" STREQUAL "${ANDROID_ABI}")
    set (SPEEXDSP_GEN_FLAGS --host=i686-linux-android)
    set (SPEEXDSP_CFG_FLAGS --host=i686-linux-android
      CC=clang\ --target=i686-linux-android${ANDROID_PLATFORM_LEVEL}
      --disable-neon)
  elseif ("x86_64" STREQUAL "${ANDROID_ABI}")
    set (SPEEXDSP_GEN_FLAGS --host=x86_64-linux-android)
    set (SPEEXDSP_CFG_FLAGS --host=x86_64-linux-android
      CC=clang\ --target=x86_64-linux-android${ANDROID_PLATFORM_LEVEL}
      --disable-neon)
  else()
    message(FATAL_ERROR "What Android architecture is this?")
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(speexdsp-src
      GIT_REPOSITORY    https://github.com/xiph/speexdsp.git
      GIT_TAG           SpeexDSP-1.2.1
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/speexdsp
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_SPEEXDSP}
      CONFIGURE_COMMAND ${SPEEXDSP_EXPORT} && <SOURCE_DIR>/autogen.sh ${SPEEXDSP_GEN_FLAGS}
      COMMAND           ${SPEEXDSP_EXPORT} && <SOURCE_DIR>/configure ${SPEEXDSP_CFG_FLAGS}
      --with-pic --prefix=<INSTALL_DIR> --disable-examples
      --enable-shared=no --enable-static=yes
      BUILD_COMMAND     ${SPEEXDSP_EXPORT} && make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   ${SPEEXDSP_EXPORT} && make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libspeexdsp.a
      )
    ExternalProject_Get_Property(speexdsp-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_SPEEXDSP})
  endif()

  add_library(speexdsp STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(speexdsp speexdsp-src)
  endif()
  target_include_directories (speexdsp INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(speexdsp PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libspeexdsp.a)

endif()
