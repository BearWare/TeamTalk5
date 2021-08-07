cmake_minimum_required(VERSION 3.1)

project(Toolchain)

include(ExternalProject)

set (PLATFORMTOOLSET "/property:PlatformToolset=v142")

add_subdirectory(openssl)
add_subdirectory(ace)
add_subdirectory(tinyxml)
add_subdirectory(zlib)
add_subdirectory(catch2)
set (CATCH_INCLUDE_DIR ${CATCH_INCLUDE_DIR} PARENT_SCOPE)

option (LIBVPX "Build libVPX codec classes" ON)
if (LIBVPX)
  add_subdirectory(libvpx)
endif()

option (OGG "Build libOgg classes" ON)
if (OGG)
  add_subdirectory(ogg)
endif()

option (OPUS "Build OPUS codec classes" ON)
if (OPUS)
  add_subdirectory(opus)
endif()

option (OPUSTOOLS "Build Opus-tools" ON)
if (OPUSTOOLS)
  add_subdirectory(opustools)
endif()

option (PORTAUDIO "Build using PortAudio" ON)
if (PORTAUDIO)
  add_subdirectory(portaudio)
endif()

option (SPEEX "Build Speex codec classes" ON)
if (SPEEX)
  add_subdirectory(speex)
endif()

option (SPEEXDSP "Build SpeexDSP codec classes" ON)
if (SPEEXDSP)
  add_subdirectory(speexdsp)
endif()

