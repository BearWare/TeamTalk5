
if (TOOLCHAIN_OPUS)
  set (OPUS_LINK_FLAGS opus)
else()

  # Ubuntu: libopus-dev
  find_library(OPUS_LIBRARY opus)
  set (OPUS_LINK_FLAGS ${OPUS_LIBRARY})

endif()
