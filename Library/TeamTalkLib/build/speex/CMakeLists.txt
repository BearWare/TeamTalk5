cmake_minimum_required(VERSION 3.1)

project(Speex)

include(ExternalProject)

##################################################
# Speex
##################################################
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  ExternalProject_Add(speex-win64
    GIT_REPOSITORY    https://github.com/bear101/speex
    GIT_TAG           62ad6a6
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 win32\\VS2015\\libspeex\\libspeex.sln -target:libspeex /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 win32\\VS2015\\libspeex\\libspeex.sln -target:libspeex /property:Configuration=Release_SSE2
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(speex-win64 SOURCE_DIR)
  set (SPEEX_DIR ${SOURCE_DIR})
  set (SPEEX_DEPEND speex-win64)
else()
  ExternalProject_Add(speex-win32
    GIT_REPOSITORY    https://github.com/bear101/speex
    GIT_TAG           62ad6a6
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 win32\\VS2015\\libspeex\\libspeex.sln -target:libspeex /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 win32\\VS2015\\libspeex\\libspeex.sln -target:libspeex /property:Configuration=Release_SSE2
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(speex-win32 SOURCE_DIR)
  set (SPEEX_DIR ${SOURCE_DIR})
  set (SPEEX_DEPEND speex-win32)
endif()

file(MAKE_DIRECTORY ${SPEEX_DIR}/include)

add_library(speex STATIC IMPORTED GLOBAL)
target_include_directories (speex INTERFACE ${SPEEX_DIR}/include)
set_target_properties(speex PROPERTIES
                      IMPORTED_LOCATION_DEBUG ${SPEEX_DIR}/lib/$(PlatformName)/libspeexd.lib
                      IMPORTED_LOCATION ${SPEEX_DIR}/lib/$(PlatformName)/libspeex_sse2.lib)
add_dependencies(speex ${SPEEX_DEPEND})

