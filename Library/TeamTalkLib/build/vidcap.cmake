if (MSVC)
  set ( VIDCAP_INCLUDE_DIR ${TTLIBS_ROOT}/libvidcap-dsbaseclasses/include )

  set ( VIDCAP_STATIC_LIB optimized ${TTLIBS_ROOT}/libvidcap-dsbaseclasses/lib/${CMAKE_VS_PLATFORM_NAME}/libvidcap.lib
    debug ${TTLIBS_ROOT}/libvidcap-dsbaseclasses/lib/${CMAKE_VS_PLATFORM_NAME}/libvidcapd.lib)

endif()
