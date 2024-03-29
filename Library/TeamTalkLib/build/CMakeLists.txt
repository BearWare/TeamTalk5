project(Toolchain)

include(ExternalProject)

set (TOOLCHAIN_BUILD_PREFIX build CACHE STRING "Specify alternative 'clone' directory to build all toolchain dependencies. Default: CMAKE_CURRENT_BINARY_DIR/build")
set (TOOLCHAIN_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install CACHE STRING "Install path for TeamTalk toolchain. Default: CMAKE_CURRENT_BINARY_DIR/install")

option (TOOLCHAIN_BUILD_EXTERNALPROJECTS "Build toolchain libraries from scratch in TOOLCHAIN_BUILD_PREFIX. Alternatively reuse existing libraries installed in TOOLCHAIN_INSTALL_PREFIX" ON)
add_feature_info(TOOLCHAIN_BUILD_EXTERNALPROJECTS TOOLCHAIN_BUILD_EXTERNALPROJECTS "Build toolchain's external projects, i.e. TOOLCHAIN_*")

set (TOOLCHAIN_BUILD_MAKEJOBS -j4 CACHE STRING "Maximum number of build jobs to pass to make-command")

option (TOOLCHAIN_OPENSSL "Build and link OpenSSL from TeamTalk toolchain" ON)
add_feature_info(TOOLCHAIN_OPENSSL TOOLCHAIN_OPENSSL "OpenSSL library is provided by TOOLCHAIN_INSTALL_PREFIX")
if (TOOLCHAIN_OPENSSL)
  add_subdirectory(openssl)
endif()

option (TOOLCHAIN_ACE "Build and link ACE from TeamTalk toolchain" ON)
add_feature_info (TOOLCHAIN_ACE TOOLCHAIN_ACE "ACE framework is provided by TOOLCHAIN_INSTALL_PREFIX")
if (TOOLCHAIN_ACE)
  add_subdirectory(ace)
endif()

option (TOOLCHAIN_TINYXML "Build and link TinyXML from TeamTalk toolchain" ON)
add_feature_info (TOOLCHAIN_TINYXML TOOLCHAIN_TINYXML "TinyXML is provided by TOOLCHAIN_INSTALL_PREFIX")
if (TOOLCHAIN_TINYXML)
  add_subdirectory(tinyxml)
endif()

option (TOOLCHAIN_ZLIB "Build and link ZLib from TeamTalk toolchain" ON)
add_feature_info (TOOLCHAIN_ZLIB TOOLCHAIN_ZLIB "ZLib is provided by TOOLCHAIN_INSTALL_PREFIX")
if (TOOLCHAIN_ZLIB)
  add_subdirectory(zlib)
endif()

option (TOOLCHAIN_CATCH2 "Build and link Catch2 from TeamTalk toolchain" ON)
add_feature_info (TOOLCHAIN_CATCH2 TOOLCHAIN_CATCH2 "Catch2 is provided by TOOLCHAIN_INSTALL_PREFIX")
if (TOOLCHAIN_CATCH2)
  add_subdirectory(catch2)
  set (CATCH_INCLUDE_DIR ${CATCH_INCLUDE_DIR} PARENT_SCOPE)
endif()

option (TOOLCHAIN_LIBVPX "Build and link LibVPX from TeamTalk toolchain" ON)
add_feature_info(TOOLCHAIN_LIBVPX TOOLCHAIN_LIBVPX "libVPX video codec is provided by TOOLCHAIN_INSTALL_PREFIX")
if (TOOLCHAIN_LIBVPX)
  add_subdirectory(libvpx)
endif()

option (TOOLCHAIN_FFMPEG "Build and link FFmpeg from TeamTalk toolchain" ON)
add_feature_info (TOOLCHAIN_FFMPEG TOOLCHAIN_FFMPEG "FFmpeg is provided by TOOLCHAIN_INSTALL_PREFIX")
if (TOOLCHAIN_FFMPEG)
  add_subdirectory(ffmpeg)
endif()

option (TOOLCHAIN_OGG "Build and link Ogg from TeamTalk toolchain" ON)
add_feature_info (TOOLCHAIN_OGG TOOLCHAIN_OGG "Ogg is provided by TOOLCHAIN_INSTALL_PREFIX")
if (TOOLCHAIN_OGG)
  add_subdirectory(ogg)
endif()

option (TOOLCHAIN_OPUS "Build and link OPUS from TeamTalk toolchain" ON)
add_feature_info (TOOLCHAIN_OPUS TOOLCHAIN_OPUS "OPUS is provided by TOOLCHAIN_INSTALL_PREFIX")
if (TOOLCHAIN_OPUS)
  add_subdirectory(opus)
endif()

option (TOOLCHAIN_PORTAUDIO "Build and link PortAudio from TeamTalk toolchain" ON)
add_feature_info (TOOLCHAIN_PORTAUDIO TOOLCHAIN_PORTAUDIO "PortAudio is provided by TOOLCHAIN_INSTALL_PREFIX")
if (TOOLCHAIN_PORTAUDIO)
  add_subdirectory(portaudio)
endif()

option (TOOLCHAIN_SPEEX "Build and link Speex from TeamTalk toolchain" ON)
add_feature_info (TOOLCHAIN_SPEEX TOOLCHAIN_SPEEX "Speex is provided by TOOLCHAIN_INSTALL_PREFIX")
if (TOOLCHAIN_SPEEX)
  add_subdirectory(speex)
endif()

option (TOOLCHAIN_SPEEXDSP "Build and link SpeexDSP from TeamTalk toolchain" ON)
add_feature_info (TOOLCHAIN_SPEEXDSP TOOLCHAIN_SPEEXDSP "SpeexDSP is provided by TOOLCHAIN_INSTALL_PREFIX")
if (TOOLCHAIN_SPEEXDSP)
  add_subdirectory(speexdsp)
endif()

if (FEATURE_OPUSTOOLS)
  add_subdirectory(opustools)
endif()

if (FEATURE_WEBRTC)
  add_subdirectory(webrtc)
endif()
