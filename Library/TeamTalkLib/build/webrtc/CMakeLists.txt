cmake_minimum_required(VERSION 3.1)

project(WebRTC)

include(ExternalProject)

##################################################
# WebRTC
##################################################

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
