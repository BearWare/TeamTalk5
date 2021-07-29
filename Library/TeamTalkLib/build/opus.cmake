
if (MSVC)
  set (OPUS_LINK_FLAGS opus)
else()

  option (OPUS_STATIC "Build using static OPUS libraries" ON)

  if (OPUS_STATIC)
    set (OPUS_INCLUDE_DIR ${TTLIBS_ROOT}/opus/include)
    set (OPUS_STATIC_LIB ${TTLIBS_ROOT}/opus/lib/libopus.a)
    set (OPUS_LINK_FLAGS ${OPUS_STATIC_LIB})
  else()
    # Ubuntu: libopus-dev
    find_library(OPUS_LIBRARY opus)
    set (OPUS_LINK_FLAGS ${OPUS_LIBRARY})
  endif()
    
endif()
