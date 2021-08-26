cmake_minimum_required(VERSION 3.1)

project(OpenSSL)

include(ExternalProject)

##################################################
# FFmpeg
##################################################
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  ExternalProject_Add(ffmpeg-src
    GIT_REPOSITORY    https://github.com/bear101/FFmpeg
    GIT_TAG           n3.4.7
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CONFIGURE_COMMAND export PKG_CONFIG_PATH=${OPENSSL_DIR}/lib/pkgconfig &&
        <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --disable-iconv
	--disable-libxcb --disable-libxcb-shm --disable-libxcb-xfixes
	--disable-libxcb-shape --disable-securetransport
	--disable-schannel --disable-xlib --disable-jack
	--disable-lzma
	--extra-cflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
	--extra-cxxflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
	--extra-ldexeflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
	--disable-videotoolbox --disable-sdl2 --pkg-config-flags=--static
	--enable-debug=3 --disable-optimizations --disable-stripping --enable-openssl
    BUILD_COMMAND     make -j4
    INSTALL_COMMAND   make install
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libavdevice.a
        <INSTALL_DIR>/lib/libavfilter.a
        <INSTALL_DIR>/lib/libavformat.a
        <INSTALL_DIR>/lib/libavcodec.a
        <INSTALL_DIR>/lib/libswresample.a
        <INSTALL_DIR>/lib/libswscale.a
        <INSTALL_DIR>/lib/libavutil.a
    DEPENDS openssl-src
    )
  ExternalProject_Get_Property(ffmpeg-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(ffmpeg-avdevice STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avdevice ffmpeg-src)
  target_include_directories (ffmpeg-avdevice INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avdevice PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavdevice.a)

  add_library(ffmpeg-avfilter STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avfilter ffmpeg-src)
  target_include_directories (ffmpeg-avfilter INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avfilter PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavfilter.a)

  add_library(ffmpeg-avformat STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avformat ffmpeg-src)
  target_include_directories (ffmpeg-avformat INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avformat PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavformat.a)
  target_link_libraries(ffmpeg-avformat INTERFACE openssl-crypto openssl-ssl)

  add_library(ffmpeg-avcodec STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avcodec ffmpeg-src)
  target_include_directories (ffmpeg-avcodec INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avcodec PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavcodec.a)

  add_library(ffmpeg-swresample STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-swresample ffmpeg-src)
  target_include_directories (ffmpeg-swresample INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-swresample PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libswresample.a)

  add_library(ffmpeg-swscale STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-swscale ffmpeg-src)
  target_include_directories (ffmpeg-swscale INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-swscale PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libswscale.a)

  add_library(ffmpeg-avutil STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avutil ffmpeg-src)
  target_include_directories (ffmpeg-avutil INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avutil PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavutil.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  ExternalProject_Add(gas-preprocessor
    GIT_REPOSITORY    https://github.com/FFmpeg/gas-preprocessor
    GIT_TAG           cbe8847
    UPDATE_COMMAND    ""
    INSTALL_DIR       ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(gas-preprocessor SOURCE_DIR)
  set (GAS_DIR ${SOURCE_DIR})

  set (FFMPEG_CFG_FLAGS --target-os=darwin --sysroot=${CMAKE_OSX_SYSROOT} --enable-pic --disable-doc --disable-programs --enable-cross-compile)

  if ("armv7" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    list (APPEND FFMPEG_CFG_FLAGS --arch=arm --cpu=cortex-a8)

    set (CCARGS "clang -arch armv7 -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    set (FFMPEG_CFG_CCFLAGS --cc=${CCARGS})
    set (ASARGS "${GAS_DIR}/gas-preprocessor.pl -arch arm -as-type apple-clang -- clang -arch armv7 -v")
    set (FFMPEG_CFG_ASFLAGS --as=${ASARGS})

    list (APPEND FFMPEG_CFG_FLAGS ${FFMPEG_CFG_CCFLAGS} ${FFMPEG_CFG_ASFLAGS})

  elseif ("arm64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    list (APPEND FFMPEG_CFG_FLAGS --arch=${CMAKE_OSX_ARCHITECTURES})

    set (CCARGS "clang -arch ${CMAKE_OSX_ARCHITECTURES} -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    set (FFMPEG_CFG_CCFLAGS --cc=${CCARGS})
    set (ASARGS "${GAS_DIR}/gas-preprocessor.pl -arch ${CMAKE_OSX_ARCHITECTURES} -as-type apple-clang -- clang -arch ${CMAKE_OSX_ARCHITECTURES} -v")
    set (FFMPEG_CFG_ASFLAGS --as=${ASARGS})

    list (APPEND FFMPEG_CFG_FLAGS ${FFMPEG_CFG_CCFLAGS} ${FFMPEG_CFG_ASFLAGS})

  elseif ("i386" STREQUAL "${CMAKE_OSX_ARCHITECTURES}" OR "x86_64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    list (APPEND FFMPEG_CFG_FLAGS --arch=${CMAKE_OSX_ARCHITECTURES})

    set (CCARGS "clang -arch ${CMAKE_OSX_ARCHITECTURES} -mios-simulator-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    set (FFMPEG_CFG_CCFLAGS --cc=${CCARGS})

    list (APPEND FFMPEG_CFG_FLAGS ${FFMPEG_CFG_CCFLAGS} ${FFMPEG_CFG_ASFLAGS})
  endif()

  ExternalProject_Add(ffmpeg-src
    GIT_REPOSITORY    https://github.com/bear101/FFmpeg
    GIT_TAG           n3.4.7
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CONFIGURE_COMMAND  <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> ${FFMPEG_CFG_FLAGS}
    BUILD_COMMAND     make -j4
    INSTALL_COMMAND   make install
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libavdevice.a
        <INSTALL_DIR>/lib/libavfilter.a
        <INSTALL_DIR>/lib/libavformat.a
        <INSTALL_DIR>/lib/libavcodec.a
        <INSTALL_DIR>/lib/libswresample.a
        <INSTALL_DIR>/lib/libswscale.a
        <INSTALL_DIR>/lib/libavutil.a
    DEPENDS openssl-src gas-preprocessor
    )
  ExternalProject_Get_Property(ffmpeg-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(ffmpeg-avdevice STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avdevice ffmpeg-src)
  target_include_directories (ffmpeg-avdevice INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avdevice PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavdevice.a)

  add_library(ffmpeg-avfilter STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avfilter ffmpeg-src)
  target_include_directories (ffmpeg-avfilter INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avfilter PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavfilter.a)

  add_library(ffmpeg-avformat STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avformat ffmpeg-src)
  target_include_directories (ffmpeg-avformat INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avformat PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavformat.a)
  target_link_libraries(ffmpeg-avformat INTERFACE openssl-crypto openssl-ssl)

  add_library(ffmpeg-avcodec STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avcodec ffmpeg-src)
  target_include_directories (ffmpeg-avcodec INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avcodec PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavcodec.a)

  add_library(ffmpeg-swresample STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-swresample ffmpeg-src)
  target_include_directories (ffmpeg-swresample INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-swresample PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libswresample.a)

  add_library(ffmpeg-swscale STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-swscale ffmpeg-src)
  target_include_directories (ffmpeg-swscale INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-swscale PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libswscale.a)

  add_library(ffmpeg-avutil STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avutil ffmpeg-src)
  target_include_directories (ffmpeg-avutil INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avutil PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavutil.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  ExternalProject_Add(ffmpeg-src
    GIT_REPOSITORY    https://github.com/bear101/FFmpeg
    GIT_TAG           n3.4.7
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
                      --enable-pic --enable-openssl --disable-jack
                      --disable-libxcb
    BUILD_COMMAND     make -j4
    INSTALL_COMMAND   make install
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libavdevice.a
        <INSTALL_DIR>/lib/libavfilter.a
        <INSTALL_DIR>/lib/libavformat.a
        <INSTALL_DIR>/lib/libavcodec.a
        <INSTALL_DIR>/lib/libswresample.a
        <INSTALL_DIR>/lib/libswscale.a
        <INSTALL_DIR>/lib/libavutil.a
    )
  ExternalProject_Get_Property(ffmpeg-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(ffmpeg-avdevice STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avdevice ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-avdevice PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      INTERFACE_LINK_OPTIONS "-Wl,-Bsymbolic"
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavdevice.a)
  else()
    target_include_directories (ffmpeg-avdevice INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-avdevice PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavdevice.a)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      target_link_options(ffmpeg-avdevice INTERFACE "-Wl,-Bsymbolic")
    endif()
  endif()

  add_library(ffmpeg-avfilter STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avfilter ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-avfilter PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavfilter.a)
  else()
    target_include_directories (ffmpeg-avfilter INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-avfilter PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavfilter.a)
  endif()

  add_library(ffmpeg-avformat STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avformat ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-avformat PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavformat.a)
  else()
    target_include_directories (ffmpeg-avformat INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-avformat PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavformat.a)
  endif()
  target_link_libraries(ffmpeg-avformat INTERFACE ssl crypto)

  add_library(ffmpeg-avcodec STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avcodec ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-avcodec PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavcodec.a)
  else()
    target_include_directories (ffmpeg-avcodec INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-avcodec PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavcodec.a)
  endif()

  add_library(ffmpeg-swresample STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-swresample ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-swresample PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libswresample.a)
  else()
    target_include_directories (ffmpeg-swresample INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-swresample PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libswresample.a)
  endif()

  add_library(ffmpeg-swscale STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-swscale ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-swscale PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libswscale.a)
  else()
    target_include_directories (ffmpeg-swscale INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-swscale PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libswscale.a)
  endif()

  add_library(ffmpeg-avutil STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avutil ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-avutil PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavutil.a)
  else()
    target_include_directories (ffmpeg-avutil INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-avutil PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavutil.a)
  endif()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  set (FFMPEG_EXPORT export PATH=${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin:$ENV{PATH} && export PKG_CONFIG_PATH=${OPENSSL_DIR}/lib/pkgconfig)
  
  if ("armeabi-v7a" STREQUAL "${ANDROID_ABI}")
    set (FFMPEG_CFG --cross-prefix=arm-linux-androideabi- --cc=clang\ --target=armv7a-linux-android${ANDROID_PLATFORM_LEVEL} --arch=arm --disable-neon)
  elseif ("arm64-v8a" STREQUAL "${ANDROID_ABI}")
    set (FFMPEG_CFG --cross-prefix=aarch64-linux-android- --cc=clang\ --target=aarch64-linux-android${ANDROID_PLATFORM_LEVEL} --arch=aarch64)
  elseif ("x86" STREQUAL "${ANDROID_ABI}")
    set (FFMPEG_CFG --cross-prefix=i686-linux-android- --cc=clang\ --target=i686-linux-android${ANDROID_PLATFORM_LEVEL} --arch=x86 --disable-asm)
  elseif ("x86_64" STREQUAL "${ANDROID_ABI}")
    set (FFMPEG_CFG --cross-prefix=x86_64-linux-android- --cc=clang\ --target=x86_64-linux-android${ANDROID_PLATFORM_LEVEL} --arch=x86_64 --disable-asm)
  else()
    message(WARNING "Unknown Android architecture")
  endif()
  
  ExternalProject_Add(ffmpeg-src
    GIT_REPOSITORY    https://github.com/bear101/FFmpeg
    GIT_TAG           n3.4.7
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CONFIGURE_COMMAND ${FFMPEG_EXPORT} && <SOURCE_DIR>/configure ${FFMPEG_CFG} --prefix=<INSTALL_DIR>
                      --enable-cross-compile --sysroot=${CMAKE_SYSROOT} --target-os=android
                      --enable-pic --disable-doc --disable-programs --disable-indev=v4l2
    BUILD_COMMAND     ${FFMPEG_EXPORT} && make -j4
    INSTALL_COMMAND   ${FFMPEG_EXPORT} && make install
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libavdevice.a
        <INSTALL_DIR>/lib/libavfilter.a
        <INSTALL_DIR>/lib/libavformat.a
        <INSTALL_DIR>/lib/libavcodec.a
        <INSTALL_DIR>/lib/libswresample.a
        <INSTALL_DIR>/lib/libswscale.a
        <INSTALL_DIR>/lib/libavutil.a
    DEPENDS openssl-src
    )
  ExternalProject_Get_Property(ffmpeg-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(ffmpeg-avdevice STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avdevice ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-avdevice PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavdevice.a)
  else()
    target_include_directories (ffmpeg-avdevice INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-avdevice PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavdevice.a)
  endif()

  add_library(ffmpeg-avfilter STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avfilter ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-avfilter PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavfilter.a)
  else()
    target_include_directories (ffmpeg-avfilter INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-avfilter PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavfilter.a)
  endif()

  add_library(ffmpeg-avformat STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avformat ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-avformat PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavformat.a)
  else()
    target_include_directories (ffmpeg-avformat INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-avformat PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavformat.a)
  endif()
  target_link_libraries(ffmpeg-avformat INTERFACE openssl-crypto openssl-ssl)

  add_library(ffmpeg-avcodec STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avcodec ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-avcodec PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavcodec.a)
  else()
    target_include_directories (ffmpeg-avcodec INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-avcodec PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavcodec.a)
  endif()

  add_library(ffmpeg-swresample STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-swresample ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-swresample PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libswresample.a)
  else()
    target_include_directories (ffmpeg-swresample INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-swresample PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libswresample.a)
  endif()

  add_library(ffmpeg-swscale STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-swscale ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-swscale PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libswscale.a)
  else()
    target_include_directories (ffmpeg-swscale INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-swscale PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libswscale.a)
  endif()

  add_library(ffmpeg-avutil STATIC IMPORTED GLOBAL)
  add_dependencies(ffmpeg-avutil ffmpeg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ffmpeg-avutil PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavutil.a)
  else()
    target_include_directories (ffmpeg-avutil INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ffmpeg-avutil PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavutil.a)
  endif()
  
endif()
