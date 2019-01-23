if (MSVC)
  set (LIBVPX_INCLUDE_DIR ${TTLIBS_ROOT}/libvpx)

  set (LIBVPX_LINK_FLAGS optimized ${TTLIBS_ROOT}/libvpx/lib/$(PlatformName)/vpxmt.lib
    debug ${TTLIBS_ROOT}/libvpx/lib/$(PlatformName)/vpxmtd.lib)

else()

  option (LIBVPX_STATIC "Build libvpx using static libraries" ON)

  if (LIBVPX_STATIC)
    set (LIBVPX_INCLUDE_DIR ${TTLIBS_ROOT}/libvpx/include)
    set (LIBVPX_LINK_FLAGS ${TTLIBS_ROOT}/libvpx/lib/libvpx.a)
  else()
    # Ubuntu: libvpx-dev
    find_library(LIBVPX_LIBRARY vpx)
    set (LIBVPX_LINK_FLAGS ${LIBVPX_LIBRARY})
  endif()
endif()
