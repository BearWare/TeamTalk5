
if (MSVC)
  set (SPEEXDSP_INCLUDE_DIR ${TTLIBS_ROOT}/speexdsp/include)

  set (SPEEXDSP_LINK_FLAGS optimized ${TTLIBS_ROOT}/speexdsp/lib/$(PlatformName)/libspeexdsp_sse.lib
    debug ${TTLIBS_ROOT}/speexdsp/lib/$(PlatformName)/libspeexdspd.lib)

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
