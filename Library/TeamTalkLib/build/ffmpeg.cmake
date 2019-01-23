
option (FFMPEG_STATIC "Build using static FFmpeg libraries" ON)

if (FFMPEG_STATIC)
  set (FFMPEG_INCLUDE_DIR ${TTLIBS_ROOT}/ffmpeg/include)

  set (FFMPEG_LINK_FLAGS
    ${TTLIBS_ROOT}/ffmpeg/lib/libavdevice.a
    ${TTLIBS_ROOT}/ffmpeg/lib/libavfilter.a
    ${TTLIBS_ROOT}/ffmpeg/lib/libavformat.a
    ${TTLIBS_ROOT}/ffmpeg/lib/libavcodec.a
    ${TTLIBS_ROOT}/ffmpeg/lib/libswresample.a
    ${TTLIBS_ROOT}/ffmpeg/lib/libswscale.a
    ${TTLIBS_ROOT}/ffmpeg/lib/libavutil.a)
else()
  # Ubuntu: libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev libavresample-dev libavutil-dev libswresample-dev libswscale-dev

  find_library(AVDEVICE_LIBRARY avdevice)
  list (APPEND FFMPEG_LINK_FLAGS ${AVDEVICE_LIBRARY})
  find_library(AVFILTER_LIBRARY avfilter)
  list (APPEND FFMPEG_LINK_FLAGS ${AVFILTER_LIBRARY})
  find_library(avformat_LIBRARY avformat)
  list (APPEND FFMPEG_LINK_FLAGS ${AVFORMAT_LIBRARY})
  find_library(AVCODEC_LIBRARY avcodec)
  list (APPEND FFMPEG_LINK_FLAGS ${AVCODEC_LIBRARY})
  find_library(SWRESAMPLE_LIBRARY swresample)
  list (APPEND FFMPEG_LINK_FLAGS ${DWRESAMPLE_LIBRARY})
  find_library(SWSCALE_LIBRARY swscale)
  list (APPEND FFMPEG_LINK_FLAGS ${SWSCALE_LIBRARY})
  find_library(AVUTIL_LIBRARY avutil)
  list (APPEND FFMPEG_LINK_FLAGS ${AVUTIL_LIBRARY})
endif()

set (FFMPEG_COMPILE_FLAGS -D__STDC_CONSTANT_MACROS)


if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  find_library(COCOA_LIBRARY Cocoa)
  list (APPEND FFMPEG_LINK_FLAGS ${COCOA_LIBRARY})
  find_library(AVFOUNDATION_LIBRARY AVFoundation)
  list (APPEND FFMPEG_LINK_FLAGS ${AVFOUNDATION_LIBRARY})
  find_library(COREMEDIA_LIBRARY CoreMedia)
  list (APPEND FFMPEG_LINK_FLAGS ${COREMEDIA_LIBRARY})
  find_library(OPENGL_LIBRARY OpenGL)
  list (APPEND FFMPEG_LINK_FLAGS ${OPENGL_LIBRARY})
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  list (APPEND FFMPEG_LINK_FLAGS ${COREFOUNDATION_LIBRARY})
  find_library(VIDEODECODEACCELERATION_LIBRARY VideoDecodeAcceleration)
  list (APPEND FFMPEG_LINK_FLAGS ${VIDEODECODEACCELERATION_LIBRARY})
  find_library(QUARTZCORE_LIBRARY QuartzCore)
  list (APPEND FFMPEG_LINK_FLAGS ${QUARTZCORE_LIBRARY})
  find_library(BZ2_LIBRARY bz2)
  list (APPEND FFMPEG_LINK_FLAGS ${BZ2_LIBRARY})
  find_library (AUDIOTOOLBOX_LIBRARY AudioToolBox)
  list (APPEND FFMPEG_LINK_FLAGS ${AUDIOTOOLBOX_LIBRARY})
endif()
