cmake_minimum_required(VERSION 3.1)

project(Toolchain)

include(ExternalProject)

if (MSVC)

  set (PLATFORMTOOLSET "/property:PlatformToolset=v142")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set (CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
    message(WARNING "No deployment target specified. Defaulting to ${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
  if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set (CMAKE_OSX_DEPLOYMENT_TARGET 7.0)
    message(WARNING "No deployment target specified. Defaulting to ${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif()

endif()

option (TOOLCHAIN_OPENSSL "Build OpenSSL from toolchain" ON)
if (TOOLCHAIN_OPENSSL)
  add_subdirectory(openssl)
endif()

option (TOOLCHAIN_ACE "Build ACE from toolchain" ON)
if (TOOLCHAIN_ACE)
  add_subdirectory(ace)
endif()

option (TOOLCHAIN_TINYXML "Build TinyXML from toolchain" ON)
if (TOOLCHAIN_TINYXML)
  add_subdirectory(tinyxml)
endif()

option (TOOLCHAIN_ZLIB "Build ZLib from toolchain" ON)
if (TOOLCHAIN_ZLIB)
  add_subdirectory(zlib)
endif()

option (TOOLCHAIN_CATCH2 "Build Catch2 from toolchain" ON)
if (TOOLCHAIN_CATCH2)
  add_subdirectory(catch2)
  set (CATCH_INCLUDE_DIR ${CATCH_INCLUDE_DIR} PARENT_SCOPE)
endif()

option (TOOLCHAIN_LIBVPX "Build LibVPX from toolchain" ON)
if (TOOLCHAIN_LIBVPX)
  add_subdirectory(libvpx)
endif()

option (TOOLCHAIN_FFMPEG "Build FFmpeg from toolchain" ON)
if (TOOLCHAIN_FFMPEG)
  add_subdirectory(ffmpeg)
endif()

option (TOOLCHAIN_OGG "Build Ogg from toolchain" ON)
if (TOOLCHAIN_OGG)
  add_subdirectory(ogg)
endif()

option (TOOLCHAIN_OPUS "Build OPUS from toolchain" ON)
if (TOOLCHAIN_OPUS)
  add_subdirectory(opus)
endif()

option (TOOLCHAIN_PORTAUDIO "Build PortAudio from toolchain" ON)
if (TOOLCHAIN_PORTAUDIO)
  add_subdirectory(portaudio)
endif()

option (TOOLCHAIN_SPEEX "Build Speex from toolchain" ON)
if (TOOLCHAIN_SPEEX)
  add_subdirectory(speex)
endif()

option (TOOLCHAIN_SPEEXDSP "Build SpeexDSP from toolchain" ON)
if (TOOLCHAIN_SPEEXDSP)
  add_subdirectory(speexdsp)
endif()

if (OPUSTOOLS)
  add_subdirectory(opustools)
endif()

if (WEBRTC)
  add_subdirectory(webrtc)
endif()
