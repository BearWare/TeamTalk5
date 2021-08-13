cmake_minimum_required(VERSION 3.1)

project(PortAudio)

include(ExternalProject)

##################################################
# PortAudio
##################################################
if (MSVC)
  ExternalProject_Add(portaudio-win
    GIT_REPOSITORY    https://github.com/bear101/portaudio
    GIT_TAG           5af7fcb
    UPDATE_COMMAND    ""
    BUILD_IN_SOURCE   TRUE
    CMAKE_ARGS        -DCMAKE_SYSTEM_VERSION=10.0
    BUILD_COMMAND     cmake --build . --target portaudio_static --config Debug
    COMMAND           cmake --build . --target portaudio_static --config Release
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(portaudio-win SOURCE_DIR)
  set (PORTAUDIO_DIR ${SOURCE_DIR})
  file(MAKE_DIRECTORY ${PORTAUDIO_DIR}/include)

  add_library(portaudio STATIC IMPORTED GLOBAL)
  target_include_directories (portaudio INTERFACE ${PORTAUDIO_DIR}/include)
  set_target_properties(portaudio PROPERTIES
    IMPORTED_LOCATION_DEBUG ${PORTAUDIO_DIR}/lib/$(PlatformName)/pastaticd.lib
    IMPORTED_LOCATION ${PORTAUDIO_DIR}/lib/$(PlatformName)/pastatic.lib)
  add_dependencies(portaudio portaudio-win)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  ExternalProject_Add(portaudio-src
    GIT_REPOSITORY    https://github.com/bear101/portaudio
    GIT_TAG           521209c
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CMAKE_ARGS        -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10 -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    BUILD_COMMAND     cmake --build . --target portaudio
    )
  ExternalProject_Get_Property(portaudio-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(portaudio STATIC IMPORTED GLOBAL)
  target_include_directories (portaudio INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(portaudio PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libportaudio.a)
  add_dependencies(portaudio portaudio-src)

endif()
