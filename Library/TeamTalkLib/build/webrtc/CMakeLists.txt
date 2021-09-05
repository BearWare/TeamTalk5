cmake_minimum_required(VERSION 3.1)

project(WebRTC)

include(ExternalProject)

##################################################
# WebRTC
##################################################

if (MSVC)

  set (WEBRTC_FETCH_PATH C:/webrtc/${CMAKE_SYSTEM_NAME})
  set (WEBRTC_SOURCE_ROOT ${WEBRTC_FETCH_PATH}/src)
  if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set (WEBRTC_INSTALL_ROOT ${CMAKE_CURRENT_BINARY_DIR}/install/x64)
    set (WEBRTC_ARCH x64)
  else()
    set (WEBRTC_INSTALL_ROOT ${CMAKE_CURRENT_BINARY_DIR}/install/x86)
    set (WEBRTC_ARCH x86)
  endif()

  file(MAKE_DIRECTORY ${WEBRTC_FETCH_PATH})

  configure_file(${CMAKE_CURRENT_LIST_DIR}/args.gn_win_debug ${WEBRTC_INSTALL_ROOT}/lib/debug/args.gn @ONLY)
  configure_file(${CMAKE_CURRENT_LIST_DIR}/args.gn_win_release ${WEBRTC_INSTALL_ROOT}/lib/release/args.gn @ONLY)

  file(TO_NATIVE_PATH ${WEBRTC_INSTALL_ROOT} WEBRTC_INSTALL_ROOT_NATIVE)
  
  ExternalProject_Add(depot-tools
    URL               https://storage.googleapis.com/chrome-infra/depot_tools.zip
    UPDATE_COMMAND    ""
    INSTALL_DIR       ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    # Run fetch.bat in order to download python.bat
    INSTALL_COMMAND   SET DEPOT_TOOLS_WIN_TOOLCHAIN=0
    COMMAND           <SOURCE_DIR>\\fetch.bat --help
    )
  ExternalProject_Get_Property(depot-tools SOURCE_DIR)
  set (DEPOTTOOLS_DIR ${SOURCE_DIR})

  set (CMAKE_MSVCIDE_RUN_PATH ${DEPOTTOOLS_DIR})

  if (NOT EXISTS ${WEBRTC_SOURCE_ROOT})
    message (WARNING "WebRTC is not present in ${WEBRTC_SOURCE_ROOT}. Downloading... This takes a loooong time")
    set (WEBRTC_FETCH_CMD1 SET DEPOT_TOOLS_WIN_TOOLCHAIN=0)
    set (WEBRTC_FETCH_CMD2 cd ${WEBRTC_FETCH_PATH})
    set (WEBRTC_FETCH_CMD3 CALL fetch --nohooks webrtc)
    set (WEBRTC_FETCH_CMD4 CALL gclient sync --with_branch_heads --with_tags)
    set (WEBRTC_FETCH_CMD5 cd ${WEBRTC_SOURCE_ROOT})
    set (WEBRTC_FETCH_CMD6 CALL git checkout branch-heads/4332)
    set (WEBRTC_FETCH_CMD7 CALL gclient sync -D)
    set (WEBRTC_PATCH_CMD1 cd ${WEBRTC_SOURCE_ROOT})
    set (WEBRTC_PATCH_CMD2 CALL git apply ${CMAKE_CURRENT_LIST_DIR}\\libteamtalk-r4332.patch)
  else()
    set (WEBRTC_FETCH_CMD1 echo "Skipping WebRTC fetching")
    set (WEBRTC_FETCH_CMD2 echo "Skipping WebRTC fetching")
    set (WEBRTC_FETCH_CMD4 echo "Skipping WebRTC fetching")
    set (WEBRTC_FETCH_CMD5 echo "Skipping WebRTC fetching")
    set (WEBRTC_FETCH_CMD6 echo "Skipping WebRTC fetching")
    set (WEBRTC_FETCH_CMD7 echo "Skipping WebRTC fetching")
    set (WEBRTC_PATCH_CMD1 echo "Skipping WebRTC patching")
    set (WEBRTC_PATCH_CMD2 echo "Skipping WebRTC patching")
    message (WARNING "WebRTC is already downloaded to ${WEBRTC_SOURCE_ROOT}. Skipping new fetch and patching")
  endif()
  
  ExternalProject_Add(webrtc-src
    DOWNLOAD_COMMAND  ${WEBRTC_FETCH_CMD1}
    COMMAND           ${WEBRTC_FETCH_CMD2}
    COMMAND           ${WEBRTC_FETCH_CMD3}
    COMMAND           ${WEBRTC_FETCH_CMD4}
    COMMAND           ${WEBRTC_FETCH_CMD5}
    COMMAND           ${WEBRTC_FETCH_CMD6}
    COMMAND           ${WEBRTC_FETCH_CMD7}
    PATCH_COMMAND     ${WEBRTC_PATCH_CMD1}
    COMMAND           ${WEBRTC_PATCH_CMD2}
    UPDATE_COMMAND    ""
    INSTALL_DIR       ${WEBRTC_INSTALL_ROOT}
    CONFIGURE_COMMAND SET DEPOT_TOOLS_WIN_TOOLCHAIN=0
    COMMAND           cd ${WEBRTC_SOURCE_ROOT}
    COMMAND           CALL gn gen <INSTALL_DIR>\\lib\\debug
    COMMAND           CALL gn gen <INSTALL_DIR>\\lib\\release
    BUILD_COMMAND     SET DEPOT_TOOLS_WIN_TOOLCHAIN=0
    COMMAND           CALL ninja -v -C <INSTALL_DIR>\\lib\\debug teamtalk
    COMMAND           CALL ninja -v -C <INSTALL_DIR>\\lib\\release teamtalk
    INSTALL_COMMAND   cd ${WEBRTC_SOURCE_ROOT}
    COMMAND           DEL /S /Q ${WEBRTC_INSTALL_ROOT_NATIVE}\\include
    COMMAND           XCOPY *.h /S ${WEBRTC_INSTALL_ROOT_NATIVE}\\include
    BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/debug/obj/modules/audio_processing/teamtalk.lib
                      <INSTALL_DIR>/lib/release/obj/modules/audio_processing/teamtalk.lib
    DEPENDS depot-tools
    )

  file(MAKE_DIRECTORY ${WEBRTC_INSTALL_ROOT}/include)
  file(MAKE_DIRECTORY ${WEBRTC_INSTALL_ROOT}/include/modules/audio_processing/include)
  file(MAKE_DIRECTORY ${WEBRTC_INSTALL_ROOT}/include/third_party/abseil-cpp)

  if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    add_library(webrtc STATIC IMPORTED GLOBAL)
    add_dependencies(webrtc webrtc-src)
    target_include_directories (webrtc INTERFACE
      ${WEBRTC_INSTALL_ROOT}/include
      ${WEBRTC_INSTALL_ROOT}/include/modules/audio_processing/include
      ${WEBRTC_INSTALL_ROOT}/include/third_party/abseil-cpp)
    set_target_properties(webrtc PROPERTIES
      IMPORTED_LOCATION_DEBUG ${WEBRTC_INSTALL_ROOT}/lib/debug/obj/modules/audio_processing/teamtalk.lib
      IMPORTED_LOCATION ${WEBRTC_INSTALL_ROOT}/lib/release/obj/modules/audio_processing/teamtalk.lib)
  else()
    add_library(webrtc STATIC IMPORTED GLOBAL)
    add_dependencies(webrtc webrtc-src)
    target_include_directories (webrtc INTERFACE
      ${WEBRTC_INSTALL_ROOT}/include
      ${WEBRTC_INSTALL_ROOT}/include/modules/audio_processing/include
      ${WEBRTC_INSTALL_ROOT}/include/third_party/abseil-cpp)
    set_target_properties(webrtc PROPERTIES
      IMPORTED_LOCATION_DEBUG ${WEBRTC_INSTALL_ROOT}/lib/debug/obj/modules/audio_processing/teamtalk.lib
      IMPORTED_LOCATION ${WEBRTC_INSTALL_ROOT}/lib/release/obj/modules/audio_processing/teamtalk.lib)
  endif()

