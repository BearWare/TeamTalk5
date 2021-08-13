cmake_minimum_required(VERSION 3.1)

project(LibVPX)

include(ExternalProject)

##################################################
# LibVPX
##################################################
if (MSVC)
  find_program(CYGWIN bash.exe PATHS C:/cygwin64/bin REQUIRED)
  if (CYGWIN-NOTFOUND)
    message(FATAL_ERROR, "CYGWIN bash.exe is required to build libvpx")
  else()
    message("Found CYGWIN for libvpx here: ${CYGWIN}")
  endif()

  if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    find_program(YASM_WIN64 yasm.exe PATHS C:/tt5dist/yasm/x64 REQUIRED)
    if (YASM_WIN64-NOTFOUND)
      message(FATAL_ERROR, "yasm.exe is required to build libvpx. See https://yasm.tortall.net")
    else()
      message("Found YASM for libvpx here: ${YASM_WIN64}")
    endif()

    set (CMAKE_MSVCIDE_RUN_PATH C:/tt5dist/yasm/x64)

    ExternalProject_Add(libvpx-win64
      GIT_REPOSITORY    https://github.com/bear101/libvpx
      GIT_TAG           v1.9.0
      UPDATE_COMMAND    ""
      CONFIGURE_COMMAND ""
      BUILD_COMMAND     if exist build-x64 rmdir /Q /S build-x64
      COMMAND           mkdir build-x64
      COMMAND           C:\\cygwin64\\bin\\bash.exe --login -c "cd $OLDPWD/build-x64 && ../configure --target=x86_64-win64-vs16 --disable-examples --disable-docs --enable-static-msvcrt --disable-unit-tests && make"
      INSTALL_COMMAND   ""
      BUILD_IN_SOURCE   TRUE
      )
    ExternalProject_Get_Property(libvpx-win64 SOURCE_DIR)
    set (LIBVPX_DIR ${SOURCE_DIR})
    set (LIBVPX_DEPEND libvpx-win64)
  else()
    find_program(YASM_WIN32 yasm.exe PATHS C:/tt5dist/yasm/win32 REQUIRED)
    if (YASM_WIN32-NOTFOUND)
      message(FATAL_ERROR, "yasm.exe is required to build libvpx. See https://yasm.tortall.net")
    else()
      message("Found YASM for libvpx here: ${YASM_WIN32}")
    endif()

    set (CMAKE_MSVCIDE_RUN_PATH C:/tt5dist/yasm/win32)

    ExternalProject_Add(libvpx-win32
      GIT_REPOSITORY    https://github.com/bear101/libvpx
      GIT_TAG           v1.9.0
      UPDATE_COMMAND    ""
      CONFIGURE_COMMAND ""
      BUILD_COMMAND     if exist build-win32 rmdir /Q /S build-win32
      COMMAND           mkdir build-win32
      COMMAND           C:\\cygwin64\\bin\\bash.exe --login -c "cd $OLDPWD/build-win32 && ../configure --target=x86-win32-vs16 --disable-examples --disable-docs --enable-static-msvcrt --disable-unit-tests && make"
      INSTALL_COMMAND   ""
      BUILD_IN_SOURCE   TRUE
      )
    ExternalProject_Get_Property(libvpx-win32 SOURCE_DIR)
    set (LIBVPX_DIR ${SOURCE_DIR})
    set (LIBVPX_DEPEND libvpx-win32)
  endif()

  add_library(libvpx STATIC IMPORTED GLOBAL)
  target_include_directories (libvpx INTERFACE ${LIBVPX_DIR})
  set_target_properties(libvpx PROPERTIES
    IMPORTED_LOCATION_DEBUG ${LIBVPX_DIR}/build-$(PlatformName)/Debug/vpxmtd.lib
    IMPORTED_LOCATION ${LIBVPX_DIR}/build-$(PlatformName)/Release/vpxmt.lib)
  add_dependencies(libvpx ${LIBVPX_DEPEND})

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  ExternalProject_Add(libvpx-src
    GIT_REPOSITORY    https://github.com/bear101/libvpx
    GIT_TAG           v1.9.0
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --extra-cflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
            --prefix=<INSTALL_DIR> --target=x86_64-darwin13-gcc
            --disable-examples --disable-docs --disable-unit-tests
    BUILD_COMMAND     make -j4
    INSTALL_COMMAND   make install
    )
  ExternalProject_Get_Property(libvpx-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(libvpx STATIC IMPORTED GLOBAL)
  add_dependencies(libvpx libvpx-src)
  target_include_directories (libvpx INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(libvpx PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libvpx.a)

endif()
