if (MSVC)
  set (WEBRTC_LINK_FLAGS webrtc)
else()
  set (WEBRTC_INCLUDE_DIR ${TTLIBS_ROOT}/webrtc/include
    ${TTLIBS_ROOT}/webrtc/include/modules/audio_processing/include
    ${TTLIBS_ROOT}/webrtc/include/third_party/abseil-cpp)

  # $ find toolchain/webrtc/obj/ -name *.a -exec nm -A -C {} \; | grep webrtc::rnn_vad::RnnBasedVad::RnnBasedVad
  set (WEBRTC_LINK_FLAGS
    ${TTLIBS_ROOT}/webrtc/obj/modules/audio_processing/libteamtalk.a)
endif()
