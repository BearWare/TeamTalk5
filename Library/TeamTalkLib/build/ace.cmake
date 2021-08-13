option (BUILD_TEAMTALK_ACE "Build customized ACE INet SSL library with SNI-enabled" ON)

include (openssl)

if (BUILD_TEAMTALK_ACE)
  set (ACE_COMPILE_FLAGS -DENABLE_TEAMTALKACE)

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    list (APPEND ACE_COMPILE_FLAGS -DACE_HAS_CUSTOM_EXPORT_MACROS=0 -D__ACE_INLINE__)
  endif()

endif()

if (MSVC)

  set (ACE_LINK_FLAGS ace_inet_ssl ace_ssl ace_inet ace)

else() # Mac & Linux

  option (ACE_STATIC "Build using static ACE libraries" ON)

  if (ACE_STATIC)
    set (ACE_LINK_FLAGS ace_inet_ssl ace_ssl ace_inet ace)
  else()
    find_library(ACE_LIBRARY ACE)
    set (ACE_LINK_FLAGS ${ACE_LIBRARY})
    find_library(ACEINET_LIBRARY ACE_INet)
    list (APPEND ACE_LINK_FLAGS ${ACEINET_LIBRARY})
    find_library(ACESSL_LIBRARY ACE_SSL)
    list (APPEND ACE_LINK_FLAGS ${ACESSL_LIBRARY})
    find_library(ACEINETSSL_LIBRARY ACE_INet_SSL)
    list (APPEND ACE_LINK_FLAGS ${ACEINETSSL_LIBRARY})
  endif()

endif()


if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list (APPEND ACE_COMPILE_FLAGS -pthread)
  find_library (PTHREAD_LIBRARY pthread)
  list (APPEND ACE_LINK_FLAGS ${PTHREAD_LIBRARY})
  find_library (DL_LIBRARY dl)
  list (APPEND ACE_LINK_FLAGS ${DL_LIBRARY})
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
  find_library (LOG_LIBRARY log)
  list (APPEND ACE_LINK_FLAGS ${LOG_LIBRARY})
endif()

# Special Raspberry Pi handling of 'off_t'
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND ${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "armv7l")
  list (APPEND ACE_COMPILE_FLAGS -D_FILE_OFFSET_BITS=64)
endif()
