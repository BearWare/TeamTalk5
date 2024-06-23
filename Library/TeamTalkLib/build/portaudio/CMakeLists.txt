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
      GIT_TAG           019a097
      UPDATE_COMMAND    ""
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
      GIT_REPOSITORY    https://github.com/PortAudio/portaudio
      GIT_TAG           ba3f91f
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

  find_library (CARBON_LIBRARY Carbon)
  find_library (AUDIOUNIT_LIBRARY AudioUnit)
  find_library (COREAUDIO_LIBRARY CoreAudio)
  find_library (AUDIOTOOLBOX_LIBRARY AudioToolBox )

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
      GIT_REPOSITORY    https://github.com/PortAudio/portaudio
      GIT_TAG           ba3f91f
      UPDATE_COMMAND    ""
      # There seems to be a bug in FindOSS.cmake on Ubuntu 18, so
      # although PA_USE_OSS=OFF then PortAudio's CMake configuration
      # still generates an error:
      #    CMake Error at cmake/modules/FindOSS.cmake:54 (target_include_directories):
      #    Cannot specify include directories for imported target "OSS::oss".
      # This portaudio_oss.patch disables OSS altogether
      PATCH_COMMAND     git apply ${CMAKE_CURRENT_LIST_DIR}/portaudio_oss.patch
      COMMAND           git apply ${CMAKE_CURRENT_LIST_DIR}/0001-Fix-pulseaudioHostApi-use-after-free-in-PaPulseAudio.patch
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

  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(portaudio PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libportaudio.a)
    target_link_libraries(portaudio INTERFACE ${ASOUND_LIBRARY} ${PULSE_LIBRARY})
  else()
    target_include_directories (portaudio INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(portaudio PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libportaudio.a)
    target_link_libraries(portaudio INTERFACE ${ASOUND_LIBRARY} ${PULSE_LIBRARY})
  endif()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)

    set (PA_OBOE_CFG_FLAGS -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -DANDROID_ABI=${ANDROID_ABI} -DANDROID_PLATFORM=${ANDROID_PLATFORM}
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON )

    ExternalProject_Add(oboe-src
      GIT_REPOSITORY    https://github.com/google/oboe
      GIT_TAG           1.8.1
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/oboe
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/oboe
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> ${PA_OBOE_CFG_FLAGS}
      BUILD_COMMAND     ${CMAKE_COMMAND} --build .
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/${ANDROID_ABI}/liboboe.a
      )
    ExternalProject_Get_Property(oboe-src SOURCE_DIR)
    set (OBOE_SOURCE_DIR ${SOURCE_DIR})
    ExternalProject_Get_Property(oboe-src INSTALL_DIR)
    set (OBOE_INSTALL_DIR ${INSTALL_DIR})

    ExternalProject_Add(portaudio-src
      GIT_REPOSITORY    https://github.com/bear101/portaudio
      GIT_TAG           portaudio_oboe_squash
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/portaudio
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX}/portaudio
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DPA_USE_OBOE=ON
                        -DPA_BUILD_SHARED_LIBS=OFF
                        -DOBOE_DIRECTORY=${OBOE_SOURCE_DIR}
                        -DPA_USE_ALSA=OFF -DPA_USE_PULSEAUDIO=OFF -DPA_USE_JACK=OFF -DPA_USE_OSS=OFF
                        ${PA_OBOE_CFG_FLAGS}
      BUILD_COMMAND     ${CMAKE_COMMAND} --build .
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libportaudio.a
      DEPENDS           oboe-src
      )
    ExternalProject_Get_Property(portaudio-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_PORTAUDIO})
    set (OBOE_INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX}/oboe)
  endif()

  add_library(oboe STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(oboe oboe-src)
  endif()
  set_target_properties(oboe PROPERTIES
    IMPORTED_LOCATION ${OBOE_INSTALL_DIR}/lib/${ANDROID_ABI}/liboboe.a)
  target_link_libraries(oboe INTERFACE log OpenSLES)

  add_library(portaudio STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(portaudio portaudio-src)
  endif()
  target_include_directories (portaudio INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(portaudio PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libportaudio.a)
  target_link_libraries(portaudio INTERFACE oboe)

endif()
