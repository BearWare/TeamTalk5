project(ZLib)

include(ExternalProject)

##################################################
# ZLib
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_ZLIB ${TOOLCHAIN_INSTALL_PREFIX}/zlib)

if (MSVC)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)

      if (NOT CMAKE_VS_PLATFORM_NAME)
        set (CMAKE_VS_PLATFORM_NAME x64)
      endif()

      ExternalProject_Add(zlib-src
        GIT_REPOSITORY    https://github.com/bear101/zlib
        GIT_TAG           0f3868f
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/zlib
        CONFIGURE_COMMAND ""
        BUILD_IN_SOURCE   TRUE
        BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 contrib\\vstudio\\vc14\\zlibvc.sln -target:zlibstat /property:Configuration=Debug
        COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 contrib\\vstudio\\vc14\\zlibvc.sln -target:zlibstat /property:Configuration=ReleaseWithoutASM
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_ZLIB}
        INSTALL_COMMAND   cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_headers.bat <INSTALL_DIR>
        COMMAND           cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_libs.bat lib <INSTALL_DIR>
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/zlibd.lib
                          <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/zlib.lib
        )
    else()

      if (NOT CMAKE_VS_PLATFORM_NAME)
        set (CMAKE_VS_PLATFORM_NAME win32)
      endif()

      ExternalProject_Add(zlib-src
        GIT_REPOSITORY    https://github.com/bear101/zlib
        GIT_TAG           0f3868f
        UPDATE_COMMAND    ""
        PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/zlib
        CONFIGURE_COMMAND ""
        BUILD_IN_SOURCE   TRUE
        BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 contrib\\vstudio\\vc14\\zlibvc.sln -target:zlibstat /property:Configuration=Debug
        COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 contrib\\vstudio\\vc14\\zlibvc.sln -target:zlibstat /property:Configuration=ReleaseWithoutASM
        INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_ZLIB}
        INSTALL_COMMAND   cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_headers.bat <INSTALL_DIR>
        COMMAND           cd <SOURCE_DIR>
        COMMAND           ${CMAKE_CURRENT_LIST_DIR}\\..\\install_libs.bat lib <INSTALL_DIR>
        BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/zlibd.lib
                          <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/zlib.lib
        )
    endif()
    ExternalProject_Get_Property(zlib-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_ZLIB})
  endif()

  add_library(zlib STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(zlib zlib-src)
  endif()
  target_include_directories (zlib INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(zlib PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/${CMAKE_VS_PLATFORM_NAME}/zlibd.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/${CMAKE_VS_PLATFORM_NAME}/zlib.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(zlib-src
      GIT_REPOSITORY    https://github.com/madler/zlib
      GIT_TAG           v1.2.13
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/zlib
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/zlib
      CONFIGURE_COMMAND CFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
                        CPPFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
                        <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --static --archs=-arch\ arm64\ -arch\ x86_64
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libz.a
      )
    ExternalProject_Get_Property(zlib-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_ZLIB})
  endif()

  add_library(zlib STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(zlib zlib-src)
  endif()
  target_include_directories (zlib INTERFACE ${INSTALL_DIR}/include)
  set_property(TARGET zlib PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libz.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(zlib-src
      GIT_REPOSITORY    https://github.com/madler/zlib
      GIT_TAG           v1.2.13
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/zlib
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/zlib
      CONFIGURE_COMMAND CFLAGS=-fPIC <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --static
      BUILD_COMMAND     make ${TOOLCHAIN_BUILD_MAKEJOBS}
      INSTALL_COMMAND   make ${TOOLCHAIN_BUILD_MAKEJOBS} install
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libz.a
      )
    ExternalProject_Get_Property(zlib-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_ZLIB})
  endif()

  add_library(zlib STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(zlib zlib-src)
  endif()
  target_include_directories (zlib INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(zlib PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libz.a)

endif()
