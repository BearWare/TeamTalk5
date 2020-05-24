#include "CppUnitTest.h"

#if defined(ENABLE_MEDIAFOUNDATION)
#include <avstream/MFStreamer.h>
#include <avstream/MFCapture.h>
#include <avstream/MFTransform.h>
#endif
#if defined(ENABLE_DSHOW)
#include <avstream/WinMedia.h>
#include <avstream/LibVidCap.h>
#endif

#include <avstream/MediaPlayback.h>
#include <avstream/AudioInputStreamer.h>

#include <codec/WaveFile.h>
#include <codec/BmpFile.h>
#include <codec/VpxEncoder.h>
#include <codec/VpxDecoder.h>
#include <codec/OpusEncoder.h>


#include <myace/MyACE.h>
#include <ace/FILE_Connector.h>

#include <teamtalk/CodecCommon.h>
#include <teamtalk/client/AudioThread.h>

#include <TeamTalk.h>
#if defined(ENABLE_TEAMTALKPRO)
#include <TeamTalkSrv.h>
#endif

#include <bin/dll/Convert.h>

#include "TTUnitTest.h"

#include <mutex>
#include <thread>
#include <condition_variable>
#include <sstream>
#include <fstream>
#include <cstdio>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std::placeholders;

namespace UnitTest
{
    std::mutex done;
    std::condition_variable cv;

