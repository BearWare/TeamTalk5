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

#include "FFMpeg3Capture.h"

namespace vidcap {

    class AVFVideoInput : public FFMpegVideoInput
    {
    public:
        AVFVideoInput(const VidCapDevice& viddevice,
                      const media::VideoFormat& fmt)
            : FFMpegVideoInput(viddevice, fmt) {
        }
    
        // FFMpegStreamer override
        bool SetupInput(struct AVInputFormat *iformat,
                        struct AVDictionary *options,
                        struct AVFormatContext*& fmt_ctx,
                        struct AVCodecContext*& aud_dec_ctx,
                        struct AVCodecContext*& vid_dec_ctx,
                        int& audio_stream_index,
                        int& video_stream_index);
        media::VideoFormat GetVideoFormat()
        {
            media::VideoFormat fmt = m_vidfmt;
            fmt.fourcc = media::FOURCC_RGB32;
            return fmt;
        }
    };

}
