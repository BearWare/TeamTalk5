#include "CppUnitTest.h"

#include <codec/MFStreamer.h>
#include <codec/WaveFile.h>
#include <codec/BmpFile.h>

#include <vidcap/MFCapture.h>

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

            MediaFileProp in_prop;
            Assert::IsTrue(GetMediaFileProp(url, in_prop));

            MediaStreamOutput out_prop;
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
    };
}