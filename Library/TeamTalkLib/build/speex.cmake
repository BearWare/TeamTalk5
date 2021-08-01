
if (MSVC)
  set (SPEEX_LINK_FLAGS speex)
else()

  option (SPEEX_STATIC "Build using static Speex libraries" ON)

  if (SPEEX_STATIC)
    set (SPEEX_INCLUDE_DIR ${TTLIBS_ROOT}/speex/include)
    set (SPEEX_LINK_FLAGS ${TTLIBS_ROOT}/speex/lib/libspeex.a)
  else()
    # Ubuntu: libspeex-dev
    find_library(SPEEX_LIBRARY speex)
    set (SPEEX_LINK_FLAGS ${SPEEX_LIBRARY})
  endif()
endif()
