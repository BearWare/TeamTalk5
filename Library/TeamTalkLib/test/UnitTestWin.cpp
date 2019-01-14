#include "CppUnitTest.h"

#if defined(ENABLE_MEDIAFOUNDATION)
#include <avstream/MFStreamer.h>
#include <avstream/MFCapture.h>
#include <avstream/MFTransform.h>
#endif
#if defined(ENABLE_DSHOW)
#include <avstream/WinMedia.h>
#endif

#include <codec/WaveFile.h>
#include <codec/BmpFile.h>
#include <codec/VpxEncoder.h>

#include <myace/MyACE.h>

#include <TeamTalk.h>

#include <mutex>
#include <condition_variable>
#include <sstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

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

            MediaStreamOutput out_prop(media::AudioFormat(48000, 2), 48000 * .04);
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
                    os << ++n_bmp;
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
                        ACE_Message_Block* mb = transform->RetrieveMBSample();
                        Assert::IsTrue(mb != nullptr, L"Transformed frame");
                        media::VideoFrame frame(mb);
                        WriteBitmap(os.str().c_str(), media::VideoFormat(video_frame.width, video_frame.height, media::FOURCC_RGB32), frame.frame, frame.frame_length);
                        mb->release();
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
            url = L"https://bearware.dk/temp/OOBEMovie_10sec.wmv";
            url = L"z:\\Media\\MVI_2526.AVI";
            //url = L"z:\\Media\\OOBEMovie.wmv";
            //url = L"z:\\Media\\OOBEMovie_10sec.wmv";
            //url = L"z:\\Media\\Seinfeld.avi";

            MediaFileProp in_prop;
            Assert::IsTrue(GetMediaFileProp(url, in_prop));

#if defined(ENABLE_DSHOW)
            MediaStreamOutput out_prop(media::AudioFormat(48000, 2), 48000 * .12, media::FOURCC_RGB32);
#else
            MediaStreamOutput out_prop(media::AudioFormat(48000, 2), 48000 * .12, media::FOURCC_RGB32);
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

#if defined(ENABLE_MEDIAFOUNDATION)
        void VideoCaptureTest(const media::VideoFormat& fmt)
        {
            std::wostringstream os;
            os << L"Testing ";
            os << fmt.width << L"x" << fmt.height;
            os << "@ " << (fmt.fps_numerator / fmt.fps_denominator);
            os << L" - " << FourCCToString(fmt.fourcc).c_str();
            os << std::endl;
            Logger::WriteMessage(os.str().c_str());

            std::vector<media::FourCC> transforms = { media::FOURCC_RGB24, media::FOURCC_RGB32 };
            std::set<media::FourCC> outputs;
            outputs.insert(transforms.begin(), transforms.end());
            outputs.insert(fmt.fourcc);

            class MyClass : public vidcap::VideoCaptureListener
            {
                std::vector<media::VideoFormat> m_receivedfmts;
                std::set<media::FourCC> m_outputs;

            public:
                MyClass(std::set<media::FourCC> outputs) : m_outputs(outputs)
                {
                }

                bool OnVideoCaptureCallback(media::VideoFrame& video_frame,
                                            ACE_Message_Block* /*mb_video*/)
                {
                    media::VideoFormat fmt = MFCaptureSingleton::instance()->GetVideoCaptureFormat(this);

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

            } listener(outputs);

            Assert::IsTrue(MFCaptureSingleton::instance()->StartVideoCapture(L"0", fmt, &listener));

            for (auto f : transforms)
                Assert::IsTrue(MFCaptureSingleton::instance()->RegisterVideoFormat(&listener, f));

            std::mutex mtx;
            std::unique_lock<std::mutex> lck(mtx);
            //cv.wait_for(lck, std::chrono::seconds(10));
            cv.wait(lck);

            Assert::IsTrue(MFCaptureSingleton::instance()->StopVideoCapture(&listener));
        }

        TEST_METHOD(TestVideoCapture)
        {
            auto devs = MFCaptureSingleton::instance()->GetDevices();
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
                    os << L" - " << FourCCToString(fmt.fourcc).c_str();
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
                VideoCaptureTest(fmt);
            }
        }


        TEST_METHOD(TestVideoEncode)
        {
            VpxEncoder encoder;

            class MyListener : public vidcap::VideoCaptureListener
            {
                VpxEncoder& encoder;
            public:
                MyListener (VpxEncoder& enc) : encoder(enc) {}
                bool OnVideoCaptureCallback(media::VideoFrame& video_frame,
                    ACE_Message_Block* mb_video)
                {
                    std::wostringstream os;
                    os << L"Got video frame " << video_frame.width << L"x" << video_frame.height << std::endl;

                    Logger::WriteMessage(os.str().c_str());

                    switch (video_frame.fourcc)
                    {
                    case media::FOURCC_I420 :
                        Assert::AreEqual(int(VPX_CODEC_OK), int(encoder.Encode(video_frame.frame, video_frame.frame_length, VPX_IMG_FMT_I420, 0, VPX_DL_BEST_QUALITY)));
                        break;
                    case media::FOURCC_RGB32 :
                        Assert::AreEqual(int(VPX_CODEC_OK), int(encoder.Encode(video_frame.frame, video_frame.frame_length, VPX_IMG_FMT_ARGB, 0, VPX_DL_BEST_QUALITY)));
                        break;
                    }

                    return false;
                }

            } listener(encoder);

            vidcap::MFCapture cap;
            auto devs = cap.GetDevices();
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
            media::VideoFormat fmt(640, 480, 30, 1, media::FOURCC_I420);

            Assert::IsTrue(encoder.Open(fmt.width, fmt.height, 1024*1024, fmt.fps_numerator/fmt.fps_denominator));
            Assert::IsTrue(cap.StartVideoCapture(L"0", fmt, &listener));

            std::mutex mtx;
            std::unique_lock<std::mutex> lck(mtx);
            std::condition_variable cv;
            cv.wait_for(lck, std::chrono::seconds(10));
            os << L"foo";
        }

        TEST_METHOD(TestTransform)
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
            ACE_Message_Block* mb = mft_rgb32_to_rgb24->RetrieveMBSample();
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
            mb = mft_rgb32_to_i420->RetrieveMBSample();
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
            mb = mft_i420_to_rgb32->RetrieveMBSample();
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
            mb = mft_rgb32_to_yuy2->RetrieveMBSample();
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
            mb = mft_yuy2_to_rgb32->RetrieveMBSample();
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
            ACE_Message_Block* mb = mft_rgb24_to_rgb32->RetrieveMBSample();
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
            mb = mft_rgb32_to_i420->RetrieveMBSample();
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
            mb = mft_i420_to_rgb32->RetrieveMBSample();
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
    };
}