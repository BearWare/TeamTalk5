project(TinyXML)

include(ExternalProject)

##################################################
# TinyXML
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_TINYXML ${TOOLCHAIN_INSTALL_PREFIX}/tinyxml)

if (MSVC)

  if (NOT CMAKE_VS_PLATFORM_NAME)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      set (CMAKE_VS_PLATFORM_NAME x64)
    else()
      set (CMAKE_VS_PLATFORM_NAME win32)
    endif()
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(tinyxml-src
      GIT_REPOSITORY    https://github.com/bear101/tinyxml.git
      GIT_TAG           bdr_v2
      UPDATE_DISCONNECTED ON
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/tinyxml
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DTINYXML_MULTITHREADED=ON
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --config Release
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_TINYXML}
      INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target install --config Release
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/tinyxmld.lib
                        <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/tinyxml.lib
      )

    ExternalProject_Get_Property(tinyxml-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_TINYXML})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  add_library(tinyxml STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(tinyxml tinyxml-src)
  endif()
  target_include_directories (tinyxml INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(tinyxml PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/${CMAKE_VS_PLATFORM_NAME}/tinyxmld.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/${CMAKE_VS_PLATFORM_NAME}/tinyxml.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      string (REPLACE ";" "\$<SEMICOLON>" ARCHS "${CMAKE_OSX_ARCHITECTURES}")
      ExternalProject_Add(tinyxml-src
        GIT_REPOSITORY    https://github.com/bear101/tinyxml.git
        GIT_TAG           bdr_v2
        UPDATE_DISCONNECTED ON
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/tinyxml
        CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                          -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
                          -DCMAKE_OSX_ARCHITECTURES=${ARCHS}
                          -DCMAKE_BUILD_TYPE=release
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_TINYXML}
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libtinyxml.a
        )

      ExternalProject_Get_Property(tinyxml-src INSTALL_DIR)
      file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
    else()
      set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_TINYXML})
    endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

    add_library(tinyxml STATIC IMPORTED GLOBAL)
    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      add_dependencies(tinyxml tinyxml-src)
    endif()
    target_include_directories (tinyxml INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(tinyxml PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libtinyxml.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      ExternalProject_Add(tinyxml-src
        GIT_REPOSITORY    https://github.com/bear101/tinyxml.git
        GIT_TAG           bdr_v2
        UPDATE_DISCONNECTED ON
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/tinyxml
        CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                          -DCMAKE_BUILD_TYPE=release
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_TINYXML}
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libtinyxml.a
        )

      ExternalProject_Get_Property(tinyxml-src INSTALL_DIR)
      file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
    else()
      set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_TINYXML})
    endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

    add_library(tinyxml STATIC IMPORTED GLOBAL)
    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      add_dependencies(tinyxml tinyxml-src)
    endif()

    target_include_directories (tinyxml INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(tinyxml PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libtinyxml.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      ExternalProject_Add(tinyxml-src
        GIT_REPOSITORY    https://github.com/bear101/tinyxml.git
        GIT_TAG           bdr_v2
        UPDATE_DISCONNECTED ON
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/tinyxml
        CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                          -DCMAKE_BUILD_TYPE=release
                          -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                          -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
                          -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                          -DANDROID_ABI=${ANDROID_ABI} -DANDROID_PLATFORM=${ANDROID_PLATFORM}
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_TINYXML}
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libtinyxml.a
        )

      ExternalProject_Get_Property(tinyxml-src INSTALL_DIR)
      file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
    else()
      set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_TINYXML})
    endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

    add_library(tinyxml STATIC IMPORTED GLOBAL)
    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      add_dependencies(tinyxml tinyxml-src)
    endif()
    target_include_directories (tinyxml INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(tinyxml PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libtinyxml.a)

endif()
