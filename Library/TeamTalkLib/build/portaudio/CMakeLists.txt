cmake_minimum_required(VERSION 3.1)

project(PortAudio)

include(ExternalProject)

##################################################
# PortAudio
##################################################

option (TOOLCHAIN_BUILD_EXTERNALPROJECT_PORTAUDIO "Build PortAudio from source" ${TOOLCHAIN_BUILD_EXTERNALPROJECTS})

if (MSVC)
  ExternalProject_Add(portaudio-src
    GIT_REPOSITORY    https://github.com/bear101/portaudio
    GIT_TAG           5af7fcb
    UPDATE_COMMAND    ""
    PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/portaudio
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target portaudio_static --config Debug
    COMMAND           ${CMAKE_COMMAND} --build . --target portaudio_static --config Release
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(portaudio-src SOURCE_DIR)
  set (PORTAUDIO_DIR ${SOURCE_DIR})
  file(MAKE_DIRECTORY ${PORTAUDIO_DIR}/include)

  add_library(portaudio STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECT_PORTAUDIO)
    add_dependencies(portaudio portaudio-src)
  endif()
  target_include_directories (portaudio INTERFACE ${PORTAUDIO_DIR}/include)
  set_target_properties(portaudio PROPERTIES
    IMPORTED_LOCATION_DEBUG ${PORTAUDIO_DIR}/lib/$(PlatformName)/pastaticd.lib
    IMPORTED_LOCATION ${PORTAUDIO_DIR}/lib/$(PlatformName)/pastatic.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  ExternalProject_Add(portaudio-src
    GIT_REPOSITORY    https://github.com/bear101/portaudio
    GIT_TAG           521209c
    #    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/portaudio
    INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/portaudio
    CMAKE_ARGS        -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10 -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target portaudio_static
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libportaudio.a
    )
  ExternalProject_Get_Property(portaudio-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(portaudio STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECT_PORTAUDIO)
    add_dependencies(portaudio portaudio-src)
  endif()
  target_include_directories (portaudio INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(portaudio PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libportaudio.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  ExternalProject_Add(portaudio-src
    GIT_REPOSITORY    https://github.com/bear101/portaudio
    GIT_TAG           521209c
    #    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/portaudio
    INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/portaudio
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DPA_USE_ALSA=ON -DPA_USE_JACK=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    BUILD_COMMAND     ${CMAKE_COMMAND} --build .
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libportaudio.a
    )
  ExternalProject_Get_Property(portaudio-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(portaudio STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECT_PORTAUDIO)
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
