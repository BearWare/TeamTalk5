cmake_minimum_required(VERSION 3.1)

project(SpeexDSP)

include(ExternalProject)

##################################################
# SpeexDSP
##################################################
if (MSVC)
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

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  ExternalProject_Add(speexdsp-src
    GIT_REPOSITORY    https://github.com/bear101/speexdsp
    GIT_TAG           SpeexDSP-1.2.0
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CONFIGURE_COMMAND <SOURCE_DIR>/autogen.sh
    COMMAND           <SOURCE_DIR>/configure CFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} CPPFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} --prefix=<INSTALL_DIR>
    BUILD_COMMAND     make -j4
    INSTALL_COMMAND   make install
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libspeexdsp.a
    )
  ExternalProject_Get_Property(speexdsp-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(speexdsp STATIC IMPORTED GLOBAL)
  target_include_directories (speexdsp INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(speexdsp PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libspeexdsp.a)
  add_dependencies(speexdsp speexdsp-src)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  if ("armv7" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (SPEEXDSP_GEN_FLAGS --host=arm-apple-darwin)
    set (SPEEXDSP_CFG_FLAGS --host=arm-apple-darwin --enable-shared=no --enable-static=yes --enable-fixed-point --enable-neon --disable-examples)
    set (SPEEXDSP_CFLAGS "-arch ${CMAKE_OSX_ARCHITECTURES} -isysroot ${CMAKE_OSX_SYSROOT} -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -fembed-bitcode")
  elseif ("arm64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (SPEEXDSP_GEN_FLAGS --host=arm-apple-darwin)
    set (SPEEXDSP_CFG_FLAGS --host=arm-apple-darwin --enable-shared=no --enable-static=yes --disable-neon --disable-examples)
    set (SPEEXDSP_CFLAGS "-arch ${CMAKE_OSX_ARCHITECTURES} -isysroot ${CMAKE_OSX_SYSROOT} -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -fembed-bitcode")
  elseif ("i386" STREQUAL "${CMAKE_OSX_ARCHITECTURES}" OR "x86_64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    set (SPEEXDSP_GEN_FLAGS --host=${CMAKE_OSX_ARCHITECTURES})
    set (SPEEXDSP_CFG_FLAGS --host=${CMAKE_OSX_ARCHITECTURES} --enable-shared=no --enable-static=yes)
    set (SPEEXDSP_CFLAGS "-arch ${CMAKE_OSX_ARCHITECTURES} -isysroot ${CMAKE_OSX_SYSROOT} -mios-simulator-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif()

  ExternalProject_Add(speexdsp-src
    GIT_REPOSITORY    https://github.com/bear101/speexdsp
    GIT_TAG           SpeexDSP-1.2.0
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CONFIGURE_COMMAND export CFLAGS=${SPEEXDSP_CFLAGS} LDFLAGS=${SPEEXDSP_CFLAGS} && <SOURCE_DIR>/autogen.sh ${SPEEXDSP_GEN_FLAGS}
    COMMAND           export CFLAGS=${SPEEXDSP_CFLAGS} LDFLAGS=${SPEEXDSP_CFLAGS} && <SOURCE_DIR>/configure ${SPEEXDSP_CFG_FLAGS} --prefix=<INSTALL_DIR>
    BUILD_COMMAND     make -j4
    INSTALL_COMMAND   make install
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libspeexdsp.a
    )
  ExternalProject_Get_Property(speexdsp-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(speexdsp STATIC IMPORTED GLOBAL)
  target_include_directories (speexdsp INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(speexdsp PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libspeexdsp.a)
  add_dependencies(speexdsp speexdsp-src)

endif()
