project(OPUS)

include(ExternalProject)

##################################################
# OPUS
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_OPUS ${TOOLCHAIN_INSTALL_PREFIX}/opus)

if (MSVC)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(opus-src
      GIT_REPOSITORY    https://github.com/xiph/opus.git
      GIT_TAG           v1.6.1
      PATCH_COMMAND     git reset --hard
      COMMAND           git apply ${CMAKE_CURRENT_LIST_DIR}/0001-Add-d-suffix-to-debug-build.patch
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/opus
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DOPUS_STATIC_RUNTIME=ON
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target opus --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target opus --config Release
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OPUS}
      INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target install --config Release
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/opusd.lib
                        <INSTALL_DIR>/lib/opus.lib
      )
    ExternalProject_Get_Property(opus-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OPUS})
  endif()

  add_library(opus STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(opus opus-src)
  endif()
  target_include_directories (opus INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(opus PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/opusd.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/opus.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    string (REPLACE ";" "\$<SEMICOLON>" ARCHS "${CMAKE_OSX_ARCHITECTURES}")
    ExternalProject_Add(opus-src
      GIT_REPOSITORY    https://github.com/xiph/opus.git
      GIT_TAG           v1.6.1
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/opus
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OPUS}
      CMAKE_ARGS        -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_OSX_ARCHITECTURES=${ARCHS} -DCMAKE_BUILD_TYPE=release
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target opus
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libopus.a
      )
    ExternalProject_Get_Property(opus-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OPUS})
  endif()

  add_library(opus STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(opus opus-src)
  endif()
  target_include_directories (opus INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(opus PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libopus.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  set (OPUS_CFG_FLAGS -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
                      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
                      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
                      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
                      -DCMAKE_C_FLAGS_RELEASE=-fembed-bitcode
                      -DCMAKE_BUILD_TYPE=release)

  if ("armv7" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    list (APPEND OPUS_CFG_FLAGS -DOPUS_FIXED_POINT=ON)
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(opus-src
      GIT_REPOSITORY    https://github.com/xiph/opus.git
      GIT_TAG           v1.6.1
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/opus
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OPUS}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> ${OPUS_CFG_FLAGS}
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target opus
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libopus.a
      )
    ExternalProject_Get_Property(opus-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OPUS})
  endif()

  add_library(opus STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(opus opus-src)
  endif()
  target_include_directories (opus INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(opus PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libopus.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    # Raspberry Pi
    set (OPUS_CFG_FLAGS -DCMAKE_POSITION_INDEPENDENT_CODE=ON)
    if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "armv7l")
      list (APPEND OPUS_CFG_FLAGS -DOPUS_DISABLE_INTRINSICS=ON -DOPUS_FIXED_POINT=ON)
    endif()

    ExternalProject_Add(opus-src
      GIT_REPOSITORY    https://github.com/xiph/opus.git
      GIT_TAG           v1.6.1
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/opus
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OPUS}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=release ${OPUS_CFG_FLAGS}
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target opus
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libopus.a
      )
    ExternalProject_Get_Property(opus-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OPUS})
  endif()

  add_library(opus STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(opus opus-src)
  endif()
  target_include_directories (opus INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(opus PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libopus.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  set (OPUS_CFG_FLAGS -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DANDROID_ABI=${ANDROID_ABI} -DANDROID_PLATFORM=${ANDROID_PLATFORM})

  # if arm platforms are compiled with Neon then I get the following
  # linker error: undefined reference to `celt_pitch_xcorr_neon'
  if ("armeabi-v7a" STREQUAL "${ANDROID_ABI}")
    list (APPEND OPUS_CFG_FLAGS -DOPUS_FIXED_POINT=ON -DOPUS_MAY_HAVE_NEON=OFF)
  elseif ("arm64-v8a" STREQUAL "${ANDROID_ABI}")
    list (APPEND OPUS_CFG_FLAGS -DOPUS_FIXED_POINT=ON -DOPUS_MAY_HAVE_NEON=OFF)
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(opus-src
      GIT_REPOSITORY    https://github.com/xiph/opus.git
      GIT_TAG           v1.6.1
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/opus
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OPUS}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_POSITION_INDEPENDENT_CODE=ON ${OPUS_CFG_FLAGS} -DCMAKE_BUILD_TYPE=release
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target opus
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libopus.a
      )
    ExternalProject_Get_Property(opus-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OPUS})
  endif()

  add_library(opus STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(opus opus-src)
  endif()
  target_include_directories (opus INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(opus PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libopus.a)
endif()
