if (TOOLCHAIN_MINIUPNPC)

  set (MINIUPNPC_LINK_FLAGS miniupnpc)

else()

  find_library(MINIUPNPC_LIBRARY miniupnpc)
  set (MINIUPNPC_LINK_FLAGS ${MINIUPNPC_LIBRARY})

endif()

if (WIN32)
  list (APPEND MINIUPNPC_LINK_FLAGS ws2_32 iphlpapi)
endif()
