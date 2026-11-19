
if (TOOLCHAIN_FLAC)
  set (FLAC_LINK_FLAGS flac)
else()
  find_library(FLAC_LIBRARY FLAC)
  set (FLAC_LINK_FLAGS ${FLAC_LIBRARY})
endif()
