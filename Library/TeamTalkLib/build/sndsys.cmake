include (ttlib)

set (SOUNDSYS_HEADERS
  ${TEAMTALKLIB_ROOT}/avstream/SoundLoopback.h
  ${TEAMTALKLIB_ROOT}/avstream/SoundSystem.h
  ${TEAMTALKLIB_ROOT}/avstream/SoundSystemBase.h
  ${TEAMTALKLIB_ROOT}/avstream/SoundSystemEx.h
  ${TEAMTALKLIB_ROOT}/avstream/SoundSystemShared.h)

set (SOUNDSYS_SOURCES
  ${TEAMTALKLIB_ROOT}/avstream/SoundLoopback.cpp
  ${TEAMTALKLIB_ROOT}/avstream/SoundSystem.cpp
  ${TEAMTALKLIB_ROOT}/avstream/SoundSystemEx.cpp)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  # Needed for CLSID_CWMAudioAEC
  set (SOUNDSYS_LINK_FLAGS wmcodecdspuuid dmoguids)
endif()

# Windows, Linux and macOS sound system
if (FEATURE_PORTAUDIO)

  include (portaudio)
  list (APPEND SOUNDSYS_INCLUDE_DIR ${PORTAUDIO_INCLUDE_DIR})
  list (APPEND SOUNDSYS_LINK_FLAGS ${PORTAUDIO_LINK_FLAGS})
  set (SOUNDSYS_COMPILE_FLAGS -DENABLE_PORTAUDIO)

  list (APPEND SOUNDSYS_HEADERS ${SOUNDSYS_HEADERS}
    ${TEAMTALKLIB_ROOT}/avstream/PortAudioWrapper.h)

  list (APPEND SOUNDSYS_SOURCES ${SOUNDSYS_SOURCES}
    ${TEAMTALKLIB_ROOT}/avstream/PortAudioWrapper.cpp)

endif()

# Android sound system
if (FEATURE_OPENSLES)

  list (APPEND SOUNDSYS_HEADERS ${SOUNDSYS_HEADERS}
    ${TEAMTALKLIB_ROOT}/avstream/OpenSLESWrapper.h)

  list (APPEND SOUNDSYS_SOURCES ${SOUNDSYS_SOURCES}
    ${TEAMTALKLIB_ROOT}/avstream/OpenSLESWrapper.cpp)

  set (SOUNDSYS_COMPILE_FLAGS -DENABLE_OPENSLES)

  find_library (OPENSLES_LIBRARY OpenSLES)
  list (APPEND SOUNDSYS_LINK_FLAGS ${OPENSLES_LIBRARY})
endif()

# iOS sound system
if (FEATURE_AUDIOUNIT)

  list (APPEND SOUNDSYS_HEADERS ${SOUNDSYS_HEADERS}
    ${TEAMTALKLIB_ROOT}/avstream/AudioUnit.h)

  list (APPEND SOUNDSYS_SOURCES ${SOUNDSYS_SOURCES}
    ${TEAMTALKLIB_ROOT}/avstream/AudioUnit.mm)

  set (SOUNDSYS_COMPILE_FLAGS -DENABLE_AUDIOUNIT)

  find_library (AUDIOTOOLBOX_LIBRARY AudioToolbox)
  list (APPEND SOUNDSYS_LINK_FLAGS ${AUDIOTOOLBOX_LIBRARY})

  find_library (AVFOUNDATION_LIBRARY AVFoundation)
  list (APPEND SOUNDSYS_LINK_FLAGS ${AVFOUNDATION_LIBRARY})

  find_library (OBJC_LIBRARY objc)
  list (APPEND SOUNDSYS_LINK_FLAGS ${OBJC_LIBRARY})

endif()
