cmake_minimum_required(VERSION 3.1)

project(Toolchain)

include(ExternalProject)

set (PLATFORMTOOLSET "/property:PlatformToolset=v142")

##################################################
# OpenSSL
##################################################
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  ExternalProject_Add(openssl-win64
    GIT_REPOSITORY    https://github.com/bear101/openssl
    GIT_TAG           2c3f861718
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     build_win64.bat
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(openssl-win64 SOURCE_DIR)
  set (OPENSSL_DIR ${SOURCE_DIR})
  set (OPENSSL_DEPEND openssl-win64)
else()
  ExternalProject_Add(openssl-win32
    GIT_REPOSITORY    https://github.com/bear101/openssl
    GIT_TAG           2c3f861718
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     build_win32.bat
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(openssl-win32 SOURCE_DIR)
  set (OPENSSL_DIR ${SOURCE_DIR})
  set (OPENSSL_DEPEND openssl-win32)
endif()

file(MAKE_DIRECTORY ${OPENSSL_DIR}/include)

add_library(openssl-crypto STATIC IMPORTED GLOBAL)
target_include_directories (openssl-crypto INTERFACE ${OPENSSL_DIR}/include)
target_link_libraries(openssl-crypto INTERFACE crypt32.lib)
set_property(TARGET openssl-crypto PROPERTY IMPORTED_LOCATION ${OPENSSL_DIR}/lib/$(PlatformName)/lib/libcrypto.lib)
add_dependencies(openssl-crypto ${OPENSSL_DEPEND})

add_library(openssl-ssl STATIC IMPORTED GLOBAL)
target_include_directories (openssl-ssl INTERFACE ${OPENSSL_DIR}/include)
target_link_libraries(openssl-ssl INTERFACE crypt32.lib)
set_property(TARGET openssl-ssl PROPERTY IMPORTED_LOCATION ${OPENSSL_DIR}/lib/$(PlatformName)/lib/libssl.lib)
add_dependencies(openssl-ssl ${OPENSSL_DEPEND})

##################################################
# ACE
##################################################
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  ExternalProject_Add(ACE-win64
    GIT_REPOSITORY    https://github.com/bear101/ACE
    GIT_TAG           4933f7d4b92
    GIT_SUBMODULES_RECURSE TRUE
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     cd ACE
    COMMAND           set SSL_ROOT=${OPENSSL_DIR}
    COMMAND           build.bat
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 ace\\ace.sln -target:ACE /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 ace\\ace.sln -target:ACE /property:Configuration=Release
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 ace\\ace.sln -target:SSL /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 ace\\ace.sln -target:SSL /property:Configuration=Release
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 protocols\\protocols.sln -target:INet /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 protocols\\protocols.sln -target:INet /property:Configuration=Release
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 protocols\\protocols.sln -target:INet_SSL /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 protocols\\protocols.sln -target:INet_SSL /property:Configuration=Release
    INSTALL_COMMAND   ""
    DEPENDS           openssl-win64
    )
  ExternalProject_Get_Property(ACE-win64 SOURCE_DIR)
  set (ACE_DIR ${SOURCE_DIR}/ACE)
  set (ACE_DEPEND ACE-win64)
else()
  ExternalProject_Add(ACE-win32
    GIT_REPOSITORY    https://github.com/bear101/ACE
    GIT_TAG           4933f7d4b92
    GIT_SUBMODULES_RECURSE TRUE
    GIT_SHALLOW       TRUE
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     cd ACE
    COMMAND           set SSL_ROOT=${OPENSSL_DIR}
    COMMAND           build.bat
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 ace\\ace.sln -target:ACE /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 ace\\ace.sln -target:ACE /property:Configuration=Release
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 ace\\ace.sln -target:SSL /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 ace\\ace.sln -target:SSL /property:Configuration=Release
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 protocols\\protocols.sln -target:INet /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 protocols\\protocols.sln -target:INet /property:Configuration=Release
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 protocols\\protocols.sln -target:INet_SSL /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 protocols\\protocols.sln -target:INet_SSL /property:Configuration=Release
    INSTALL_COMMAND   ""
    DEPENDS           openssl-win32
    )
  ExternalProject_Get_Property(ACE-win32 SOURCE_DIR)
  set (ACE_DIR ${SOURCE_DIR}/ACE)
  set (ACE_DEPEND ACE-win32)
endif()

# Autogenerated paths must be created prior for
# INTERFACE_INCLUDE_DIRECTORIES to work (target_include_directories ... INTERFACE ...)
file(MAKE_DIRECTORY ${ACE_DIR})
file(MAKE_DIRECTORY ${ACE_DIR}/protocols)

add_library(ace STATIC IMPORTED GLOBAL)
add_dependencies(ace ${ACE_DEPEND})
target_include_directories (ace INTERFACE ${ACE_DIR})
set_target_properties(ace PROPERTIES
                      IMPORTED_LOCATION_DEBUG ${ACE_DIR}/lib/$(PlatformName)/ACEsd.lib
                      IMPORTED_LOCATION ${ACE_DIR}/lib/$(PlatformName)/ACEs.lib)

add_library(ace_ssl STATIC IMPORTED GLOBAL)
target_include_directories (ace_ssl INTERFACE ${ACE_DIR})
set_target_properties(ace_ssl PROPERTIES
                      IMPORTED_LOCATION_DEBUG ${ACE_DIR}/lib/$(PlatformName)/ACE_SSLsd.lib
                      IMPORTED_LOCATION ${ACE_DIR}/lib/$(PlatformName)/ACE_SSLs.lib)
add_dependencies(ace_ssl ace openssl-crypto openssl-ssl)

add_library(ace_inet STATIC IMPORTED GLOBAL)
add_dependencies(ace_inet ace)
target_include_directories (ace_inet INTERFACE ${ACE_DIR}/protocols)
set_target_properties(ace_inet PROPERTIES
                      IMPORTED_LOCATION_DEBUG ${ACE_DIR}/lib/$(PlatformName)/ACE_INetsd.lib
                      IMPORTED_LOCATION ${ACE_DIR}/lib/$(PlatformName)/ACE_INets.lib)

add_library(ace_inet_ssl STATIC IMPORTED GLOBAL)
add_dependencies(ace_inet_ssl ace_inet ace_ssl)
target_include_directories (ace_inet_ssl INTERFACE ${ACE_DIR}/protocols)
set_target_properties(ace_inet_ssl PROPERTIES
                      IMPORTED_LOCATION_DEBUG ${ACE_DIR}/lib/$(PlatformName)/ACE_INet_SSLsd.lib
                      IMPORTED_LOCATION ${ACE_DIR}/lib/$(PlatformName)/ACE_INet_SSLs.lib)

##################################################
# TinyXML
##################################################
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  ExternalProject_Add(tinyxml-win64
    GIT_REPOSITORY    https://github.com/bear101/tinyxml
    GIT_TAG           224ab4ea35ded8ee95f13c84c30592abf5292af8
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 tinyxml.sln -target:tinyxml /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 tinyxml.sln -target:tinyxml /property:Configuration=Release
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(tinyxml-win64 SOURCE_DIR)
  set (TINYXML_DIR ${SOURCE_DIR})
  set (TINYXML_DEPEND tinyxml-win64)
else()
  ExternalProject_Add(tinyxml-win32
    GIT_REPOSITORY    https://github.com/bear101/tinyxml
    GIT_TAG           224ab4ea35ded8ee95f13c84c30592abf5292af8
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 tinyxml.sln -target:tinyxml /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 tinyxml.sln -target:tinyxml /property:Configuration=Release
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(tinyxml-win32 SOURCE_DIR)
  set (TINYXML_DIR ${SOURCE_DIR})
  set (TINYXML_DEPEND tinyxml-win32)
endif()

add_library(tinyxml STATIC IMPORTED GLOBAL)
target_include_directories (tinyxml INTERFACE ${TINYXML_DIR})
set_target_properties(tinyxml PROPERTIES
                      IMPORTED_LOCATION_DEBUG ${TINYXML_DIR}/lib/$(PlatformName)/tinyxmld.lib
                      IMPORTED_LOCATION ${TINYXML_DIR}/lib/$(PlatformName)/tinyxml.lib)
add_dependencies(tinyxml ${TINYXML_DEPEND})

##################################################
# ZLib
##################################################
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  ExternalProject_Add(zlib-win64
    GIT_REPOSITORY    https://github.com/bear101/zlib
    GIT_TAG           56c3219
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 contrib\\vstudio\\vc14\\zlibvc.sln -target:zlibstat /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=x64 contrib\\vstudio\\vc14\\zlibvc.sln -target:zlibstat /property:Configuration=ReleaseWithoutASM
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(zlib-win64 SOURCE_DIR)
  set (ZLIB_DIR ${SOURCE_DIR})
  set (ZLIB_DEPEND zlib-win64)
else()
  ExternalProject_Add(zlib-win32
    GIT_REPOSITORY    https://github.com/bear101/zlib
    GIT_TAG           56c3219
    UPDATE_COMMAND    ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE   TRUE
    BUILD_COMMAND     msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 contrib\\vstudio\\vc14\\zlibvc.sln -target:zlibstat /property:Configuration=Debug
    COMMAND           msbuild -maxCpuCount ${PLATFORMTOOLSET} /p:Platform=win32 contrib\\vstudio\\vc14\\zlibvc.sln -target:zlibstat /property:Configuration=ReleaseWithoutASM
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(zlib-win32 SOURCE_DIR)
  set (ZLIB_DIR ${SOURCE_DIR})
  set (ZLIB_DEPEND zlib-win32)
endif()

add_library(zlib STATIC IMPORTED GLOBAL)
target_include_directories (zlib INTERFACE ${ZLIB_DIR})
set_target_properties(zlib PROPERTIES
                      IMPORTED_LOCATION_DEBUG ${ZLIB_DIR}/lib/$(PlatformName)/zlibd.lib
                      IMPORTED_LOCATION ${ZLIB_DIR}/lib/$(PlatformName)/zlib.lib)
add_dependencies(zlib ${ZLIB_DEPEND})


##################################################
# OPUS
##################################################
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
