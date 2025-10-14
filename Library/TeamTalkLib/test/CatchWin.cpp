#include "catch2/catch.hpp"

#include <ace/OS.h>

#include "TTUnitTest.h"

#include <bin/dll/Convert.h>

#if defined(ENABLE_MEDIAFOUNDATION)
#include <avstream/MFTransform.h>
#endif

#if defined (ENABLE_PORTAUDIO)
#include <avstream/PortAudioWrapper.h>
#include <pa_win_wasapi.h>
#endif

#include <codec/WaveFile.h>
#include <ace/FILE_Connector.h>
#include <iostream>
#include <sstream>

#if defined(ENABLE_MEDIAFOUNDATION)

TEST_CASE("TestAudioTransform")
{
    media::AudioFormat input(16000, 1), output(32000, 1);
            
    std::vector<short> buff(input.samplerate * input.channels);

    media::AudioFrame frame;
    frame.inputfmt = input;
    frame.input_buffer = &buff[0];
    frame.input_samples = input.samplerate;

    WavePCMFile inwavefile, outwavefile;
    REQUIRE(inwavefile.NewFile(ACE_TEXT("hest_in.wav"), input.samplerate, input.channels));
    REQUIRE(outwavefile.NewFile(ACE_TEXT("hest_out.wav"), output.samplerate, output.channels));

    int output_samples = int(output.samplerate * .1);

    auto transform = MFTransform::Create(input, output, output_samples);
    REQUIRE(transform.get() != nullptr);
    REQUIRE(transform->RetrieveAudioFrames().empty());

    int sampleindex = 0;
    for(int i=0;i<10;i++)
    {
        sampleindex = GenerateTone(frame, sampleindex, 600);
        inwavefile.AppendSamples(frame.input_buffer, frame.input_samples);

        auto mbs = transform->ProcessAudioResampler(frame);
        for (auto& mb : mbs)
        {
            media::AudioFrame outframe(mb);
            REQUIRE(outframe.inputfmt.samplerate == output.samplerate);
            REQUIRE(outframe.inputfmt.channels == output.channels);
            //Assert::AreEqual(output_samples, outframe.input_samples);
            outwavefile.AppendSamples(outframe.input_buffer, outframe.input_samples);
            mb->release();
        }
    }
}

TEST_CASE("TestAudioEncoderTransform")
{
    media::AudioFormat input(48000, 2);

    std::vector<short> buff(input.samplerate * input.channels);

    media::AudioFrame frame;
    frame.inputfmt = input;
    frame.input_buffer = &buff[0];
    frame.input_samples = input.samplerate;

    WavePCMFile inwavefile;
    REQUIRE(inwavefile.NewFile(ACE_TEXT("hest_in.wav"), input.samplerate, input.channels));

    MyFile outwavefile;
    REQUIRE(outwavefile.NewFile(ACE_TEXT("hest_out.wav")));
            
    auto transform = MFTransform::CreateMP3(input, 64000);
    REQUIRE(transform.get() != nullptr);
            
    REQUIRE(transform->RetrieveRawFrames().empty());
    REQUIRE(transform->GetInputType());
    REQUIRE(transform->GetOutputType());

    std::vector<char> header;
    WAVEFORMATEX* pWaveFormat = MediaTypeToWaveFormatEx(transform->GetOutputType(), header);
    REQUIRE(header.size());
    REQUIRE(pWaveFormat);

    WriteWaveFileHeader(outwavefile, pWaveFormat, int(header.size()));

    int sampleindex = 0;
    for(int i = 0; i<10; i++)
    {
        sampleindex = GenerateTone(frame, sampleindex, 600);
        inwavefile.AppendSamples(frame.input_buffer, frame.input_samples);

        auto mbs = transform->ProcessAudioEncoder(frame, false);
        for(auto& mb : mbs)
        {
            outwavefile.Write(mb->rd_ptr(), mb->length());
            mb->release();
        }
    }

    REQUIRE(transform->Drain());
    auto mbs = transform->RetrieveAudioFrames();
    for(auto& mb : mbs)
    {
        outwavefile.Write(mb->rd_ptr(), mb->length());
        mb->release();
    }

    UpdateWaveFileHeader(outwavefile);
}

TEST_CASE("TestAACEncoder")
{
    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = 2;
    mfi.audioFmt.nSampleRate = 48000;
    wcsncpy(mfi.szFileName, _T("aacinputfile.wav"), TT_STRLEN);
    mfi.uDurationMSec = 10*1000;

    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));

    WavePCMFile wavfile;
    REQUIRE(wavfile.OpenFile(mfi.szFileName, true));

    media::AudioFormat aacout(mfi.audioFmt.nSampleRate, mfi.audioFmt.nChannels);

    auto transform = MFTransform::CreateAAC(aacout, 192000, _T("aacoutputfile.wav"));
    REQUIRE(transform);

    std::vector<short> buf(mfi.audioFmt.nChannels * mfi.audioFmt.nSampleRate);
    media::AudioFrame frm(aacout, &buf[0], mfi.audioFmt.nSampleRate);
    int samples;
    while ((samples = wavfile.ReadSamples(&buf[0], int(mfi.audioFmt.nSampleRate * 0.01))) > 0)
    {
        frm.input_samples = samples;
        transform->ProcessAudioEncoder(frm, true);
        frm.sample_no += samples;
    }
}

