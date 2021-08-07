cmake_minimum_required(VERSION 3.1)

project(SpeexDSP)

include(ExternalProject)

##################################################
# SpeexDSP
##################################################
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  ExternalProject_Add(speexdsp-win64
    GIT_REPOSITORY    https://github.com/bear101/speexdsp
    GIT_TAG           2907b00
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 win32\\VS2015\\libspeexdsp.sln -target:libspeexdsp /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 win32\\VS2015\\libspeexdsp.sln -target:libspeexdsp /property:Configuration=Release_SSE
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(speexdsp-win64 SOURCE_DIR)
  set (SPEEXDSP_DIR ${SOURCE_DIR})
  set (SPEEXDSP_DEPEND speexdsp-win64)
else()
  ExternalProject_Add(speexdsp-win32
    GIT_REPOSITORY    https://github.com/bear101/speexdsp
    GIT_TAG           2907b00
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 win32\\VS2015\\libspeexdsp.sln -target:libspeexdsp /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 win32\\VS2015\\libspeexdsp.sln -target:libspeexdsp /property:Configuration=Release_SSE
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(speexdsp-win32 SOURCE_DIR)
  set (SPEEXDSP_DIR ${SOURCE_DIR})
  set (SPEEXDSP_DEPEND speexdsp-win32)
endif()

file(MAKE_DIRECTORY ${SPEEXDSP_DIR}/include)

add_library(speexdsp STATIC IMPORTED GLOBAL)
target_include_directories (speexdsp INTERFACE ${SPEEXDSP_DIR}/include)
set_target_properties(speexdsp PROPERTIES
                      IMPORTED_LOCATION_DEBUG ${SPEEXDSP_DIR}/lib/$(PlatformName)/libspeexdspd.lib
                      IMPORTED_LOCATION ${SPEEXDSP_DIR}/lib/$(PlatformName)/libspeexdsp_sse.lib)
add_dependencies(speexdsp ${SPEEXDSP_DEPEND})

