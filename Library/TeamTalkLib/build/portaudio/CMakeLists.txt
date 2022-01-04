cmake_minimum_required(VERSION 3.1)

project(PortAudio)

include(ExternalProject)

##################################################
# PortAudio
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_PORTAUDIO ${TOOLCHAIN_INSTALL_PREFIX}/portaudio)

if (MSVC)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(portaudio-src
      GIT_REPOSITORY    https://github.com/bear101/portaudio
      GIT_TAG           11cbdc1
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX_PORTAUDIO}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target portaudio_static --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target portaudio_static --config Release
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_PORTAUDIO}
      INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target install --config Release
      )
    ExternalProject_Get_Property(portaudio-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_PORTAUDIO})
  endif()

  add_library(portaudio STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(portaudio portaudio-src)
  endif()
  target_include_directories (portaudio INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(portaudio PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/pastaticd.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/pastatic.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    string (REPLACE ";" "\$<SEMICOLON>" ARCHS "${CMAKE_OSX_ARCHITECTURES}")
    ExternalProject_Add(portaudio-src
      GIT_REPOSITORY    https://github.com/bear101/portaudio
      GIT_TAG           521209c
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX_PORTAUDIO}
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/portaudio
      CMAKE_ARGS        -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_OSX_ARCHITECTURES=${ARCHS} -DCMAKE_BUILD_TYPE=release
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target portaudio_static
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libportaudio.a
      )
    ExternalProject_Get_Property(portaudio-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_PORTAUDIO})
  endif()

  add_library(portaudio STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(portaudio portaudio-src)
  endif()
  target_include_directories (portaudio INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(portaudio PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libportaudio.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(portaudio-src
      GIT_REPOSITORY    https://github.com/bear101/portaudio
      GIT_TAG           521209c
      #    GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX_PORTAUDIO}
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/portaudio
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DPA_USE_ALSA=ON -DPA_USE_JACK=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_BUILD_TYPE=release
      BUILD_COMMAND     ${CMAKE_COMMAND} --build .
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libportaudio.a
      )
    ExternalProject_Get_Property(portaudio-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_PORTAUDIO})
  endif()

  add_library(portaudio STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(portaudio portaudio-src)
  endif()
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(portaudio PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libportaudio.a)
  else()
    target_include_directories (portaudio INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(portaudio PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libportaudio.a)
  endif()

endif()
