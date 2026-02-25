project(TinyXML2)

include(ExternalProject)

##################################################
# TinyXML2
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_TINYXML2 ${TOOLCHAIN_INSTALL_PREFIX}/tinyxml2)

if (MSVC)

  if (NOT CMAKE_VS_PLATFORM_NAME)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      set (CMAKE_VS_PLATFORM_NAME x64)
    else()
      set (CMAKE_VS_PLATFORM_NAME win32)
    endif()
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(tinyxml2-src
      GIT_REPOSITORY    https://github.com/leethomason/tinyxml2.git
      GIT_TAG           11.0.0
      UPDATE_COMMAND    ""
      PATCH_COMMAND     git reset --hard
      COMMAND           git apply ${CMAKE_CURRENT_LIST_DIR}/0001-Build-MT-configurations-for-Visual-Studio.patch
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/tinyxml2
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                        -DCMAKE_DEBUG_POSTFIX=d
                        -Dtinyxml2_INSTALL_PKGCONFIG=OFF
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --config Release
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_TINYXML2}
      INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target install --config Release
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/tinyxml2d.lib
                        <INSTALL_DIR>/lib/tinyxml2.lib
      )

    ExternalProject_Get_Property(tinyxml2-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_TINYXML2})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  add_library(tinyxml2 STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(tinyxml2 tinyxml2-src)
  endif()
  target_include_directories (tinyxml2 INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(tinyxml2 PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/tinyxml2d.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/tinyxml2.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      string (REPLACE ";" "\$<SEMICOLON>" ARCHS "${CMAKE_OSX_ARCHITECTURES}")
      ExternalProject_Add(tinyxml2-src
        GIT_REPOSITORY    https://github.com/leethomason/tinyxml2.git
        GIT_TAG           11.0.0
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/tinyxml2
        CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                          -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
                          -DCMAKE_OSX_ARCHITECTURES=${ARCHS}
                          -DCMAKE_BUILD_TYPE=release
                          -Dtinyxml2_INSTALL_PKGCONFIG=OFF
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_TINYXML2}
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libtinyxml2.a
        )

      ExternalProject_Get_Property(tinyxml2-src INSTALL_DIR)
      file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
    else()
      set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_TINYXML2})
    endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

    add_library(tinyxml2 STATIC IMPORTED GLOBAL)
    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      add_dependencies(tinyxml2 tinyxml2-src)
    endif()
    target_include_directories (tinyxml2 INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(tinyxml2 PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libtinyxml2.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      ExternalProject_Add(tinyxml2-src
        GIT_REPOSITORY    https://github.com/leethomason/tinyxml2.git
        GIT_TAG           11.0.0
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/tinyxml2
        CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                          -DCMAKE_BUILD_TYPE=release
                          -Dtinyxml2_INSTALL_PKGCONFIG=OFF
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_TINYXML2}
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libtinyxml2.a
        )

      ExternalProject_Get_Property(tinyxml2-src INSTALL_DIR)
      file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
    else()
      set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_TINYXML2})
    endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

    add_library(tinyxml2 STATIC IMPORTED GLOBAL)
    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      add_dependencies(tinyxml2 tinyxml2-src)
    endif()

    target_include_directories (tinyxml2 INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(tinyxml2 PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libtinyxml2.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      ExternalProject_Add(tinyxml2-src
        GIT_REPOSITORY    https://github.com/leethomason/tinyxml2.git
        GIT_TAG           11.0.0
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/tinyxml2
        CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                          -DCMAKE_BUILD_TYPE=release
                          -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                          -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
                          -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                          -DANDROID_ABI=${ANDROID_ABI} -DANDROID_PLATFORM=${ANDROID_PLATFORM}
                          -Dtinyxml2_INSTALL_PKGCONFIG=OFF
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_TINYXML2}
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libtinyxml2.a
        )

      ExternalProject_Get_Property(tinyxml2-src INSTALL_DIR)
      file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
    else()
      set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_TINYXML2})
    endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

    add_library(tinyxml2 STATIC IMPORTED GLOBAL)
    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      add_dependencies(tinyxml2 tinyxml2-src)
    endif()
    target_include_directories (tinyxml2 INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(tinyxml2 PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libtinyxml2.a)

endif()
