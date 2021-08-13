
if (MSVC)
  set (SPEEX_LINK_FLAGS speex)
else()

  option (SPEEX_STATIC "Build using static Speex libraries" ON)

  if (SPEEX_STATIC)
    set (SPEEX_LINK_FLAGS speex)
  else()
    # Ubuntu: libspeex-dev
    find_library(SPEEX_LIBRARY speex)
    set (SPEEX_LINK_FLAGS ${SPEEX_LIBRARY})
  endif()
endif()
