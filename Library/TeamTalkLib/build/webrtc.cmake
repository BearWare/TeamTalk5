if (MSVC)
  # Due to long file names webrtc must be installed in root folder.
  # This is caused by src/third_party/blink submodule
  set (WEBRTC_DIR c:/webrtc/webrtc)
  set (WEBRTC_INCLUDE_DIR ${WEBRTC_DIR}/src/modules/audio_processing/include
    ${WEBRTC_DIR}/src ${WEBRTC_DIR}/src/third_party/abseil-cpp)
  set (WEBRTC_COMPILE_FLAGS -DRTC_DISABLE_CHECK_MSG)
  set (WEBRTC_LINK_FLAGS optimized ${TTLIBS_ROOT}/../webrtc/release/obj/modules/audio_processing/teamtalk.lib
    debug ${TTLIBS_ROOT}/../webrtc/debug/obj/modules/audio_processing/teamtalk.lib)
else()
  set (WEBRTC_INCLUDE_DIR ${TTLIBS_ROOT}/build/webrtc/src/modules/audio_processing/include
    ${TTLIBS_ROOT}/build/webrtc/src
    ${TTLIBS_ROOT}/build/webrtc/src/third_party/abseil-cpp)
  # $ find toolchain/webrtc/obj/ -name *.a -exec nm -A -C {} \; | grep webrtc::rnn_vad::RnnBasedVad::RnnBasedVad
  set (WEBRTC_LINK_FLAGS
    ${TTLIBS_ROOT}/webrtc/obj/modules/audio_processing/libteamtalk.a)
endif()
