cmake_minimum_required(VERSION 3.1)

project(Ogg)

include(ExternalProject)

##################################################
# Ogg
##################################################
ExternalProject_Add(ogg-win
  GIT_REPOSITORY    https://github.com/bear101/ogg
  GIT_TAG           49d8bab
  UPDATE_COMMAND    ""
  BUILD_IN_SOURCE   TRUE
  CMAKE_ARGS        -DCMAKE_SYSTEM_VERSION=10.0
  BUILD_COMMAND     cmake --build . --target ogg --config Debug
  COMMAND           cmake --build . --target ogg --config Release
  INSTALL_COMMAND   ""
  )
ExternalProject_Get_Property(ogg-win SOURCE_DIR)
set (OGG_DIR ${SOURCE_DIR})
file(MAKE_DIRECTORY ${OGG_DIR}/include)

add_library(ogg STATIC IMPORTED GLOBAL)
target_include_directories (ogg INTERFACE ${OGG_DIR}/include)
set_target_properties(ogg PROPERTIES
                      IMPORTED_LOCATION_DEBUG ${OGG_DIR}/lib/$(PlatformName)/oggd.lib
                      IMPORTED_LOCATION ${OGG_DIR}/lib/$(PlatformName)/ogg.lib)
add_dependencies(ogg ogg-win)

