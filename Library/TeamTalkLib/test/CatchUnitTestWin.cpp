#include "catch.hpp"

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

enum {
    MP3, WMA };

void AudioTransformEncoder(const media::AudioFormat& input, int bitrate, int fmt)
{
    std::wostringstream wos;
    wos << (fmt == MP3? L"MP3" : L"WMA") << L" channels " << input.channels << L" samplerate " << input.samplerate << L" bitrate " << bitrate << L"bps";
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
    std::vector<int> samplerates = {8000, 12000, 16000, 24000, 32000, 48000}, channels = {1, 2}, bitrates = {16000, 32000, 64000, 128000, 256000}, fmts = { MP3 , WMA};

    for (auto sr :samplerates)
        for (auto ch : channels)
            for (auto br : bitrates)
                for (auto fm : fmts)
                {
                    if (fm == WMA)
                        switch(sr)
                        {
                        case 12000 :
                        case 24000 :
                        case 48000 :
                            continue;
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
