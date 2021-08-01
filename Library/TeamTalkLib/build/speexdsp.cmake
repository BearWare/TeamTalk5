if (MSVC)
  set (SPEEXDSP_LINK_FLAGS speexdsp)
else()

  option (SPEEXDSP_STATIC "Build using static SpeexDSP libraries" ON)

  if (SPEEXDSP_STATIC)
    set (SPEEXDSP_INCLUDE_DIR ${TTLIBS_ROOT}/speex/include)
    set (SPEEXDSP_LINK_FLAGS ${TTLIBS_ROOT}/speex/lib/libspeexdsp.a)
  else()
    # Ubuntu: libspeexdsp-dev
    find_library(SPEEXDSP_LIBRARY speexdsp)
    set (SPEEXDSP_LINK_FLAGS ${SPEEXDSP_LIBRARY})
  endif()
endif()
