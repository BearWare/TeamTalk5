project(Tolk)

include(ExternalProject)

##################################################
# TOLK
##################################################

ExternalProject_Add(tolk-src
  GIT_REPOSITORY    https://github.com/bear101/tolk
  GIT_TAG           1e538e2
  UPDATE_COMMAND    ""
  PATCH_COMMAND     ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/zdsr.h <SOURCE_DIR>/src/zdsr.h &&
                    ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ScreenReaderDriverZDSR.h <SOURCE_DIR>/src/ScreenReaderDriverZDSR.h &&
                    ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ScreenReaderDriverZDSR.cpp <SOURCE_DIR>/src/ScreenReaderDriverZDSR.cpp &&
                    git apply ${CMAKE_CURRENT_SOURCE_DIR}/0002-Add-ZDSR-support-to-tolk.patch || echo "ZDSR patch already applied or not applicable"
  CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DTOLK_WITH_JAVAJNI=OFF -DTOLK_MULTITHREADED=ON
  BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Release
  INSTALL_DIR       ${CMAKE_CURRENT_BINARY_DIR}/Tolk
  INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Release
  BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/Tolk.lib
  )
ExternalProject_Get_Property(tolk-src INSTALL_DIR)
file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

add_library(Tolk STATIC IMPORTED GLOBAL)
add_dependencies(Tolk tolk-src)
target_include_directories (Tolk INTERFACE ${INSTALL_DIR}/include)
set_target_properties(Tolk PROPERTIES
  IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/Tolk.lib
  IMPORTED_LOCATION ${INSTALL_DIR}/lib/Tolk.lib)

if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  set (TOLK_DLL_FILES ${INSTALL_DIR}/lib/nvdaControllerClient64.dll ${INSTALL_DIR}/lib/SAAPI64.dll ${INSTALL_DIR}/lib/Tolk.dll PARENT_SCOPE)
else()
  set (TOLK_DLL_FILES ${INSTALL_DIR}/lib/dolapi32.dll ${INSTALL_DIR}/lib/nvdaControllerClient32.dll ${INSTALL_DIR}/lib/SAAPI32.dll ${INSTALL_DIR}/lib/Tolk.dll PARENT_SCOPE)
endif()
