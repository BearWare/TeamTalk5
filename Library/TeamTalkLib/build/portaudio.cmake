include (ttlib)

if (MSVC)
  set (PORTAUDIO_INCLUDE_DIR ${TTLIBS_ROOT}/portaudio/include)

  set (PORTAUDIO_STATIC_LIB optimized ${TTLIBS_ROOT}/portaudio/lib/$(PlatformName)/pastatic.lib
    debug ${TTLIBS_ROOT}/portaudio/lib/$(PlatformName)/pastaticd.lib)

else()
  set (PORTAUDIO_INCLUDE_DIR ${TTLIBS_ROOT}/portaudio/include)
  set (PORTAUDIO_STATIC_LIB ${TTLIBS_ROOT}/portaudio/lib/libportaudio.a)
endif()

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  find_library (CARBON_LIBRARY Carbon)
  set (PORTAUDIO_LINK_FLAGS ${CARBON_LIBRARY})
  find_library (AUDIOUNIT_LIBRARY AudioUnit)
  list (APPEND PORTAUDIO_LINK_FLAGS ${AUDIOUNIT_LIBRARY} )
  find_library (COREAUDIO_LIBRARY CoreAudio)
  list (APPEND PORTAUDIO_LINK_FLAGS ${COREAUDIO_LIBRARY} )
  find_library (AUDIOTOOLBOX_LIBRARY AudioToolBox )
  list (APPEND PORTAUDIO_LINK_FLAGS ${AUDIOTOOLBOX_LIBRARY} )
endif()

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  find_library(ASOUND_LIBRARY asound)
  set (PORTAUDIO_LINK_FLAGS ${ASOUND_LIBRARY})
endif()
