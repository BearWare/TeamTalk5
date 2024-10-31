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
#include "FFmpegStreamer.h"
#include <myace/MyACE.h>

#include <map>

namespace vidcap {

    class FFmpegVideoInput : public FFmpegStreamer
    {
    protected:
        VidCapDevice m_dev;
        media::VideoFormat m_vidfmt;

    public:
        FFmpegVideoInput(const VidCapDevice& viddevice,
                         const media::VideoFormat& fmt);
        virtual media::VideoFormat GetVideoFormat() = 0;

        bool IsSystemTime() const override { return true; }
    };

    typedef std::unique_ptr<FFmpegVideoInput> ffmpegvideoinput_t;
    
    class FFmpegCapture : public VideoCapture
    {
    protected:
        virtual ffmpegvideoinput_t createStreamer(const VidCapDevice& viddevice,
                                                  const media::VideoFormat& fmt) = 0;
        ffmpegvideoinput_t m_videoinput;
        VideoCaptureCallback m_callback;
        
    public:
        FFmpegCapture();
        virtual ~FFmpegCapture();

        // VideoCapture interface
        bool InitVideoCapture(const ACE_TString& deviceid,
                              const media::VideoFormat& vidfmt);
        
        bool StartVideoCapture();

        void StopVideoCapture();

        media::VideoFormat GetVideoCaptureFormat();

        bool RegisterVideoFormat(VideoCaptureCallback callback, media::FourCC fcc);
        void UnregisterVideoFormat(media::FourCC fcc);
        
        // MediaStreamListener interface
        bool MediaStreamVideoCallback(media::VideoFrame& video_frame,
                                      ACE_Message_Block* mb_video);

        void MediaStreamStatusCallback(const MediaFileProp& mfp,
                                       MediaStreamStatus status);
    };

}

#endif

