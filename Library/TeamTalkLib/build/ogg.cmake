if (MSVC)
  set (OGG_LINK_FLAGS ogg)
else()

  option (OGG_STATIC "Build ogg using static libraries" ON)

  if (OGG_STATIC)
    set (OGG_LINK_FLAGS ogg)
  else()
    # Ubuntu: libogg-dev
    find_library(LIBOGG_LIBRARY ogg)
    set (OGG_LINK_FLAGS ${LIBOGG_LIBRARY})
  endif()
endif()
