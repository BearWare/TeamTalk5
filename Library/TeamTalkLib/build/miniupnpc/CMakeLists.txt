project(MiniUPnPc)

include(ExternalProject)

##################################################
# MiniUPnPc
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_MINIUPNPC ${TOOLCHAIN_INSTALL_PREFIX}/miniupnpc)

if (MSVC)

  if (NOT CMAKE_VS_PLATFORM_NAME)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      set (CMAKE_VS_PLATFORM_NAME x64)
    else()
      set (CMAKE_VS_PLATFORM_NAME win32)
    endif()
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(miniupnpc-src
      GIT_REPOSITORY    https://github.com/miniupnp/miniupnp.git
      GIT_TAG           miniupnpc_2_3_3
      UPDATE_COMMAND    ""
      PATCH_COMMAND     git apply ${CMAKE_CURRENT_LIST_DIR}/0001-Build-MT-configurations-for-Visual-Studio.patch
      SOURCE_SUBDIR     miniupnpc
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/miniupnpc
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                        -DCMAKE_DEBUG_POSTFIX=d
                        -DUPNPC_BUILD_SHARED=FALSE
                        -DUPNPC_BUILD_TESTS=FALSE
                        -DUPNPC_BUILD_SAMPLE=FALSE
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --config Release
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_MINIUPNPC}
      INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target install --config Release
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libminiupnpcd.lib
                        <INSTALL_DIR>/lib/libminiupnpc.lib
      )

    ExternalProject_Get_Property(miniupnpc-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_MINIUPNPC})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  add_library(miniupnpc STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(miniupnpc miniupnpc-src)
  endif()
  target_include_directories (miniupnpc INTERFACE ${INSTALL_DIR}/include)
  target_compile_definitions (miniupnpc INTERFACE MINIUPNP_STATICLIB)
  set_target_properties(miniupnpc PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/libminiupnpcd.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libminiupnpc.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    string (REPLACE ";" "\$<SEMICOLON>" ARCHS "${CMAKE_OSX_ARCHITECTURES}")
    ExternalProject_Add(miniupnpc-src
      GIT_REPOSITORY    https://github.com/miniupnp/miniupnp.git
      GIT_TAG           miniupnpc_2_3_3
      UPDATE_COMMAND    ""
      SOURCE_SUBDIR     miniupnpc
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/miniupnpc
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                        -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
                        -DCMAKE_OSX_ARCHITECTURES=${ARCHS}
                        -DCMAKE_BUILD_TYPE=release
                        -DUPNPC_BUILD_SHARED=FALSE
                        -DUPNPC_BUILD_TESTS=FALSE
                        -DUPNPC_BUILD_SAMPLE=FALSE
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_MINIUPNPC}
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libminiupnpc.a
      )

    ExternalProject_Get_Property(miniupnpc-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_MINIUPNPC})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  add_library(miniupnpc STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(miniupnpc miniupnpc-src)
  endif()
  target_include_directories (miniupnpc INTERFACE ${INSTALL_DIR}/include)
  target_compile_definitions (miniupnpc INTERFACE MINIUPNP_STATICLIB)
  set_target_properties(miniupnpc PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libminiupnpc.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(miniupnpc-src
      GIT_REPOSITORY    https://github.com/miniupnp/miniupnp.git
      GIT_TAG           miniupnpc_2_3_3
      UPDATE_COMMAND    ""
      SOURCE_SUBDIR     miniupnpc
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/miniupnpc
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                        -DCMAKE_BUILD_TYPE=release
                        -DUPNPC_BUILD_SHARED=FALSE
                        -DUPNPC_BUILD_TESTS=FALSE
                        -DUPNPC_BUILD_SAMPLE=FALSE
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_MINIUPNPC}
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libminiupnpc.a
      )

    ExternalProject_Get_Property(miniupnpc-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_MINIUPNPC})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  add_library(miniupnpc STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(miniupnpc miniupnpc-src)
  endif()
  target_include_directories (miniupnpc INTERFACE ${INSTALL_DIR}/include)
  target_compile_definitions (miniupnpc INTERFACE MINIUPNP_STATICLIB)
  set_target_properties(miniupnpc PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libminiupnpc.a)

endif()
