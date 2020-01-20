
if (MSVC)
  set (ZLIB_INCLUDE_DIR ${TTLIBS_ROOT}/zlib)
  set (ZLIB_STATIC_LIB optimized ${TTLIBS_ROOT}/zlib/lib/$(PlatformName)/zlib.lib debug ${TTLIBS_ROOT}/zlib/lib/$(PlatformName)/zlibd.lib)
  set (ZLIB_LINK_FLAGS ${ZLIB_STATIC_LIB})
else()
  option (ZLIB_STATIC "Build using zlib static libraries" ON)

  if (ZLIB_STATIC)
    set (ZLIB_INCLUDE_DIR ${TTLIBS_ROOT}/zlib/include)
    set (ZLIB_STATIC_LIB ${TTLIBS_ROOT}/zlib/lib/libz.a)
    set (ZLIB_LINK_FLAGS ${ZLIB_STATIC_LIB})
  else()
    # Ubuntu: zlib1g-dev
    find_library(ZLIB_LIBRARY z)
    set (ZLIB_LINK_FLAGS ${ZLIB_LIBRARY})
  endif()
endif()


