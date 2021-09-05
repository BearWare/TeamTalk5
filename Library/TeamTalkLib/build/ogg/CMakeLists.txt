cmake_minimum_required(VERSION 3.1)

project(Ogg)

include(ExternalProject)

##################################################
# Ogg
##################################################
if (MSVC)
  ExternalProject_Add(ogg-win
    GIT_REPOSITORY    https://github.com/bear101/ogg
    GIT_TAG           49d8bab
    UPDATE_COMMAND    ""
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target ogg --config Debug
    COMMAND           ${CMAKE_COMMAND} --build . --target ogg --config Release
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(ogg-win SOURCE_DIR)
  set (OGG_DIR ${SOURCE_DIR})
  file(MAKE_DIRECTORY ${OGG_DIR}/include)

  add_library(ogg STATIC IMPORTED GLOBAL)
  target_include_directories (ogg INTERFACE ${OGG_DIR}/include)
  set_target_properties(ogg PROPERTIES
    IMPORTED_LOCATION_DEBUG ${OGG_DIR}/lib/$(PlatformName)/oggd.lib
    IMPORTED_LOCATION ${OGG_DIR}/lib/$(PlatformName)/ogg.lib)
  add_dependencies(ogg ogg-win)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  ExternalProject_Add(ogg-src
    GIT_REPOSITORY    https://github.com/bear101/ogg
    GIT_TAG           v1.3.3
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CMAKE_ARGS        -DCMAKE_OSX_DEPLOYMENT_TARGET=10.10 -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target ogg
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libogg.a
    )
  ExternalProject_Get_Property(ogg-src INSTALL_DIR)
  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(ogg STATIC IMPORTED GLOBAL)
  target_include_directories (ogg INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ogg PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libogg.a)
  add_dependencies(ogg ogg-src)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  set (OGG_CFG_FLAGS -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
                      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
                      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
                      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES})

  ExternalProject_Add(ogg-src
    GIT_REPOSITORY    https://github.com/bear101/ogg
    GIT_TAG           v1.3.3
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> ${OGG_CFG_FLAGS}
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target ogg
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libogg.a
    )
  ExternalProject_Get_Property(ogg-src INSTALL_DIR)
  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(ogg STATIC IMPORTED GLOBAL)
  target_include_directories (ogg INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ogg PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libogg.a)
  add_dependencies(ogg ogg-src)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  ExternalProject_Add(ogg-src
    GIT_REPOSITORY    https://github.com/bear101/ogg
    GIT_TAG           v1.3.3
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target ogg
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libogg.a
    )
  ExternalProject_Get_Property(ogg-src INSTALL_DIR)
  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(ogg STATIC IMPORTED GLOBAL)
  add_dependencies(ogg ogg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ogg PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libogg.a)
  else()
    target_include_directories (ogg INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ogg PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libogg.a)
  endif()

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  set (OGG_CFG_FLAGS -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DANDROID_ABI=${ANDROID_ABI} -DANDROID_PLATFORM=${ANDROID_PLATFORM})

  ExternalProject_Add(ogg-src
    GIT_REPOSITORY    https://github.com/bear101/ogg
    GIT_TAG           v1.3.3
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    INSTALL_DIR       install
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> ${OGG_CFG_FLAGS}
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target ogg
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libogg.a
    )
  ExternalProject_Get_Property(ogg-src INSTALL_DIR)
  file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

  add_library(ogg STATIC IMPORTED GLOBAL)
  add_dependencies(ogg ogg-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set_target_properties(ogg PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libogg.a)
  else()
    target_include_directories (ogg INTERFACE ${INSTALL_DIR}/include)
    set_target_properties(ogg PROPERTIES
      IMPORTED_LOCATION ${INSTALL_DIR}/lib/libogg.a)
  endif()

  
endif()
