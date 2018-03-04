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

#include "AVFCapture.h"

#import <AVFoundation/AVFoundation.h>

#include <memory>
#include <sstream>

using namespace vidcap;
using namespace media;

class AFVideoInput : public FFMpegVideoInput
{
public:
    AFVideoInput(MediaStreamListener* listener,
                 const VidCapDevice& viddevice,
                 const media::VideoFormat& fmt)
        : FFMpegVideoInput(listener, viddevice, fmt) {
    }
    
    // FFMpegStreamer override
    bool SetupInput(AVInputFormat *iformat,
                    AVDictionary *options,
                    AVFormatContext*& fmt_ctx,
                    AVCodecContext*& aud_dec_ctx,
                    AVCodecContext*& vid_dec_ctx,
                    int& audio_stream_index,
                    int& video_stream_index) {

        iformat = av_find_input_format(m_dev.api.c_str());
        int fps = 1;
        if (m_media_in.video_fps_denominator)
        {
            fps = m_media_in.video_fps_numerator / m_media_in.video_fps_denominator;
            fps = std::max(1, fps);
        }

        std::ostringstream os;
        os << fps;
        av_dict_set(&options, "framerate", os.str().c_str(), 0);

        os.str("");
        os << m_media_in.video_width << "x" << m_media_in.video_height;
        av_dict_set(&options, "video_size", os.str().c_str(), 0);

        av_dict_set(&options, "pixel_format", "0rgb", 0);
        av_dict_set_int(&options, "video_device_index", atoi(m_dev.deviceid.c_str()), 0);

        return FFMpegVideoInput::SetupInput(iformat, options, fmt_ctx,
                                            aud_dec_ctx, vid_dec_ctx,
                                            audio_stream_index,
                                            video_stream_index);
    }
    media::VideoFormat GetVideoFormat()
    {
        media::VideoFormat fmt = m_vidfmt;
        fmt.fourcc = FOURCC_RGB32;
        return fmt;
    }
};

AVFCapture::AVFCapture()
{
}

AVFCapture::~AVFCapture()
{
}

FFMpegVideoInput* AVFCapture::createStreamer(MediaStreamListener* listener,
                                             const VidCapDevice& viddevice,
                                             const media::VideoFormat& fmt)
{
    return new AFVideoInput(listener, viddevice, fmt);
}

vidcap_devices_t AVFCapture::GetDevices()
{
    vidcap_devices_t devs;

    NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    int n_devices = [devices count];

    for (AVCaptureDevice *device in devices) {
        VidCapDevice dev;
        dev.api = ACE_TEXT("avfoundation");
        dev.devicename = [[device localizedName] UTF8String];
        dev.deviceid = i2string(int([devices indexOfObject:device]));

        for(AVCaptureDeviceFormat* devfmt in [device formats])
        {
            CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions([devfmt formatDescription]);
            for(AVFrameRateRange* range in [devfmt videoSupportedFrameRateRanges])
            {
                VideoFormat fmt;
                fmt.width = dimensions.width;
                fmt.height = dimensions.height;
                fmt.fourcc = FOURCC_RGB32;
                fmt.fps_numerator = int([range minFrameRate]);
                fmt.fps_denominator = 1;
                
                if(std::find(dev.vidcapformats.begin(), dev.vidcapformats.end(), fmt) == dev.vidcapformats.end())
                    dev.vidcapformats.push_back(fmt);
                // MYTRACE(ACE_TEXT("%dx%d - min %g max %g %f %f\n"), 
                //         dimensions.width, dimensions.height,
                //         [range minFrameRate], [range maxFrameRate], 
                //         CMTimeGetSeconds([range minFrameDuration]),
                //                          CMTimeGetSeconds([range maxFrameDuration]));
                
            }
        }

        devs.push_back(dev);
    }

    return devs;
}
