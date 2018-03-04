
if (MSVC)
  set ( SPEEXDSP_INCLUDE_DIR ${TTLIBS_ROOT}/speexdsp/include )

  set ( SPEEXDSP_STATIC_LIB optimized ${TTLIBS_ROOT}/speexdsp/lib/$(PlatformName)/libspeexdsp_sse.lib
    debug ${TTLIBS_ROOT}/speexdsp/lib/$(PlatformName)/libspeexdspd.lib)

else()
  set ( SPEEXDSP_INCLUDE_DIR ${TTLIBS_ROOT}/speex/include )
  set ( SPEEXDSP_STATIC_LIB ${TTLIBS_ROOT}/speex/lib/libspeexdsp.a )
endif()
