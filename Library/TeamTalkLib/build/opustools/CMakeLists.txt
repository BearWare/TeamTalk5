cmake_minimum_required(VERSION 3.1)

project(OPUSTools)

include(ExternalProject)

##################################################
# OPUS Tools
##################################################
ExternalProject_Add(opustools-src
  GIT_REPOSITORY    https://github.com/bear101/opus-tools
  GIT_TAG           5aecfed
  UPDATE_COMMAND    ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  )
ExternalProject_Get_Property(opustools-src SOURCE_DIR)
file(MAKE_DIRECTORY ${SOURCE_DIR}/src)
file(TOUCH ${SOURCE_DIR}/src/opus_header.c)
file(TOUCH ${SOURCE_DIR}/src/opus_header.h)

add_library(opustools STATIC ${SOURCE_DIR}/src/opus_header.h ${SOURCE_DIR}/src/opus_header.c)
set_property(TARGET opustools PROPERTY OUTPUT_NAME_DEBUG opustoolsd)
add_dependencies(opustools opustools-src)
target_link_libraries(opustools ogg)
target_include_directories (opustools INTERFACE ${SOURCE_DIR}/src)

