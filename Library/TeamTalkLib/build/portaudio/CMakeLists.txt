cmake_minimum_required(VERSION 3.1)

project(PortAudio)

include(ExternalProject)

##################################################
# PortAudio
##################################################
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

