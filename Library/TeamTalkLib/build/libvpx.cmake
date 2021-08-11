if (MSVC)
  set (LIBVPX_LINK_FLAGS libvpx)
else()

  option (LIBVPX_STATIC "Build libvpx using static libraries" ON)

  if (LIBVPX_STATIC)
    set (LIBVPX_LINK_FLAGS libvpx)
  else()
    # Ubuntu: libvpx-dev
    find_library(LIBVPX_LIBRARY vpx)
    set (LIBVPX_LINK_FLAGS ${LIBVPX_LIBRARY})
  endif()
endif()
