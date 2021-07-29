if (MSVC)

  set (TINYXML_LINK_FLAGS tinyxml)

else()

  option (TINYXML_STATIC "Build using static tinyxml libraries" ON)

  if (TINYXML_STATIC)
    set (TINYXML_INCLUDE_DIR ${TTLIBS_ROOT}/tinyxml )
    set (TINYXML_STATIC_LIB ${TTLIBS_ROOT}/tinyxml/libTinyXML.a )
    set (TINYXML_LINK_FLAGS ${TINYXML_STATIC_LIB})
  else()
    # Ubuntu: libtinyxml-dev
    find_library(TINYXML_LIBRARY tinyxml)
    set (TINYXML_LINK_FLAGS ${TINYXML_LIBRARY})
  endif()
endif()