else()

  if (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set (WEBRTC_FETCH_PATH $ENV{HOME}/webrtc/${CMAKE_SYSTEM_NAME})

    if ("armv7" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
      set (WEBRTC_ARCH arm)
      set (WEBRTC_CFG_EXTRA arm_use_neon=true)
    elseif ("arm64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
      set (WEBRTC_ARCH arm64)
    elseif ("i386" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
      set (WEBRTC_ARCH x86)
    elseif ("x86_64" STREQUAL "${CMAKE_OSX_ARCHITECTURES}")
      set (WEBRTC_ARCH x64)
    else()
      message(WARNING "Unknown iOS architecture")
    endif()

    set (WEBRTC_ARGS_FILE ${CMAKE_CURRENT_LIST_DIR}/args.gn_ios)

  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set (WEBRTC_FETCH_PATH $ENV{HOME}/webrtc/${CMAKE_SYSTEM_NAME})

    if ("armeabi-v7a" STREQUAL "${ANDROID_ABI}")
      set (WEBRTC_ARCH arm)
    elseif ("arm64-v8a" STREQUAL "${ANDROID_ABI}")
      set (WEBRTC_ARCH arm64)
    elseif ("x86" STREQUAL "${ANDROID_ABI}")
      set (WEBRTC_ARCH x86)
    elseif ("x86_64" STREQUAL "${ANDROID_ABI}")
      set (WEBRTC_ARCH x64)
    else()
      message(WARNING "Unknown Android architecture")
    endif()

    set (WEBRTC_ARGS_FILE ${CMAKE_CURRENT_LIST_DIR}/args.gn_android)

  elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (WEBRTC_FETCH_PATH $ENV{HOME}/webrtc/${CMAKE_SYSTEM_NAME})
    set (WEBRTC_ARGS_FILE ${CMAKE_CURRENT_LIST_DIR}/args.gn_mac)

  elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (WEBRTC_ARGS_FILE ${CMAKE_CURRENT_LIST_DIR}/args.gn_deb)
    set (WEBRTC_FETCH_PATH $ENV{HOME}/webrtc/${CMAKE_SYSTEM_NAME})

  else()
      message(WARNING "Unknown platform for WebRTC")
  endif()

  set (WEBRTC_INSTALL_ROOT ${CMAKE_CURRENT_BINARY_DIR}/install)
  set (WEBRTC_SOURCE_ROOT ${WEBRTC_FETCH_PATH}/src)
  file(MAKE_DIRECTORY ${WEBRTC_FETCH_PATH})

  ExternalProject_Add(depot-tools
    GIT_REPOSITORY    https://chromium.googlesource.com/chromium/tools/depot_tools.git
    GIT_TAG           3b39cefc
    UPDATE_COMMAND    ""
    INSTALL_DIR       ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ""
    )
  ExternalProject_Get_Property(depot-tools SOURCE_DIR)
  set (DEPOTTOOLS_DIR ${SOURCE_DIR})
  set (DEPOTTOOLS_EXPORT export PATH=${DEPOTTOOLS_DIR}:$ENV{PATH} OPENSSL_DIR=${OPENSSL_DIR})

  configure_file(${WEBRTC_ARGS_FILE} ${WEBRTC_INSTALL_ROOT}/args.gn @ONLY)

  if (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set (WEBRTC_REPO_NAME webrtc_ios)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set (WEBRTC_REPO_NAME webrtc_android)
  else()
    set (WEBRTC_REPO_NAME webrtc)
  endif()

  # We use a shared WebRTC source directory to save disk space. A
  # WebRTC checkout uses ~16GB, so if each architecture should have
  # its own checkout then it would take a long time to download.
  #
  # Danger: This assumes all iOS architectures use the same branch
  # and no modifications have been made to WebRTC folder
  # (WEBRTC_FETCH_PATH)
  if (NOT EXISTS ${WEBRTC_SOURCE_ROOT})
    message (WARNING "WebRTC is not present in ${WEBRTC_SOURCE_ROOT}. Downloading... This takes a loooong time")
    set (WEBRTC_FETCH_CMD1 ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_FETCH_PATH} && fetch --nohooks ${WEBRTC_REPO_NAME} && gclient sync --with_branch_heads --with_tags)
    set (WEBRTC_FETCH_CMD2 ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_SOURCE_ROOT} && git checkout branch-heads/4332)
    set (WEBRTC_FETCH_CMD3 ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_FETCH_PATH} && gclient sync -D)
    set (WEBRTC_PATCH_CMD1 cd ${WEBRTC_SOURCE_ROOT} && git apply ${CMAKE_CURRENT_LIST_DIR}/libteamtalk-r4332.patch)
  else()
    set (WEBRTC_FETCH_CMD1 true)
    set (WEBRTC_FETCH_CMD2 true)
    set (WEBRTC_FETCH_CMD3 true)
    set (WEBRTC_PATCH_CMD1 true)
    message (WARNING "WebRTC is already downloaded to ${WEBRTC_SOURCE_ROOT}. Skipping new fetch and patching")
  endif()


  if (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    ExternalProject_Add(webrtc-src
      DOWNLOAD_COMMAND  ${WEBRTC_FETCH_CMD1}
      COMMAND           ${WEBRTC_FETCH_CMD2}
      COMMAND           ${WEBRTC_FETCH_CMD3}
      PATCH_COMMAND     ${WEBRTC_PATCH_CMD1}
      UPDATE_COMMAND    ""
      INSTALL_DIR       ${WEBRTC_INSTALL_ROOT}
      CONFIGURE_COMMAND ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_SOURCE_ROOT} && gn gen <INSTALL_DIR>
      BUILD_COMMAND     ${DEPOTTOOLS_EXPORT} && ninja -v -C <INSTALL_DIR> teamtalk
      INSTALL_COMMAND   cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.h" -exec ditto \{\} <INSTALL_DIR>/include/\{\} \$<SEMICOLON>
      BUILD_BYPRODUCTS  <INSTALL_DIR>/obj/modules/audio_processing/libteamtalk.a
      DEPENDS depot-tools
      )
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    ExternalProject_Add(webrtc-src
      DOWNLOAD_COMMAND  ${WEBRTC_FETCH_CMD1}
      COMMAND           ${WEBRTC_FETCH_CMD2}
      COMMAND           ${WEBRTC_FETCH_CMD3}
      PATCH_COMMAND     ${WEBRTC_PATCH_CMD1}
      UPDATE_COMMAND    ""
      INSTALL_DIR       ${WEBRTC_INSTALL_ROOT}
      CONFIGURE_COMMAND ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_SOURCE_ROOT} && gn gen <INSTALL_DIR>
      BUILD_COMMAND     ${DEPOTTOOLS_EXPORT} && ninja -v -C <INSTALL_DIR> teamtalk
      INSTALL_COMMAND   cd ${WEBRTC_SOURCE_ROOT} && mkdir -p <INSTALL_DIR>/include
      COMMAND           cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.h" -exec cp --parents \{\} <INSTALL_DIR>/include/ \$<SEMICOLON>
      BUILD_BYPRODUCTS  <INSTALL_DIR>/obj/modules/audio_processing/libteamtalk.a
      DEPENDS depot-tools
      )
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    ExternalProject_Add(webrtc-src
      DOWNLOAD_COMMAND  ${WEBRTC_FETCH_CMD1}
      COMMAND           ${WEBRTC_FETCH_CMD2}
      COMMAND           ${WEBRTC_FETCH_CMD3}
      PATCH_COMMAND     ${WEBRTC_PATCH_CMD1}
      UPDATE_COMMAND    ""
      INSTALL_DIR       ${WEBRTC_INSTALL_ROOT}
      CONFIGURE_COMMAND ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_SOURCE_ROOT} && gn gen <INSTALL_DIR>
      BUILD_COMMAND     ${DEPOTTOOLS_EXPORT} && ninja -v -C <INSTALL_DIR> teamtalk
      INSTALL_COMMAND   cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.h" -exec ditto \{\} <INSTALL_DIR>/include/\{\} \$<SEMICOLON>
      BUILD_BYPRODUCTS  <INSTALL_DIR>/obj/modules/audio_processing/libteamtalk.a
      DEPENDS depot-tools
      )
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    ExternalProject_Add(webrtc-src
      DOWNLOAD_COMMAND  ${WEBRTC_FETCH_CMD1}
      COMMAND           ${WEBRTC_FETCH_CMD2}
      COMMAND           ${WEBRTC_FETCH_CMD3}
      PATCH_COMMAND     ${WEBRTC_PATCH_CMD1}
      UPDATE_COMMAND    ""
      INSTALL_DIR       ${WEBRTC_INSTALL_ROOT}
      CONFIGURE_COMMAND ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_SOURCE_ROOT} && gn gen <INSTALL_DIR>
      BUILD_COMMAND     ${DEPOTTOOLS_EXPORT} && ninja -v -C <INSTALL_DIR> teamtalk
      INSTALL_COMMAND   cd ${WEBRTC_SOURCE_ROOT} && mkdir -p <INSTALL_DIR>/include
      COMMAND           cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.h" -exec cp --parents \{\} <INSTALL_DIR>/include/ \$<SEMICOLON>
      BUILD_BYPRODUCTS  <INSTALL_DIR>/obj/modules/audio_processing/libteamtalk.a
      DEPENDS depot-tools
      )
  else()
    message(WARNING "Unknown platform. No target make for webrtc-src")
  endif()

  file(MAKE_DIRECTORY ${WEBRTC_INSTALL_ROOT}/include)
  file(MAKE_DIRECTORY ${WEBRTC_INSTALL_ROOT}/include/modules/audio_processing/include)
  file(MAKE_DIRECTORY ${WEBRTC_INSTALL_ROOT}/include/third_party/abseil-cpp)

  add_library(webrtc STATIC IMPORTED GLOBAL)
  add_dependencies(webrtc webrtc-src)
  if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
    set (WEBRTC_INCLUDE_DIRS ${WEBRTC_INSTALL_ROOT}/include ${WEBRTC_INSTALL_ROOT}/include/modules/audio_processing/include ${WEBRTC_INSTALL_ROOT}/include/third_party/abseil-cpp)
    set_target_properties(webrtc PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${WEBRTC_INCLUDE_DIRS}"
      IMPORTED_LOCATION ${WEBRTC_INSTALL_ROOT}/obj/modules/audio_processing/libteamtalk.a)
  else()
    target_include_directories (webrtc INTERFACE
      ${WEBRTC_INSTALL_ROOT}/include
      ${WEBRTC_INSTALL_ROOT}/include/modules/audio_processing/include
      ${WEBRTC_INSTALL_ROOT}/include/third_party/abseil-cpp)
    set_target_properties(webrtc PROPERTIES
      IMPORTED_LOCATION ${WEBRTC_INSTALL_ROOT}/obj/modules/audio_processing/libteamtalk.a)
  endif()

  # $ find webrtc/obj/ -name *.a -exec nm -A -C {} \; | grep webrtc::rnn_vad::RnnBasedVad::RnnBasedVad
endif()
