cmake_minimum_required(VERSION 3.1)

project(OPUS)

include(ExternalProject)

##################################################
# OPUS
##################################################
if (MSVC)
  ExternalProject_Add(opus-win
    GIT_REPOSITORY    https://github.com/bear101/opus
    GIT_TAG           6c18dfab
    UPDATE_COMMAND    ""
    BUILD_IN_SOURCE   TRUE
    CMAKE_ARGS        -DCMAKE_SYSTEM_VERSION=10.0 -DAVX_SUPPORTED=OFF
    BUILD_COMMAND     cmake --build . --target opus --config Debug
    COMMAND           cmake --build . --target opus --config Release
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(opus-win SOURCE_DIR)
  set (OPUS_DIR ${SOURCE_DIR})
  file(MAKE_DIRECTORY ${OPUS_DIR}/include)

  add_library(opus STATIC IMPORTED GLOBAL)
  target_include_directories (opus INTERFACE ${OPUS_DIR}/include)
  set_target_properties(opus PROPERTIES
    IMPORTED_LOCATION_DEBUG ${OPUS_DIR}/lib/$(PlatformName)/opusd.lib
    IMPORTED_LOCATION ${OPUS_DIR}/lib/$(PlatformName)/opus.lib)
  add_dependencies(opus opus-win)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  ExternalProject_Add(opus-src
    GIT_REPOSITORY    https://github.com/bear101/opus
    GIT_TAG           v1.3.1
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CMAKE_ARGS        -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10 -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    BUILD_COMMAND     cmake --build . --target opus
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libopus.a
    )
  ExternalProject_Get_Property(opus-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(opus STATIC IMPORTED GLOBAL)
  add_dependencies(opus opus-src)
  target_include_directories (opus INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(opus PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libopus.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  set (OPUS_CFG_FLAGS -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
                      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
                      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
                      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES})

  if ("armv7" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
    list (APPEND OPUS_CFG_FLAGS -DOPUS_FIXED_POINT=ON)
  endif()

  ExternalProject_Add(opus-src
    GIT_REPOSITORY    https://github.com/bear101/opus
    GIT_TAG           v1.3.1
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> ${OPUS_CFG_FLAGS}
    BUILD_COMMAND     cmake --build . --target opus
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libopus.a
    )
  ExternalProject_Get_Property(opus-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(opus STATIC IMPORTED GLOBAL)
  add_dependencies(opus opus-src)
  target_include_directories (opus INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(opus PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libopus.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  ExternalProject_Add(opus-src
    GIT_REPOSITORY    https://github.com/bear101/opus
    GIT_TAG           v1.3.1
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    BUILD_COMMAND     cmake --build . --target opus
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libopus.a
    )
  ExternalProject_Get_Property(opus-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(opus STATIC IMPORTED GLOBAL)
  add_dependencies(opus opus-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(opus PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libopus.a)
  else()
    target_include_directories (opus INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(opus PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libopus.a)
  endif()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  set (OPUS_CFG_FLAGS -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DANDROID_ABI=${ANDROID_ABI} -DANDROID_PLATFORM=${ANDROID_PLATFORM})

  if ("armeabi-v7a" STREQUAL "${ANDROID_ABI}")
    list (APPEND OPUS_CFG_FLAGS -DOPUS_FIXED_POINT=ON)
  elseif ("arm64-v8a" STREQUAL "${ANDROID_ABI}")
    list (APPEND OPUS_CFG_FLAGS -DOPUS_FIXED_POINT=ON)
  endif()

  ExternalProject_Add(opus-src
    GIT_REPOSITORY    https://github.com/bear101/opus
    GIT_TAG           v1.3.1
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_POSITION_INDEPENDENT_CODE=ON ${OPUS_CFG_FLAGS}
    BUILD_COMMAND     cmake --build . --target opus
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libopus.a
    )
  ExternalProject_Get_Property(opus-src INSTALL_DIR)

  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(opus STATIC IMPORTED GLOBAL)
  add_dependencies(opus opus-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(opus PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libopus.a)
  else()
    target_include_directories (opus INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(opus PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libopus.a)
  endif()
endif()
