
if (MSVC)
  set ( ACE_ROOT ${TTLIBS_ROOT}/ACE/ACE )
  set ( ACE_INCLUDE_DIR ${ACE_ROOT} )
  list (APPEND ACE_INCLUDE_DIR ${ACE_ROOT}/protocols)

  set ( ACE_STATIC_LIB optimized ${ACE_ROOT}/lib/$(PlatformName)/ACEs.lib debug ${ACE_ROOT}/lib/$(PlatformName)/ACEsd.lib)
  set ( ACESSL_STATIC_LIB optimized ${ACE_ROOT}/lib/$(PlatformName)/ACE_SSLs.lib debug ${ACE_ROOT}/lib/$(PlatformName)/ACE_SSLsd.lib)
  set ( ACEINET_STATIC_LIB optimized ${ACE_ROOT}/lib/$(PlatformName)/ACE_INets.lib debug ${ACE_ROOT}/lib/$(PlatformName)/ACE_INetsd.lib)
  set ( ACEINETSSL_STATIC_LIB optimized ${ACE_ROOT}/lib/$(PlatformName)/ACE_INet_SSLs.lib debug ${ACE_ROOT}/lib/$(PlatformName)/ACE_INet_SSLsd.lib)
  
else()
  
  set ( ACE_INCLUDE_DIR ${TTLIBS_ROOT}/ACE/include )
  
  set (ACE_STATIC_LIB ${TTLIBS_ROOT}/ACE/lib/libACE.a)
  set (ACESSL_STATIC_LIB ${TTLIBS_ROOT}/ACE/lib/libACE_SSL.a)
  set (ACEINET_STATIC_LIB ${TTLIBS_ROOT}/ACE/lib/libACE_INet.a)
  set (ACEINETSSL_STATIC_LIB ${TTLIBS_ROOT}/ACE/lib/libACE_INet_SSL.a)
  
endif()


if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  set (ACE_COMPILE_FLAGS -pthread)
  find_library( PTHREAD_LIBRARY pthread )
  set (ACE_LINK_FLAGS ${PTHREAD_LIBRARY} )
  find_library( DL_LIBRARY dl )
  list (APPEND ACE_LINK_FLAGS ${DL_LIBRARY} )
endif()
