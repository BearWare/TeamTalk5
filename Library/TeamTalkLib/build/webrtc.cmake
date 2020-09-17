option (WEBRTC "Build using WebRTC libraries" ON)

set (WEBRTC_INCLUDE_DIR ${TTLIBS_ROOT}/build/webrtc/src
  ${TTLIBS_ROOT}/build/webrtc/src/third_party/abseil-cpp)

set (WEBRTC_LINK_FLAGS
  ${TTLIBS_ROOT}/webrtc/obj/common_audio/libcommon_audio.a
  ${TTLIBS_ROOT}/webrtc/obj/common_audio/libcommon_audio_sse2.a
  ${TTLIBS_ROOT}/webrtc/obj/common_audio/libcommon_audio_c.a
  ${TTLIBS_ROOT}/webrtc/obj/modules/audio_processing/libaudio_buffer.a
  ${TTLIBS_ROOT}/webrtc/obj/rtc_base/libchecks.a
  ${TTLIBS_ROOT}/webrtc/obj/rtc_base/memory/libaligned_malloc.a
  ${TTLIBS_ROOT}/webrtc/obj/system_wrappers/libsystem_wrappers.a)
  
