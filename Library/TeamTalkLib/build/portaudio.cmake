
if (TOOLCHAIN_PORTAUDIO)
  set (PORTAUDIO_LINK_FLAGS portaudio)

else()

  # Ubuntu: portaudio19-dev
  find_library(PORTAUDIO_LIBRARY portaudio)
  set (PORTAUDIO_LINK_FLAGS ${PORTAUDIO_LIBRARY})

endif()
