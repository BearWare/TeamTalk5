project(Prism)

include(ExternalProject)

##################################################
# Prism
##################################################

if (MSVC)

  set (PRISM_INSTALL_DIR_MD ${CMAKE_CURRENT_BINARY_DIR}/prism-md)
  set (PRISM_INSTALL_DIR_MT ${CMAKE_CURRENT_BINARY_DIR}/prism-mt)

  set (PRISM_STATIC_LIB_MD ${PRISM_INSTALL_DIR_MD}/lib/prism.lib)
  set (PRISM_STATIC_LIB_MT ${PRISM_INSTALL_DIR_MT}/lib/prism.lib)

  ExternalProject_Add(prism-md-src
    GIT_REPOSITORY    https://github.com/ethindp/prism.git
    GIT_TAG           v0.13.0
    PATCH_COMMAND     git apply ${CMAKE_CURRENT_LIST_DIR}/0001-Add-PRISM_USE_STATIC_CRT-option-to-select-MT-d-runti.patch
    UPDATE_COMMAND    ""
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                      -DBUILD_SHARED_LIBS=OFF
                      -DPRISM_ENABLE_GDEXTENSION=OFF
                      -DCMAKE_DEBUG_POSTFIX=d
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Debug
    COMMAND           ${CMAKE_COMMAND} --build . --config Release
    INSTALL_DIR       ${PRISM_INSTALL_DIR_MD}
    INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Debug
    COMMAND           ${CMAKE_COMMAND} --build . --target install --config Release
    COMMAND           ${CMAKE_COMMAND} -E copy_if_different <BINARY_DIR>/ZDSR.lib <BINARY_DIR>/BoyCtrl.lib <BINARY_DIR>/PCTalker.lib <BINARY_DIR>/PrismOrcaBridge.lib <BINARY_DIR>/PrismSpeechDispatcherBridge.lib <INSTALL_DIR>/lib/
    BUILD_BYPRODUCTS  ${PRISM_INSTALL_DIR_MD}/lib/prismd.lib
                      ${PRISM_STATIC_LIB_MD}
  )

  ExternalProject_Add(prism-mt-src
    GIT_REPOSITORY    https://github.com/ethindp/prism.git
    GIT_TAG           v0.13.0
    PATCH_COMMAND     git apply ${CMAKE_CURRENT_LIST_DIR}/0001-Add-PRISM_USE_STATIC_CRT-option-to-select-MT-d-runti.patch
    UPDATE_COMMAND    ""
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                      -DBUILD_SHARED_LIBS=OFF
                      -DPRISM_ENABLE_GDEXTENSION=OFF
                      -DPRISM_USE_STATIC_CRT=ON
                      -DCMAKE_DEBUG_POSTFIX=d
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Debug
    COMMAND           ${CMAKE_COMMAND} --build . --config Release
    INSTALL_DIR       ${PRISM_INSTALL_DIR_MT}
    INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Debug
    COMMAND           ${CMAKE_COMMAND} --build . --target install --config Release
    COMMAND           ${CMAKE_COMMAND} -E copy_if_different <BINARY_DIR>/ZDSR.lib <BINARY_DIR>/BoyCtrl.lib <BINARY_DIR>/PCTalker.lib <BINARY_DIR>/PrismOrcaBridge.lib <BINARY_DIR>/PrismSpeechDispatcherBridge.lib <INSTALL_DIR>/lib/
    BUILD_BYPRODUCTS  ${PRISM_INSTALL_DIR_MT}/lib/prismd.lib
                      ${PRISM_STATIC_LIB_MT}
  )

  file(MAKE_DIRECTORY ${PRISM_INSTALL_DIR_MD}/include)

  add_library(Prism STATIC IMPORTED GLOBAL)
  add_dependencies(Prism prism-md-src)
  target_include_directories(Prism INTERFACE ${PRISM_INSTALL_DIR_MD}/include)
  target_compile_definitions(Prism INTERFACE PRISM_STATIC)
  set_target_properties(Prism PROPERTIES
    IMPORTED_LOCATION_DEBUG ${PRISM_INSTALL_DIR_MD}/lib/prismd.lib
    IMPORTED_LOCATION ${PRISM_STATIC_LIB_MD})

  foreach (implib ZDSR BoyCtrl PCTalker PrismOrcaBridge PrismSpeechDispatcherBridge)
    add_library(Prism_${implib} SHARED IMPORTED GLOBAL)
    add_dependencies(Prism_${implib} prism-md-src)
    set_target_properties(Prism_${implib} PROPERTIES
      IMPORTED_IMPLIB ${PRISM_INSTALL_DIR_MD}/lib/${implib}.lib)
  endforeach()

  target_link_libraries(Prism INTERFACE
    rpcrt4 delayimp onecore uiautomationcore ole32 oleaut32
    Prism_ZDSR Prism_BoyCtrl Prism_PCTalker Prism_PrismOrcaBridge Prism_PrismSpeechDispatcherBridge)
  target_link_options(Prism INTERFACE
    "/WHOLEARCHIVE:$<TARGET_LINKER_FILE:Prism>"
    /delayload:ZDSRAPI_x64.dll
    /delayload:BoyCtrl-x64.dll /delayload:PCTKUSR.dll
    /delayload:prism_orca_bridge.dll /delayload:prism_speech_dispatcher_bridge.dll)

  file(MAKE_DIRECTORY ${PRISM_INSTALL_DIR_MT}/include)

  add_library(PrismMT STATIC IMPORTED GLOBAL)
  add_dependencies(PrismMT prism-mt-src)
  target_include_directories(PrismMT INTERFACE ${PRISM_INSTALL_DIR_MT}/include)
  target_compile_definitions(PrismMT INTERFACE PRISM_STATIC)
  set_target_properties(PrismMT PROPERTIES
    IMPORTED_LOCATION_DEBUG ${PRISM_INSTALL_DIR_MT}/lib/prismd.lib
    IMPORTED_LOCATION ${PRISM_STATIC_LIB_MT})

  foreach (implib ZDSR BoyCtrl PCTalker PrismOrcaBridge PrismSpeechDispatcherBridge)
    add_library(PrismMT_${implib} SHARED IMPORTED GLOBAL)
    add_dependencies(PrismMT_${implib} prism-mt-src)
    set_target_properties(PrismMT_${implib} PROPERTIES
      IMPORTED_IMPLIB ${PRISM_INSTALL_DIR_MT}/lib/${implib}.lib)
  endforeach()

  target_link_libraries(PrismMT INTERFACE
    rpcrt4 delayimp onecore uiautomationcore ole32 oleaut32
    PrismMT_ZDSR PrismMT_BoyCtrl PrismMT_PCTalker PrismMT_PrismOrcaBridge PrismMT_PrismSpeechDispatcherBridge)
  target_link_options(PrismMT INTERFACE
    "/WHOLEARCHIVE:$<TARGET_LINKER_FILE:PrismMT>"
    /delayload:ZDSRAPI_x64.dll
    /delayload:BoyCtrl-x64.dll /delayload:PCTKUSR.dll
    /delayload:prism_orca_bridge.dll /delayload:prism_speech_dispatcher_bridge.dll)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  set (PRISM_INSTALL_DIR_ARM64 ${CMAKE_CURRENT_BINARY_DIR}/prism-arm64)
  set (PRISM_INSTALL_DIR_X86_64 ${CMAKE_CURRENT_BINARY_DIR}/prism-x86_64)
  set (PRISM_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/prism)
  set (PRISM_STATIC_LIB ${PRISM_INSTALL_DIR}/lib/libprism.a)

  set (PRISM_COMMON_CMAKE_ARGS_MAC
    -DBUILD_SHARED_LIBS=OFF
    -DPRISM_ENABLE_GDEXTENSION=OFF
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})

  ExternalProject_Add(prism-arm64-src
    GIT_REPOSITORY    https://github.com/ethindp/prism.git
    GIT_TAG           v0.13.0
    UPDATE_COMMAND    ""
    CMAKE_GENERATOR   Xcode
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                      -DCMAKE_OSX_ARCHITECTURES=arm64
                      ${PRISM_COMMON_CMAKE_ARGS_MAC}
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Release
    INSTALL_DIR       ${PRISM_INSTALL_DIR_ARM64}
    INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Release
    BUILD_BYPRODUCTS  ${PRISM_INSTALL_DIR_ARM64}/lib/libprism.a
  )

  ExternalProject_Add(prism-x86_64-src
    GIT_REPOSITORY    https://github.com/ethindp/prism.git
    GIT_TAG           v0.13.0
    UPDATE_COMMAND    ""
    CMAKE_GENERATOR   Xcode
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                      -DCMAKE_OSX_ARCHITECTURES=x86_64
                      ${PRISM_COMMON_CMAKE_ARGS_MAC}
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --config Release
    INSTALL_DIR       ${PRISM_INSTALL_DIR_X86_64}
    INSTALL_COMMAND   ${CMAKE_COMMAND} --build . --target install --config Release
    BUILD_BYPRODUCTS  ${PRISM_INSTALL_DIR_X86_64}/lib/libprism.a
  )

  file(MAKE_DIRECTORY ${PRISM_INSTALL_DIR}/lib)
  file(MAKE_DIRECTORY ${PRISM_INSTALL_DIR}/include)
  add_custom_command (OUTPUT ${PRISM_STATIC_LIB}
    COMMAND lipo -create ${PRISM_INSTALL_DIR_ARM64}/lib/libprism.a ${PRISM_INSTALL_DIR_X86_64}/lib/libprism.a -output ${PRISM_STATIC_LIB}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PRISM_INSTALL_DIR_ARM64}/include ${PRISM_INSTALL_DIR}/include
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating universal Prism binary"
    DEPENDS prism-arm64-src prism-x86_64-src)

  add_custom_target (prism-src DEPENDS ${PRISM_STATIC_LIB})

  add_library(Prism STATIC IMPORTED GLOBAL)
  add_dependencies(Prism prism-src)
  target_include_directories(Prism INTERFACE ${PRISM_INSTALL_DIR}/include)
  target_compile_definitions(Prism INTERFACE PRISM_STATIC)
  set_target_properties(Prism PROPERTIES IMPORTED_LOCATION ${PRISM_STATIC_LIB})

  find_library(AVFOUNDATION_FRAMEWORK AVFoundation)
  find_library(FOUNDATION_FRAMEWORK Foundation)
  target_link_libraries(Prism INTERFACE ${AVFOUNDATION_FRAMEWORK} ${FOUNDATION_FRAMEWORK})
  target_link_options(Prism INTERFACE "LINKER:-force_load,$<TARGET_LINKER_FILE:Prism>")

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  set (PRISM_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/prism)
  set (PRISM_STATIC_LIB ${PRISM_INSTALL_DIR}/lib/libprism.a)

  ExternalProject_Add(prism-src
    GIT_REPOSITORY    https://github.com/ethindp/prism.git
    GIT_TAG           v0.13.0
    UPDATE_COMMAND    ""
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                      -DBUILD_SHARED_LIBS=OFF
                      -DPRISM_ENABLE_GDEXTENSION=OFF
                      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                      -DCMAKE_BUILD_TYPE=release
    INSTALL_DIR       ${PRISM_INSTALL_DIR}
    BUILD_BYPRODUCTS  ${PRISM_STATIC_LIB}
  )

  file(MAKE_DIRECTORY ${PRISM_INSTALL_DIR}/include)

  add_library(Prism STATIC IMPORTED GLOBAL)
  add_dependencies(Prism prism-src)
  target_include_directories(Prism INTERFACE ${PRISM_INSTALL_DIR}/include)
  target_compile_definitions(Prism INTERFACE PRISM_STATIC)
  set_target_properties(Prism PROPERTIES IMPORTED_LOCATION ${PRISM_STATIC_LIB})

  find_package(PkgConfig)
  if (PkgConfig_FOUND)
    pkg_check_modules(SPEECHD IMPORTED_TARGET speech-dispatcher)
    pkg_check_modules(GLIB IMPORTED_TARGET glib-2.0)
    pkg_check_modules(GIO IMPORTED_TARGET gio-2.0)
    if (SPEECHD_FOUND)
      target_link_libraries(Prism INTERFACE PkgConfig::SPEECHD)
    endif()
    if (GLIB_FOUND AND GIO_FOUND)
      target_link_libraries(Prism INTERFACE PkgConfig::GLIB PkgConfig::GIO)
    endif()
  endif()
  target_link_options(Prism INTERFACE
    "LINKER:--whole-archive" "$<TARGET_LINKER_FILE:Prism>" "LINKER:--no-whole-archive")

endif()
