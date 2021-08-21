if (TOOLCHAIN_SPEEXDSP)
  set (SPEEXDSP_LINK_FLAGS speexdsp)
else()

  # Ubuntu: libspeexdsp-dev
  find_library(SPEEXDSP_LIBRARY speexdsp)
  set (SPEEXDSP_LINK_FLAGS ${SPEEXDSP_LIBRARY})

endif()
