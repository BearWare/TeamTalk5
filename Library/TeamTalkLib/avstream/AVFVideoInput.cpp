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

#include "AVFVideoInput.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

#include <sstream>

using namespace vidcap;

bool AVFVideoInput::SetupInput(AVInputFormat *iformat,
                               AVDictionary *options,
                               AVFormatContext*& fmt_ctx,
                               AVCodecContext*& aud_dec_ctx,
                               AVCodecContext*& vid_dec_ctx,
                               int& audio_stream_index,
                               int& video_stream_index)
{

    auto vidfmt = GetMediaOutput().video;
    
    iformat = av_find_input_format(m_dev.api.c_str());
    int fps = 1;
    if (vidfmt.fps_denominator)
    {
        fps = vidfmt.fps_numerator / vidfmt.fps_denominator;
        fps = std::max(1, fps);
    }

    std::ostringstream os;
    os << fps;
    av_dict_set(&options, "framerate", os.str().c_str(), 0);

    os.str("");
    os << vidfmt.width << "x" << vidfmt.height;
    av_dict_set(&options, "video_size", os.str().c_str(), 0);

    av_dict_set(&options, "pixel_format", "0rgb", 0);
    av_dict_set_int(&options, "video_device_index", atoi(m_dev.deviceid.c_str()), 0);

    return FFMpegVideoInput::SetupInput(iformat, options, fmt_ctx,
                                        aud_dec_ctx, vid_dec_ctx,
                                        audio_stream_index,
                                        video_stream_index);
}

