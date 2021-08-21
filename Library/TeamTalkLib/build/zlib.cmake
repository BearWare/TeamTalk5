if (TOOLCHAIN_ZLIB)
  set (ZLIB_LINK_FLAGS zlib)
else()

  # Ubuntu: zlib1g-dev
  find_library(ZLIB_LIBRARY z)
  set (ZLIB_LINK_FLAGS ${ZLIB_LIBRARY})
  
endif()


