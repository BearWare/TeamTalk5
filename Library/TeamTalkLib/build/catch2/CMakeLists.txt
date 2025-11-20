project(Catch2)

include(ExternalProject)

##################################################
# Catch2
##################################################
ExternalProject_Add(catch2
  GIT_REPOSITORY    https://github.com/catchorg/Catch2.git
  GIT_TAG           v3.11.0
  UPDATE_COMMAND    ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  )
ExternalProject_Get_Property(catch2 SOURCE_DIR)
set (CATCH_INCLUDE_DIR ${SOURCE_DIR}/extras PARENT_SCOPE)
