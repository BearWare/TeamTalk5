
if (MSVC)
  set (OPUS_LINK_FLAGS opus)
else()

  option (OPUS_STATIC "Build using static OPUS libraries" ON)

  if (OPUS_STATIC)
    set (OPUS_LINK_FLAGS opus)
  else()
    # Ubuntu: libopus-dev
    find_library(OPUS_LIBRARY opus)
    set (OPUS_LINK_FLAGS ${OPUS_LIBRARY})
  endif()

endif()
