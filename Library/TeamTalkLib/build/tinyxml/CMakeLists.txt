cmake_minimum_required(VERSION 3.1)

project(TinyXML)

include(ExternalProject)

##################################################
# TinyXML
##################################################
if (MSVC)

  if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    ExternalProject_Add(tinyxml-win64
      GIT_REPOSITORY    https://github.com/bear101/tinyxml
      GIT_TAG           224ab4ea35ded8ee95f13c84c30592abf5292af8
      UPDATE_COMMAND    ""
      CONFIGURE_COMMAND ""
      BUILD_IN_SOURCE   TRUE
      BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 tinyxml.sln -target:tinyxml /property:Configuration=Debug
      COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 tinyxml.sln -target:tinyxml /property:Configuration=Release
      INSTALL_COMMAND   ""
      )
    ExternalProject_Get_Property(tinyxml-win64 SOURCE_DIR)
    set (TINYXML_DIR ${SOURCE_DIR})
    set (TINYXML_DEPEND tinyxml-win64)
  else()
    ExternalProject_Add(tinyxml-win32
      GIT_REPOSITORY    https://github.com/bear101/tinyxml
      GIT_TAG           224ab4ea35ded8ee95f13c84c30592abf5292af8
      UPDATE_COMMAND    ""
      CONFIGURE_COMMAND ""
      BUILD_IN_SOURCE   TRUE
      BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 tinyxml.sln -target:tinyxml /property:Configuration=Debug
      COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 tinyxml.sln -target:tinyxml /property:Configuration=Release
      INSTALL_COMMAND   ""
      )
    ExternalProject_Get_Property(tinyxml-win32 SOURCE_DIR)
    set (TINYXML_DIR ${SOURCE_DIR})
    set (TINYXML_DEPEND tinyxml-win32)
  endif()

  add_library(tinyxml STATIC IMPORTED GLOBAL)
  target_include_directories (tinyxml INTERFACE ${TINYXML_DIR})
  set_target_properties(tinyxml PROPERTIES
    IMPORTED_LOCATION_DEBUG ${TINYXML_DIR}/lib/$(PlatformName)/tinyxmld.lib
    IMPORTED_LOCATION ${TINYXML_DIR}/lib/$(PlatformName)/tinyxml.lib)
  add_dependencies(tinyxml ${TINYXML_DEPEND})

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  
  ExternalProject_Add(tinyxml-src
    GIT_REPOSITORY    https://github.com/bear101/tinyxml
    GIT_TAG           bdr/mac
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    INSTALL_DIR       install
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     make CPPFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
    INSTALL_COMMAND   mkdir -p <INSTALL_DIR>/include && mkdir -p <INSTALL_DIR>/lib
    COMMAND           cp <SOURCE_DIR>/tinyxml.h <INSTALL_DIR>/include/
    COMMAND           cp <SOURCE_DIR>/tinystr.h <INSTALL_DIR>/include/
    COMMAND           cp <SOURCE_DIR>/libTinyXML.a <INSTALL_DIR>/lib/
    )
  ExternalProject_Get_Property(tinyxml-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  file(MAKE_DIRECTORY ${INSTALL_DIR}/lib)
  
  add_library(tinyxml STATIC IMPORTED GLOBAL)
  target_include_directories (tinyxml INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(tinyxml PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libTinyXML.a)
  add_dependencies(tinyxml tinyxml-src)
  
endif()
