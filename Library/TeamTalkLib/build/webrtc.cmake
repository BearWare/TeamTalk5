set (WEBRTC_INCLUDE_DIR ${TTLIBS_ROOT}/build/webrtc/src/modules/audio_processing/include
  ${TTLIBS_ROOT}/build/webrtc/src
  ${TTLIBS_ROOT}/build/webrtc/src/third_party/abseil-cpp)

# $ find toolchain/webrtc/obj/ -name *.a -exec nm -A -C {} \; | grep webrtc::rnn_vad::RnnBasedVad::RnnBasedVad
set (WEBRTC_LINK_FLAGS
    ${TTLIBS_ROOT}/webrtc/obj/modules/audio_processing/libteamtalk.a)
