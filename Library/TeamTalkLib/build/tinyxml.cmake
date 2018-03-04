
if (MSVC)
  set ( TINYXML_INCLUDE_DIR ${TTLIBS_ROOT}/tinyxml )

  set ( TINYXML_STATIC_LIB optimized ${TTLIBS_ROOT}/tinyxml/lib/$(PlatformName)/tinyxml.lib
    debug ${TTLIBS_ROOT}/tinyxml/lib/$(PlatformName)/tinyxmld.lib)

else()
  set ( TINYXML_INCLUDE_DIR ${TTLIBS_ROOT}/tinyxml )
  set ( TINYXML_STATIC_LIB ${TTLIBS_ROOT}/tinyxml/libTinyXML.a )
endif()
