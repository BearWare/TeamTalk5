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

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR 
    ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR
    ${CMAKE_SYSTEM_NAME} MATCHES "Windows")

  if (PORTAUDIO)
    
    include (portaudio)

    list (APPEND SOUNDSYS_HEADERS ${SOUNDSYS_HEADERS}
      ${TEAMTALKLIB_ROOT}/avstream/PortAudioWrapper.h)

    list (APPEND SOUNDSYS_SOURCES ${SOUNDSYS_SOURCES} 
      ${TEAMTALKLIB_ROOT}/avstream/PortAudioWrapper.cpp)

    list (APPEND SOUNDSYS_INCLUDE_DIR ${PORTAUDIO_INCLUDE_DIR})
    list (APPEND SOUNDSYS_LINK_FLAGS ${PORTAUDIO_LINK_FLAGS})
    set (SOUNDSYS_COMPILE_FLAGS -DENABLE_PORTAUDIO)
    
  endif()
  
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  option (OPENSLES "Build using OpenSL ES" ON)

  list (APPEND SOUNDSYS_HEADERS ${SOUNDSYS_HEADERS}
        ${TEAMTALKLIB_ROOT}/avstream/OpenSLESWrapper.h)

  list (APPEND SOUNDSYS_SOURCES ${SOUNDSYS_SOURCES} 
        ${TEAMTALKLIB_ROOT}/avstream/OpenSLESWrapper.cpp)

  set (SOUNDSYS_COMPILE_FLAGS -DENABLE_OPENSLES)

  find_library (OPENSLES_LIBRARY OpenSLES)
  list (APPEND SOUNDSYS_LINK_FLAGS ${OPENSLES_LIBRARY})
  
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  option (AUDIOUNIT "Build using AudioUnit" ON)

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