TEST_CASE("TestWavEncoder")
{
    MediaFileInfo mfi = {};
    mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
    mfi.audioFmt.nChannels = 2;
    mfi.audioFmt.nSampleRate = 48000;
    wcsncpy(mfi.szFileName, _T("aacinputfile.wav"), TT_STRLEN);
    mfi.uDurationMSec = 10*1000;

    REQUIRE(TT_DBG_WriteAudioFileTone(&mfi, 500));

    WavePCMFile wavfile;
    REQUIRE(wavfile.OpenFile(mfi.szFileName, true));

    media::AudioFormat aacout(mfi.audioFmt.nSampleRate, mfi.audioFmt.nChannels);

    auto transform = MFTransform::CreateWav(aacout, media::AudioFormat(mfi.audioFmt.nSampleRate, mfi.audioFmt.nChannels), _T("aacoutputfile.wav"));
    REQUIRE(transform);

    std::vector<short> buf(mfi.audioFmt.nChannels * mfi.audioFmt.nSampleRate);
    media::AudioFrame frm(aacout, &buf[0], mfi.audioFmt.nSampleRate);
    int samples;
    while ((samples = wavfile.ReadSamples(&buf[0], int(mfi.audioFmt.nSampleRate * 0.01))) > 0)
    {
        frm.input_samples = samples;
        transform->ProcessAudioEncoder(frm, true);
        frm.sample_no += samples;
    }
}

enum {
    MP3, WMA, AAC };

void AudioTransformEncoder(const media::AudioFormat& input, int bitrate, int fmt)
{
    std::wostringstream wos;
    switch (fmt)
    {
    case MP3 :
        wos << L"MP3";
        break;
    case WMA :
        wos << L"WMA";
        break;
    case AAC :
        wos << L"AAC";
        break;
    }
    
    wos << L" channels " << input.channels << L" samplerate " << input.samplerate << L" bitrate " << bitrate << L"bps" << std::endl;
    MYTRACE(wos.str().c_str());
    wos.str(L"");

    std::vector<short> buff(input.samplerate * input.channels);

    media::AudioFrame frame;
    frame.inputfmt = input;
    frame.input_buffer = &buff[0];
    frame.input_samples = input.samplerate;

    WavePCMFile inwavefile;
    wos << L"hest_in_" << input.channels << L"_" << input.samplerate << L".wav";
    REQUIRE(inwavefile.NewFile(wos.str().c_str(), input.samplerate, input.channels));

    wos.str(L"");
    mftransform_t transform;
    switch (fmt)
    {
    case WMA:
        wos << L"hest_wma_" << input.channels << L"_" << input.samplerate << L"_" << bitrate << L"bps" << L".wav";
        transform = MFTransform::CreateWMA(input, bitrate, wos.str().c_str());
        break;
    case MP3 :
        wos << L"hest_mp3_" << input.channels << L"_" << input.samplerate << L"_" << bitrate << L"bps" << L".wav";
        transform = MFTransform::CreateMP3(input, bitrate, wos.str().c_str());
        break;
    case AAC :
        wos << L"hest_aac_" << input.channels << L"_" << input.samplerate << L"_" << bitrate << L"bps" << L".wav";
        transform = MFTransform::CreateAAC(input, bitrate, wos.str().c_str());
        break;
    }

    REQUIRE(transform.get() != nullptr);

    REQUIRE(transform->RetrieveRawFrames().empty());
    REQUIRE(transform->GetInputType());
    REQUIRE(transform->GetOutputType());

    int sampleindex = 0;
    for(int i = 0; i<10; i++)
    {
        sampleindex = GenerateTone(frame, sampleindex, 600);
        inwavefile.AppendSamples(frame.input_buffer, frame.input_samples);
        transform->ProcessAudioEncoder(frame, true);
    }
}

