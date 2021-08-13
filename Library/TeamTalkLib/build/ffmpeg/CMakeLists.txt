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

endif()
