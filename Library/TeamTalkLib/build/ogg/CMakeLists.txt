project(Ogg)

include(ExternalProject)

##################################################
# Ogg
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_OGG ${TOOLCHAIN_INSTALL_PREFIX}/ogg)

if (MSVC)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(ogg-src
      GIT_REPOSITORY    https://github.com/xiph/ogg.git
      GIT_TAG           v1.3.3
      PATCH_COMMAND     git apply ${CMAKE_CURRENT_LIST_DIR}/0001-Add-d-suffix-to-debug-build-and-switch-to-MT.patch
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/ogg
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_POLICY_VERSION_MINIMUM=3.5
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target ogg --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target ogg --config Release
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OGG}
      INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target install --config Release
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/oggd.lib
                        <INSTALL_DIR>/lib/ogg.lib
      )
    ExternalProject_Get_Property(ogg-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OGG})
  endif()

  add_library(ogg STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ogg ogg-src)
  endif()
  target_include_directories (ogg INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ogg PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/oggd.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/ogg.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    string (REPLACE ";" "\$<SEMICOLON>" ARCHS "${CMAKE_OSX_ARCHITECTURES}")

    ExternalProject_Add(ogg-src
      GIT_REPOSITORY    https://github.com/xiph/ogg.git
      GIT_TAG           v1.3.3
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/ogg
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OGG}
      CMAKE_ARGS        -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_OSX_ARCHITECTURES=${ARCHS} -DCMAKE_BUILD_TYPE=release -DCMAKE_POLICY_VERSION_MINIMUM=3.5
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target ogg
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libogg.a
      )
    ExternalProject_Get_Property(ogg-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OGG})
  endif()


  add_library(ogg STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ogg ogg-src)
  endif()
  target_include_directories (ogg INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ogg PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libogg.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  set (OGG_CFG_FLAGS -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
                      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
                      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
                      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
                      -DCMAKE_C_FLAGS_RELEASE=-fembed-bitcode
                      -DCMAKE_BUILD_TYPE=release)

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(ogg-src
      GIT_REPOSITORY    https://github.com/xiph/ogg.git
      GIT_TAG           v1.3.3
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/ogg
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OGG}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> ${OGG_CFG_FLAGS} -DCMAKE_POLICY_VERSION_MINIMUM=3.5
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target ogg
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libogg.a
      )
    ExternalProject_Get_Property(ogg-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OGG})
  endif()

  add_library(ogg STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ogg ogg-src)
  endif()
  target_include_directories (ogg INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ogg PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libogg.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(ogg-src
      GIT_REPOSITORY    https://github.com/xiph/ogg.git
      GIT_TAG           v1.3.3
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/ogg
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OGG}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_BUILD_TYPE=release -DCMAKE_POLICY_VERSION_MINIMUM=3.5
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target ogg
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libogg.a
      )
    ExternalProject_Get_Property(ogg-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OGG})
  endif()

  add_library(ogg STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ogg ogg-src)
  endif()
  target_include_directories (ogg INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ogg PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libogg.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  set (OGG_CFG_FLAGS -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DANDROID_ABI=${ANDROID_ABI} -DANDROID_PLATFORM=${ANDROID_PLATFORM})

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(ogg-src
      GIT_REPOSITORY    https://github.com/xiph/ogg.git
      GIT_TAG           v1.3.3
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/ogg
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_OGG}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> ${OGG_CFG_FLAGS} -DCMAKE_BUILD_TYPE=release -DCMAKE_POLICY_VERSION_MINIMUM=3.5
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --target ogg
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libogg.a
      )
    ExternalProject_Get_Property(ogg-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_OGG})
  endif()

  add_library(ogg STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(ogg ogg-src)
  endif()
  target_include_directories (ogg INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(ogg PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libogg.a)

endif()
