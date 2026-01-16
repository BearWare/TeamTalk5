project(PortAudio)

include(ExternalProject)

##################################################
# PortAudio
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_PORTAUDIO ${TOOLCHAIN_INSTALL_PREFIX}/portaudio)

if (MSVC)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(portaudio-src
      GIT_REPOSITORY    https://github.com/PortAudio/portaudio.git
      GIT_TAG           213c048
      UPDATE_COMMAND    ""
      PATCH_COMMAND     git reset --hard
      COMMAND           git apply ${CMAKE_CURRENT_LIST_DIR}/0001-Add-uniqueID-to-PaDeviceInfo-that-identifies-sound-d.patch
      COMMAND           git apply ${CMAKE_CURRENT_LIST_DIR}/0002-Allow-MT-option-for-static-library-and-append-d-to-d.patch
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/portaudio
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DPA_BUILD_SHARED_LIBS=OFF -DPA_USE_WDMKS=OFF -DPA_USE_WDMKS_DEVICE_INFO=OFF
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --config Release
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_PORTAUDIO}
      INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target install --config Release
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/portaudiod.lib
                        <INSTALL_DIR>/lib/${CMAKE_VS_PLATFORM_NAME}/portaudio.lib
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
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/portaudiod.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/portaudio.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    string (REPLACE ";" "\$<SEMICOLON>" ARCHS "${CMAKE_OSX_ARCHITECTURES}")
    ExternalProject_Add(portaudio-src
      GIT_REPOSITORY    https://github.com/PortAudio/portaudio.git
      GIT_TAG           213c048
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/portaudio
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/portaudio
      CMAKE_ARGS        -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_OSX_ARCHITECTURES=${ARCHS} -DCMAKE_BUILD_TYPE=release -DPA_BUILD_SHARED_LIBS=OFF
      BUILD_COMMAND     ${CMAKE_COMMAND} --build .
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libportaudio.a
      )
    ExternalProject_Get_Property(portaudio-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_PORTAUDIO})
  endif()

  find_library (AUDIOTOOLBOX_LIBRARY AudioToolBox)
  find_library (AUDIOUNIT_LIBRARY AudioUnit)
  find_library (CARBON_LIBRARY Carbon)
  find_library (COREAUDIO_LIBRARY CoreAudio)

  add_library(portaudio STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(portaudio portaudio-src)
  endif()
  target_include_directories (portaudio INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(portaudio PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libportaudio.a)
  target_link_libraries(portaudio INTERFACE ${CARBON_LIBRARY} ${AUDIOUNIT_LIBRARY} ${COREAUDIO_LIBRARY} ${AUDIOTOOLBOX_LIBRARY})

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(portaudio-src
      GIT_REPOSITORY    https://github.com/PortAudio/portaudio.git
      GIT_TAG           213c048
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/portaudio
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/portaudio
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DPA_USE_ALSA=ON -DPA_USE_PULSEAUDIO=ON -DPA_USE_JACK=OFF -DPA_USE_OSS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DPA_BUILD_SHARED_LIBS=OFF
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

  # Ubuntu: libasound2-dev
  find_library(ASOUND_LIBRARY asound)
  # Ubuntu: libpulse-dev
  find_library(PULSE_LIBRARY pulse)

  target_include_directories (portaudio INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(portaudio PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libportaudio.a)
  target_link_libraries(portaudio INTERFACE ${ASOUND_LIBRARY} ${PULSE_LIBRARY})

endif()
