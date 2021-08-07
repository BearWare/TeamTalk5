cmake_minimum_required(VERSION 3.1)

project(ZLib)

include(ExternalProject)

##################################################
# ZLib
##################################################
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  ExternalProject_Add(zlib-win64
    GIT_REPOSITORY    https://github.com/bear101/zlib
    GIT_TAG           56c3219
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 contrib\\vstudio\\vc14\\zlibvc.sln -target:zlibstat /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 contrib\\vstudio\\vc14\\zlibvc.sln -target:zlibstat /property:Configuration=ReleaseWithoutASM
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(zlib-win64 SOURCE_DIR)
  set (ZLIB_DIR ${SOURCE_DIR})
  set (ZLIB_DEPEND zlib-win64)
else()
  ExternalProject_Add(zlib-win32
    GIT_REPOSITORY    https://github.com/bear101/zlib
    GIT_TAG           56c3219
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 contrib\\vstudio\\vc14\\zlibvc.sln -target:zlibstat /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 contrib\\vstudio\\vc14\\zlibvc.sln -target:zlibstat /property:Configuration=ReleaseWithoutASM
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(zlib-win32 SOURCE_DIR)
  set (ZLIB_DIR ${SOURCE_DIR})
  set (ZLIB_DEPEND zlib-win32)
endif()

add_library(zlib STATIC IMPORTED GLOBAL)
target_include_directories (zlib INTERFACE ${ZLIB_DIR})
set_target_properties(zlib PROPERTIES
                      IMPORTED_LOCATION_DEBUG ${ZLIB_DIR}/lib/$(PlatformName)/zlibd.lib
                      IMPORTED_LOCATION ${ZLIB_DIR}/lib/$(PlatformName)/zlib.lib)
add_dependencies(zlib ${ZLIB_DEPEND})

