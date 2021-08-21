if (TOOLCHAIN_LIBVPX)

  set (LIBVPX_LINK_FLAGS libvpx)

else()

  # Ubuntu: libvpx-dev
  find_library(LIBVPX_LIBRARY vpx)
  set (LIBVPX_LINK_FLAGS ${LIBVPX_LIBRARY})

endif()
