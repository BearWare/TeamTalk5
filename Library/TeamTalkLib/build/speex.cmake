
if (MSVC)
  set ( SPEEX_INCLUDE_DIR ${TTLIBS_ROOT}/speex/include )  

  set ( SPEEX_STATIC_LIB optimized ${TTLIBS_ROOT}/speex/lib/$(PlatformName)/libspeex_sse2.lib
    debug ${TTLIBS_ROOT}/speex/lib/$(PlatformName)/libspeexd.lib)

else()
  set ( SPEEX_INCLUDE_DIR ${TTLIBS_ROOT}/speex/include )
  set ( SPEEX_STATIC_LIB ${TTLIBS_ROOT}/speex/lib/libspeex.a )
endif()
