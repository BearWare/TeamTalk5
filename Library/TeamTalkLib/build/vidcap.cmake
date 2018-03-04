if (MSVC)
  set ( VIDCAP_INCLUDE_DIR ${TTLIBS_ROOT}/libvidcap-dsbaseclasses/include )

  set ( VIDCAP_STATIC_LIB optimized ${TTLIBS_ROOT}/libvidcap-dsbaseclasses/lib/$(PlatformName)/libvidcap.lib
    debug ${TTLIBS_ROOT}/libvidcap-dsbaseclasses/lib/$(PlatformName)/libvidcapd.lib)

endif()
