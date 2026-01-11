if (TOOLCHAIN_TINYXML2)

  set (TINYXML2_LINK_FLAGS tinyxml2)

else()

  # Ubuntu: libtinyxml2-dev
  find_library(TINYXML2_LIBRARY tinyxml2)
  set (TINYXML2_LINK_FLAGS ${TINYXML2_LIBRARY})

endif()
