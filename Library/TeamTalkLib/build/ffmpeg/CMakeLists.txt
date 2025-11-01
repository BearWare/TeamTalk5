project(FFmpeg)

include(ExternalProject)

##################################################
# FFmpeg
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_FFMPEG ${TOOLCHAIN_INSTALL_PREFIX}/ffmpeg)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)

    ExternalProject_Add(ffmpeg-arm64-src
      GIT_REPOSITORY    https://github.com/FFmpeg/FFmpeg.git
      GIT_TAG           n6.1.3
      GIT_SHALLOW       TRUE
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/ffmpeg-arm64
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-arm64
      CONFIGURE_COMMAND export PKG_CONFIG_PATH=${OPENSSL_DIR}/lib/pkgconfig && <SOURCE_DIR>/configure
      --prefix=<INSTALL_DIR>
      # External libraries:
      --disable-iconv
      --disable-libxcb
      --disable-libxcb-shape
      --disable-libxcb-shm
      --disable-libxcb-xfixes
      --disable-lzma
      --disable-schannel
      --disable-sdl2
      --disable-securetransport
      --disable-xlib
      --enable-openssl
      # Programs/docs
      --disable-doc
      --disable-programs
      # Compile options (yasm does not work for arm64)
      --disable-asm
      --disable-optimizations
      --disable-stripping
      --enable-debug=3
      --extra-cflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}\ -arch\ arm64
      --extra-cxxflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}\ -arch\ arm64
      --extra-ldexeflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}\ -arch\ arm64
      --pkg-config-flags=--static
      --enable-cross-compile --target-os=darwin --arch=arm64
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libavdevice.a
      <INSTALL_DIR>/lib/libavfilter.a
      <INSTALL_DIR>/lib/libavformat.a
      <INSTALL_DIR>/lib/libavcodec.a
      <INSTALL_DIR>/lib/libswresample.a
      <INSTALL_DIR>/lib/libswscale.a
      <INSTALL_DIR>/lib/libavutil.a
      DEPENDS openssl-src
      )
    ExternalProject_Get_Property(ffmpeg-arm64-src INSTALL_DIR)

    ExternalProject_Add(ffmpeg-intel-src
      GIT_REPOSITORY    https://github.com/FFmpeg/FFmpeg.git
      GIT_TAG           n6.1.3
      GIT_SHALLOW       TRUE
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/ffmpeg-intel
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-intel
      CONFIGURE_COMMAND export PKG_CONFIG_PATH=${OPENSSL_DIR}/lib/pkgconfig && <SOURCE_DIR>/configure
      --prefix=<INSTALL_DIR>
      # External libraries:
      --disable-iconv
      --disable-libxcb
      --disable-libxcb-shape
      --disable-libxcb-shm
      --disable-libxcb-xfixes
      --disable-lzma
      --disable-schannel
      --disable-sdl2
      --disable-securetransport
      --disable-xlib
      --enable-openssl
      # Programs/docs
      --disable-doc
      --disable-programs
      # Compile options
      --disable-optimizations
      --disable-stripping
      --enable-debug=3
      --extra-cflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}\ -arch\ x86_64
      --extra-cxxflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}\ -arch\ x86_64
      --extra-ldexeflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}\ -arch\ x86_64
      --pkg-config-flags=--static
      --enable-cross-compile --target-os=darwin --arch=x86_64
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libavdevice.a
      <INSTALL_DIR>/lib/libavfilter.a
      <INSTALL_DIR>/lib/libavformat.a
      <INSTALL_DIR>/lib/libavcodec.a
      <INSTALL_DIR>/lib/libswresample.a
      <INSTALL_DIR>/lib/libswscale.a
      <INSTALL_DIR>/lib/libavutil.a
      DEPENDS openssl-src
      )
    ExternalProject_Get_Property(ffmpeg-intel-src INSTALL_DIR)

    # Use 'lipo' to create universal binary
    file(MAKE_DIRECTORY ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib)
    file(MAKE_DIRECTORY ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/include)
    add_custom_command (OUTPUT ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavdevice.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavfilter.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavformat.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavcodec.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libswresample.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libswscale.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavutil.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/include/ffmpeg/libavformat/version.h
      COMMAND lipo -create ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-arm64/lib/libavdevice.a ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-intel/lib/libavdevice.a -output ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavdevice.a
      COMMAND lipo -create ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-arm64/lib/libavfilter.a ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-intel/lib/libavfilter.a -output ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavfilter.a
      COMMAND lipo -create ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-arm64/lib/libavformat.a ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-intel/lib/libavformat.a -output ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavformat.a
      COMMAND lipo -create ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-arm64/lib/libavcodec.a ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-intel/lib/libavcodec.a -output ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavcodec.a
      COMMAND lipo -create ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-arm64/lib/libswresample.a ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-intel/lib/libswresample.a -output ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libswresample.a
      COMMAND lipo -create ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-arm64/lib/libswscale.a ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-intel/lib/libswscale.a -output ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libswscale.a
      COMMAND lipo -create ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-arm64/lib/libavutil.a ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-intel/lib/libavutil.a -output ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavutil.a
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}-intel/include ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/include
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating universal FFmpeg binaries"
      DEPENDS ffmpeg-intel-src ffmpeg-arm64-src)

    add_custom_target (ffmpeg-src DEPENDS ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavdevice.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavfilter.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavformat.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavcodec.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libswresample.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libswscale.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/lib/libavutil.a
      ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}/include/ffmpeg/libavformat/version.h)
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG})
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG})
  endif()

  find_library (AUDIOTOOLBOX_LIBRARY AudioToolBox)
  find_library (AVFOUNDATION_LIBRARY AVFoundation)
  find_library (BZ2_LIBRARY bz2)
  find_library (COCOA_LIBRARY Cocoa)
  find_library (COREAUDIO_LIBRARY CoreAudio)
  find_library (COREFOUNDATION_LIBRARY CoreFoundation)
  find_library (COREMEDIA_LIBRARY CoreMedia)
  find_library (METAL_LIBRARY Metal)
  find_library (OPENGL_LIBRARY OpenGL)
  find_library (QUARTZCORE_LIBRARY QuartzCore)
  find_library (VIDEODECODEACCELERATION_LIBRARY VideoDecodeAcceleration)
  find_library (VIDEOTOOLBOX_LIBRARY VideoToolBox)

  set (FFMPEG_LINK ${AUDIOTOOLBOX_LIBRARY}
    ${AVFOUNDATION_LIBRARY}
    ${BZ2_LIBRARY}
    ${COCOA_LIBRARY}
    ${COREAUDIO_LIBRARY}
    ${COREFOUNDATION_LIBRARY}
    ${COREMEDIA_LIBRARY}
    ${METAL_LIBRARY}
    ${OPENGL_LIBRARY}
    ${QUARTZCORE_LIBRARY}
    ${VIDEODECODEACCELERATION_LIBRARY}
    ${VIDEOTOOLBOX_LIBRARY})

  add_library(ffmpeg-avdevice STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avdevice ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avdevice INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-avdevice PROPERTIES
    INTERFACE_COMPILE_OPTIONS -D__STDC_CONSTANT_MACROS
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavdevice.a)
  target_link_libraries(ffmpeg-avdevice INTERFACE ${FFMPEG_LINK})

  add_library(ffmpeg-avfilter STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avfilter ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avfilter INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avfilter PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavfilter.a)

  add_library(ffmpeg-avformat STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avformat ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avformat INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avformat PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavformat.a)
  target_link_libraries(ffmpeg-avformat INTERFACE openssl-crypto openssl-ssl)

  add_library(ffmpeg-avcodec STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avcodec ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avcodec INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avcodec PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavcodec.a)

  add_library(ffmpeg-swresample STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-swresample ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-swresample INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-swresample PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libswresample.a)

  add_library(ffmpeg-swscale STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-swscale ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-swscale INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-swscale PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libswscale.a)

  add_library(ffmpeg-avutil STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avutil ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avutil INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avutil PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavutil.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(gas-preprocessor
      GIT_REPOSITORY    https://github.com/FFmpeg/gas-preprocessor.git
      GIT_TAG           a120373
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/gas-preprocessor
      )
    ExternalProject_Get_Property(gas-preprocessor SOURCE_DIR)
    set (GAS_DIR ${SOURCE_DIR})

    set (FFMPEG_CFG_FLAGS --target-os=darwin --sysroot=${CMAKE_OSX_SYSROOT}
      # External libraries:
      --disable-videotoolbox
      --disable-audiotoolbox
      --disable-securetransport
      # Programs/docs
      --disable-doc
      --disable-programs
      # Compile options
      --enable-pic
      --enable-cross-compile)

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
      GIT_REPOSITORY    https://github.com/FFmpeg/FFmpeg.git
      GIT_TAG           n6.1.3
      GIT_SHALLOW       TRUE
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/ffmpeg
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}
      CONFIGURE_COMMAND  <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> ${FFMPEG_CFG_FLAGS}
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
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
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG})
  endif()

  add_library(ffmpeg-avdevice STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avdevice ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avdevice INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-avdevice PROPERTIES
    INTERFACE_COMPILE_OPTIONS -D__STDC_CONSTANT_MACROS
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavdevice.a)

  add_library(ffmpeg-avfilter STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avfilter ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avfilter INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avfilter PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavfilter.a)

  add_library(ffmpeg-avformat STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avformat ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avformat INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avformat PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavformat.a)
  target_link_libraries(ffmpeg-avformat INTERFACE openssl-crypto openssl-ssl)

  add_library(ffmpeg-avcodec STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avcodec ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avcodec INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avcodec PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavcodec.a)

  add_library(ffmpeg-swresample STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-swresample ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-swresample INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-swresample PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libswresample.a)

  add_library(ffmpeg-swscale STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-swscale ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-swscale INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-swscale PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libswscale.a)

  add_library(ffmpeg-avutil STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avutil ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avutil INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET ffmpeg-avutil PROPERTY IMPORTED_LOCATION
    ${INSTALL_DIR}/lib/libavutil.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(ffmpeg-src
      GIT_REPOSITORY    https://github.com/FFmpeg/FFmpeg.git
      GIT_TAG           n6.1.3
      GIT_SHALLOW       TRUE
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/ffmpeg
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}
      CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
      # External libraries:
      --disable-libxcb
      --enable-openssl
      # Programs/docs
      --disable-doc
      --disable-programs
      # Compile options
      --enable-pic
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
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
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG})
  endif()

  # Ubuntu: $ apt install libasound2-dev
  find_library(ASOUND_LIBRARY asound)

  set (FFMPEG_LINK -Wl,-Bsymbolic ${ASOUND_LIBRARY})

  add_library(ffmpeg-avdevice STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avdevice ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avdevice INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-avdevice PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavdevice.a)
  target_compile_options(ffmpeg-avdevice INTERFACE -D__STDC_CONSTANT_MACROS)
  target_link_libraries(ffmpeg-avdevice INTERFACE "${FFMPEG_LINK}")

  add_library(ffmpeg-avfilter STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avfilter ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avfilter INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-avfilter PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavfilter.a)

  add_library(ffmpeg-avformat STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avformat ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avformat INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-avformat PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavformat.a)
  target_link_libraries(ffmpeg-avformat INTERFACE ssl crypto)

  add_library(ffmpeg-avcodec STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avcodec ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avcodec INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-avcodec PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavcodec.a)

  add_library(ffmpeg-swresample STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-swresample ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-swresample INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-swresample PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libswresample.a)

  add_library(ffmpeg-swscale STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-swscale ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-swscale INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-swscale PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libswscale.a)

  add_library(ffmpeg-avutil STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avutil ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avutil INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-avutil PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavutil.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    set (FFMPEG_EXPORT export PATH=${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin:$ENV{PATH} && export PKG_CONFIG_PATH=${OPENSSL_DIR}/lib/pkgconfig)

    if ("armeabi-v7a" STREQUAL "${ANDROID_ABI}")
      set (FFMPEG_CFG --cc=armv7a-linux-androideabi${ANDROID_PLATFORM_LEVEL}-clang --arch=arm --disable-neon)
    elseif ("arm64-v8a" STREQUAL "${ANDROID_ABI}")
      # neon assembly causes linker errors (disable neon is not enough)
      set (FFMPEG_CFG --cross-prefix=llvm- --cc=aarch64-linux-android${ANDROID_PLATFORM_LEVEL}-clang --cxx=aarch64-linux-android${ANDROID_PLATFORM_LEVEL}-clang++ --arch=aarch64 --disable-asm)
    elseif ("x86" STREQUAL "${ANDROID_ABI}")
      set (FFMPEG_CFG --cross-prefix=llvm- --cc=i686-linux-android${ANDROID_PLATFORM_LEVEL}-clang --cxx=i686-linux-android${ANDROID_PLATFORM_LEVEL}-clang++ --arch=x86 --disable-asm)
    elseif ("x86_64" STREQUAL "${ANDROID_ABI}")
      set (FFMPEG_CFG --cross-prefix=llvm- --cc=x86_64-linux-android${ANDROID_PLATFORM_LEVEL}-clang --cxx=x86_64-linux-android${ANDROID_PLATFORM_LEVEL}-clang+ --arch=x86_64 --disable-asm)
    else()
      message(WARNING "Unknown Android architecture")
    endif()

    ExternalProject_Add(ffmpeg-src
      GIT_REPOSITORY    https://github.com/FFmpeg/FFmpeg.git
      GIT_TAG           n6.1.3
      GIT_SHALLOW       TRUE
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/ffmpeg
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG}
      CONFIGURE_COMMAND ${FFMPEG_EXPORT} && <SOURCE_DIR>/configure ${FFMPEG_CFG} --prefix=<INSTALL_DIR>
      # External libraries:
      --disable-indev=v4l2
      --disable-indev=android_camera
      --disable-vulkan
      # Programs/docs
      --disable-doc
      --disable-programs
      # Compile options
      --enable-pic
      --enable-cross-compile --sysroot=${CMAKE_SYSROOT} --target-os=android
      BUILD_COMMAND     ${FFMPEG_EXPORT} && make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   ${FFMPEG_EXPORT} && make ${TOOLCHAIN_BUILD_MAKEJOBS} install
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
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_FFMPEG})
  endif()

  add_library(ffmpeg-avdevice STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avdevice ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avdevice INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-avdevice PROPERTIES
    INTERFACE_COMPILE_OPTIONS -D__STDC_CONSTANT_MACROS
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavdevice.a)

  add_library(ffmpeg-avfilter STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avfilter ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avfilter INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-avfilter PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavfilter.a)

  add_library(ffmpeg-avformat STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avformat ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avformat INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-avformat PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavformat.a)
  target_link_libraries(ffmpeg-avformat INTERFACE openssl-crypto openssl-ssl)

  add_library(ffmpeg-avcodec STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avcodec ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avcodec INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-avcodec PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavcodec.a)

  add_library(ffmpeg-swresample STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-swresample ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-swresample INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-swresample PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libswresample.a)

  add_library(ffmpeg-swscale STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-swscale ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-swscale INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-swscale PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libswscale.a)

  add_library(ffmpeg-avutil STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ffmpeg-avutil ffmpeg-src)
  endif()
  target_include_directories (ffmpeg-avutil INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ffmpeg-avutil PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libavutil.a)

endif()
