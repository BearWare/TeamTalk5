cmake_minimum_required(VERSION 3.1)

project(WebRTC)

include(ExternalProject)

##################################################
# WebRTC
##################################################

if (MSVC)
  set (WEBRTC_ROOT C:/webrtc)

  if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    add_library(webrtc STATIC IMPORTED GLOBAL)
    target_include_directories (webrtc INTERFACE
      ${WEBRTC_ROOT}/install/x64/include
      ${WEBRTC_ROOT}/install/x64/include/modules/audio_processing/include
      ${WEBRTC_ROOT}/install/x64/include/third_party/abseil-cpp)
    set_target_properties(webrtc PROPERTIES
      IMPORTED_LOCATION_DEBUG ${WEBRTC_ROOT}/install/x64/lib/debug/obj/modules/audio_processing/teamtalk.lib
      IMPORTED_LOCATION ${WEBRTC_ROOT}/install/x64/lib/release/obj/modules/audio_processing/teamtalk.lib)
  else()
    add_library(webrtc STATIC IMPORTED GLOBAL)
    target_include_directories (webrtc INTERFACE
      ${WEBRTC_ROOT}/install/x86/include
      ${WEBRTC_ROOT}/install/x86/include/modules/audio_processing/include
      ${WEBRTC_ROOT}/install/x86/include/third_party/abseil-cpp)
    set_target_properties(webrtc PROPERTIES
      IMPORTED_LOCATION_DEBUG ${WEBRTC_ROOT}/install/x86/lib/debug/obj/modules/audio_processing/teamtalk.lib
      IMPORTED_LOCATION ${WEBRTC_ROOT}/install/x86/lib/release/obj/modules/audio_processing/teamtalk.lib)
  endif()

else()

  ExternalProject_Add(depot-tools
    GIT_REPOSITORY    https://chromium.googlesource.com/chromium/tools/depot_tools.git
    GIT_TAG           3b39cefc
    UPDATE_COMMAND    ""
    INSTALL_DIR       ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ""
    )

  if (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set (WEBRTC_ROOT $ENV{HOME}/webrtc/${CMAKE_SYSTEM_NAME}/${CMAKE_OSX_ARCHITECTURES}/install)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set (WEBRTC_ROOT $ENV{HOME}/webrtc/${CMAKE_SYSTEM_NAME}/${ANDROID_ABI}/install)
  else()
    set (WEBRTC_ROOT $ENV{HOME}/webrtc/${CMAKE_SYSTEM_NAME}/install)
  endif()

  find_file(WEBRTC_INSTALL_PATH ${WEBRTC_ROOT})
  if (NOT WEBRTC_INSTALL_PATH)
    message(WARNING "Cannot find WebRTC installation in ${WEBRTC_ROOT}. Expecting 'include' and 'obj' subdirectories. Specify -DFEATURE_WEBRTC=OFF to disable WebRTC.")
  else()

    add_library(webrtc STATIC IMPORTED GLOBAL)
    if (${CMAKE_VERSION} VERSION_LESS "3.11.0")
      set_target_properties(webrtc PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${WEBRTC_INSTALL_PATH}/include
        ${WEBRTC_INSTALL_PATH}/include/modules/audio_processing/include
        ${WEBRTC_INSTALL_PATH}/include/third_party/abseil-cpp
        IMPORTED_LOCATION ${WEBRTC_INSTALL_PATH}/obj/modules/audio_processing/libteamtalk.a)
    else()
      target_include_directories (webrtc INTERFACE
        ${WEBRTC_INSTALL_PATH}/include
        ${WEBRTC_INSTALL_PATH}/include/modules/audio_processing/include
        ${WEBRTC_INSTALL_PATH}/include/third_party/abseil-cpp)
      set_target_properties(webrtc PROPERTIES
        IMPORTED_LOCATION ${WEBRTC_INSTALL_PATH}/obj/modules/audio_processing/libteamtalk.a)
    endif()
  endif()

  # $ find webrtc/obj/ -name *.a -exec nm -A -C {} \; | grep webrtc::rnn_vad::RnnBasedVad::RnnBasedVad
endif()
