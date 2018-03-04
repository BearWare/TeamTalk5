include (ttlib)

set (SOUNDSYS_HEADERS
  ${TEAMTALKLIB_ROOT}/soundsystem/SoundLoopback.h
  ${TEAMTALKLIB_ROOT}/soundsystem/SoundSystem.h
  ${TEAMTALKLIB_ROOT}/soundsystem/SoundSystemBase.h )

set (SOUNDSYS_SOURCES
  ${TEAMTALKLIB_ROOT}/soundsystem/SoundLoopback.cpp
  ${TEAMTALKLIB_ROOT}/soundsystem/SoundSystem.cpp
  ${TEAMTALKLIB_ROOT}/soundsystem/SoundSystemBase.cpp )

set (SOUNDSYS_COMPILE_FLAGS -DENABLE_SOUNDSYSTEM)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR 
    ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR
    ${CMAKE_SYSTEM_NAME} MATCHES "Windows")

  include (portaudio)

  list (APPEND SOUNDSYS_HEADERS ${SOUNDSYS_HEADERS}
    ${TEAMTALKLIB_ROOT}/soundsystem/PortAudioWrapper.h )

  list (APPEND SOUNDSYS_SOURCES ${SOUNDSYS_SOURCES} 
    ${TEAMTALKLIB_ROOT}/soundsystem/PortAudioWrapper.cpp )

  list (APPEND SOUNDSYS_INCLUDE_DIR ${PORTAUDIO_INCLUDE_DIR})
  list (APPEND SOUNDSYS_LINK_FLAGS ${PORTAUDIO_STATIC_LIB})
  list (APPEND SOUNDSYS_LINK_FLAGS ${PORTAUDIO_LINK_FLAGS})
  list (APPEND SOUNDSYS_COMPILE_FLAGS -DENABLE_PORTAUDIO)
endif()