TEST_CASE("TestAudioTransformEncoder")
{
    std::vector<int> samplerates = soundsystem::standardSampleRates, channels = {1, 2},
        bitrates = {teamtalk::AFFToMP3Bitrate(teamtalk::AFF_MP3_16KBIT_FORMAT),
                    teamtalk::AFFToMP3Bitrate(teamtalk::AFF_MP3_32KBIT_FORMAT),
                    teamtalk::AFFToMP3Bitrate(teamtalk::AFF_MP3_64KBIT_FORMAT),
                    teamtalk::AFFToMP3Bitrate(teamtalk::AFF_MP3_128KBIT_FORMAT), 
                    teamtalk::AFFToMP3Bitrate(teamtalk::AFF_MP3_256KBIT_FORMAT),
                    teamtalk::AFFToMP3Bitrate(teamtalk::AFF_MP3_320KBIT_FORMAT)},
        fmts = { MP3 , WMA, AAC};

    for (auto sr :samplerates)
        for (auto ch : channels)
            for (auto br : bitrates)
                for (auto fm : fmts)
                {
                    switch (fm)
                    {
                    case WMA :
                        switch(sr)
                        {
                        case 12000 :
                        case 24000 :
                        case 48000 :
                            continue;
                        }
                        break;
                    case AAC :
                        switch(sr)
                        {
                        case 48000 :
                            break;
                        default :
                            continue;
                        }
                    }

                    media::AudioFormat input(sr, ch);
                    AudioTransformEncoder(input, br, fm);
                }
}

TEST_CASE("TestAudioTransformMP3")
{
    media::AudioFormat input(48000, 2);

    std::vector<short> buff(input.samplerate * input.channels);

    media::AudioFrame frame;
    frame.inputfmt = input;
    frame.input_buffer = &buff[0];
    frame.input_samples = input.samplerate;

    WavePCMFile inwavefile;
    REQUIRE(inwavefile.NewFile(ACE_TEXT("hest_in.wav"), input.samplerate, input.channels));

    auto transformmp3 = MFTransform::CreateMP3(input, 64000, L"hest_mp3.wav");
    REQUIRE(transformmp3.get() != nullptr);

    REQUIRE(transformmp3->RetrieveRawFrames().empty());
    REQUIRE(transformmp3->GetInputType());
    REQUIRE(transformmp3->GetOutputType());

    int sampleindex = 0;
    for(int i = 0; i<10; i++)
    {
        sampleindex = GenerateTone(frame, sampleindex, 600);
        inwavefile.AppendSamples(frame.input_buffer, frame.input_samples);
        transformmp3->ProcessAudioEncoder(frame, true);
    }
}
#endif


#if defined (ENABLE_PORTAUDIO) && 0

int duplexSamples = 0;
int Foo_StreamDuplexCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* pUserData)
{
    memcpy(outputBuffer, inputBuffer, framesPerBuffer * sizeof(short));
    duplexSamples += framesPerBuffer;
    return paContinue;
}

// This unit-test calls Pa_Terminate() which will break PortAudioWrapper-class (singleton)
TEST_CASE("PortAudio_ExclusiveMode")
{
    if (GITHUBSKIP)
    {
        std::cout << "Skipping \"PortAudio_ExclusiveMode\"... No WASAPI" << std::endl;
        return;
    }

    PaError err = Pa_Initialize();

    PaDeviceIndex inputdeviceid = -1, outputdeviceid = -1;
    PaHostApiIndex hostApi = Pa_HostApiTypeIdToHostApiIndex(paWASAPI);
    if (hostApi != paHostApiNotFound)
    {
        const PaHostApiInfo* hostapi = Pa_GetHostApiInfo(hostApi);
        if (hostapi)
        {
            inputdeviceid = hostapi->defaultInputDevice;
            outputdeviceid = hostapi->defaultOutputDevice;
        }
    }
    REQUIRE(inputdeviceid >= 0);
    REQUIRE(outputdeviceid >= 0);

    const PaDeviceInfo* ininfo = Pa_GetDeviceInfo(inputdeviceid);
    const PaDeviceInfo* outinfo = Pa_GetDeviceInfo(outputdeviceid);
    REQUIRE(ininfo);
    REQUIRE(outinfo);
    PaStreamParameters inputParameters = {}, outputParameters = {};

    inputParameters.device = inputdeviceid;
    inputParameters.channelCount = 1;
    inputParameters.hostApiSpecificStreamInfo = nullptr;
    inputParameters.sampleFormat = paInt16;
    inputParameters.suggestedLatency = ininfo->defaultLowInputLatency;

    PaWasapiStreamInfo outputWasapi = {};
    outputWasapi.size = sizeof(outputWasapi);
    outputWasapi.hostApiType = paWASAPI;
    outputWasapi.version = 1;
    outputWasapi.flags = paWinWasapiAutoConvert;

    outputParameters.device = outputdeviceid;
    outputParameters.channelCount = 1;
    outputParameters.hostApiSpecificStreamInfo = &outputWasapi;
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = outinfo->defaultLowOutputLatency;

    PaStream* stream;
    err = Pa_OpenStream(&stream, &inputParameters, &outputParameters,
        ininfo->defaultSampleRate, uint32_t(ininfo->defaultSampleRate * .04),
        paClipOff, Foo_StreamDuplexCallback, static_cast<void*> (0));

    REQUIRE(err == paNoError);
    REQUIRE(stream);

    REQUIRE(Pa_StartStream(stream) == paNoError);
    while (duplexSamples < ininfo->defaultSampleRate * 50)
    {
        Pa_Sleep(1000);
    }

    Pa_Terminate();
}
#endif
