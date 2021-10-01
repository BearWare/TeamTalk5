cmake_minimum_required(VERSION 3.1)

project(Catch2)

include(ExternalProject)

##################################################
# Catch2
##################################################
ExternalProject_Add(catch2
  GIT_REPOSITORY    https://github.com/catchorg/Catch2
  GIT_TAG           v1.12.2
  UPDATE_COMMAND    ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  )
ExternalProject_Get_Property(catch2 SOURCE_DIR)
set (CATCH_INCLUDE_DIR ${SOURCE_DIR}/include PARENT_SCOPE)
set (CATCH_INCLUDE_DIR ${SOURCE_DIR}/include)
