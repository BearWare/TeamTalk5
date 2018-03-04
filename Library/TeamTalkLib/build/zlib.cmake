
if (MSVC)
  set ( ZLIB_INCLUDE_DIR ${TTLIBS_ROOT}/zlib )

  set ( ZLIB_STATIC_LIB optimized ${TTLIBS_ROOT}/zlib/lib/$(PlatformName)/zlib.lib debug ${TTLIBS_ROOT}/zlib/lib/$(PlatformName)/zlibd.lib)

else()
  set ( ZLIB_INCLUDE_DIR ${TTLIBS_ROOT}/zlib/include )
  set ( ZLIB_STATIC_LIB ${TTLIBS_ROOT}/zlib/lib/libz.a )
endif()


