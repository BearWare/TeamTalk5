project(Tolk)

include(ExternalProject)

##################################################
# TOLK
##################################################

set (TOLK_CFLAGS "/nologo /O2 /EHsc /LD /Gw /W4 /WL /MT /D_EXPORTING /DUNICODE /D_UNICODE")

ExternalProject_Add(tolk-src
  GIT_REPOSITORY    https://github.com/dkager/tolk.git
  GIT_TAG           e5149f0
  UPDATE_COMMAND    ""
  CONFIGURE_COMMAND ""
  BINARY_DIR        <SOURCE_DIR>/src
  BUILD_COMMAND     nmake OUTDIR=<INSTALL_DIR>/lib "CFLAGS=${TOLK_CFLAGS}"
  INSTALL_DIR       ${CMAKE_CURRENT_BINARY_DIR}/Tolk
  INSTALL_COMMAND   ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/include
    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/src/Tolk.h <INSTALL_DIR>/include/
  BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/Tolk.lib
  )
ExternalProject_Get_Property(tolk-src INSTALL_DIR SOURCE_DIR)
file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

# Copy screen reader DLLs after build
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  ExternalProject_Add_Step(tolk-src copy-dlls
    COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_DIR}/libs/x64/nvdaControllerClient64.dll ${INSTALL_DIR}/lib/
    COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_DIR}/libs/x64/SAAPI64.dll ${INSTALL_DIR}/lib/
    DEPENDEES install
    )
  set (TOLK_DLL_FILES ${INSTALL_DIR}/lib/nvdaControllerClient64.dll ${INSTALL_DIR}/lib/SAAPI64.dll ${INSTALL_DIR}/lib/Tolk.dll PARENT_SCOPE)
else()
  ExternalProject_Add_Step(tolk-src copy-dlls
    COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_DIR}/libs/x86/dolapi32.dll ${INSTALL_DIR}/lib/
    COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_DIR}/libs/x86/nvdaControllerClient32.dll ${INSTALL_DIR}/lib/
    COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_DIR}/libs/x86/SAAPI32.dll ${INSTALL_DIR}/lib/
    DEPENDEES install
    )
  set (TOLK_DLL_FILES ${INSTALL_DIR}/lib/dolapi32.dll ${INSTALL_DIR}/lib/nvdaControllerClient32.dll ${INSTALL_DIR}/lib/SAAPI32.dll ${INSTALL_DIR}/lib/Tolk.dll PARENT_SCOPE)
endif()

add_library(Tolk STATIC IMPORTED GLOBAL)
add_dependencies(Tolk tolk-src)
target_include_directories (Tolk INTERFACE ${INSTALL_DIR}/include)
set_target_properties(Tolk PROPERTIES
  IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/Tolk.lib
  IMPORTED_LOCATION ${INSTALL_DIR}/lib/Tolk.lib)