    TEST_CLASS(UnitTest1)
    {
        TEST_CLASS_INITIALIZE(InitClass)
        {
#if defined(ENABLE_MEDIAFOUNDATION)
            TTInstance* ttInit = TT_InitTeamTalkPoll(); //MF_Startup()
            TT_CloseTeamTalk(ttInit);
#endif
        }

        TEST_CLASS_CLEANUP(TearDownClass)
        {
        }

    public:

        TEST_METHOD(TestAudioStream)
        {

            class MediaStreamListener
            {
                WavePCMFile wavfile;
                MediaStreamOutput out_prop;
            public:
                void setOutput(const MediaStreamOutput& o) { out_prop = o; }
                bool MediaStreamAudioCallback(media::AudioFrame& audio_frame,
                                              ACE_Message_Block* mb_audio)
                {
                    wavfile.AppendSamples(audio_frame.input_buffer, audio_frame.input_samples);
                    return false;
                }
                void MediaStreamStatusCallback(const MediaFileProp& mfp,
                                               MediaStreamStatus status)
                {
                    switch (status)
                    {
                    case MEDIASTREAM_STARTED :
                        wavfile.NewFile(L"output.wav", out_prop.audio.samplerate, out_prop.audio.channels);
                        break;
                    case MEDIASTREAM_ERROR :
                        break;
                    case MEDIASTREAM_FINISHED :
                        cv.notify_all();
                        break;
                    }
                }
            } listener;

            //ACE_TString url = L"https://bearware.dk/temp/giana_10sec.wma";
            ACE_TString url = L"z:\\Media\\giana_10sec.wma";
            //ACE_TString url = L"z:\\Media\\tone.wav";
            //ACE_TString url = L"z:\\Media\\darwin2_441khz.wav";

            MediaFileProp in_prop;
            Assert::IsTrue(GetMediaFileProp(url, in_prop));

            MediaStreamOutput out_prop(media::AudioFormat(48000, 2), int(48000 * .04));
            listener.setOutput(out_prop);
            
#if defined(ENABLE_DSHOW)
            DSWrapperThread streamer(&listener);
#endif
#if defined(ENABLE_MEDIAFOUNDATION)
            MFStreamer streamer;
            streamer.RegisterAudioCallback(std::bind(&MediaStreamListener::MediaStreamAudioCallback, &listener, _1, _2), true);
            streamer.RegisterStatusCallback(std::bind(&MediaStreamListener::MediaStreamStatusCallback, &listener, _1, _2), true);
#endif

            Assert::IsTrue(streamer.OpenFile(in_prop.filename, out_prop));

            Assert::IsTrue(streamer.StartStream());

            std::unique_lock<std::mutex> lk(done);
            cv.wait(lk);
        }

        TEST_METHOD(TestVideoStream)
        {
            class MediaStreamListener
            {
                WavePCMFile wavfile;
                MediaStreamOutput out_prop;
#if defined(ENABLE_MEDIAFOUNDATION)
                mftransform_t transform;
#endif
            public:
                void setOutput(const MediaStreamOutput& o) { out_prop = o; }
                bool MediaStreamVideoCallback(media::VideoFrame& video_frame,
                    ACE_Message_Block* mb_video)
                {
                    static int n_bmp = 0;
                    std::wostringstream os;
                    
                    os << L"video_";
                    os.fill('0');
                    os.width(20);
                    //os << ++n_bmp;
                    os << video_frame.timestamp;
                    os << L".bmp";
                    switch (video_frame.fourcc)
                    {
                    case media::FOURCC_RGB24 :
                    case media::FOURCC_RGB32 :
                        WriteBitmap(os.str().c_str(), media::VideoFormat(video_frame.width, video_frame.height, video_frame.fourcc), video_frame.frame, video_frame.frame_length);
                        break;
                    default :
#if defined(ENABLE_MEDIAFOUNDATION)
                        if (!transform.get())
                            transform = MFTransform::Create(media::VideoFormat(video_frame.width, video_frame.height, video_frame.fourcc), media::FOURCC_RGB32);
                        Assert::IsTrue(transform.get());
                        Assert::IsTrue(transform->SubmitSample(video_frame), L"Submit frame");
                        auto mbs = transform->RetrieveVideoFrames();
                        Assert::IsTrue(mbs.size()>0, L"Transformed frame");
                        for (auto& mb : mbs)
                        {
                            media::VideoFrame frame(mb);
                            WriteBitmap(os.str().c_str(), media::VideoFormat(video_frame.width, video_frame.height, media::FOURCC_RGB32), frame.frame, frame.frame_length);
                            mb->release();
                        }
#endif
                        break;
                    }

                    return false;
                }
                bool MediaStreamAudioCallback(media::AudioFrame& audio_frame,
                    ACE_Message_Block* mb_audio)
                {
                    wavfile.AppendSamples(audio_frame.input_buffer, audio_frame.input_samples);
                    return false;
                }
                void MediaStreamStatusCallback(const MediaFileProp& mfp,
                    MediaStreamStatus status)
                {
                    switch(status)
                    {
                    case MEDIASTREAM_STARTED:
                        wavfile.NewFile(L"output.wav", out_prop.audio.samplerate, out_prop.audio.channels);
                        break;
                    case MEDIASTREAM_ERROR:
                        break;
                    case MEDIASTREAM_FINISHED:
                        cv.notify_all();
                        break;
                    }
                }
            } listener;

            ACE_TString url;
            //url = L"https://bearware.dk/temp/OOBEMovie_10sec.wmv";
            url = L"z:\\Media\\MVI_2526.AVI";
            //url = L"z:\\Media\\OOBEMovie.wmv";
            //url = L"z:\\Media\\OOBEMovie_10sec.wmv";
            //url = L"z:\\Media\\Seinfeld.avi";
            //url = L"z:\\Media\\Wildlife.wmv";

            MediaFileProp in_prop;
            Assert::IsTrue(GetMediaFileProp(url, in_prop));

#if defined(ENABLE_DSHOW)
            MediaStreamOutput out_prop(media::AudioFormat(48000, 2), int(48000 * .12), media::FOURCC_RGB32);
#else
            MediaStreamOutput out_prop(media::AudioFormat(48000, 2), int(48000 * .12), media::FOURCC_RGB32);
            //MediaStreamOutput out_prop(media::AudioFormat(), int(48000 * .12), media::FOURCC_RGB32);
#endif
            listener.setOutput(out_prop);

#if defined(ENABLE_MEDIAFOUNDATION)
            MFStreamer streamer;
            streamer.RegisterAudioCallback(std::bind(&MediaStreamListener::MediaStreamAudioCallback, &listener, _1, _2), true);
            streamer.RegisterVideoCallback(std::bind(&MediaStreamListener::MediaStreamVideoCallback, &listener, _1, _2), true);
            streamer.RegisterStatusCallback(std::bind(&MediaStreamListener::MediaStreamStatusCallback, &listener, _1, _2), true);
#endif
#if defined(ENABLE_DSHOW)
            DSWrapperThread streamer(&listener);
#endif
            Assert::IsTrue(streamer.OpenFile(in_prop.filename, out_prop));

            Assert::IsTrue(streamer.StartStream());

            std::unique_lock<std::mutex> lk(done);
            cv.wait(lk);
        }

        void VideoCaptureTest(ACE_TString szDev, const media::VideoFormat& fmt)
        {
            vidcap::videocapture_t dev = vidcap::VideoCapture::Create();

            std::wostringstream os;
            os << L"Testing ";
            os << fmt.width << L"x" << fmt.height;
            os << "@ " << (fmt.fps_numerator / fmt.fps_denominator);
#if defined(ENABLE_MEDIAFOUNDATION)
            os << L" - " << FourCCToString(fmt.fourcc).c_str();
#endif
            os << std::endl;
            Logger::WriteMessage(os.str().c_str());

#if defined(ENABLE_MEDIAFOUNDATION)
            std::vector<media::FourCC> transforms = { media::FOURCC_RGB24, media::FOURCC_RGB32 };
#else
            std::vector<media::FourCC> transforms = {};
#endif
            std::set<media::FourCC> outputs;
            outputs.insert(transforms.begin(), transforms.end());
            outputs.insert(fmt.fourcc);

            class MyClass
            {
                std::vector<media::VideoFormat> m_receivedfmts;
                std::set<media::FourCC> m_outputs;
                vidcap::videocapture_t& m_dev;

            public:
                MyClass(vidcap::videocapture_t& dev, std::set<media::FourCC> outputs) : m_outputs(outputs), m_dev(dev)
                {
                }

                bool OnVideoCaptureCallback(media::VideoFrame& video_frame,
                                            ACE_Message_Block* /*mb_video*/)
                {
                    media::VideoFormat fmt = m_dev->GetVideoCaptureFormat();

                    std::wostringstream os;
                    os << L"Got video frame ";
                    os << video_frame.width << L"x" << video_frame.height << L"@" << fmt.fps_numerator / fmt.fps_denominator;
                    os << L" fcc" << video_frame.fourcc << " size " << video_frame.frame_length << L" diff time " << (GETTIMESTAMP() - video_frame.timestamp);
                    os << std::endl;
                    Logger::WriteMessage(os.str().c_str());

                    os.str(L"");

                    static int n_bmp = 0;

                    switch(video_frame.fourcc)
                    {
                    case media::FOURCC_RGB24 :
                    case media::FOURCC_RGB32 :
                        os << L"video_" << video_frame.width << L"x" << video_frame.height << L"@" << fmt.fps_numerator / fmt.fps_denominator
                           << L"_fcc" << video_frame.fourcc << L"_" << ++n_bmp << L".bmp";
                        WriteBitmap(os.str().c_str(), video_frame.GetVideoFormat(), video_frame.frame, video_frame.frame_length);
                        break;
                    }

                    if (std::find(m_receivedfmts.begin(), m_receivedfmts.end(), video_frame.GetVideoFormat()) != m_receivedfmts.end())
                    {
                        if (m_receivedfmts.size() == m_outputs.size())
                            cv.notify_all();
                    }
                    else
                        m_receivedfmts.push_back(video_frame.GetVideoFormat());

                    return false;
                }

            } listener(dev, outputs);


            Assert::IsTrue(dev->InitVideoCapture(szDev, fmt));
            for(auto f : outputs)
                Assert::IsTrue(dev->RegisterVideoFormat(std::bind(&MyClass::OnVideoCaptureCallback, &listener, _1, _2), f));
            Assert::IsTrue(dev->StartVideoCapture());


            std::mutex mtx;
            std::unique_lock<std::mutex> lck(mtx);
            //cv.wait_for(lck, std::chrono::seconds(10));
            cv.wait(lck);

            dev->StopVideoCapture();
        }

        TEST_METHOD(TestVideoCapture)
        {
            auto devs = vidcap::VideoCapture::Create()->GetDevices();

            std::wostringstream os;
            for(auto dev : devs)
            {
                os << L"Device: " << dev.devicename.c_str();
                Logger::WriteMessage(os.str().c_str());
                for(auto fmt : dev.vidcapformats)
                {
                    os.str(L"");
                    os << L"\t" << fmt.width << L"x" << fmt.height;
                    os << "@ " << (fmt.fps_numerator / fmt.fps_denominator);
#if defined(ENABLE_MEDIAFOUNDATION)
                    os << L" - " << FourCCToString(fmt.fourcc).c_str();
#endif
                    os << std::endl;
                    Logger::WriteMessage(os.str().c_str());
                }
            }

            Assert::IsTrue(devs.size());

            std::vector<media::VideoFormat> test_fmts;
            std::copy_if(devs[0].vidcapformats.begin(), devs[0].vidcapformats.end(), 
                std::back_inserter(test_fmts), [](media::VideoFormat f) {return f.fourcc == media::FOURCC_I420; });

            for(auto fmt : test_fmts)
            {
                VideoCaptureTest(devs[0].deviceid.c_str(), fmt);
            }
        }


        TEST_METHOD(TestVideoEncode)
        {
            auto device = vidcap::VideoCapture::Create();
            VpxEncoder encoder;
            VpxDecoder decoder;

            media::VideoFormat fmt(640, 480, 30, 1, media::FOURCC_I420);

            auto devs = device->GetDevices();
            std::wostringstream os;
            for(auto a : devs)
            {
                os << L"Device: " << a.devicename.c_str();
                Logger::WriteMessage(os.str().c_str());
                for(auto f : a.vidcapformats)
                {
                    os.str(L"");
                    os << L"\t" << f.width << L"x" << f.height;
                    os << "@ " << (f.fps_numerator / f.fps_denominator);
                    switch(f.fourcc)
                    {
                    case media::FOURCC_I420:
                        os << L" - I420";
                        break;
                    case media::FOURCC_RGB32:
                        os << L" - RGB32";
                        break;
                    case media::FOURCC_YUY2:
                        os << L" - YUY2";
                        break;
                    }
                    os << std::endl;
                    Logger::WriteMessage(os.str().c_str());
                }
            }
            Assert::IsTrue(devs.size());

            class MyListener
            {
                media::VideoFormat& fmt;
                VpxEncoder& encoder;
                VpxDecoder& decoder;
#if defined(ENABLE_MEDIAFOUNDATION)
                mftransform_t rgb32_transform;
#endif
                int encoded = 0, decoded = 0;

            public:
                MyListener (media::VideoFormat& format, VpxEncoder& enc, VpxDecoder& dec) : fmt(format), encoder(enc), decoder(dec)
                {
#if defined(ENABLE_MEDIAFOUNDATION)
                    rgb32_transform = MFTransform::Create(media::VideoFormat(format.width, format.height, media::FOURCC_I420), media::FOURCC_RGB32);
                    Assert::IsTrue(rgb32_transform.get() != nullptr);
#endif
                }

                bool OnVideoCaptureCallback(media::VideoFrame& video_frame,
                    ACE_Message_Block* mb_video)
                {
                    std::wostringstream os;
                    os << L"Got video frame " << video_frame.width << L"x" << video_frame.height << std::endl;

                    Logger::WriteMessage(os.str().c_str());

                    switch (video_frame.fourcc)
                    {
                    case media::FOURCC_I420 :
                        //Assert::AreEqual(int(VPX_CODEC_OK), int(encoder.Encode(video_frame.frame, VPX_IMG_FMT_I420, 1, !video_frame.top_down, video_frame.timestamp, VPX_DL_BEST_QUALITY)));
                        //encoded += 1;
                        break;
                    case media::FOURCC_RGB32 :
                        Assert::AreEqual(int(VPX_CODEC_OK), int(encoder.EncodeRGB32(video_frame.frame, video_frame.frame_length, !video_frame.top_down, video_frame.timestamp, VPX_DL_BEST_QUALITY)));
                        encoded += 1;
                        break;
                    case media::FOURCC_NV12 :
                        //Assert::AreEqual(int(VPX_CODEC_OK), int(encoder.Encode(video_frame.frame, VPX_IMG_FMT_NV1, 1, video_frame.timestamp, VPX_DL_BEST_QUALITY)));
                        break;
                    }


                    if (encoded != decoded)
                    {
                        const char* enc_data;
                        int len;

                        while(enc_data = encoder.GetEncodedData(len))
                        {
                            static int dec_img = 0;
                            os.str(L"");
                            Assert::AreEqual(int(VPX_CODEC_OK), decoder.PushDecoder(enc_data, len));

#if defined(ENABLE_MEDIAFOUNDATION)
                                media::VideoFrame i420_frame;
                                do
                                {
                                    i420_frame = decoder.GetImage();
                                    if (i420_frame.IsValid())
                                    {
                                        Assert::IsTrue(rgb32_transform->SubmitSample(i420_frame));

                                        auto mbs = rgb32_transform->RetrieveVideoFrames();
                                        Assert::AreEqual(int(1), int(mbs.size()), L"Got frame");
                                        for (auto& mb : mbs)
                                        {
                                            media::VideoFrame rgb32_frame(mb);
                                            decoded += 1;
                                            os << L"decode_i420_";
                                            os.fill('0');
                                            os.width(20);
                                            os << ++dec_img << L".bmp";
                                            WriteBitmap(os.str().c_str(), rgb32_frame.GetVideoFormat(), rgb32_frame.frame, rgb32_frame.frame_length);
                                            mb->release();
                                        }
                                    }
                                }
                                while(i420_frame.IsValid());
#else
                                std::vector<char> rgb32_frame(RGB32_BYTES(video_frame.width, video_frame.height));
                                while(decoder.GetRGB32Image(&rgb32_frame[0], rgb32_frame.size()))
                                {
                                    decoded += 1;
                                    os << L"decode_";
                                    os.fill('0');
                                    os.width(20);
                                    os << ++dec_img << L".bmp";
                                    WriteBitmap(os.str().c_str(), media::VideoFormat(video_frame.width, video_frame.height, media::FOURCC_RGB32), &rgb32_frame[0], rgb32_frame.size());
                                }
#endif
                        }
                        Assert::AreEqual(encoded, decoded);
                    }

                    return false;
                }

            } listener(fmt, encoder, decoder);

            Assert::IsTrue(encoder.Open(fmt.width, fmt.height, 1024, fmt.fps_numerator/fmt.fps_denominator));
            Assert::IsTrue(decoder.Open(fmt.width, fmt.height));

            Assert::IsTrue(device->InitVideoCapture(devs[0].deviceid.c_str(), fmt));
            Assert::IsTrue(device->RegisterVideoFormat(std::bind(&MyListener::OnVideoCaptureCallback, &listener, _1, _2), fmt.fourcc));
            Assert::IsTrue(device->RegisterVideoFormat(std::bind(&MyListener::OnVideoCaptureCallback, &listener, _1, _2), media::FOURCC_I420));
            Assert::IsTrue(device->RegisterVideoFormat(std::bind(&MyListener::OnVideoCaptureCallback, &listener, _1, _2), media::FOURCC_RGB32));

            Assert::IsTrue(device->StartVideoCapture());


            std::mutex mtx;
            std::unique_lock<std::mutex> lck(mtx);
            std::condition_variable cv;
            cv.wait_for(lck, std::chrono::seconds(10));
            //cv.wait(lck);

            device->StopVideoCapture();
        }

#if defined(ENABLE_MEDIAFOUNDATION)
        TEST_METHOD(TestVideoTransform)
        {
            std::wostringstream os;
            const auto WIDTH=1600, HEIGHT=1200, FPS_N = 30, FPS_D = 1;
            media::VideoFormat rgb24fmt(WIDTH, HEIGHT, FPS_N, FPS_D, media::FOURCC_RGB24);
            media::VideoFormat rgb32fmt(WIDTH, HEIGHT, FPS_N, FPS_D, media::FOURCC_RGB32);
            media::VideoFormat i420fmt(WIDTH, HEIGHT, FPS_N, FPS_D, media::FOURCC_I420);
            media::VideoFormat yuy2fmt(WIDTH, HEIGHT, FPS_N, FPS_D, media::FOURCC_YUY2);

            std::vector<char> buff_rgb32(RGB32_BYTES(rgb32fmt.width, rgb32fmt.height));
            for (int y=0; y < rgb32fmt.height / 2; ++y)
            {
                for(int x=0;x<rgb32fmt.width;++x)
                {
                    int pos = y * rgb32fmt.width * 4;
                    pos += x * 4;
                    buff_rgb32[pos] = 0;
                    buff_rgb32[pos + 1] = (x < rgb32fmt.width/2 ? 255 : 0);
                    buff_rgb32[pos + 2] = 0;
                    buff_rgb32[pos + 3] = 0;
                }
            }
            for(int y = rgb32fmt.height / 2; y < rgb32fmt.height; ++y)
            {
                for(int x = 0; x<rgb32fmt.width; ++x)
                {
                    int pos = y * rgb32fmt.width * 4;
                    pos += x * 4;
                    buff_rgb32[pos] = (x >= rgb32fmt.width / 2 ? 255 : 0);
                    buff_rgb32[pos + 1] = 0;
                    buff_rgb32[pos + 2] = (x < rgb32fmt.width / 2 ? 255 : 0);
                    buff_rgb32[pos + 3] = 0;
                }
            }

            media::VideoFrame rgb32frame(rgb32fmt, &buff_rgb32[0], int(buff_rgb32.size()));
            rgb32frame.timestamp = 12;

            os.str(L"");
            os << L"RGB32 " << rgb32frame.width << L"x" << rgb32frame.height << L" is size: " << rgb32frame.frame_length << std::endl;
            Logger::WriteMessage(os.str().c_str());

            // Convert RGB32 to RGB24
            auto mft_rgb32_to_rgb24 = MFTransform::Create(rgb32fmt, media::FOURCC_RGB24);
            Assert::IsTrue(mft_rgb32_to_rgb24.get());
            Assert::IsTrue(mft_rgb32_to_rgb24->SubmitSample(rgb32frame));
            //CComPtr<IMFSample> pSample = mft_rgb32_to_rgb24->RetrieveSample();
            auto mbs = mft_rgb32_to_rgb24->RetrieveVideoFrames();
            Assert::AreEqual(1, int(mbs.size()));
            ACE_Message_Block* mb = mbs[0];
            Assert::IsTrue(mb != nullptr);
            media::VideoFrame rgb24frame(mb);
            Assert::AreEqual(rgb32fmt.width, rgb24frame.width);
            Assert::AreEqual(rgb32fmt.height, rgb24frame.height);
            Assert::AreEqual(int(rgb24fmt.fourcc), int(rgb24frame.fourcc));
            WriteBitmap(L"myvideo_rgb24.bmp", rgb24frame.GetVideoFormat(),
                rgb24frame.frame, rgb24frame.frame_length);
            os.str(L"");
            os << L"RGB24 " << rgb24frame.width << L"x" << rgb24frame.height << L" is size: " << rgb24frame.frame_length << std::endl;
            Logger::WriteMessage(os.str().c_str());
            mb->release();

            // Convert RGB32 to I420
            auto mft_rgb32_to_i420 = MFTransform::Create(rgb32fmt, media::FOURCC_I420);
            Assert::IsTrue(mft_rgb32_to_i420.get());
            Assert::IsTrue(mft_rgb32_to_i420->SubmitSample(rgb32frame));
            mbs = mft_rgb32_to_i420->RetrieveVideoFrames();
            Assert::AreEqual(int(1), int(mbs.size()));
            mb = mbs[0];
            Assert::IsTrue(mb != nullptr);
            media::VideoFrame i420frame(mb);
            Assert::AreEqual(rgb32fmt.width, i420frame.width);
            Assert::AreEqual(rgb32fmt.height, i420frame.height);
            Assert::IsTrue(i420frame.fourcc == media::FOURCC_I420);
            os.str(L"");
            os << L"I420 " << i420frame.width << L"x" << i420frame.height << L" is size: " << i420frame.frame_length << std::endl;
            Logger::WriteMessage(os.str().c_str());

            // Convert I420 to RGB32
            auto mft_i420_to_rgb32 = MFTransform::Create(i420fmt, media::FOURCC_RGB32);
            Assert::IsTrue(mft_i420_to_rgb32.get());
            Assert::IsTrue(mft_i420_to_rgb32->SubmitSample(i420frame));
            mb->release();
            mbs = mft_i420_to_rgb32->RetrieveVideoFrames();
            Assert::AreEqual(int(1), int(mbs.size()));
            mb = mbs[0];
            Assert::IsTrue(mb != nullptr);
            media::VideoFrame rgb32frame_ret(mb);
            Assert::AreEqual(rgb32fmt.width, rgb32frame_ret.width);
            Assert::AreEqual(rgb32fmt.height, rgb32frame_ret.height);
            Assert::AreEqual(int(rgb32fmt.fourcc), int(rgb32frame_ret.fourcc));
            WriteBitmap(L"myvideo_i420_rgb32.bmp", rgb32frame_ret.GetVideoFormat(),
                        rgb32frame_ret.frame, rgb32frame_ret.frame_length);
            mb->release();

            // Convert RGB32 to YUY2
            auto mft_rgb32_to_yuy2 = MFTransform::Create(rgb32fmt, media::FOURCC_YUY2);
            Assert::IsTrue(mft_rgb32_to_yuy2.get());
            Assert::IsTrue(mft_rgb32_to_yuy2->SubmitSample(rgb32frame));
            mbs = mft_rgb32_to_yuy2->RetrieveVideoFrames();
            Assert::AreEqual(int(1), int(mbs.size()));
            mb = mbs[0];
            Assert::IsTrue(mb != nullptr);
            media::VideoFrame yuy2frame(mb);
            Assert::AreEqual(rgb32fmt.width, yuy2frame.width);
            Assert::AreEqual(rgb32fmt.height, yuy2frame.height);
            Assert::IsTrue(yuy2frame.fourcc == media::FOURCC_YUY2);
            os.str(L"");
            os << L"YUY2 " << yuy2frame.width << L"x" << yuy2frame.height << L" is size: " << yuy2frame.frame_length << std::endl;
            Logger::WriteMessage(os.str().c_str());

            // Convert YUY2 to RGB32
            auto mft_yuy2_to_rgb32 = MFTransform::Create(yuy2fmt, media::FOURCC_RGB32);
            Assert::IsTrue(mft_yuy2_to_rgb32.get());
            Assert::IsTrue(mft_yuy2_to_rgb32->SubmitSample(yuy2frame));
            mb->release();
            mbs = mft_yuy2_to_rgb32->RetrieveVideoFrames();
            Assert::AreEqual(int(1), int(mbs.size()));
            mb = mbs[0];
            Assert::IsTrue(mb != nullptr);
            rgb32frame_ret = media::VideoFrame(mb);
            Assert::AreEqual(rgb32fmt.width, rgb32frame_ret.width);
            Assert::AreEqual(rgb32fmt.height, rgb32frame_ret.height);
            Assert::AreEqual(int(rgb32fmt.fourcc), int(rgb32frame_ret.fourcc));
            WriteBitmap(L"myvideo_yuy2_rgb32.bmp", rgb32frame_ret.GetVideoFormat(),
                rgb32frame_ret.frame, rgb32frame_ret.frame_length);
            mb->release();

            auto transform = MFTransform::Create(yuy2fmt, media::FOURCC_I420);
            Assert::IsTrue(transform.get());
            transform = MFTransform::Create(media::VideoFormat(640, 480, media::FOURCC_RGB24), media::FOURCC_RGB32);
            Assert::IsTrue(transform.get());
            transform = MFTransform::Create(media::VideoFormat(640, 480, media::FOURCC_NV12), media::FOURCC_RGB32);
            Assert::IsTrue(transform.get());
        }

        TEST_METHOD(TestFormats)
        {
            media::VideoFormat fmt_rgb24;
            std::vector<char> buff_rgb24 = LoadRawBitmap(L"test_rgb24.bmp", fmt_rgb24);
            Assert::IsTrue(buff_rgb24.size());
            Assert::AreEqual(800, fmt_rgb24.width);
            Assert::AreEqual(600, fmt_rgb24.height);
            Assert::AreEqual(int(media::FOURCC_RGB24), int(fmt_rgb24.fourcc));

            auto mft_rgb24_to_rgb32 = MFTransform::Create(fmt_rgb24, media::FOURCC_RGB32);
            Assert::IsTrue(mft_rgb24_to_rgb32.get());
            Assert::IsTrue(mft_rgb24_to_rgb32->SubmitSample(media::VideoFrame(fmt_rgb24, &buff_rgb24[0], int(buff_rgb24.size()))));

            media::VideoFormat fmt_rgb32 = fmt_rgb24;
            fmt_rgb32.fourcc = media::FOURCC_RGB32;
            std::vector<char> buff_rgb32(RGB32_BYTES(fmt_rgb32.width, fmt_rgb32.height));
            auto mbs = mft_rgb24_to_rgb32->RetrieveVideoFrames();
            Assert::AreEqual(int(1), int(mbs.size()));
            ACE_Message_Block* mb = mbs[0];
            Assert::IsTrue(mb);
            media::VideoFrame frame_rgb32(mb);
            Assert::IsTrue(WriteBitmap(L"test_rgb32.bmp", media::VideoFormat(fmt_rgb32.width, fmt_rgb32.height, media::FOURCC_RGB32), 
                frame_rgb32.frame, frame_rgb32.frame_length));

            // Convert RGB32 to I420
            media::VideoFormat fmt_i420 = fmt_rgb24;
            fmt_i420.fourcc = media::FOURCC_I420;
            auto mft_rgb32_to_i420 = MFTransform::Create(fmt_rgb32, media::FOURCC_I420);
            Assert::IsTrue(mft_rgb32_to_i420.get());
            Assert::IsTrue(mft_rgb32_to_i420->SubmitSample(frame_rgb32));
            mbs = mft_rgb32_to_i420->RetrieveVideoFrames();
            Assert::AreEqual(int(1), int(mbs.size()));
            mb = mbs[0];
            Assert::IsTrue(mb != nullptr);
            const media::VideoFrame* i420frame = reinterpret_cast<const media::VideoFrame*>(mb->rd_ptr());
            Assert::AreEqual(fmt_rgb32.width, i420frame->width);
            Assert::AreEqual(fmt_rgb32.height, i420frame->height);
            Assert::IsTrue(i420frame->fourcc == media::FOURCC_I420);

            // Convert I420 to RGB32
            auto mft_i420_to_rgb32 = MFTransform::Create(fmt_i420, media::FOURCC_RGB32);
            Assert::IsTrue(mft_i420_to_rgb32.get());
            Assert::IsTrue(mft_i420_to_rgb32->SubmitSample(*i420frame));
            mb->release();
            mbs = mft_i420_to_rgb32->RetrieveVideoFrames();
            Assert::AreEqual(int(1), int(mbs.size()));
            mb = mbs[0];
            Assert::IsTrue(mb != nullptr);
            const media::VideoFrame* rgb32frame_ret = reinterpret_cast<const media::VideoFrame*>(mb->rd_ptr());
            Assert::AreEqual(fmt_rgb32.width, rgb32frame_ret->width);
            Assert::AreEqual(fmt_rgb32.height, rgb32frame_ret->height);
            Assert::AreEqual(int(fmt_rgb32.fourcc), int(rgb32frame_ret->fourcc));

            WriteBitmap(L"myvideo_rgb32.bmp", media::VideoFormat(rgb32frame_ret->width, rgb32frame_ret->height, media::FOURCC_RGB32),
                        rgb32frame_ret->frame, rgb32frame_ret->frame_length);
            mb->release();

        }
#endif

#if defined(ENABLE_DSHOW)
        TEST_METHOD(TestDirectShow)
        {
            ACE_TString url = L"z:\\Media\\MVI_2526.AVI";
            MediaFileProp in_prop;
            Assert::IsTrue(GetMediaFileProp(url, in_prop));

            class MyClass : public MediaStreamListener
            {
                media::VideoFormat m_fmt;

            public:
                MyClass(const media::VideoFormat& fmt) : m_fmt(fmt)
                {
                }

                bool MediaStreamVideoCallback(MediaStreamer* streamer,
                    media::VideoFrame& video_frame,
                    ACE_Message_Block* mb_video)
                {
                    static int x = 0;
                    std::wostringstream os;
                    os << L"Video frame #" << ++x << L" at " << video_frame.timestamp << std::endl;
                    Logger::WriteMessage(os.str().c_str());

                    return false;
                }
                bool MediaStreamAudioCallback(MediaStreamer* streamer,
                    media::AudioFrame& audio_frame,
                    ACE_Message_Block* mb_audio)
                {
                    return false;
                }
                void MediaStreamStatusCallback(MediaStreamer* streamer,
                    const MediaFileProp& mfp,
                    MediaStreamStatus status)
                {
                    switch(status)
                    {
                    case MEDIASTREAM_STARTED:
                        break;
                    case MEDIASTREAM_ERROR:
                        break;
                    case MEDIASTREAM_FINISHED:
                        cv.notify_all();
                        break;
                    }
                }

            } listener(media::VideoFormat(in_prop.video.width, in_prop.video.height, media::FOURCC_NONE));
            
            media_streamer_t streamer = MakeMediaStreamer(&listener);
            Assert::IsTrue(streamer->OpenFile(in_prop, MediaStreamOutput(media::AudioFormat(48000, 2), 4800, media::FOURCC_RGB32)));
            Assert::IsTrue(streamer->StartStream());

            std::unique_lock<std::mutex> lk(done);
            cv.wait(lk);
        }
#endif

        TEST_METHOD(TestTeamTalkVideoCapture)
        {
            TTInstance* ttInst = TT_InitTeamTalkPoll();
            Assert::IsTrue(ttInst != nullptr);

            std::vector<VideoCaptureDevice> devs(100);
            INT32 nDevs = int(devs.size());
            Assert::IsTrue(TT_GetVideoCaptureDevices(&devs[0], &nDevs));

            Assert::IsTrue(TT_InitVideoCaptureDevice(ttInst, devs[0].szDeviceID, &devs[0].videoFormats[0]));

            VideoCodec codec;
            codec.nCodec = WEBM_VP8_CODEC;
            codec.webm_vp8.nEncodeDeadline = WEBM_VPX_DL_REALTIME;
            codec.webm_vp8.nRcTargetBitrate = 1024;
            Assert::IsTrue(TT_StartVideoCaptureTransmission(ttInst, &codec));

            INT32 nCount = 10;
            INT32 nWaitMax = 10000;
            TTMessage msg;
            while (nCount > 0 && TT_GetMessage(ttInst, &msg, nullptr))
            {
                switch (msg.nClientEvent)
                {
                case CLIENTEVENT_USER_VIDEOCAPTURE :
                {
                    VideoFrame* frame = TT_AcquireUserVideoCaptureFrame(ttInst, msg.nSource);
                    Assert::IsTrue(frame != nullptr);
                    nCount--;
                    std::wostringstream os;
                    os << L"teamtalk_vidcap" << nCount << L".bmp";
                    WriteBitmap(os.str().c_str(), media::VideoFormat(frame->nWidth, frame->nHeight, media::FOURCC_RGB32), 
                        reinterpret_cast<char*>(frame->frameBuffer), frame->nFrameBufferSize);
                    break;
                }
                }
            }

            Assert::IsTrue(TT_CloseVideoCaptureDevice(ttInst));
        }

#if defined(ENABLE_TEAMTALKPRO)

        static void UserLogin(IN TTSInstance* lpTTSInstance,
            IN VOID* lpUserData,
            OUT ClientErrorMsg* lpClientErrorMsg,
            IN const User* lpUser,
            IN OUT UserAccount* lpUserAccount)
        {
            lpClientErrorMsg->nErrorNo = CMDERR_SUCCESS;
            lpUserAccount->uUserType = USERTYPE_DEFAULT;
            lpUserAccount->uUserRights = USERRIGHT_MULTI_LOGIN | USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                USERRIGHT_UPLOAD_FILES | USERRIGHT_DOWNLOAD_FILES;
        }

        void RunServer(bool* started, bool* stop)
        {
            Assert::IsTrue(TTS_SetEncryptionContext(0, L"ttservercert.pem", L"ttserverkey.pem"));

            TTSInstance* ttServer = TTS_InitTeamTalk();
            Assert::IsTrue(ttServer != nullptr, L"valid instance");

            Assert::IsTrue(TTS_RegisterUserLoginCallback(ttServer, UserLogin, 0, TRUE));
            ServerProperties srvprop = {};
            std::wstring(L"TeamTalk 5 Pro Server").copy(srvprop.szServerName, TT_STRLEN);
            std::wstring(L"This is my message of the day").copy(srvprop.szMOTDRaw, TT_STRLEN);
            srvprop.nUserTimeout = 60;
            srvprop.nMaxUsers = 100;

            Assert::AreEqual(int(CMDERR_SUCCESS), TTS_SetChannelFilesRoot(ttServer, L".", 1024 * 1024 * 1024, 1024 * 1024 * 1024));

            Assert::AreEqual(int(CMDERR_SUCCESS), TTS_UpdateServer(ttServer, &srvprop));

            Channel chan = {};
            chan.nParentID = 0;
            chan.nChannelID = 1;
            chan.nMaxUsers = 100;
            chan.uChannelType = CHANNEL_PERMANENT;
            chan.nDiskQuota = 1024*1024*1024;
            std::wstring(L"This is the root channel").copy(chan.szTopic, TT_STRLEN);

            Assert::AreEqual(int(CMDERR_SUCCESS), TTS_MakeChannel(ttServer, &chan));

            Assert::IsTrue(TTS_StartServer(ttServer, L"127.0.0.1", 10443, 10443, TRUE), L"Start Server");
            INT32 nWaitMSec = 10;
            *started = true;
            while(!(*stop))
                TTS_RunEventLoop(ttServer, &nWaitMSec);

            TTS_StopServer(ttServer);

        }

        TEST_METHOD(TestSSL)
        {
            bool started = false, stop = false;
            std::thread serverthread(&UnitTest1::RunServer, this, &started, &stop);

            TTInstance* ttClient = TT_InitTeamTalkPoll();
            Assert::IsTrue(ttClient != nullptr, L"valid instance");

            INT32 nWaitMSec = 10;

            TTMessage msg = {};
            while(!started)
                TT_GetMessage(ttClient, &msg, &nWaitMSec);

            Assert::IsTrue(TT_Connect(ttClient, L"127.0.0.1", 10443, 10443, 0, 0, TRUE), L"Connect");

            Assert::IsTrue(WaitForEvent(ttClient, CLIENTEVENT_CON_SUCCESS));

            ServerProperties srvprop2 = {};
            Assert::IsTrue(TT_GetServerProperties(ttClient, &srvprop2));

            auto cmdid = TT_DoLogin(ttClient, L"My Nickname", L"HEST", L"HEST");
            Assert::IsTrue(WaitForCmdSuccess(ttClient, cmdid));

            int rootid = TT_GetRootChannelID(ttClient);
            cmdid = TT_DoJoinChannelByID(ttClient, rootid, L"");
            Assert::IsTrue(WaitForCmdSuccess(ttClient, cmdid));

            int filesize = 7;
            for (int f=1;f<9;f++)
            {
                std::ostringstream os;
                std::wostringstream wos;
                os << "test" << filesize << ".txt";
                wos << "test" << filesize << ".txt";
                std::ofstream ofs(os.str(), std::ofstream::out | std::ofstream::trunc);
                for(int i = 0; i<filesize; i++)
                {
                    ofs << (char)('0' + (i % 10));
                }
                ofs.close();

                cmdid = TT_DoSendFile(ttClient, rootid, wos.str().c_str());
                Assert::IsTrue(WaitForCmdSuccess(ttClient, cmdid));

                Assert::IsTrue(WaitForEvent(ttClient, CLIENTEVENT_FILETRANSFER, [cmdid](TTMessage m)
                {
                    return m.filetransfer.nStatus == FILETRANSFER_ACTIVE;
                }));

                Assert::IsTrue(WaitForEvent(ttClient, CLIENTEVENT_FILETRANSFER, [cmdid](TTMessage m)
                {
                    return m.filetransfer.nStatus == FILETRANSFER_FINISHED;
                }, msg));

                FileTransfer& ftup = msg.filetransfer;

                std::remove(os.str().c_str());

                INT32 nFiles = 1;
                RemoteFile remotefile;
                Assert::IsTrue(TT_GetChannelFiles(ttClient, ftup.nChannelID, &remotefile, &nFiles));

                Assert::IsTrue(TT_DoRecvFile(ttClient, remotefile.nChannelID, remotefile.nFileID, wos.str().c_str()));

                Assert::IsTrue(WaitForEvent(ttClient, CLIENTEVENT_FILETRANSFER, [cmdid](TTMessage m)
                {
                    return m.filetransfer.nStatus == FILETRANSFER_ACTIVE;
                }));

                Assert::IsTrue(WaitForEvent(ttClient, CLIENTEVENT_FILETRANSFER, [cmdid](TTMessage m)
                {
                    return m.filetransfer.nStatus == FILETRANSFER_FINISHED;
                }, msg));

                FileTransfer& ftdown = msg.filetransfer;

                std::ifstream ifs(os.str(), std::ifstream::in);
                for(int i = 0; i<filesize; i++)
                {
                    char c;
                    ifs >> c;
                    Assert::AreEqual(c, (char)('0' + (i % 10)));
                }

                Assert::IsTrue(WaitForCmdSuccess(ttClient, TT_DoDeleteFile(ttClient, remotefile.nChannelID, remotefile.nFileID)));

                filesize = filesize * 10 + 7;
            }


            stop = true;
            serverthread.join();
        }
#endif

        TEST_METHOD(TestMediaPlayback)
        {
            auto inst = TT_InitTeamTalkPoll(); // init required for MFStartup

            auto filename = L"C:\\Temp\\Music.wav";
            MediaFileProp inprop;
            Assert::IsTrue(GetMediaFileProp(filename, inprop));

            MediaPlayback mpb([](int userdata, const MediaFileProp& mfp,
                                 MediaStreamStatus status)
            {
                switch (status)
                {
                case MEDIASTREAM_STARTED :
                    Logger::WriteMessage(L"Media playback started");
                    break;
                case MEDIASTREAM_ERROR:
                    break;
                case MEDIASTREAM_FINISHED:
                    Logger::WriteMessage(L"Media playback completed");
                    break;
                }
            }, 13, soundsystem::GetInstance());

            Assert::IsTrue(mpb.OpenFile(filename), L"Load file");

            INT32 nInputDeviceID, nOutputDeviceID;
            Assert::IsTrue(TT_GetDefaultSoundDevices(&nInputDeviceID, &nOutputDeviceID), L"Get default devices");

            int sndgrpid = soundsystem::GetInstance()->OpenSoundGroup();
            Assert::IsTrue(mpb.OpenSoundSystem(sndgrpid, nOutputDeviceID), L"Open sound system");

            Assert::IsTrue(mpb.PlayMedia());
            WaitForEvent(inst, CLIENTEVENT_NONE, 3000);
            mpb.MuteSound(true, false);
            WaitForEvent(inst, CLIENTEVENT_NONE, 3000);
            mpb.MuteSound(false, true);
            WaitForEvent(inst, CLIENTEVENT_NONE, 3000);
            mpb.MuteSound(false, false);
            WaitForEvent(inst, CLIENTEVENT_NONE, 3000);

            TT_CloseTeamTalk(inst);
        }

        std::function<bool(TTMessage)> funcPlaying = [](TTMessage tmp) {
            std::wostringstream os;
            os << L"Playing " << tmp.mediafileinfo.szFileName << L" offset: " << tmp.mediafileinfo.uElapsedMSec << std::endl;
            Logger::WriteMessage(os.str().c_str());
            return tmp.mediafileinfo.nStatus != MFS_PLAYING; };

        TEST_METHOD(TestMediaPlaybackAPI)
        {
            auto inst = TT_InitTeamTalkPoll(); // init required for MFStartup

            INT32 nInputDeviceID, nOutputDeviceID;
            Assert::IsTrue(TT_GetDefaultSoundDevices(&nInputDeviceID, &nOutputDeviceID), L"Get default devices");

            Assert::IsTrue(TT_InitSoundOutputDevice(inst, nOutputDeviceID));

            auto filename = L"C:\\Temp\\Music.wav";
            MediaFilePlayback mfp = {};
            mfp.bPaused = FALSE;
            mfp.uOffsetMSec = 0;
            mfp.audioPreprocessor.nPreprocessor = TEAMTALK_AUDIOPREPROCESSOR;
            mfp.audioPreprocessor.ttpreprocessor.bMuteLeftSpeaker = FALSE;
            mfp.audioPreprocessor.ttpreprocessor.bMuteRightSpeaker = TRUE;
            mfp.audioPreprocessor.ttpreprocessor.nGainLevel = SOUND_GAIN_DEFAULT;
            INT32 nSessionID = TT_InitLocalPlayback(inst, filename, &mfp);
            Assert::IsTrue(nSessionID > 0);

            TTMessage msg;
            Assert::IsTrue(WaitForEvent(inst, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
            Assert::AreEqual(int(MFS_STARTED), int(msg.mediafileinfo.nStatus));
            Assert::AreEqual(std::wstring(filename), std::wstring(msg.mediafileinfo.szFileName));

            WaitForEvent(inst, CLIENTEVENT_NONE, 3000);
            mfp.audioPreprocessor.ttpreprocessor.bMuteLeftSpeaker = TRUE;
            mfp.audioPreprocessor.ttpreprocessor.bMuteRightSpeaker = FALSE;
            mfp.audioPreprocessor.ttpreprocessor.nGainLevel = SOUND_GAIN_MAX;
            Assert::IsTrue(TT_UpdateLocalPlayback(inst, nSessionID, &mfp));
            WaitForEvent(inst, CLIENTEVENT_NONE, 3000);

            auto filename2 = L"C:\\Temp\\giana_10sec.wma";

            MediaFilePlayback mfp2 = {};
            mfp2.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
            mfp2.bPaused = FALSE;
            mfp2.uOffsetMSec = 0;
            INT32 nSessionID2 = TT_InitLocalPlayback(inst, filename2, &mfp2);
            Assert::IsTrue(nSessionID2 > 0);

            auto funcPlaySession = [nSessionID2](TTMessage tmp) {
                return tmp.mediafileinfo.nStatus != MFS_PLAYING && tmp.nSource == nSessionID2;
            };

            Assert::IsTrue(WaitForEvent(inst, CLIENTEVENT_LOCAL_MEDIAFILE, funcPlaySession, &msg));
            Assert::AreEqual(int(MFS_STARTED), int(msg.mediafileinfo.nStatus));

            Assert::IsTrue(WaitForEvent(inst, CLIENTEVENT_LOCAL_MEDIAFILE, funcPlaySession, &msg, 12000));
            Assert::AreEqual(int(MFS_FINISHED), int(msg.mediafileinfo.nStatus));

            TT_CloseTeamTalk(inst);
        }

        TEST_METHOD(TestMediaPlaybackSpeexDSP)
        {
            auto inst = TT_InitTeamTalkPoll(); // init required for MFStartup

            INT32 nInputDeviceID, nOutputDeviceID;
            Assert::IsTrue(TT_GetDefaultSoundDevices(&nInputDeviceID, &nOutputDeviceID), L"Get default devices");

            Assert::IsTrue(TT_InitSoundOutputDevice(inst, nOutputDeviceID));

            auto filename3 = L"tone_8khz.wav";
            MediaFileInfo mfi = {};
            wcsncpy(mfi.szFileName, filename3, TT_STRLEN);
            mfi.uDurationMSec = 60 * 1000;
            mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
            mfi.audioFmt.nChannels = 2;
            mfi.audioFmt.nSampleRate = 8000;
            Assert::IsTrue(TT_DBG_WriteAudioFileTone(&mfi, 700));

            MediaFilePlayback mfp3 = {};
            mfp3.audioPreprocessor.nPreprocessor = SPEEXDSP_AUDIOPREPROCESSOR;
            mfp3.audioPreprocessor.speexdsp.bEnableDenoise = TRUE;
            mfp3.audioPreprocessor.speexdsp.nMaxNoiseSuppressDB = -30;
            mfp3.audioPreprocessor.speexdsp.bEnableAGC = TRUE;
            mfp3.audioPreprocessor.speexdsp.nGainLevel = 8000;
            mfp3.audioPreprocessor.speexdsp.nMaxGainDB = 30;
            mfp3.audioPreprocessor.speexdsp.nMaxIncDBSec = 12;
            mfp3.audioPreprocessor.speexdsp.nMaxDecDBSec = -40;
            mfp3.bPaused = FALSE;
            mfp3.uOffsetMSec = 0;
            INT32 nSessionID3 = TT_InitLocalPlayback(inst, filename3, &mfp3);
            Assert::IsTrue(nSessionID3 > 0);

            TTMessage msg;
            Assert::IsTrue(WaitForEvent(inst, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
            Assert::AreEqual(int(MFS_STARTED), int(msg.mediafileinfo.nStatus));

            WaitForEvent(inst, CLIENTEVENT_NONE, msg, 3000);

            Logger::WriteMessage(L"Enabling gain");
            mfp3.audioPreprocessor.speexdsp.nGainLevel = 32000;
            mfp3.audioPreprocessor.speexdsp.nMaxGainDB = 40;
            mfp3.audioPreprocessor.speexdsp.nMaxIncDBSec = 30;
            Assert::IsTrue(TT_UpdateLocalPlayback(inst, nSessionID3, &mfp3));

            WaitForEvent(inst, CLIENTEVENT_NONE, msg, 3000);

            Assert::IsTrue(TT_StopLocalPlayback(inst, nSessionID3));

            TT_CloseTeamTalk(inst);
        }

        TEST_METHOD(TestMediaPlaybackPause)
        {
            std::wostringstream os;
            
            auto inst = TT_InitTeamTalkPoll(); // init required for MFStartup

            INT32 nInputDeviceID, nOutputDeviceID;
            Assert::IsTrue(TT_GetDefaultSoundDevices(&nInputDeviceID, &nOutputDeviceID), L"Get default devices");

            Assert::IsTrue(TT_InitSoundOutputDevice(inst, nOutputDeviceID));

            auto filename3 = L"tone_8khz.wav";
            MediaFileInfo mfi = {};
            wcsncpy(mfi.szFileName, filename3, TT_STRLEN);
            mfi.uDurationMSec = 5 * 1000;
            mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
            mfi.audioFmt.nChannels = 2;
            mfi.audioFmt.nSampleRate = 8000;
            Assert::IsTrue(TT_DBG_WriteAudioFileTone(&mfi, 700));

            MediaFilePlayback mfp3 = {};
            mfp3.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
            mfp3.bPaused = FALSE;
            mfp3.uOffsetMSec = 0;
            INT32 nSessionID3 = TT_InitLocalPlayback(inst, filename3, &mfp3);
            Assert::IsTrue(nSessionID3 > 0);

            TTMessage msg;
            Assert::IsTrue(WaitForEvent(inst, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
            Assert::AreEqual(int(MFS_STARTED), int(msg.mediafileinfo.nStatus));
            auto starttime = GETTIMESTAMP();

            WaitForEvent(inst, CLIENTEVENT_NONE, msg, 1000);

            mfp3.bPaused = TRUE;
            Assert::IsTrue(TT_UpdateLocalPlayback(inst, nSessionID3, &mfp3));

            Assert::IsTrue(WaitForEvent(inst, CLIENTEVENT_LOCAL_MEDIAFILE, funcPlaying, &msg));
            Assert::AreEqual(int(MFS_PAUSED), int(msg.mediafileinfo.nStatus));
            os << L"Elapsed time: " << msg.mediafileinfo.uElapsedMSec << std::endl;
            Logger::WriteMessage(os.str().c_str());
            WaitForEvent(inst, CLIENTEVENT_NONE, msg, 3000);

            mfp3.bPaused = FALSE;
            Assert::IsTrue(TT_UpdateLocalPlayback(inst, nSessionID3, &mfp3));
            
            Assert::IsTrue(WaitForEvent(inst, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
            Assert::AreEqual(int(MFS_STARTED), int(msg.mediafileinfo.nStatus));

            Assert::IsTrue(WaitForEvent(inst, CLIENTEVENT_LOCAL_MEDIAFILE, funcPlaying, &msg, mfi.uDurationMSec * 2));
            Assert::AreEqual(int(MFS_FINISHED), int(msg.mediafileinfo.nStatus));

            os.str(L"");
            os << L"Duration: " << GETTIMESTAMP() - starttime << std::endl;
            Logger::WriteMessage(os.str().c_str());

            TT_CloseTeamTalk(inst);
        }

        TEST_METHOD(TestMediaPlaybackSeek)
        {
            std::wostringstream os;

            auto inst = TT_InitTeamTalkPoll(); // init required for MFStartup

            INT32 nInputDeviceID, nOutputDeviceID;
            Assert::IsTrue(TT_GetDefaultSoundDevices(&nInputDeviceID, &nOutputDeviceID), L"Get default devices");

            Assert::IsTrue(TT_InitSoundOutputDevice(inst, nOutputDeviceID));

            auto filename3 = L"tone441khz.wav";
            MediaFileInfo mfi = {};
            wcsncpy(mfi.szFileName, filename3, TT_STRLEN);
            mfi.uDurationMSec = 60 * 1000;
            mfi.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
            mfi.audioFmt.nChannels = 2;
            mfi.audioFmt.nSampleRate = 44100;
            Assert::IsTrue(TT_DBG_WriteAudioFileTone(&mfi, 700));

            INT32 nSessionID3;
            TTMessage msg;

            // play close to end
            MediaFilePlayback mfp3 = {};
            mfp3.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
            mfp3.bPaused = FALSE;
            mfp3.uOffsetMSec = 58000;
            nSessionID3 = TT_InitLocalPlayback(inst, filename3, &mfp3);
            Assert::IsTrue(nSessionID3 > 0);

            Assert::IsTrue(WaitForEvent(inst, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
            Assert::AreEqual(int(MFS_STARTED), int(msg.mediafileinfo.nStatus));
            Assert::IsTrue(msg.mediafileinfo.uElapsedMSec >= mfp3.uOffsetMSec);
            auto starttime = GETTIMESTAMP();

            Assert::IsTrue(WaitForEvent(inst, CLIENTEVENT_LOCAL_MEDIAFILE, funcPlaying, &msg));
            Assert::AreEqual(int(MFS_FINISHED), int(msg.mediafileinfo.nStatus));

            // play in the middle
            mfp3.audioPreprocessor.nPreprocessor = NO_AUDIOPREPROCESSOR;
            mfp3.bPaused = TRUE;
            mfp3.uOffsetMSec = 30000;
            nSessionID3 = TT_InitLocalPlayback(inst, filename3, &mfp3);
            Assert::IsTrue(nSessionID3 > 0);

            mfp3.bPaused = FALSE;
            Assert::IsTrue(TT_UpdateLocalPlayback(inst, nSessionID3, &mfp3));

            Assert::IsTrue(WaitForEvent(inst, CLIENTEVENT_LOCAL_MEDIAFILE, msg));
            Assert::AreEqual(int(MFS_STARTED), int(msg.mediafileinfo.nStatus));
            Assert::IsTrue(msg.mediafileinfo.uElapsedMSec >= mfp3.uOffsetMSec);

            mfp3.uOffsetMSec = 58000;
            Assert::IsTrue(TT_UpdateLocalPlayback(inst, nSessionID3, &mfp3));

            Assert::IsTrue(WaitForEvent(inst, CLIENTEVENT_LOCAL_MEDIAFILE, funcPlaying, &msg));
            Assert::AreEqual(int(MFS_STARTED), int(msg.mediafileinfo.nStatus));
            Assert::IsTrue(msg.mediafileinfo.uElapsedMSec >= mfp3.uOffsetMSec);

            Assert::IsTrue(WaitForEvent(inst, CLIENTEVENT_LOCAL_MEDIAFILE, funcPlaying, &msg));
            Assert::AreEqual(int(MFS_FINISHED), int(msg.mediafileinfo.nStatus));

            TT_CloseTeamTalk(inst);
        }

        TEST_METHOD(TestOPUSFramesPerPacket)
        {
            Assert::AreEqual(int(48000 * 0.0025), OPUS_GetCbSize(48000, 3));
            Assert::AreEqual(int(48000 * 0.005), OPUS_GetCbSize(48000, 5));
            Assert::AreEqual(int(48000 * 0.01), OPUS_GetCbSize(48000, 10));
            Assert::AreEqual(int(48000 * 0.02), OPUS_GetCbSize(48000, 20));
            Assert::AreEqual(int(48000 * 0.04), OPUS_GetCbSize(48000, 40));
            Assert::AreEqual(int(48000 * 0.04), OPUS_GetCbSize(48000, 50));
            Assert::AreEqual(int(48000 * 0.04), OPUS_GetCbSize(48000, 59));
            Assert::AreEqual(int(48000 * 0.06), OPUS_GetCbSize(48000, 60));
            Assert::AreEqual(int(48000 * 0.06), OPUS_GetCbSize(48000, 61));
            Assert::AreEqual(int(48000 * 0.08), OPUS_GetCbSize(48000, 80));
            Assert::AreEqual(int(48000 * 0.1), OPUS_GetCbSize(48000, 100));
            Assert::AreEqual(int(48000 * 0.12), OPUS_GetCbSize(48000, 120));


            AudioCodec ttcodec = {};
            ttcodec.nCodec = OPUS_CODEC;
            ttcodec.opus.nApplication = OPUS_APPLICATION_AUDIO;
            ttcodec.opus.nBitRate = 48000;
            ttcodec.opus.nChannels = 2;
            ttcodec.opus.nComplexity = 8;
            ttcodec.opus.nSampleRate = 48000;
            ttcodec.opus.nTxIntervalMSec = 200;
            ttcodec.opus.nFrameSizeMSec = 3;
            ttcodec.opus.bFEC = TRUE;
            ttcodec.opus.bVBR = TRUE;

            teamtalk::AudioCodec codec;
            Convert(ttcodec, codec);
            int samples = teamtalk::GetAudioCodecCbSamples(codec);

            OpusDecode decode;
            Assert::IsTrue(decode.Open(codec.opus.samplerate, codec.opus.channels));

            auto acb = [&codec, &decode](const teamtalk::AudioCodec& codec,
                const char* enc_data, int enc_len,
                const std::vector<int>& enc_frame_sizes,
                const media::AudioFrame& org_frame)
            {
                Assert::AreEqual(codec.opus.frames_per_packet, int(enc_frame_sizes.size()));
                int framesize = teamtalk::GetAudioCodecFrameSize(codec);
                std::vector<short> buffer(framesize*codec.opus.channels);
                int encoffset = 0;
                for (size_t i=0;i<enc_frame_sizes.size();i++)
                {
                    Assert::AreEqual(framesize, decode.Decode(&enc_data[encoffset], enc_frame_sizes[i], &buffer[0], framesize));
                    encoffset += enc_frame_sizes[i];
                }
            };

            AudioThread aud;
            Assert::IsTrue(aud.StartEncoder(acb, codec, false));
            aud.EnableTone(400);

            std::vector<short> buffer(samples*codec.opus.channels);
            media::AudioFormat fmt(codec.opus.samplerate, codec.opus.channels);
            media::AudioFrame frame(fmt, &buffer[0], samples);
            frame.force_enc = true;
            aud.QueueAudio(frame);
            ACE_Time_Value tm;
            aud.ProcessQueue(&tm);
        }

        TEST_METHOD(TestAudioInputNoResample)
        {
            WavePCMFile wavfile;
            Assert::IsTrue(wavfile.NewFile(L"myfile.wav", 16000, 1));

            audioinput_streamer_t ais(new AudioInputStreamer(1));
            std::vector<ACE_UINT32> timestamps;
            auto acb = [&](media::AudioFrame& audio_frame,
                ACE_Message_Block* mb_audio)
            {
                wavfile.AppendSamples(audio_frame.input_buffer, audio_frame.input_samples);
                timestamps.push_back(audio_frame.timestamp);
                cv.notify_all();
                return false;
            };

            ais->RegisterAudioCallback(acb, true);

            auto scb = [&](const AudioInputStatus& stat)
            {
                std::wostringstream oss;
                oss << L"Elapsed: " << stat.elapsed_msec << ". Queued: " << stat.queueduration_msec << std::endl;
                Logger::WriteMessage(oss.str().c_str());
            };
            ais->RegisterAudioInputStatusCallback(scb, true);

            media::AudioFormat infmt(16000, 1);

            const int ASAMPLES = 300;

            MediaStreamOutput mso(infmt, ASAMPLES);
            ACE_UINT32 outframeduration = ACE_UINT32((ASAMPLES * 1000) / mso.audio.samplerate);
            Assert::IsTrue(ais->Open(mso));
            Assert::IsTrue(ais->StartStream());

            short val = 1;
            std::vector<short> buff(ASAMPLES, val);
            media::AudioFrame frame(infmt, &buff[0], ASAMPLES);
            Assert::IsTrue(ais->InsertAudio(frame));

            std::unique_lock<std::mutex> lk(done);
            cv.wait(lk);

            Assert::AreEqual(ASAMPLES, wavfile.GetSamplesCount());

            buff.assign(ASAMPLES, ++val);
            Assert::IsTrue(ais->InsertAudio(frame));
            cv.wait(lk);
            Assert::AreEqual(ASAMPLES * 2, wavfile.GetSamplesCount());

            Assert::AreEqual(outframeduration + timestamps[0], timestamps[1]);
            Assert::AreEqual(size_t(2), timestamps.size());

            buff.resize(16000);
            for (auto i=0;i<buff.size();++i)
            {
                if ((i % ASAMPLES) == 0)
                    ++val;
                buff[i] = val;
            }
            frame = media::AudioFrame(infmt, &buff[0], 16000);
            Assert::IsTrue(ais->InsertAudio(frame));

            size_t expect_frames = 2 + (16000 / ASAMPLES);
            while (expect_frames != timestamps.size())
            {
                cv.wait(lk);
            }
            Assert::AreEqual(expect_frames, timestamps.size());

            ais->Flush();
            cv.wait(lk);

            Assert::AreEqual(expect_frames + 1, timestamps.size());
            Assert::AreEqual(timestamps[timestamps.size()-1], timestamps[timestamps.size() - 2] + outframeduration);

            Assert::IsTrue(wavfile.SeekSamplesBegin());
            short lastval = val;
            val = 1;
            buff.resize(300);
            int c = 0;
            do
            {
                Assert::AreEqual(ASAMPLES, wavfile.ReadSamples(&buff[0], ASAMPLES));
                for(int i=0;i<ASAMPLES;++i)
                {
                    Assert::AreEqual(buff[i], val);
                }
                val++;
            }
            while (val < lastval);

            int remain = (16000 + 2 * ASAMPLES) % ASAMPLES, s;
            Assert::AreEqual(ASAMPLES, wavfile.ReadSamples(&buff[0], ASAMPLES));
            for(s = 0; s<remain; ++s)
            {
                Assert::AreEqual(val, buff[s]);
            }
            for(s = remain; s<ASAMPLES; ++s)
            {
                Assert::AreEqual(short(0), buff[s]);
            }
        }

        TEST_METHOD(TestAudioInputResample)
        {
            WavePCMFile wavfile;
            Assert::IsTrue(wavfile.NewFile(L"myfile.wav", 48000, 2));

            audioinput_streamer_t ais(new AudioInputStreamer(2));
            std::vector<ACE_UINT32> timestamps;
            auto cb = [&](media::AudioFrame& audio_frame,
                ACE_Message_Block* mb_audio)
            {
                wavfile.AppendSamples(audio_frame.input_buffer, audio_frame.input_samples);
                timestamps.push_back(audio_frame.timestamp);
                cv.notify_all();
                return false;
            };

            ais->RegisterAudioCallback(cb, true);

            media::AudioFormat infmt(44100, 1);

            MediaStreamOutput mso(media::AudioFormat(48000, 2), 48000 * .05);
            ACE_UINT32 outframeduration = ACE_UINT32((48000 * .05 * 1000) / mso.audio.samplerate);
            Assert::IsTrue(ais->Open(mso));
            Assert::IsTrue(ais->StartStream());

            std::vector<short> buff(44100 * .05);
            media::AudioFrame frame(infmt, &buff[0], 44100 * .05);
            Assert::IsTrue(ais->InsertAudio(frame));

            std::unique_lock<std::mutex> lk(done);
            cv.wait(lk);

            Assert::AreEqual(int(48000 * .05), wavfile.GetSamplesCount());

            Assert::IsTrue(ais->InsertAudio(frame));
            cv.wait(lk);
            Assert::AreEqual(int(48000 * .05) * 2, wavfile.GetSamplesCount());

            Assert::AreEqual(outframeduration + timestamps[0], timestamps[1]);
            Assert::AreEqual(size_t(2), timestamps.size());
        }


    };
}