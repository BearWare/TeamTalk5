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

#include <codec/WaveFile.h>
#include <codec/BmpFile.h>
#include <codec/VpxEncoder.h>
#include <codec/VpxDecoder.h>

#include <myace/MyACE.h>
#include <ace/FILE_Connector.h>

#include <TeamTalk.h>
#if defined(ENABLE_TEAMTALKPRO)
#include <TeamTalkSrv.h>
#endif

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

    public:

        TEST_METHOD(TestAudioStream)
        {
            class : public MediaStreamListener
            {
                WaveFile wavfile;
                MediaStreamOutput out_prop;
            public:
                void setOutput(const MediaStreamOutput& o) { out_prop = o; }
                bool MediaStreamVideoCallback(MediaStreamer* streamer,
                    media::VideoFrame& video_frame,
                    ACE_Message_Block* mb_video)
                {
                    return false;
                }
                bool MediaStreamAudioCallback(MediaStreamer* streamer,
                    media::AudioFrame& audio_frame,
                    ACE_Message_Block* mb_audio)
                {
                    wavfile.AppendSamples(audio_frame.input_buffer, audio_frame.input_samples);
                    return false;
                }
                void MediaStreamStatusCallback(MediaStreamer* streamer,
                    const MediaFileProp& mfp,
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
            MFStreamer streamer(&listener);
#endif

            Assert::IsTrue(streamer.OpenFile(in_prop, out_prop));

            Assert::IsTrue(streamer.StartStream());

            std::unique_lock<std::mutex> lk(done);
            cv.wait(lk);
        }

        TEST_METHOD(TestVideoStream)
        {

            class : public MediaStreamListener
            {
                WaveFile wavfile;
                MediaStreamOutput out_prop;
#if defined(ENABLE_MEDIAFOUNDATION)
                mftransform_t transform;
#endif
            public:
                void setOutput(const MediaStreamOutput& o) { out_prop = o; }
                bool MediaStreamVideoCallback(MediaStreamer* streamer,
                    media::VideoFrame& video_frame,
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
                bool MediaStreamAudioCallback(MediaStreamer* streamer,
                    media::AudioFrame& audio_frame,
                    ACE_Message_Block* mb_audio)
                {
                    wavfile.AppendSamples(audio_frame.input_buffer, audio_frame.input_samples);
                    return false;
                }
                void MediaStreamStatusCallback(MediaStreamer* streamer,
                    const MediaFileProp& mfp,
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
            MFStreamer streamer(&listener);
#endif
#if defined(ENABLE_DSHOW)
            DSWrapperThread streamer(&listener);
#endif
            Assert::IsTrue(streamer.OpenFile(in_prop, out_prop));

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
                                        Assert::AreEqual(1u, mbs.size(), L"Got frame");
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

            media::VideoFrame rgb32frame(rgb32fmt, &buff_rgb32[0], buff_rgb32.size());
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
            Assert::AreEqual(1u, mbs.size());
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
            Assert::AreEqual(1u, mbs.size());
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
            Assert::AreEqual(1u, mbs.size());
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
            Assert::AreEqual(1u, mbs.size());
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
            Assert::IsTrue(mft_rgb24_to_rgb32->SubmitSample(media::VideoFrame(fmt_rgb24, &buff_rgb24[0], buff_rgb24.size())));

            media::VideoFormat fmt_rgb32 = fmt_rgb24;
            fmt_rgb32.fourcc = media::FOURCC_RGB32;
            std::vector<char> buff_rgb32(RGB32_BYTES(fmt_rgb32.width, fmt_rgb32.height));
            auto mbs = mft_rgb24_to_rgb32->RetrieveVideoFrames();
            Assert::AreEqual(1u, mbs.size());
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
            Assert::AreEqual(1u, mbs.size());
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
            Assert::AreEqual(1u, mbs.size());
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

#if defined(ENABLE_MEDIAFOUNDATION)
        TEST_METHOD(TestAudioTransform)
        {
            media::AudioFormat input(16000, 1), output(32000, 1);
            
            std::vector<short> buff(input.samplerate * input.channels);

            media::AudioFrame frame;
            frame.inputfmt = input;
            frame.input_buffer = &buff[0];
            frame.input_samples = input.samplerate;

            WaveFile inwavefile, outwavefile;
            Assert::IsTrue(inwavefile.NewFile(ACE_TEXT("hest_in.wav"), input.samplerate, input.channels));
            Assert::IsTrue(outwavefile.NewFile(ACE_TEXT("hest_out.wav"), output.samplerate, output.channels));

            int output_samples = output.samplerate * .1;

            auto transform = MFTransform::Create(input, output, output_samples);
            Assert::IsTrue(transform.get() != nullptr);
            Assert::IsTrue(transform->RetrieveAudioFrames().empty());

            int sampleindex = 0;
            for(int i=0;i<10;i++)
            {
                sampleindex = GenerateTone(frame, sampleindex, 600);
                inwavefile.AppendSamples(frame.input_buffer, frame.input_samples);

                auto mbs = transform->ProcessAudioResampler(frame);
                for (auto& mb : mbs)
                {
                    media::AudioFrame outframe(mb);
                    Assert::AreEqual(outframe.inputfmt.samplerate, output.samplerate);
                    Assert::AreEqual(outframe.inputfmt.channels, output.channels);
                    //Assert::AreEqual(output_samples, outframe.input_samples);
                    outwavefile.AppendSamples(outframe.input_buffer, outframe.input_samples);
                    mb->release();
                }
            }
        }

        TEST_METHOD(TestAudioEncoderTransform)
        {
            media::AudioFormat input(48000, 2);

            std::vector<short> buff(input.samplerate * input.channels);

            media::AudioFrame frame;
            frame.inputfmt = input;
            frame.input_buffer = &buff[0];
            frame.input_samples = input.samplerate;

            WaveFile inwavefile;
            Assert::IsTrue(inwavefile.NewFile(ACE_TEXT("hest_in.wav"), input.samplerate, input.channels));

            ACE_FILE_Connector con;
            ACE_FILE_IO outwavefile;
            Assert::IsTrue(con.connect(outwavefile, ACE_FILE_Addr(L"hest_out.wav"),
                0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT | O_BINARY, FILE_SHARE_READ | FILE_SHARE_WRITE) >= 0);
            
            //auto transform = MFTransform::CreateMP3(input, 40000*8);
            auto transform = MFTransform::CreateWMA(input, 40000*8);
            Assert::IsTrue(transform.get() != nullptr);
            Assert::IsTrue(transform->RetrieveRawFrames().empty());

            Assert::IsTrue(transform->GetInputType());
            Assert::IsTrue(transform->GetOutputType());

            std::vector<char> header = MediaTypeToWaveFormatEx(transform->GetOutputType());
            Assert::IsTrue(header.size());
            WAVEFORMATEX* pWaveFormat = reinterpret_cast<WAVEFORMATEX*>(&header[0]);

            WriteWaveFileHeader(outwavefile, pWaveFormat, header.size());

            int sampleindex = 0;
            for(int i = 0; i<10; i++)
            {
                sampleindex = GenerateTone(frame, sampleindex, 600);
                inwavefile.AppendSamples(frame.input_buffer, frame.input_samples);

                auto mbs = transform->ProcessAudioEncoder(frame);
                for(auto& mb : mbs)
                {
                    outwavefile.send_n(mb->rd_ptr(), mb->length());
                    mb->release();
                }
            }

            Assert::IsTrue(transform->Drain());
            auto mbs = transform->ProcessAudioEncoder(frame);
            for(auto& mb : mbs)
            {
                outwavefile.send_n(mb->rd_ptr(), mb->length());
                mb->release();
            }

            UpdateWaveFileHeader(outwavefile);
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
        TEST_METHOD(TestHTTPS)
        {
            //ACE::HTTPS::Context::set_default_ssl_mode(ACE_SSL_Context::SSLv23);
            //ACE::HTTPS::Context::set_default_verify_mode(true);
            //ACE::HTTPS::Context::instance().use_default_ca();
            //ACE::INet::SSL_CallbackManager::instance()->set_certificate_callback(new ACE::INet::SSL_CertificateAcceptor);

            std::string response1, response2, response3;
            Assert::AreEqual(1, HttpRequest("http://www.bearware.dk/teamtalk/weblogin.php?ping=1", response1));
#if defined(ENABLE_ENCRYPTION)
            Assert::AreEqual(1, HttpRequest("https://www.bearware.dk/teamtalk/weblogin.php?ping=1", response2));
            Assert::AreEqual(response1, response2);
            Assert::AreEqual(1, HttpRequest("https://www.google.com", response3));
#endif
        }

        TEST_METHOD(TestTeamTalkVideoCapture)
        {
            TTInstance* ttInst = TT_InitTeamTalkPoll();
            Assert::IsTrue(ttInst != nullptr);

            std::vector<VideoCaptureDevice> devs(100);
            INT32 nDevs = devs.size();
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

#define DEFWAIT 5000

        bool WaitForEvent(TTInstance* ttClient, ClientEvent ttevent, std::function<bool(TTMessage)> pred, TTMessage& outmsg = TTMessage(), int timeout = DEFWAIT)
        {
            auto start = GETTIMESTAMP();
            while (GETTIMESTAMP() < start + timeout)
            {
                INT32 waitMsec = 10;
                if (TT_GetMessage(ttClient, &outmsg, &waitMsec) &&
                    outmsg.nClientEvent == ttevent &&
                    pred(outmsg))
                    return true;
            }
            return false;
        }

        bool WaitForEvent(TTInstance* ttClient, ClientEvent ttevent, TTMessage& outmsg = TTMessage(), int timeout = DEFWAIT)
        {
            return WaitForEvent(ttClient, ttevent, [] (TTMessage) { return true; }, outmsg, timeout);
        }

        bool WaitForCmdSuccess(TTInstance* ttClient, int cmdid, TTMessage& outmsg = TTMessage(), int timeout = DEFWAIT)
        {
            return WaitForEvent(ttClient, CLIENTEVENT_CMD_SUCCESS, [cmdid](TTMessage msg) {
                return msg.nSource == cmdid;
            }, outmsg, timeout);
        }

        bool WaitForCmdComplete(TTInstance* ttClient, int cmdid, TTMessage& outmsg = TTMessage(), int timeout = DEFWAIT)
        {
            return WaitForEvent(ttClient, CLIENTEVENT_CMD_PROCESSING, [cmdid](TTMessage msg) {
                return msg.nSource == cmdid && !msg.bActive;
            }, outmsg, timeout);
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
    };
}