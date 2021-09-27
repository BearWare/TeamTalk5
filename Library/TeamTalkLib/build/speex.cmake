
if (TOOLCHAIN_SPEEX)
  set (SPEEX_LINK_FLAGS speex)
else()

  # Ubuntu: libspeex-dev
  find_library(SPEEX_LIBRARY speex)
  set (SPEEX_LINK_FLAGS ${SPEEX_LIBRARY})

endif()
