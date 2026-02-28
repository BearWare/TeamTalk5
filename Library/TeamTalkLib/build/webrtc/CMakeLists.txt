project(WebRTC)

include(ExternalProject)

##################################################
# WebRTC
##################################################

set (TOOLCHAIN_INSTALL_PREFIX_WEBRTC ${TOOLCHAIN_INSTALL_PREFIX}/webrtc)

if (MSVC)
  # WebRTC for Windows depends on "Windows 11 SDK (10.0.22621.0)". This
  # can be installed from Visual Studio Installer.
  #
  # WebRTC for Windows depends on "C++ Clang Compiler for Windows (18.1.8)".
  # This can be installed from Visual Studio Installer.

  # Force C:\webrtc to avoid build errors due to long path names (MAX_PATH=260)
  set (WEBRTC_FETCH_PATH C:/webrtc/${CMAKE_SYSTEM_NAME} CACHE FILEPATH "Root folder for WebRTC repository")
  file(TO_NATIVE_PATH ${WEBRTC_FETCH_PATH} WEBRTC_FETCH_PATH_NATIVE)

  set (WEBRTC_SOURCE_ROOT ${WEBRTC_FETCH_PATH}/src)
  file(TO_NATIVE_PATH ${WEBRTC_SOURCE_ROOT} WEBRTC_SOURCE_ROOT_NATIVE)

  if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    if (IS_ABSOLUTE ${TOOLCHAIN_INSTALL_PREFIX})
      set (WEBRTC_INSTALL_ROOT ${TOOLCHAIN_INSTALL_PREFIX_WEBRTC}/x64)
    else()
      set (WEBRTC_INSTALL_ROOT ${CMAKE_CURRENT_BINARY_DIR}/${TOOLCHAIN_INSTALL_PREFIX_WEBRTC}/x64)
    endif()
    set (WEBRTC_ARCH x64)
  else()
    if (IS_ABSOLUTE ${TOOLCHAIN_INSTALL_PREFIX})
      set (WEBRTC_INSTALL_ROOT ${TOOLCHAIN_INSTALL_PREFIX_WEBRTC}/x86)
    else()
      set (WEBRTC_INSTALL_ROOT ${CMAKE_CURRENT_BINARY_DIR}/${TOOLCHAIN_INSTALL_PREFIX_WEBRTC}/x86)
    endif()
    set (WEBRTC_ARCH x86)
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)

    # To list possible args for "gn gen" use: gn args --list OUTPUTDIR
    configure_file(${CMAKE_CURRENT_LIST_DIR}/args.gn_win_debug ${WEBRTC_INSTALL_ROOT}/lib/debug/args.gn @ONLY)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/args.gn_win_release ${WEBRTC_INSTALL_ROOT}/lib/release/args.gn @ONLY)

    file(TO_NATIVE_PATH ${WEBRTC_INSTALL_ROOT} WEBRTC_INSTALL_ROOT_NATIVE)

    ExternalProject_Add(depot-tools
      # Use WEBRTC_FETCH_PATH to avoid issues with MAX_PATH on Windows
      SOURCE_DIR        ${WEBRTC_FETCH_PATH}/depot-tools
      BUILD_IN_SOURCE   TRUE
      GIT_REPOSITORY    https://chromium.googlesource.com/chromium/tools/depot_tools.git
      # Tested with 23ad8839bff8e65fff2f1aabd4bc48fef894c4ae but hopefully 'main' continues to work
      GIT_TAG           main
      UPDATE_COMMAND    ""
      INSTALL_DIR       ""
      CONFIGURE_COMMAND ""
      BUILD_COMMAND     ""
      INSTALL_COMMAND   fetch.bat --help
      )
    ExternalProject_Get_Property(depot-tools SOURCE_DIR)
    set (DEPOTTOOLS_DIR ${SOURCE_DIR})
    file (TO_NATIVE_PATH ${DEPOTTOOLS_DIR} DEPOTTOOLS_DIR_NATIVE)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/depottools-win.bat_sample ${CMAKE_CURRENT_BINARY_DIR}/depottools-win.bat @ONLY)

    if (NOT EXISTS ${WEBRTC_SOURCE_ROOT})
      message (WARNING "WebRTC is not present in ${WEBRTC_SOURCE_ROOT}. Downloading... This takes a loooong time")
      file(MAKE_DIRECTORY ${WEBRTC_FETCH_PATH})
      set (WEBRTC_DOWNLOAD_CMD ${CMAKE_CURRENT_BINARY_DIR}/depottools-win.bat)
      set (WEBRTC_PATCH_CMD cd ${WEBRTC_SOURCE_ROOT})
      set (WEBRTC_PATCH_CMD2 CALL git apply ${CMAKE_CURRENT_LIST_DIR}\\libteamtalk-r6818.patch)
      set (WEBRTC_PATCH_CMD3 CALL git apply ${CMAKE_CURRENT_LIST_DIR}\\0001-Add-support-for-Visual-Studio-2026.patch)
    else()
      message (WARNING "WebRTC is already downloaded to ${WEBRTC_SOURCE_ROOT}. Skipping new fetch and patching")
      set (WEBRTC_DOWNLOAD_CMD "")
      set (WEBRTC_PATCH_CMD "")
      set (WEBRTC_PATCH_CMD2 "")
      set (WEBRTC_PATCH_CMD3 "")
    endif()

    configure_file(${CMAKE_CURRENT_LIST_DIR}/webrtc_cfg_win.bat_sample ${CMAKE_CURRENT_BINARY_DIR}/webrtc_cfg_win.bat)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/webrtc_copy_win.bat_sample ${CMAKE_CURRENT_BINARY_DIR}/webrtc_copy_win.bat)

    ExternalProject_Add(webrtc-src
      SOURCE_DIR        ${WEBRTC_SOURCE_ROOT}
      DOWNLOAD_COMMAND  ${WEBRTC_DOWNLOAD_CMD}
      PATCH_COMMAND     ${WEBRTC_PATCH_CMD}
      COMMAND           ${WEBRTC_PATCH_CMD2}
      COMMAND           ${WEBRTC_PATCH_CMD3}
      INSTALL_DIR       ${WEBRTC_INSTALL_ROOT}
      CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/webrtc_cfg_win.bat
      COMMAND           cd ${WEBRTC_SOURCE_ROOT}
      COMMAND           CALL gn gen <INSTALL_DIR>\\lib\\debug
      COMMAND           CALL gn gen <INSTALL_DIR>\\lib\\release
      BUILD_COMMAND     ${CMAKE_CURRENT_BINARY_DIR}/webrtc_cfg_win.bat
      COMMAND           CALL ninja -v ${TOOLCHAIN_BUILD_MAKEJOBS} -C <INSTALL_DIR>\\lib\\debug teamtalk
      COMMAND           CALL ninja -v ${TOOLCHAIN_BUILD_MAKEJOBS} -C <INSTALL_DIR>\\lib\\release teamtalk
      INSTALL_COMMAND   ${CMAKE_COMMAND} -E rm -rf ${WEBRTC_INSTALL_ROOT}/include
      COMMAND           ${CMAKE_CURRENT_BINARY_DIR}/webrtc_copy_win.bat
      BUILD_BYPRODUCTS  <INSTALL_DIR>/lib/debug/obj/modules/audio_processing/teamtalk.lib
      <INSTALL_DIR>/lib/release/obj/modules/audio_processing/teamtalk.lib
      DEPENDS depot-tools
      )

    file(MAKE_DIRECTORY ${WEBRTC_INSTALL_ROOT}/include)
    file(MAKE_DIRECTORY ${WEBRTC_INSTALL_ROOT}/include/third_party/abseil-cpp)
  endif()

  if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    add_library(webrtc STATIC IMPORTED GLOBAL)
    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      add_dependencies(webrtc webrtc-src)
    endif()
    target_include_directories (webrtc INTERFACE
      ${WEBRTC_INSTALL_ROOT}/include
      ${WEBRTC_INSTALL_ROOT}/include/third_party/abseil-cpp)
    set_target_properties(webrtc PROPERTIES
      IMPORTED_LOCATION_DEBUG ${WEBRTC_INSTALL_ROOT}/lib/debug/obj/modules/audio_processing/teamtalk.lib
      IMPORTED_LOCATION ${WEBRTC_INSTALL_ROOT}/lib/release/obj/modules/audio_processing/teamtalk.lib)
  else()
    add_library(webrtc STATIC IMPORTED GLOBAL)
    if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
      add_dependencies(webrtc webrtc-src)
    endif()
    target_include_directories (webrtc INTERFACE
      ${WEBRTC_INSTALL_ROOT}/include
      ${WEBRTC_INSTALL_ROOT}/include/third_party/abseil-cpp)
    set_target_properties(webrtc PROPERTIES
      IMPORTED_LOCATION_DEBUG ${WEBRTC_INSTALL_ROOT}/lib/debug/obj/modules/audio_processing/teamtalk.lib
      IMPORTED_LOCATION ${WEBRTC_INSTALL_ROOT}/lib/release/obj/modules/audio_processing/teamtalk.lib)
  endif()

