if (TOOLCHAIN_TINYXML)

  set (TINYXML_LINK_FLAGS tinyxml)

else()

  # Ubuntu: libtinyxml-dev
  find_library(TINYXML_LIBRARY tinyxml)
  set (TINYXML_LINK_FLAGS ${TINYXML_LIBRARY})

endif()
