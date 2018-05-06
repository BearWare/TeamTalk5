
set (FFMPEG_INCLUDE_DIR ${TTLIBS_ROOT}/ffmpeg-3.1/include)

set (FFMPEG_STATIC_LIB 
  ${TTLIBS_ROOT}/ffmpeg-3.1/lib/libavdevice.a
  ${TTLIBS_ROOT}/ffmpeg-3.1/lib/libavfilter.a
  ${TTLIBS_ROOT}/ffmpeg-3.1/lib/libavformat.a
  ${TTLIBS_ROOT}/ffmpeg-3.1/lib/libavcodec.a
  ${TTLIBS_ROOT}/ffmpeg-3.1/lib/libswresample.a
  ${TTLIBS_ROOT}/ffmpeg-3.1/lib/libswscale.a
  ${TTLIBS_ROOT}/ffmpeg-3.1/lib/libavutil.a)

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
