#include <catch.hpp>
#include <iostream>
#include <codec/MediaUtil.h>
#include <codec/WaveFile.h>

#include <modules/audio_processing/audio_buffer.h>
#include <modules/audio_processing/ns/noise_suppressor.h>

TEST_CASE("webrtc-audiobuf") {

    const int IN_SR = 16000, OUT_SR = 24000, IN_CH = 1, OUT_CH = 1;

    const int IN_SAMPLES = 5 * IN_SR, OUT_SAMPLES = 5 * OUT_SR;
    std::vector<short> in_buff(IN_SAMPLES * IN_CH), out_buff(OUT_SAMPLES * OUT_CH);
    media::AudioFrame in_af(media::AudioFormat(IN_SR, IN_CH), &in_buff[0], IN_SAMPLES),
        out_af(media::AudioFormat(OUT_SR, OUT_CH), &out_buff[0], OUT_SAMPLES);

    WavePCMFile inwavfile, outwavfile;
    REQUIRE(inwavfile.NewFile(ACE_TEXT("in_tone.wav"), IN_SR, IN_CH));
    REQUIRE(outwavfile.NewFile(ACE_TEXT("out_tone.wav"), OUT_SR, OUT_CH));
    
    REQUIRE(GenerateTone(in_af, 0, 500));

    REQUIRE(inwavfile.AppendSamples(in_af.input_buffer, in_af.input_samples));

    webrtc::StreamConfig in_cfg(IN_SR, IN_CH), out_cfg(OUT_SR, OUT_CH);
    webrtc::AudioBuffer in_ab(IN_SR, IN_CH, IN_SR, IN_CH, OUT_SR, OUT_CH);
    
    size_t in_index = 0, out_index = 0;
    while (in_index + in_cfg.num_frames() < IN_SAMPLES)
    {
        in_ab.CopyFrom(&in_af.input_buffer[in_index * IN_CH], in_cfg);
        in_index += in_cfg.num_frames();

        in_ab.CopyTo(out_cfg, &out_af.input_buffer[out_index]);
        out_index += out_cfg.num_frames();
    }

    REQUIRE(outwavfile.AppendSamples(out_af.input_buffer, out_af.input_samples));
}
