#include "CppUnitTest.h"

#include <avstream/MFStreamer.h>
#include <avstream/MFCapture.h>
#include <avstream/MFTransform.h>
#include <codec/WaveFile.h>
#include <codec/BmpFile.h>
#include <codec/VpxEncoder.h>


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
                        wavfile.NewFile(L"output.wav", out_prop.audio_samplerate, out_prop.audio_channels);
                        break;
                    case MEDIASTREAM_ERROR :
                        break;
                    case MEDIASTREAM_FINISHED :
                        cv.notify_all();
                        break;
                    }
                }
            } listener;

            ACE_TString url = L"https://bearware.dk/temp/giana_10sec.wma";
            //ACE_TString url = L"z:\\Media\\giana.wav";

            MediaFileProp in_prop;
            Assert::IsTrue(GetMediaFileProp(url, in_prop));

            MediaStreamOutput out_prop;
            out_prop.audio = true;
            out_prop.audio_channels = 2;
            out_prop.audio_samplerate = 48000;
            out_prop.audio_samples = 48000 * .04;

            listener.setOutput(out_prop);
            
            MFStreamer streamer(&listener);

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
            public:
                void setOutput(const MediaStreamOutput& o) { out_prop = o; }
                bool MediaStreamVideoCallback(MediaStreamer* streamer,
                    media::VideoFrame& video_frame,
                    ACE_Message_Block* mb_video)
                {
                    static int n_bmp = 0;
                    std::wostringstream os;
                    os << L"video_" << ++n_bmp << L".bmp";
                    WriteBitmap(os.str().c_str(), video_frame.width, video_frame.height, 4, video_frame.frame, video_frame.frame_length);

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
                        wavfile.NewFile(L"output.wav", out_prop.audio_samplerate, out_prop.audio_channels);
                        break;
                    case MEDIASTREAM_ERROR:
                        break;
                    case MEDIASTREAM_FINISHED:
                        cv.notify_all();
                        break;
                    }
                }
            } listener;

            ACE_TString url = L"https://bearware.dk/temp/OOBEMovie_10sec.wmv";
            //ACE_TString url = L"z:\\Media\\MVI_2526.AVI";

            MediaFileProp in_prop;
            Assert::IsTrue(GetMediaFileProp(url, in_prop));

            MediaStreamOutput out_prop;
            out_prop.video = true;
            out_prop.audio = true;
            out_prop.audio_channels = 2;
            out_prop.audio_samplerate = 48000;
            out_prop.audio_samples = 48000 * .12;

            listener.setOutput(out_prop);

            MFStreamer streamer(&listener);

            Assert::IsTrue(streamer.OpenFile(in_prop, out_prop));

            Assert::IsTrue(streamer.StartStream());

            std::unique_lock<std::mutex> lk(done);
            cv.wait(lk);
        }

        TEST_METHOD(TestVideoCapture)
        {
            class : public vidcap::VideoCaptureListener
            {
            public:
                bool OnVideoCaptureCallback(media::VideoFrame& video_frame,
                                            ACE_Message_Block* mb_video)
                {
                    std::wostringstream os;
                    os << L"Got video frame "<< video_frame.width << L"x" << video_frame.height << std::endl;

                    Logger::WriteMessage(os.str().c_str());
                    Assert::AreEqual(int(media::FOURCC_RGB32), int(video_frame.fourcc));

                    os.str(L"");
                    static int n_bmp = 0;
                    os << L"video_" << ++n_bmp << L".bmp";
                    WriteBitmap(os.str().c_str(), video_frame.width, video_frame.height, 4, video_frame.frame, video_frame.frame_length);

                    return false;
                }

            } listener;

            vidcap::MFCapture cap;
            auto devs = cap.GetDevices();
            std::wostringstream os;
            for (auto a : devs)
            {
                os << L"Device: " << a.devicename.c_str();
                Logger::WriteMessage(os.str().c_str());
                for (auto f : a.vidcapformats)
                {
                    os.str(L"");
                    os << L"\t" << f.width << L"x" << f.height;
                    os << "@ " << (f.fps_numerator / f.fps_denominator);
                    switch (f.fourcc)
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
            
            Assert::IsTrue(cap.StartVideoCapture(L"0", media::VideoFormat(640, 480, 30, 1, media::FOURCC_I420), &listener));

            std::mutex mtx;
            std::unique_lock<std::mutex> lck(mtx);
            std::condition_variable cv;
            cv.wait_for(lck, std::chrono::seconds(10));
            os << L"foo";


            // cap.StopVideoCapture(&listener);
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
            const auto WIDTH=640, HEIGHT=480, FPS_N = 30, FPS_D = 1;
            media::VideoFormat rgb24fmt(WIDTH, HEIGHT, FPS_N, FPS_D, media::FOURCC_RGB24);
            media::VideoFormat rgb32fmt(WIDTH, HEIGHT, FPS_N, FPS_D, media::FOURCC_RGB32);
            media::VideoFormat i420fmt(WIDTH, HEIGHT, FPS_N, FPS_D, media::FOURCC_I420);

            std::vector<char> buff_rgb32(RGB32_BYTES(rgb32fmt.width, rgb32fmt.height), 0x80);
            media::VideoFrame rgb32frame(&buff_rgb32[0], buff_rgb32.size(), rgb32fmt.width, rgb32fmt.height, media::FOURCC_RGB32, false);
            rgb32frame.timestamp = 12;

            // Convert RGB32 to RGB24
            auto mft_rgb32_to_rgb24 = MFTransform::Create(rgb32fmt, media::FOURCC_RGB24);
            Assert::IsTrue(mft_rgb32_to_rgb24.get());
            Assert::IsTrue(mft_rgb32_to_rgb24->SubmitSample(rgb32frame));
            //CComPtr<IMFSample> pSample = mft_rgb32_to_rgb24->RetrieveSample();
            ACE_Message_Block* mb = mft_rgb32_to_rgb24->RetrieveSample(rgb24fmt);
            Assert::IsTrue(mb != nullptr);
            const media::VideoFrame* rgb24frame_ret = reinterpret_cast<const media::VideoFrame*>(mb->rd_ptr());
            WriteBitmap(L"myvideo_rgb24.bmp", rgb24frame_ret->width, rgb24frame_ret->height, 3, rgb24frame_ret->frame, rgb24frame_ret->frame_length);
            Assert::AreEqual(rgb32fmt.width, rgb24frame_ret->width);
            Assert::AreEqual(rgb32fmt.height, rgb24frame_ret->height);
            Assert::AreEqual(int(rgb24fmt.fourcc), int(rgb24frame_ret->fourcc));
            mb->release();

            // Convert RGB32 to I420
            auto mft_rgb32_to_i420 = MFTransform::Create(rgb32fmt, media::FOURCC_I420);
            Assert::IsTrue(mft_rgb32_to_i420.get());
            Assert::IsTrue(mft_rgb32_to_i420->SubmitSample(rgb32frame));
            mb = mft_rgb32_to_i420->RetrieveSample(i420fmt);
            Assert::IsTrue(mb != nullptr);
            const media::VideoFrame* i420frame = reinterpret_cast<const media::VideoFrame*>(mb->rd_ptr());
            Assert::AreEqual(rgb32fmt.width, i420frame->width);
            Assert::AreEqual(rgb32fmt.height, i420frame->height);
            Assert::IsTrue(i420frame->fourcc == media::FOURCC_I420);

            // Convert I420 to RGB32
            auto mft_i420_to_rgb32 = MFTransform::Create(i420fmt, media::FOURCC_RGB32);
            Assert::IsTrue(mft_i420_to_rgb32.get());
            Assert::IsTrue(mft_i420_to_rgb32->SubmitSample(*i420frame));
            mb->release();
            mb = mft_i420_to_rgb32->RetrieveSample(rgb32fmt);
            Assert::IsTrue(mb != nullptr);
            const media::VideoFrame* rgb32frame_ret = reinterpret_cast<const media::VideoFrame*>(mb->rd_ptr());
            Assert::AreEqual(rgb32fmt.width, rgb32frame_ret->width);
            Assert::AreEqual(rgb32fmt.height, rgb32frame_ret->height);
            Assert::AreEqual(int(rgb32fmt.fourcc), int(rgb32frame_ret->fourcc));

            WriteBitmap(L"myvideo_rgb32.bmp", rgb32frame_ret->width, rgb32frame_ret->height, 4, rgb32frame_ret->frame, rgb32frame_ret->frame_length);
            mb->release();
        }

    };
}