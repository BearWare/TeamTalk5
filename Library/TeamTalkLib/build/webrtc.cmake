set (WEBRTC_INCLUDE_DIR ${TTLIBS_ROOT}/build/webrtc/src/modules/audio_processing/include
  ${TTLIBS_ROOT}/build/webrtc/src
  ${TTLIBS_ROOT}/build/webrtc/src/third_party/abseil-cpp)

set (WEBRTC_COMPILE_FLAGS -DABSL_ALLOCATOR_NOTHROW=1)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (WEBRTC_LINK_FLAGS
    ${TTLIBS_ROOT}/webrtc/obj/modules/audio_processing/libteamtalk.a)
else()
  # Search for function definitions:
  # $ find toolchain/webrtc/obj/ -name *.a -exec nm -A -C {} \; | grep webrtc::rnn_vad::RnnBasedVad::RnnBasedVad
  set (WEBRTC_LINK_FLAGS ${TTLIBS_ROOT}/webrtc/obj/libwebrtc.a)
endif()
