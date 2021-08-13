cmake_minimum_required(VERSION 3.1)

project(ZLib)

include(ExternalProject)

##################################################
# ZLib
##################################################
if (MSVC)
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

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  ExternalProject_Add(zlib-src
    GIT_REPOSITORY    https://github.com/bear101/zlib
    GIT_TAG           v1.2.9
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CONFIGURE_COMMAND CFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} CPPFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --static
    BUILD_COMMAND     make
    INSTALL_COMMAND   make install
    )
  ExternalProject_Get_Property(zlib-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(zlib STATIC IMPORTED GLOBAL)
  add_dependencies(zlib zlib-src)
  target_include_directories (zlib INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET zlib PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libz.a)

endif()
