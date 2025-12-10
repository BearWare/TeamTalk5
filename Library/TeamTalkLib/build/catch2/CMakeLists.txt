project(Catch2)

include(ExternalProject)

##################################################
# Catch2
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_CATCH2 ${TOOLCHAIN_INSTALL_PREFIX}/catch2)

if (MSVC)
    ExternalProject_Add(catch2-src
      GIT_REPOSITORY    https://github.com/catchorg/Catch2.git
      GIT_TAG           v3.11.0
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/catch2
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --config Release
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_CATCH2}
      INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target install --config Release
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/Catch2.lib <INSTALL_DIR>/lib/Catch2d.lib
                        <INSTALL_DIR>/lib/Catch2Main.lib <INSTALL_DIR>/lib/Catch2Maind.lib
      )
    ExternalProject_Get_Property(catch2-src INSTALL_DIR)
else()
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
endif()

