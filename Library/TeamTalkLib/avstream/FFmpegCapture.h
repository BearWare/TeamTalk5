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

#include "FFmpegStreamer.h"
#include "MediaStreamer.h"
#include "VideoCapture.h"
#include "codec/MediaUtil.h"

#include <ace/SString.h>
#include <ace/Message_Block.h>

#include <memory>

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

    using ffmpegvideoinput_t = std::unique_ptr<FFmpegVideoInput>;
    
    class FFmpegCapture : public VideoCapture
    {
    protected:
        virtual ffmpegvideoinput_t CreateStreamer(const VidCapDevice& viddevice,
                                                  const media::VideoFormat& fmt) = 0;
        ffmpegvideoinput_t m_videoinput;
        VideoCaptureCallback m_callback;

    public:
        FFmpegCapture();
        ~FFmpegCapture() override;

        // VideoCapture interface
        bool InitVideoCapture(const ACE_TString& deviceid,
                              const media::VideoFormat& vidfmt) override;

        bool StartVideoCapture() override;

        void StopVideoCapture() override;

        media::VideoFormat GetVideoCaptureFormat() override;

        bool RegisterVideoFormat(VideoCaptureCallback callback, media::FourCC fcc) override;
        void UnregisterVideoFormat(media::FourCC fcc) override;

        // MediaStreamListener interface
        bool MediaStreamVideoCallback(media::VideoFrame& video_frame,
                                      ACE_Message_Block* mb_video);

        void MediaStreamStatusCallback(const MediaFileProp& mfp,
                                       MediaStreamStatus status);
    };

} // namespace vidcap

#endif
