#include "catch.hpp"

#include <ace/OS.h>

#include "TTUnitTest.h"

#include <bin/dll/Convert.h>

#if defined(ENABLE_MEDIAFOUNDATION)
#include <avstream/MFTransform.h>
#endif

#include <codec/WaveFile.h>
#include <ace/FILE_Connector.h>
#include <iostream>

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

    ACE_FILE_Connector con;
    ACE_FILE_IO outwavefile;
    REQUIRE(con.connect(outwavefile, ACE_FILE_Addr(L"hest_out.wav"),
                               0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT | O_BINARY, FILE_SHARE_READ | FILE_SHARE_WRITE) >= 0);
            
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
            outwavefile.send_n(mb->rd_ptr(), mb->length());
            mb->release();
        }
    }

    REQUIRE(transform->Drain());
    auto mbs = transform->RetrieveAudioFrames();
    for(auto& mb : mbs)
    {
        outwavefile.send_n(mb->rd_ptr(), mb->length());
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
    while ((samples = wavfile.ReadSamples(&buf[0], mfi.audioFmt.nSampleRate * 0.01)) > 0)
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
    while ((samples = wavfile.ReadSamples(&buf[0], mfi.audioFmt.nSampleRate * 0.01)) > 0)
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
    
    wos << L" channels " << input.channels << L" samplerate " << input.samplerate << L" bitrate " << bitrate << L"bps";
    std::wcout << (wos.str().c_str()) << std::endl;
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
    std::vector<int> samplerates = {8000, 12000, 16000, 24000, 32000, 48000}, channels = {1, 2}, bitrates = {16000, 32000, 64000, 128000, 256000}, fmts = { MP3 , WMA, AAC};

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
