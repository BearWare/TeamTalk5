
if (MSVC)
  set (ZLIB_LINK_FLAGS zlib)
else()
  option (ZLIB_STATIC "Build using zlib static libraries" ON)

  if (ZLIB_STATIC)
    set (ZLIB_LINK_FLAGS zlib)
  else()
    # Ubuntu: zlib1g-dev
    find_library(ZLIB_LIBRARY z)
    set (ZLIB_LINK_FLAGS ${ZLIB_LIBRARY})
  endif()
endif()


