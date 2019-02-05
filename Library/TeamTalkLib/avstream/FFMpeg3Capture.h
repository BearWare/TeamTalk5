/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#if !defined(FFMPEG3CAPTURE_H)
#define FFMPEG3CAPTURE_H

#include "VideoCapture.h"
#include "FFMpeg3Streamer.h"
#include <myace/MyACE.h>

#include <ace/Bound_Ptr.h>
#include <ace/Null_Mutex.h>

#include <map>

namespace vidcap {

    class FFMpegVideoInput : public FFMpegStreamer
    {
    protected:
        VidCapDevice m_dev;
        media::VideoFormat m_vidfmt;

    public:
        FFMpegVideoInput(MediaStreamListener* listener,
                         const VidCapDevice& viddevice,
                         const media::VideoFormat& fmt)
            : FFMpegStreamer(listener), m_dev(viddevice), m_vidfmt(fmt) { }
        virtual media::VideoFormat GetVideoFormat() = 0;

        bool AddStartTime() const { return false; }
    };

    typedef std::unique_ptr<FFMpegVideoInput> ffmpegvideoinput_t;
    
    class FFMpeg3Capture : public VideoCapture,
                           public MediaStreamListener
    {
    protected:
        virtual ffmpegvideoinput_t createStreamer(MediaStreamListener* listener,
                                                  const VidCapDevice& viddevice,
                                                  const media::VideoFormat& fmt) = 0;
        ffmpegvideoinput_t m_videoinput;
        VideoCaptureCallback m_callback;
        
    public:
        FFMpeg3Capture();
        virtual ~FFMpeg3Capture();

        // VideoCapture interface
        bool InitVideoCapture(const ACE_TString& deviceid,
                              const media::VideoFormat& vidfmt);
        
        bool StartVideoCapture();

        void StopVideoCapture();

        media::VideoFormat GetVideoCaptureFormat();

        bool RegisterVideoFormat(VideoCaptureCallback callback, media::FourCC fcc);
        void UnregisterVideoFormat(media::FourCC fcc);
        
        // MediaStreamListener interface
        bool MediaStreamVideoCallback(MediaStreamer* streamer,
                                      media::VideoFrame& video_frame,
                                      ACE_Message_Block* mb_video);

        bool MediaStreamAudioCallback(MediaStreamer* streamer,
                                      media::AudioFrame& audio_frame,
                                      ACE_Message_Block* mb_audio);

        void MediaStreamStatusCallback(MediaStreamer* streamer,
                                       const MediaFileProp& mfp,
                                       MediaStreamStatus status);
    };

}

#endif

