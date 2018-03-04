
if (MSVC)
  set ( OPUS_INCLUDE_DIR ${TTLIBS_ROOT}/opus/include )

  set ( OPUS_STATIC_LIB optimized ${TTLIBS_ROOT}/opus/lib/$(PlatformName)/opus.lib
    debug ${TTLIBS_ROOT}/opus/lib/$(PlatformName)/opusd.lib)
else()
  set ( OPUS_INCLUDE_DIR ${TTLIBS_ROOT}/opus/include )
  set ( OPUS_STATIC_LIB ${TTLIBS_ROOT}/opus/lib/libopus.a )
endif()
