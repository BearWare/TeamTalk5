if (TOOLCHAIN_OGG)

  set (OGG_LINK_FLAGS ogg)

else()

  # Ubuntu: libogg-dev
  find_library(LIBOGG_LIBRARY ogg)
  set (OGG_LINK_FLAGS ${LIBOGG_LIBRARY})

endif()
