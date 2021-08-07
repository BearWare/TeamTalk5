cmake_minimum_required(VERSION 3.1)

project(OPUS)

include(ExternalProject)

##################################################
# OPUS
##################################################
ExternalProject_Add(opus-win
  GIT_REPOSITORY    https://github.com/bear101/opus
  GIT_TAG           6c18dfab
  UPDATE_COMMAND    ""
  BUILD_IN_SOURCE   TRUE
  CMAKE_ARGS        -DCMAKE_SYSTEM_VERSION=10.0 -DAVX_SUPPORTED=OFF
  BUILD_COMMAND     cmake --build . --target opus --config Debug
  COMMAND           cmake --build . --target opus --config Release
  INSTALL_COMMAND   ""
  )
ExternalProject_Get_Property(opus-win SOURCE_DIR)
set (OPUS_DIR ${SOURCE_DIR})
file(MAKE_DIRECTORY ${OPUS_DIR}/include)

add_library(opus STATIC IMPORTED GLOBAL)
target_include_directories (opus INTERFACE ${OPUS_DIR}/include)
set_target_properties(opus PROPERTIES
                      IMPORTED_LOCATION_DEBUG ${OPUS_DIR}/lib/$(PlatformName)/opusd.lib
                      IMPORTED_LOCATION ${OPUS_DIR}/lib/$(PlatformName)/opus.lib)
add_dependencies(opus opus-win)

