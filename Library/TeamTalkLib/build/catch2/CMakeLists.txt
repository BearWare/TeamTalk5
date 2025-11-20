project(Catch2)

include(ExternalProject)

##################################################
# Catch2
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_CATCH2 ${TOOLCHAIN_INSTALL_PREFIX}/catch2)

if (MSVC)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(catch2-src
      GIT_REPOSITORY    https://github.com/catchorg/Catch2.git
      GIT_TAG           v3.11.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/catch2
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>
                        -DBUILD_TESTING=OFF
                        -DCATCH_INSTALL_DOCS=OFF
      BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --config Release
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_CATCH2}
      INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Debug
      COMMAND           ${CMAKE_COMMAND} --build . --target install --config Release
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/Catch2.lib <INSTALL_DIR>/lib/Catch2d.lib
                        <INSTALL_DIR>/lib/Catch2Main.lib <INSTALL_DIR>/lib/Catch2Maind.lib
      )
    ExternalProject_Get_Property(catch2-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_CATCH2})
  endif()

  add_library(catch2 STATIC IMPORTED GLOBAL)
  add_library(catch2main STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(catch2 catch2-src)
    add_dependencies(catch2main catch2-src)
  endif()
  target_include_directories (catch2 INTERFACE ${INSTALL_DIR}/include)
  target_include_directories (catch2main INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(catch2 PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/Catch2d.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/Catch2.lib)
  set_target_properties(catch2main PROPERTIES
    IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/Catch2Maind.lib
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/Catch2Main.lib)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    string (REPLACE ";" "\$<SEMICOLON>" ARCHS "${CMAKE_OSX_ARCHITECTURES}")

    ExternalProject_Add(catch2-src
      GIT_REPOSITORY    https://github.com/catchorg/Catch2.git
      GIT_TAG           v3.11.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/catch2
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_CATCH2}
      CMAKE_ARGS        -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
                        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                        -DCMAKE_OSX_ARCHITECTURES=${ARCHS}
                        -DCMAKE_BUILD_TYPE=release
                        -DBUILD_TESTING=OFF
                        -DCATCH_INSTALL_DOCS=OFF
      BUILD_COMMAND     ${CMAKE_COMMAND} --build .
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libCatch2.a <INSTALL_DIR>/lib/libCatch2Main.a
      )
    ExternalProject_Get_Property(catch2-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_CATCH2})
  endif()

  add_library(catch2 STATIC IMPORTED GLOBAL)
  add_library(catch2main STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(catch2 catch2-src)
    add_dependencies(catch2main catch2-src)
  endif()
  target_include_directories (catch2 INTERFACE ${INSTALL_DIR}/include)
  target_include_directories (catch2main INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(catch2 PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libCatch2.a)
  set_target_properties(catch2main PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libCatch2Main.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  set (CATCH2_CFG_FLAGS -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
                        -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
                        -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
                        -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
                        -DCMAKE_C_FLAGS_RELEASE=-fembed-bitcode
                        -DCMAKE_CXX_FLAGS_RELEASE=-fembed-bitcode
                        -DCMAKE_BUILD_TYPE=release)

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(catch2-src
      GIT_REPOSITORY    https://github.com/catchorg/Catch2.git
      GIT_TAG           v3.11.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/catch2
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_CATCH2}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                        ${CATCH2_CFG_FLAGS}
                        -DBUILD_TESTING=OFF
                        -DCATCH_INSTALL_DOCS=OFF
      BUILD_COMMAND     ${CMAKE_COMMAND} --build .
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libCatch2.a <INSTALL_DIR>/lib/libCatch2Main.a
      )
    ExternalProject_Get_Property(catch2-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_CATCH2})
  endif()

  add_library(catch2 STATIC IMPORTED GLOBAL)
  add_library(catch2main STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(catch2 catch2-src)
    add_dependencies(catch2main catch2-src)
  endif()
  target_include_directories (catch2 INTERFACE ${INSTALL_DIR}/include)
  target_include_directories (catch2main INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(catch2 PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libCatch2.a)
  set_target_properties(catch2main PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libCatch2Main.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(catch2-src
      GIT_REPOSITORY    https://github.com/catchorg/Catch2.git
      GIT_TAG           v3.11.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/catch2
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_CATCH2}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                        -DCMAKE_BUILD_TYPE=release
                        -DBUILD_TESTING=OFF
                        -DCATCH_INSTALL_DOCS=OFF
      BUILD_COMMAND     ${CMAKE_COMMAND} --build .
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libCatch2.a <INSTALL_DIR>/lib/libCatch2Main.a
      )
    ExternalProject_Get_Property(catch2-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_CATCH2})
  endif()

  add_library(catch2 STATIC IMPORTED GLOBAL)
  add_library(catch2main STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(catch2 catch2-src)
    add_dependencies(catch2main catch2-src)
  endif()
  target_include_directories (catch2 INTERFACE ${INSTALL_DIR}/include)
  target_include_directories (catch2main INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(catch2 PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libCatch2.a)
  set_target_properties(catch2main PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libCatch2Main.a)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  set (CATCH2_CFG_FLAGS -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DANDROID_ABI=${ANDROID_ABI} -DANDROID_PLATFORM=${ANDROID_PLATFORM})

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    ExternalProject_Add(catch2-src
      GIT_REPOSITORY    https://github.com/catchorg/Catch2.git
      GIT_TAG           v3.11.0
      GIT_SHALLOW       TRUE
      UPDATE_COMMAND    ""
      PREFIX            ${TOOLCHAIN_BUILD_PREFIX}/catch2
      INSTALL_DIR       ${TOOLCHAIN_INSTALL_PREFIX_CATCH2}
      CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                        ${CATCH2_CFG_FLAGS}
                        -DCMAKE_BUILD_TYPE=release
                        -DBUILD_TESTING=OFF
                        -DCATCH_INSTALL_DOCS=OFF
      BUILD_COMMAND     ${CMAKE_COMMAND} --build .
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/libCatch2.a <INSTALL_DIR>/lib/libCatch2Main.a
      )
    ExternalProject_Get_Property(catch2-src INSTALL_DIR)
    file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
  else()
    set (INSTALL_DIR ${TOOLCHAIN_INSTALL_PREFIX_CATCH2})
  endif()

  add_library(catch2 STATIC IMPORTED GLOBAL)
  add_library(catch2main STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(catch2 catch2-src)
    add_dependencies(catch2main catch2-src)
  endif()
  target_include_directories (catch2 INTERFACE ${INSTALL_DIR}/include)
  target_include_directories (catch2main INTERFACE ${INSTALL_DIR}/include)
  set_target_properties(catch2 PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libCatch2.a)
  set_target_properties(catch2main PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/lib/libCatch2Main.a)

endif()
