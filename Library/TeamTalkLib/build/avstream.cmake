include (ttlib)

set (AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/MediaStreamer.cpp)
set (AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/MediaStreamer.h)

list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/MediaPlayback.cpp)
list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/MediaPlayback.h)

list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/AudioInputStreamer.cpp)
list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/AudioInputStreamer.h)

if (FEATURE_SPEEXDSP)
  include (speexdsp)
  list (APPEND AVSTREAM_COMPILE_FLAGS -DENABLE_SPEEXDSP)
  list (APPEND AVSTREAM_LINK_FLAGS ${SPEEXDSP_LINK_FLAGS})
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/SpeexPreprocess.cpp)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/SpeexResampler.cpp)
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/SpeexPreprocess.h)
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/SpeexResampler.h)
endif()

if (FEATURE_FFMPEG)
  include (ffmpeg)
  list (APPEND AVSTREAM_COMPILE_FLAGS -DENABLE_FFMPEG3 ${FFMPEG_COMPILE_FLAGS})
  list (APPEND AVSTREAM_LINK_FLAGS ${FFMPEG_LINK_FLAGS})

  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/FFmpegStreamer.cpp)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/FFmpegResampler.cpp)
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/FFmpegStreamer.h)
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/FFmpegResampler.h)

  if (FEATURE_V4L2)
    list (APPEND AVSTREAM_COMPILE_FLAGS -DENABLE_V4L2)
    list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/FFmpegCapture.cpp)
    list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/VideoCapture.cpp)
    list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/V4L2Capture.cpp)
    list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/FFmpegCapture.h)
    list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/VideoCapture.h)
    list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/V4L2Capture.h)
  endif()

  if (FEATURE_AVF)
    list (APPEND AVSTREAM_COMPILE_FLAGS -DENABLE_AVF)
    list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/AVFCapture.mm)
    list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/AVFVideoInput.cpp)
    list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/FFmpegCapture.cpp)
    list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/VideoCapture.cpp)
    list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/AVFCapture.h)
    list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/AVFVideoInput.h)
    list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/FFmpegCapture.h)
    list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/VideoCapture.h)
  endif()
endif()

if (FEATURE_WEBRTC)
  include (webrtc)
  list (APPEND AVSTREAM_COMPILE_FLAGS -DENABLE_WEBRTC ${WEBRTC_COMPILE_FLAGS})
  list (APPEND AVSTREAM_LINK_FLAGS ${WEBRTC_LINK_FLAGS})
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/WebRTCPreprocess.cpp)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/WebRTCPreprocess.h)
endif()

if (FEATURE_MSDMO)
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/DMOResampler.h)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/DMOResampler.cpp)
  list (APPEND AVSTREAM_COMPILE_FLAGS -DENABLE_DMORESAMPLER)
  list (APPEND AVSTREAM_LINK_FLAGS Msdmo strmiids)
endif()

if (FEATURE_DSHOW)
  include (dshow)
  list (APPEND AVSTREAM_COMPILE_FLAGS -DENABLE_DSHOW)
  list (APPEND AVSTREAM_LINK_FLAGS ${DSHOW_STATIC_LIB})

  list (APPEND AVSTREAM_INCLUDE_DIR ${DSHOW_INCLUDE_DIR})
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/WinMedia.h)
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/MediaStreamer.h)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/WinMedia.cpp)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/MediaStreamer.cpp)
endif()

if (FEATURE_VIDCAP)
  include (vidcap)
  list (APPEND AVSTREAM_INCLUDE_DIR ${VIDCAP_INCLUDE_DIR})
  list (APPEND AVSTREAM_LINK_FLAGS ${VIDCAP_STATIC_LIB})
  list (APPEND AVSTREAM_COMPILE_FLAGS -DENABLE_LIBVIDCAP )
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/VideoCapture.h)
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/LibVidCap.h)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/VideoCapture.cpp)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/LibVidCap.cpp)
endif()

if (FEATURE_MEDIAFOUNDATION)
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/MediaStreamer.h)
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/MFCapture.h)
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/MFStreamer.h)
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/MFTransform.h)
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/VideoCapture.h)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/MediaStreamer.cpp)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/MFCapture.cpp)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/MFStreamer.cpp)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/MFTransform.cpp)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/VideoCapture.cpp)
  list (APPEND AVSTREAM_LINK_FLAGS mf mfplat mfreadwrite mfuuid shlwapi propsys)
  list (APPEND AVSTREAM_COMPILE_FLAGS -DWINVER=0x0601 -DENABLE_MEDIAFOUNDATION) # WINVER=_WIN32_WINNT_WIN7
endif()

if (FEATURE_OPUSTOOLS AND FEATURE_OPUS)
  list (APPEND AVSTREAM_HEADERS ${TEAMTALKLIB_ROOT}/avstream/OpusFileStreamer.h)
  list (APPEND AVSTREAM_SOURCES ${TEAMTALKLIB_ROOT}/avstream/OpusFileStreamer.cpp)
  list (APPEND AVSTREAM_LINK_FLAGS ${OPUSTOOLS_LINK_FLAGS})
endif()