else()

  if (IS_ABSOLUTE ${TOOLCHAIN_INSTALL_PREFIX_WEBRTC})
    set (WEBRTC_INSTALL_ROOT ${TOOLCHAIN_INSTALL_PREFIX_WEBRTC})
  else()
    set (WEBRTC_INSTALL_ROOT ${CMAKE_CURRENT_BINARY_DIR}/${TOOLCHAIN_INSTALL_PREFIX_WEBRTC})
  endif()

  if (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set (WEBRTC_FETCH_PATH $ENV{HOME}/webrtc/${CMAKE_SYSTEM_NAME} CACHE FILEPATH "Root folder for WebRTC repository")

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

    configure_file(${CMAKE_CURRENT_LIST_DIR}/args.gn_ios ${WEBRTC_INSTALL_ROOT}/args.gn @ONLY)

  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set (WEBRTC_FETCH_PATH $ENV{HOME}/webrtc/${CMAKE_SYSTEM_NAME} CACHE FILEPATH "Root folder for WebRTC repository")

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

    set (ANDROID_NDK_HOME $ENV{ANDROID_NDK_HOME})
    configure_file(${CMAKE_CURRENT_LIST_DIR}/args.gn_android ${WEBRTC_INSTALL_ROOT}/args.gn @ONLY)

  elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (WEBRTC_FETCH_PATH $ENV{HOME}/webrtc/${CMAKE_SYSTEM_NAME} CACHE FILEPATH "Root folder for WebRTC repository")
    set (WEBRTC_ARCH x64)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/args.gn_mac ${WEBRTC_INSTALL_ROOT}-intel/args.gn @ONLY)
    set (WEBRTC_ARCH arm64)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/args.gn_mac ${WEBRTC_INSTALL_ROOT}-arm64/args.gn @ONLY)

  elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (WEBRTC_FETCH_PATH $ENV{HOME}/webrtc/${CMAKE_SYSTEM_NAME} CACHE FILEPATH "Root folder for WebRTC repository")
    configure_file(${CMAKE_CURRENT_LIST_DIR}/args.gn_deb ${WEBRTC_INSTALL_ROOT}/args.gn @ONLY)

  else()
      message(WARNING "Unknown platform for WebRTC")
  endif()

  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)

    set (WEBRTC_SOURCE_ROOT ${WEBRTC_FETCH_PATH}/src)

    ExternalProject_Add(depot-tools
      BUILD_IN_SOURCE   TRUE
      GIT_REPOSITORY    https://chromium.googlesource.com/chromium/tools/depot_tools.git
      # Tested with 23ad8839bff8e65fff2f1aabd4bc48fef894c4ae but hopefully 'main' continues to work
      GIT_TAG           main
      UPDATE_COMMAND    ""
      INSTALL_DIR       ""
      CONFIGURE_COMMAND export PATH=<SOURCE_DIR>:$ENV{PATH} && ensure_bootstrap
      BUILD_COMMAND     ""
      INSTALL_COMMAND   ""
      )
    ExternalProject_Get_Property(depot-tools SOURCE_DIR)
    set (DEPOTTOOLS_DIR ${SOURCE_DIR})
    set (DEPOTTOOLS_EXPORT export PATH=$ENV{PATH}:${DEPOTTOOLS_DIR})

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
    #
    # Ensure HomeBrew path (/usr/local/bin) is not active when 'fetch'
    # is executed.
    if (NOT EXISTS ${WEBRTC_SOURCE_ROOT})
      message (WARNING "WebRTC is not present in ${WEBRTC_SOURCE_ROOT}. Downloading... This takes a loooong time")
      file(MAKE_DIRECTORY ${WEBRTC_FETCH_PATH})
      set (WEBRTC_DOWNLOAD_CMD1 ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_FETCH_PATH} && fetch --nohooks ${WEBRTC_REPO_NAME})
      set (WEBRTC_DOWNLOAD_CMD2 ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_SOURCE_ROOT} && git checkout branch-heads/6818)
      set (WEBRTC_DOWNLOAD_CMD3 ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_FETCH_PATH} && gclient sync -D ${TOOLCHAIN_BUILD_MAKEJOBS})
      set (WEBRTC_PATCH_CMD cd ${WEBRTC_SOURCE_ROOT} && git apply ${CMAKE_CURRENT_LIST_DIR}/libteamtalk-r6818.patch)
    else()
      message (WARNING "WebRTC is already downloaded to ${WEBRTC_SOURCE_ROOT}. Skipping new fetch and patching")
      set (WEBRTC_DOWNLOAD_CMD1 "")
      set (WEBRTC_DOWNLOAD_CMD2 "")
      set (WEBRTC_DOWNLOAD_CMD3 "")
      set (WEBRTC_PATCH_CMD "")
    endif()

    if (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
      ExternalProject_Add(webrtc-src
        SOURCE_DIR        ${WEBRTC_SOURCE_ROOT}
        DOWNLOAD_COMMAND  ${WEBRTC_DOWNLOAD_CMD1}
        COMMAND           ${WEBRTC_DOWNLOAD_CMD2}
        COMMAND           ${WEBRTC_DOWNLOAD_CMD3}
        PATCH_COMMAND     ${WEBRTC_PATCH_CMD}
        INSTALL_DIR       ${WEBRTC_INSTALL_ROOT}
        CONFIGURE_COMMAND ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_SOURCE_ROOT} && gn gen <INSTALL_DIR>
        BUILD_COMMAND     ${DEPOTTOOLS_EXPORT} && ninja -v ${TOOLCHAIN_BUILD_MAKEJOBS} -C <INSTALL_DIR> teamtalk
        INSTALL_COMMAND   cd ${WEBRTC_SOURCE_ROOT} && mkdir -p <INSTALL_DIR>/include
        COMMAND           cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.h" -exec ditto \{\} <INSTALL_DIR>/include/\{\} \$<SEMICOLON>
        COMMAND           cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.inc" -exec ditto \{\} <INSTALL_DIR>/include/\{\} \$<SEMICOLON>
        BUILD_BYPRODUCTS  <INSTALL_DIR>/obj/modules/audio_processing/libteamtalk.a
        DEPENDS depot-tools
        )
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
      ExternalProject_Add(webrtc-src
        SOURCE_DIR        ${WEBRTC_SOURCE_ROOT}
        DOWNLOAD_COMMAND  ${WEBRTC_DOWNLOAD_CMD1}
        COMMAND           ${WEBRTC_DOWNLOAD_CMD2}
        COMMAND           ${WEBRTC_DOWNLOAD_CMD3}
        PATCH_COMMAND     ${WEBRTC_PATCH_CMD}
        INSTALL_DIR       ${WEBRTC_INSTALL_ROOT}
        CONFIGURE_COMMAND ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_SOURCE_ROOT} && gn gen <INSTALL_DIR>
        BUILD_COMMAND     ${DEPOTTOOLS_EXPORT} && ninja -v ${TOOLCHAIN_BUILD_MAKEJOBS} -C <INSTALL_DIR> teamtalk
        INSTALL_COMMAND   cd ${WEBRTC_SOURCE_ROOT} && mkdir -p <INSTALL_DIR>/include
        COMMAND           cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.h" -exec cp --parents \{\} <INSTALL_DIR>/include/ \$<SEMICOLON>
        COMMAND           cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.inc" -exec cp -r --parents \{\} <INSTALL_DIR>/include/ \$<SEMICOLON>
        BUILD_BYPRODUCTS  <INSTALL_DIR>/obj/modules/audio_processing/libteamtalk.a
        DEPENDS depot-tools
        )
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      ExternalProject_Add(webrtc-arm64-src
        SOURCE_DIR        ${WEBRTC_SOURCE_ROOT}
        DOWNLOAD_COMMAND  ${WEBRTC_DOWNLOAD_CMD1}
        COMMAND           ${WEBRTC_DOWNLOAD_CMD2}
        COMMAND           ${WEBRTC_DOWNLOAD_CMD3}
        PATCH_COMMAND     ${WEBRTC_PATCH_CMD}
        INSTALL_DIR       ${WEBRTC_INSTALL_ROOT}-arm64
        CONFIGURE_COMMAND ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_SOURCE_ROOT} && gn gen <INSTALL_DIR>
        BUILD_COMMAND     ${DEPOTTOOLS_EXPORT} && ninja -v ${TOOLCHAIN_BUILD_MAKEJOBS} -C <INSTALL_DIR> teamtalk
        INSTALL_COMMAND   cd ${WEBRTC_SOURCE_ROOT} && mkdir -p <INSTALL_DIR>/include
        COMMAND           cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.h" -exec ditto \{\} <INSTALL_DIR>/include/\{\} \$<SEMICOLON>
        COMMAND           cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.inc" -exec ditto \{\} <INSTALL_DIR>/include/\{\} \$<SEMICOLON>
        BUILD_BYPRODUCTS  <INSTALL_DIR>/obj/modules/audio_processing/libteamtalk.a
        DEPENDS depot-tools
        )
      ExternalProject_Add(webrtc-intel-src
        SOURCE_DIR        ${WEBRTC_SOURCE_ROOT}
        DOWNLOAD_COMMAND  ""
        PATCH_COMMAND     ""
        INSTALL_DIR       ${WEBRTC_INSTALL_ROOT}-intel
        CONFIGURE_COMMAND ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_SOURCE_ROOT} && gn gen <INSTALL_DIR>
        BUILD_COMMAND     ${DEPOTTOOLS_EXPORT} && ninja -v ${TOOLCHAIN_BUILD_MAKEJOBS} -C <INSTALL_DIR> teamtalk
        INSTALL_COMMAND   cd ${WEBRTC_SOURCE_ROOT} && mkdir -p <INSTALL_DIR>/include
        COMMAND           cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.h" -exec ditto \{\} <INSTALL_DIR>/include/\{\} \$<SEMICOLON>
        COMMAND           cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.inc" -exec ditto \{\} <INSTALL_DIR>/include/\{\} \$<SEMICOLON>
        BUILD_BYPRODUCTS  <INSTALL_DIR>/obj/modules/audio_processing/libteamtalk.a
        DEPENDS webrtc-arm64-src
        )

      # Use 'lipo' to create universal binary
      file(MAKE_DIRECTORY ${WEBRTC_INSTALL_ROOT}/include)
      add_custom_command (OUTPUT ${WEBRTC_INSTALL_ROOT}/obj/modules/audio_processing/libteamtalk.a
        COMMAND lipo -create ${WEBRTC_INSTALL_ROOT}-arm64/obj/modules/audio_processing/libteamtalk.a
                ${WEBRTC_INSTALL_ROOT}-intel/obj/modules/audio_processing/libteamtalk.a
                -output ${WEBRTC_INSTALL_ROOT}/obj/modules/audio_processing/libteamtalk.a
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${WEBRTC_INSTALL_ROOT}-intel/include ${WEBRTC_INSTALL_ROOT}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating universal WebRTC binaries"
        DEPENDS webrtc-intel-src webrtc-arm64-src)

      add_custom_target (webrtc-src DEPENDS ${WEBRTC_INSTALL_ROOT}/obj/modules/audio_processing/libteamtalk.a)

    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      ExternalProject_Add(webrtc-src
        SOURCE_DIR        ${WEBRTC_SOURCE_ROOT}
        DOWNLOAD_COMMAND  ${WEBRTC_DOWNLOAD_CMD1}
        COMMAND           ${WEBRTC_DOWNLOAD_CMD2}
        COMMAND           ${WEBRTC_DOWNLOAD_CMD3}
        PATCH_COMMAND     ${WEBRTC_PATCH_CMD}
        INSTALL_DIR       ${WEBRTC_INSTALL_ROOT}
        CONFIGURE_COMMAND ${DEPOTTOOLS_EXPORT} && cd ${WEBRTC_SOURCE_ROOT} && gn gen <INSTALL_DIR>
        BUILD_COMMAND     ${DEPOTTOOLS_EXPORT} && ninja -v ${TOOLCHAIN_BUILD_MAKEJOBS} -C <INSTALL_DIR> teamtalk
        INSTALL_COMMAND   cd ${WEBRTC_SOURCE_ROOT} && mkdir -p <INSTALL_DIR>/include
        COMMAND           cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.h" -exec cp --parents \{\} <INSTALL_DIR>/include/ \$<SEMICOLON>
        COMMAND           cd ${WEBRTC_SOURCE_ROOT} && find . -type f -name "*.inc" -exec cp -r --parents \{\} <INSTALL_DIR>/include/ \$<SEMICOLON>
        BUILD_BYPRODUCTS  <INSTALL_DIR>/obj/modules/audio_processing/libteamtalk.a
        DEPENDS depot-tools
        )
    else()
      message(WARNING "Unknown platform. No target make for webrtc-src")
    endif()

    file(MAKE_DIRECTORY ${WEBRTC_INSTALL_ROOT}/include)
    file(MAKE_DIRECTORY ${WEBRTC_INSTALL_ROOT}/include/third_party/abseil-cpp)
  endif()

  add_library(webrtc STATIC IMPORTED GLOBAL)
  if (TOOLCHAIN_BUILD_EXTERNALPROJECTS)
    add_dependencies(webrtc webrtc-src)
  endif()

  target_include_directories (webrtc INTERFACE
    ${WEBRTC_INSTALL_ROOT}/include
    ${WEBRTC_INSTALL_ROOT}/include/third_party/abseil-cpp)
  set_target_properties(webrtc PROPERTIES
    IMPORTED_LOCATION ${WEBRTC_INSTALL_ROOT}/obj/modules/audio_processing/libteamtalk.a)

  # $ find webrtc/obj/ -name *.a -exec nm -A -C {} \; | grep webrtc::rnn_vad::RnnBasedVad::RnnBasedVad
endif()
