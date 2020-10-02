#include <catch.hpp>
#include <iostream>
#include <codec/MediaUtil.h>
#include <codec/WaveFile.h>

#include <modules/audio_processing/audio_buffer.h>
#include <modules/audio_processing/ns/noise_suppressor.h>
#include <modules/audio_processing/agc/agc.h>
#include <modules/audio_processing/gain_control_impl.h>

TEST_CASE("webrtc-audiobuf") {

    const int IN_SR = 16000, OUT_SR = 48000, IN_CH = 1, OUT_CH = 1;

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

TEST_CASE("webrtc-noise") {

    WavePCMFile infile, outfile;
    REQUIRE(infile.OpenFile(ACE_TEXT("in_noise_16khz.wav"), true));
    int IN_SR = infile.GetSampleRate(), IN_CH = infile.GetChannels();
    REQUIRE(outfile.NewFile(ACE_TEXT("out_noise.wav"), IN_SR, IN_CH));

    webrtc::StreamConfig in_cfg(IN_SR, IN_CH);
    webrtc::AudioBuffer in_ab(IN_SR, IN_CH, IN_SR, IN_CH, IN_SR, IN_CH), out_ab(IN_SR, IN_CH, IN_SR, IN_CH, IN_SR, IN_CH);
    std::vector<int16_t> in_buff(in_ab.num_frames() * IN_CH), out_buff(in_ab.num_frames() * IN_CH);

    webrtc::NsConfig nscfg;
    webrtc::NoiseSuppressor ns(nscfg, IN_SR, IN_CH);
    
    while (infile.ReadSamples(&in_buff[0], in_ab.num_frames()) > 0)
    {
        in_ab.CopyFrom(&in_buff[0], in_cfg);
        ns.Analyze(in_ab);
        ns.Process(&in_ab);
        in_ab.CopyTo(in_cfg, &out_buff[0]);
        REQUIRE(outfile.AppendSamples(&out_buff[0], in_ab.num_frames()));
    }
}

// webrtc::GainControlImpl queries this feature. Field trials is
// excluded by passing rtc_exclude_field_trial_default=true to GN.
namespace webrtc { namespace field_trial {
std::string FindFullName(const std::string& trial)
{
    std::cout << "Querying feature: " << trial << std::endl;
    return "Disabled";
}
} }

TEST_CASE("webrtc-agc")
{
    const int IN_SR = 48000, IN_CH = 1;
    const int IN_SAMPLES = 5 * IN_SR;
    
    WavePCMFile rawfile, agcfile;
    REQUIRE(rawfile.NewFile(ACE_TEXT("agcnone.wav"), IN_SR, IN_CH));
    REQUIRE(agcfile.NewFile(ACE_TEXT("agcfile.wav"), IN_SR, IN_CH));

    std::vector<int16_t> in_buff(IN_SAMPLES * IN_CH);
    media::AudioFrame af(media::AudioFormat(IN_SR, IN_CH), &in_buff[0], IN_SAMPLES);
    REQUIRE(GenerateTone(af, 0, 500));

    REQUIRE(rawfile.AppendSamples(&in_buff[0], IN_SAMPLES));

    webrtc::AudioBuffer ab(IN_SR, IN_CH, IN_SR, IN_CH, IN_SR, IN_CH);
    webrtc::StreamConfig in_cfg(IN_SR, IN_CH);
    std::vector<int16_t> agc_buff = in_buff;
    webrtc::GainControlImpl gain;
    gain.Initialize(IN_CH, IN_SR);
    REQUIRE(gain.set_mode(webrtc::GainControl::kAdaptiveDigital) == webrtc::AudioProcessing::kNoError);
    REQUIRE(gain.set_target_level_dbfs(1) == webrtc::AudioProcessing::kNoError);
    int index = 0;
    while (index + ab.num_frames() < IN_SAMPLES)
    {
        ab.CopyFrom(&in_buff[index * IN_CH], in_cfg);
        ab.SplitIntoFrequencyBands();
        REQUIRE(gain.AnalyzeCaptureAudio(ab) == webrtc::AudioProcessing::kNoError);
        REQUIRE(gain.ProcessCaptureAudio(&ab, false) == webrtc::AudioProcessing::kNoError);
        ab.MergeFrequencyBands();
        ab.CopyTo(in_cfg, &agc_buff[index]);
        index += ab.num_frames();
    }

    REQUIRE(agcfile.AppendSamples(&agc_buff[0], IN_SAMPLES));
}

TEST_CASE("webrtc-apm")
{
    webrtc::AudioProcessing* apm = webrtc::AudioProcessingBuilder().Create();
    apm->Initialize();
}
