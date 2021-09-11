cmake_minimum_required(VERSION 3.1)

project(Qt)

include(ExternalProject)

set (QT_BUILD_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/build CACHE STRING "Specify alternative 'clone' directory to build Qt Framework dependencies. Default: CMAKE_CURRENT_BINARY_DIR/build")
set (QT_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install CACHE STRING "Install path for Qt Framework. Default: CMAKE_CURRENT_BINARY_DIR/install")

if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  ExternalProject_Add(Qt-openssl-src
    PREFIX            ${QT_BUILD_PREFIX}
    GIT_REPOSITORY    https://github.com/bear101/openssl
    GIT_TAG           2c3f861718
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND perl <SOURCE_DIR>\\Configure no-shared no-asm --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR>\\ssl VC-WIN64A
    BUILD_COMMAND     set CL=/MP
    COMMAND           nmake
    INSTALL_DIR       ${QT_INSTALL_PREFIX}/openssl
    INSTALL_COMMAND   nmake install
    )
  ExternalProject_Get_Property(Qt-openssl-src INSTALL_DIR)
  set (OPENSSL_DIR ${INSTALL_DIR})
else()
  ExternalProject_Add(Qt-openssl-src
    PREFIX            ${QT_BUILD_PREFIX}
    GIT_REPOSITORY    https://github.com/bear101/openssl
    GIT_TAG           2c3f861718
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND perl <SOURCE_DIR>\\Configure no-shared no-asm --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR>\\ssl VC-WIN32
    BUILD_COMMAND     set CL=/MP
    COMMAND           nmake
    INSTALL_DIR       ${QT_INSTALL_PREFIX}/openssl
    INSTALL_COMMAND   nmake install
    )
  ExternalProject_Get_Property(Qt-openssl-src INSTALL_DIR)
  set (OPENSSL_DIR ${INSTALL_DIR})
endif()

file(MAKE_DIRECTORY ${OPENSSL_DIR}/include)

add_library(Qt-openssl-src-crypto STATIC IMPORTED GLOBAL)
add_dependencies(Qt-openssl-src-crypto Qt-openssl-src)
target_include_directories (Qt-openssl-src-crypto INTERFACE ${OPENSSL_DIR}/include)
target_link_libraries(Qt-openssl-src-crypto INTERFACE crypt32.lib)
set_property(TARGET Qt-openssl-src-crypto PROPERTY IMPORTED_LOCATION ${OPENSSL_DIR}/lib/$(PlatformName)/lib/libcrypto.lib)

add_library(Qt-openssl-src-ssl STATIC IMPORTED GLOBAL)
add_dependencies(Qt-openssl-src-ssl Qt-openssl-src)
target_include_directories (Qt-openssl-src-ssl INTERFACE ${OPENSSL_DIR}/include)
target_link_libraries(Qt-openssl-src-ssl INTERFACE crypt32.lib)
set_property(TARGET Qt-openssl-src-ssl PROPERTY IMPORTED_LOCATION ${OPENSSL_DIR}/lib/$(PlatformName)/lib/libssl.lib)

file (TO_NATIVE_PATH ${OPENSSL_DIR} OPENSSL_DIR_NATIVE)
file (TO_NATIVE_PATH ${QT_INSTALL_PREFIX} QT_INSTALL_PREFIX_NATIVE)

ExternalProject_Add(Qt-src
  PREFIX            ${QT_BUILD_PREFIX}
  GIT_REPOSITORY    https://github.com/qt/qt5
  GIT_TAG           v5.15.1
  GIT_SUBMODULES    qtbase qttools qtmultimedia qtactiveqt qtspeech
  GIT_SHALLOW       TRUE
  UPDATE_COMMAND    ""
  PATCH_COMMAND     cd qtbase
  COMMAND           git apply ${CMAKE_CURRENT_LIST_DIR}\\qt_v5.15.1_win.patch
  COMMAND           cd ..
  CONFIGURE_COMMAND CALL <SOURCE_DIR>\\configure.bat -opensource -confirm-license -static -nomake examples -openssl-linked -I${OPENSSL_DIR_NATIVE}\\include "OPENSSL_LIBS= -L${OPENSSL_DIR_NATIVE}\\lib -lUser32 -lAdvapi32 -lGdi32 -lCrypt32 -lws2_32 -llibssl -llibcrypto" -prefix ${QT_INSTALL_PREFIX_NATIVE}
  BUILD_COMMAND     set CL=/MP
  COMMAND           CALL nmake
  INSTALL_DIR       ${QT_INSTALL_PREFIX}
  INSTALL_COMMAND   set CL=/MP
  COMMAND           CALL nmake install
  DEPENDS           Qt-openssl-src
  )
ExternalProject_Get_Property(Qt-src SOURCE_DIR)
