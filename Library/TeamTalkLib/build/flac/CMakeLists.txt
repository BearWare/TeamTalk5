project(FLAC)

include(ExternalProject)

##################################################
# FLAC
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_FLAC ${TOOLCHAIN_INSTALL_PREFIX}/flac)

if (MSVC)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(flac-src
      GIT_REPOSITORY    https://github.com/xiph/flac.git
      GIT_TAG           1.5.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/flac
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_FLAC}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                        -DCMAKE_PROJECT_INCLUDE=${CMAKE_CURRENT_LIST_DIR}/msvc-static-runtime.cmake
                        -DBUILD_CXXLIBS=OFF -DBUILD_PROGRAMS=OFF -DBUILD_EXAMPLES=OFF
                        -DBUILD_TESTING=OFF -DBUILD_DOCS=OFF -DINSTALL_MANPAGES=OFF
                        -DINSTALL_PKGCONFIG_MODULES=OFF -DINSTALL_CMAKE_CONFIG_MODULE=OFF
                        -DWITH_OGG=OFF -DENABLE_MULTITHREADING=OFF -DBUILD_SHARED_LIBS=OFF
                        -DCMAKE_INSTALL_LIBDIR=lib
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target FLAC --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target FLAC --config Release
      INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target install --config Release
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/FLACd.lib
                        <INSTALL_DIR>/lib/FLAC.lib
      )
    ExternalProject_Get_Property(flac-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_FLAC})
  endif()

  add_library(flac STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(flac flac-src)
  endif()
  target_include_directories (flac INTERFACE ${INSTALL_DIR}/include)
  target_compile_definitions (flac INTERFACE FLAC__NO_DLL)
  set_target_properties(flac PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/FLACd.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/FLAC.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  string (REPLACE ";" "\$<SEMICOLON>" ARCHS "${CMAKE_OSX_ARCHITECTURES}")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(flac-src
      GIT_REPOSITORY    https://github.com/xiph/flac.git
      GIT_TAG           1.5.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/flac
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_FLAC}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_OSX_ARCHITECTURES=${ARCHS} -DCMAKE_BUILD_TYPE=release
                        -DBUILD_CXXLIBS=OFF -DBUILD_PROGRAMS=OFF -DBUILD_EXAMPLES=OFF
                        -DBUILD_TESTING=OFF -DBUILD_DOCS=OFF -DINSTALL_MANPAGES=OFF
                        -DINSTALL_PKGCONFIG_MODULES=OFF -DINSTALL_CMAKE_CONFIG_MODULE=OFF
                        -DWITH_OGG=OFF -DENABLE_MULTITHREADING=OFF -DBUILD_SHARED_LIBS=OFF
                        -DCMAKE_INSTALL_LIBDIR=lib
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target FLAC
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libFLAC.a
      )
    ExternalProject_Get_Property(flac-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_FLAC})
  endif()

  add_library(flac STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(flac flac-src)
  endif()
  target_include_directories (flac INTERFACE ${INSTALL_DIR}/include)
  target_compile_definitions (flac INTERFACE FLAC__NO_DLL)
  set_target_properties(flac PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libFLAC.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  set (FLAC_CFG_FLAGS -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
                      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
                      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
                      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
                      "-DCMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG -fembed-bitcode"
                      -DCMAKE_BUILD_TYPE=release)

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(flac-src
      GIT_REPOSITORY    https://github.com/xiph/flac.git
      GIT_TAG           1.5.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/flac
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_FLAC}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> ${FLAC_CFG_FLAGS}
                        -DBUILD_CXXLIBS=OFF -DBUILD_PROGRAMS=OFF -DBUILD_EXAMPLES=OFF
                        -DBUILD_TESTING=OFF -DBUILD_DOCS=OFF -DINSTALL_MANPAGES=OFF
                        -DINSTALL_PKGCONFIG_MODULES=OFF -DINSTALL_CMAKE_CONFIG_MODULE=OFF
                        -DWITH_OGG=OFF -DENABLE_MULTITHREADING=OFF -DBUILD_SHARED_LIBS=OFF
                        -DCMAKE_INSTALL_LIBDIR=lib
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target FLAC
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libFLAC.a
      )
    ExternalProject_Get_Property(flac-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_FLAC})
  endif()

  add_library(flac STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(flac flac-src)
  endif()
  target_include_directories (flac INTERFACE ${INSTALL_DIR}/include)
  target_compile_definitions (flac INTERFACE FLAC__NO_DLL)
  set_target_properties(flac PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libFLAC.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(flac-src
      GIT_REPOSITORY    https://github.com/xiph/flac.git
      GIT_TAG           1.5.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/flac
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_FLAC}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=release -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                        -DBUILD_CXXLIBS=OFF -DBUILD_PROGRAMS=OFF -DBUILD_EXAMPLES=OFF
                        -DBUILD_TESTING=OFF -DBUILD_DOCS=OFF -DINSTALL_MANPAGES=OFF
                        -DINSTALL_PKGCONFIG_MODULES=OFF -DINSTALL_CMAKE_CONFIG_MODULE=OFF
                        -DWITH_OGG=OFF -DENABLE_MULTITHREADING=OFF -DBUILD_SHARED_LIBS=OFF
                        -DCMAKE_INSTALL_LIBDIR=lib
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target FLAC
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libFLAC.a
      )
    ExternalProject_Get_Property(flac-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_FLAC})
  endif()

  add_library(flac STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(flac flac-src)
  endif()
  target_include_directories (flac INTERFACE ${INSTALL_DIR}/include)
  target_compile_definitions (flac INTERFACE FLAC__NO_DLL)
  set_target_properties(flac PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libFLAC.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  set (FLAC_CFG_FLAGS -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DANDROID_ABI=${ANDROID_ABI} -DANDROID_PLATFORM=${ANDROID_PLATFORM})

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(flac-src
      GIT_REPOSITORY    https://github.com/xiph/flac.git
      GIT_TAG           1.5.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/flac
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_FLAC}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_POSITION_INDEPENDENT_CODE=ON ${FLAC_CFG_FLAGS} -DCMAKE_BUILD_TYPE=release
                        -DBUILD_CXXLIBS=OFF -DBUILD_PROGRAMS=OFF -DBUILD_EXAMPLES=OFF
                        -DBUILD_TESTING=OFF -DBUILD_DOCS=OFF -DINSTALL_MANPAGES=OFF
                        -DINSTALL_PKGCONFIG_MODULES=OFF -DINSTALL_CMAKE_CONFIG_MODULE=OFF
                        -DWITH_OGG=OFF -DENABLE_MULTITHREADING=OFF -DBUILD_SHARED_LIBS=OFF
                        -DCMAKE_INSTALL_LIBDIR=lib
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target FLAC
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libFLAC.a
      )
    ExternalProject_Get_Property(flac-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_FLAC})
  endif()

  add_library(flac STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(flac flac-src)
  endif()
  target_include_directories (flac INTERFACE ${INSTALL_DIR}/include)
  target_compile_definitions (flac INTERFACE FLAC__NO_DLL)
  set_target_properties(flac PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libFLAC.a)
endif()
