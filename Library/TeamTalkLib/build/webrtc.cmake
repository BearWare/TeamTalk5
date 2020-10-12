set (WEBRTC_INCLUDE_DIR ${TTLIBS_ROOT}/build/webrtc/src
  ${TTLIBS_ROOT}/build/webrtc/src/third_party/abseil-cpp)

# Search for function definitions:
# $ find toolchain/webrtc/obj/ -name *.a -exec nm -A -C {} \; | grep webrtc::rnn_vad::RnnBasedVad::RnnBasedVad
set (WEBRTC_LINK_FLAGS ${TTLIBS_ROOT}/webrtc/obj/libwebrtc.a)
