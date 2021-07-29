include (ttlib)

include (zlib)
include (sndsys)
include (codec)
include (avstream)

set (TTCLIENT_INCLUDE_DIR
  ${ZLIB_INCLUDE_DIR}
  ${CODEC_INCLUDE_DIR}
  ${AVSTREAM_INCLUDE_DIR}
  ${SOUNDSYS_INCLUDE_DIR}
  ${TEAMTALKLIB_ROOT})

set (TTCLIENT_COMPILE_FLAGS
  ${ACE_COMPILE_FLAGS}
  ${SOUNDSYS_COMPILE_FLAGS}
  ${CODEC_COMPILE_FLAGS}
  ${AVSTREAM_COMPILE_FLAGS}
  -DENABLE_HTTP_AUTH
  -DENABLE_ENCRYPTION)

set (TTCLIENT_LINK_FLAGS
  ${ACE_LINK_FLAGS}
  ${CODEC_LINK_FLAGS}
  ${AVSTREAM_LINK_FLAGS}
  ${ZLIB_LINK_FLAGS}
  ${SOUNDSYS_LINK_FLAGS}
  ${OPENSSL_LINK_FLAGS})

set (TTCLIENT_HEADERS
  ${TEAMTALKLIB_ROOT}/TeamTalkDefs.h
  ${TEAMTALKLIB_ROOT}/avstream/AudioResampler.h
  ${TEAMTALKLIB_ROOT}/avstream/VideoCapture.h
  ${TEAMTALKLIB_ROOT}/codec/BmpFile.h
  ${TEAMTALKLIB_ROOT}/codec/WaveFile.h
  ${TEAMTALKLIB_ROOT}/myace/MyACE.h
  ${TEAMTALKLIB_ROOT}/myace/TimerHandler.h
  ${TEAMTALKLIB_ROOT}/mystd/MyStd.h
  ${TEAMTALKLIB_ROOT}/TeamTalkDefs.h
  ${TEAMTALKLIB_ROOT}/teamtalk/Channel.h
  ${TEAMTALKLIB_ROOT}/teamtalk/CodecCommon.h
  ${TEAMTALKLIB_ROOT}/teamtalk/Commands.h
  ${TEAMTALKLIB_ROOT}/teamtalk/Common.h
  ${TEAMTALKLIB_ROOT}/teamtalk/DesktopSession.h
  ${TEAMTALKLIB_ROOT}/teamtalk/Log.h
  ${TEAMTALKLIB_ROOT}/teamtalk/PacketHandler.h
  ${TEAMTALKLIB_ROOT}/teamtalk/PacketHelper.h
  ${TEAMTALKLIB_ROOT}/teamtalk/PacketLayout.h
  ${TEAMTALKLIB_ROOT}/teamtalk/StreamHandler.h
  ${TEAMTALKLIB_ROOT}/teamtalk/ttassert.h
  ${TEAMTALKLIB_ROOT}/teamtalk/User.h
  ${TEAMTALKLIB_ROOT}/teamtalk/client/AudioContainer.h
  ${TEAMTALKLIB_ROOT}/teamtalk/client/AudioThread.h
  ${TEAMTALKLIB_ROOT}/teamtalk/client/ClientChannel.h
  ${TEAMTALKLIB_ROOT}/teamtalk/client/Client.h
  ${TEAMTALKLIB_ROOT}/teamtalk/client/ClientNode.h
  ${TEAMTALKLIB_ROOT}/teamtalk/client/ClientNodeBase.h
  ${TEAMTALKLIB_ROOT}/teamtalk/client/ClientUser.h
  ${TEAMTALKLIB_ROOT}/teamtalk/client/FileNode.h
  ${TEAMTALKLIB_ROOT}/teamtalk/client/StreamPlayers.h
  ${TEAMTALKLIB_ROOT}/teamtalk/client/VideoThread.h
  ${TEAMTALKLIB_ROOT}/teamtalk/client/VoiceLogger.h
  ${TEAMTALKLIB_ROOT}/teamtalk/client/AudioMuxer.h
  ${TEAMTALKLIB_ROOT}/teamtalk/client/DesktopShare.h
  ${TEAMTALKLIB_ROOT}/teamtalk/PacketLayout.inl )

set ( TTCLIENT_SOURCES
  ${TEAMTALKLIB_ROOT}/myace/MyACE.cpp
  ${TEAMTALKLIB_ROOT}/myace/TimerHandler.cpp
  ${TEAMTALKLIB_ROOT}/mystd/MyStd.cpp
  ${TEAMTALKLIB_ROOT}/avstream/AudioResampler.cpp
  ${TEAMTALKLIB_ROOT}/avstream/VideoCapture.cpp
  ${TEAMTALKLIB_ROOT}/codec/BmpFile.cpp
  ${TEAMTALKLIB_ROOT}/codec/WaveFile.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/Channel.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/CodecCommon.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/Commands.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/Common.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/DesktopSession.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/PacketHandler.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/PacketHelper.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/PacketLayout.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/StreamHandler.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/ttassert.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/User.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/client/AudioContainer.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/client/AudioThread.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/client/ClientChannel.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/client/ClientNode.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/client/ClientNodeBase.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/client/ClientUser.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/client/FileNode.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/client/StreamPlayers.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/client/VideoThread.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/client/VoiceLogger.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/client/AudioMuxer.cpp
  ${TEAMTALKLIB_ROOT}/teamtalk/client/DesktopShare.cpp)

list (APPEND TTCLIENT_SOURCES ${AVSTREAM_SOURCES} ${CODEC_SOURCES} ${SOUNDSYS_SOURCES})
list (APPEND TTCLIENT_HEADERS ${AVSTREAM_HEADERS} ${CODEC_HEADERS} ${SOUNDSYS_HEADERS})

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  find_library(COCOA_LIBRARY Cocoa)
  list (APPEND TTCLIENT_LINK_FLAGS ${COCOA_LIBRARY} )

endif()

if (WIN32)
  list (APPEND TTCLIENT_HEADERS
    ${TEAMTALKLIB_ROOT}/win32/HotKey.h
    ${TEAMTALKLIB_ROOT}/win32/Mixer.h
    ${TEAMTALKLIB_ROOT}/win32/WinFirewall.h )

  list (APPEND TTCLIENT_SOURCES
    ${TEAMTALKLIB_ROOT}/win32/HotKey.cpp
    ${TEAMTALKLIB_ROOT}/win32/Mixer.cpp
    ${TEAMTALKLIB_ROOT}/win32/WinFirewall.cpp )

  list (APPEND TTCLIENT_LINK_FLAGS winmm)
endif()
