if (MSVC)
  set ( LIBVPX_INCLUDE_DIR ${TTLIBS_ROOT}/libvpx )

  set ( LIBVPX_STATIC_LIB optimized ${TTLIBS_ROOT}/libvpx/lib/$(PlatformName)/vpxmt.lib
    debug ${TTLIBS_ROOT}/libvpx/lib/$(PlatformName)/vpxmtd.lib)

else()
  set ( LIBVPX_INCLUDE_DIR ${TTLIBS_ROOT}/libvpx/include )
  set ( LIBVPX_STATIC_LIB ${TTLIBS_ROOT}/libvpx/lib/libvpx.a )
endif()
