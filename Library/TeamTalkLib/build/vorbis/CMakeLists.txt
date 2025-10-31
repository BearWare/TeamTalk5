project(Vorbis)

include(ExternalProject)

##################################################
# Vorbis (Windows only)
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_VORBIS ${TOOLCHAIN_INSTALL_PREFIX}/vorbis)

if (MSVC)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(vorbis-src
      GIT_REPOSITORY    https://github.com/xiph/vorbis.git
      GIT_TAG           v1.3.7
      PATCH_COMMAND     git apply ${CMAKE_CURRENT_LIST_DIR}/0001-Update-cmake_minimum_required-to-3.10.patch
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/vorbis
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DOGG_INCLUDE_DIR=${TOOLCHAIN_INSTALL_PREFIX}/ogg/include -DOGG_LIBRARY=$<IF:$<CONFIG:Debug>,${TOOLCHAIN_INSTALL_PREFIX}/ogg/lib/oggd.lib,${TOOLCHAIN_INSTALL_PREFIX}/ogg/lib/ogg.lib>
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target vorbis --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target vorbisenc --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target vorbisfile --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target vorbis --config Release
      COMMAND           ${CMAKE_COMMAND} --build . --target vorbisenc --config Release
      COMMAND           ${CMAKE_COMMAND} --build . --target vorbisfile --config Release
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_VORBIS}
      INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target install --config Release
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/vorbis.lib
                        <INSTALL_DIR>/lib/vorbisenc.lib
                        <INSTALL_DIR>/lib/vorbisfile.lib
      DEPENDS           ogg
      )
    ExternalProject_Get_Property(vorbis-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_VORBIS})
  endif()

  add_library(vorbis STATIC IMPORTED GLOBAL)
  add_library(vorbisenc STATIC IMPORTED GLOBAL)
  add_library(vorbisfile STATIC IMPORTED GLOBAL)

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(vorbis vorbis-src)
    add_dependencies(vorbisenc vorbis-src)
    add_dependencies(vorbisfile vorbis-src)
  endif()

  target_include_directories (vorbis INTERFACE ${INSTALL_DIR}/include)
  target_include_directories (vorbisenc INTERFACE ${INSTALL_DIR}/include)
  target_include_directories (vorbisfile INTERFACE ${INSTALL_DIR}/include)

  set_target_properties(vorbis PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/vorbis.lib)
  set_target_properties(vorbisenc PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/vorbisenc.lib)
  set_target_properties(vorbisfile PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/vorbisfile.lib)

endif()
