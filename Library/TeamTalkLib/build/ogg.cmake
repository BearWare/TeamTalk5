if (MSVC)
  set ( OGG_INCLUDE_DIR ${TTLIBS_ROOT}/ogg/include )

  set ( OGG_STATIC_LIB optimized ${TTLIBS_ROOT}/ogg/lib/$(PlatformName)/ogg.lib
    debug ${TTLIBS_ROOT}/ogg/lib/$(PlatformName)/oggd.lib)

else()
  set ( OGG_INCLUDE_DIR ${TTLIBS_ROOT}/ogg/include )
  set ( OGG_STATIC_LIB ${TTLIBS_ROOT}/ogg/lib/libogg.a )
endif()
