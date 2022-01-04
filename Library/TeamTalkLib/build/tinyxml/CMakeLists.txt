cmake_minimum_required(VERSION 3.1)

project(TinyXML)

include(ExternalProject)

##################################################
# TinyXML
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_TINYXML ${TOOLCHAIN_INSTALL_PREFIX}/tinyxml)

if (MSVC)

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      ExternalProject_Add(tinyxml-src
        GIT_REPOSITORY    https://github.com/bear101/tinyxml
        GIT_TAG           224ab4ea35ded8ee95f13c84c30592abf5292af8
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/tinyxml
        CONFIGURE_COMMAND ""
        BUILD_IN_SOURCE   TRUE
        BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 tinyxml.sln -target:tinyxml /property:Configuration=Debug
        COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 tinyxml.sln -target:tinyxml /property:Configuration=Release
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_TINYXML}
        INSTALL_COMMAND   cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_headers.bat <INSTALL_DIR>
        COMMAND           cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_libs.bat lib <INSTALL_DIR>
        )
    else()
      ExternalProject_Add(tinyxml-src
        GIT_REPOSITORY    https://github.com/bear101/tinyxml
        GIT_TAG           224ab4ea35ded8ee95f13c84c30592abf5292af8
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/tinyxml
        CONFIGURE_COMMAND ""
        BUILD_IN_SOURCE   TRUE
        BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 tinyxml.sln -target:tinyxml /property:Configuration=Debug
        COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 tinyxml.sln -target:tinyxml /property:Configuration=Release
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_TINYXML}
        INSTALL_COMMAND   cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_headers.bat <INSTALL_DIR>
        COMMAND           cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_libs.bat lib <INSTALL_DIR>
        )
    endif()
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
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/$(PlatformName)/tinyxmld.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/$(PlatformName)/tinyxml.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(tinyxml-src
      GIT_REPOSITORY    https://github.com/bear101/tinyxml
      GIT_TAG           bdr/mac
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/tinyxml
      CONFIGURE_COMMAND ""
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/tinyxml
      BUILD_IN_SOURCE   TRUE
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS} CPPFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}\ -arch\ x86_64\ -arch\ arm64
      INSTALL_COMMAND   mkdir -p <INSTALL_DIR>/include && mkdir -p <INSTALL_DIR>/lib
      COMMAND           cp <SOURCE_DIR>/tinyxml.h <INSTALL_DIR>/include/
      COMMAND           cp <SOURCE_DIR>/tinystr.h <INSTALL_DIR>/include/
      COMMAND           cp <SOURCE_DIR>/libTinyXML.a <INSTALL_DIR>/lib/
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libTinyXML.a
      )
    ExternalProject_Get_Property(tinyxml-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/lib)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_TINYXML})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  add_library(tinyxml STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(tinyxml tinyxml-src)
  endif()
  target_include_directories (tinyxml INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(tinyxml PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libTinyXML.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(tinyxml-src
      GIT_REPOSITORY    https://github.com/bear101/tinyxml
      GIT_TAG           bdr/deb64
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/tinyxml
      CONFIGURE_COMMAND ""
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/tinyxml
      BUILD_IN_SOURCE   TRUE
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   mkdir -p <INSTALL_DIR>/include && mkdir -p <INSTALL_DIR>/lib
      COMMAND           cp <SOURCE_DIR>/tinyxml.h <INSTALL_DIR>/include/
      COMMAND           cp <SOURCE_DIR>/tinystr.h <INSTALL_DIR>/include/
      COMMAND           cp <SOURCE_DIR>/libTinyXML.a <INSTALL_DIR>/lib/
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libTinyXML.a
      )
    ExternalProject_Get_Property(tinyxml-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/lib)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_TINYXML})
  endif(TOOLCHAIN_BUILD_EXTERNALPROJECTS)

  add_library(tinyxml STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(tinyxml tinyxml-src)
  endif()
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(tinyxml PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libTinyXML.a)
  else()
    target_include_directories (tinyxml INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(tinyxml PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libTinyXML.a)
  endif()

endif()
