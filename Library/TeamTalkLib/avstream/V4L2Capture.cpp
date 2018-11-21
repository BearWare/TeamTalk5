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

#include "V4L2Capture.h"

#include <sys/ioctl.h>
#include <linux/videodev2.h>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avstring.h>
#include <libavutil/opt.h>
}

#include <memory>
#include <sstream>
#include <iostream>

#include <codec/MediaUtil.h>

using namespace std;
using namespace vidcap;
using namespace media;

class V4L2Input : public FFMpegVideoInput
{
public:
    V4L2Input(MediaStreamListener* listener,
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

        return FFMpegVideoInput::SetupInput(iformat, options, fmt_ctx,
                                            aud_dec_ctx, vid_dec_ctx,
                                            audio_stream_index,
                                            video_stream_index);
    }

    media::VideoFormat GetVideoFormat()
    {
        media::VideoFormat fmt = m_vidfmt;
        // fmt.fourcc = FOURCC_RGB32;
        return fmt;
    }
};

V4L2Capture::V4L2Capture()
{
}

V4L2Capture::~V4L2Capture()
{
}

FFMpegVideoInput* V4L2Capture::createStreamer(MediaStreamListener* listener,
                                            const VidCapDevice& viddevice,
                                            const media::VideoFormat& fmt)
{
    return new V4L2Input(listener, viddevice, fmt);
}

void FillVidCapDevice(int fd, VidCapDevice& dev);

vidcap_devices_t V4L2Capture::GetDevices()
{
    vidcap_devices_t devs;

    AVInputFormat* in_fmt = av_input_video_device_next(NULL), *indev_fmt = NULL;
    while(in_fmt) {
        if (av_match_name("v4l2", in_fmt->name)) {
            indev_fmt = in_fmt;
        }
        in_fmt = av_input_video_device_next(in_fmt);
    }

    if(!indev_fmt)
        return devs;

    AVDeviceInfoList* device_list = NULL;
    AVDictionary *device_options = NULL;

    int ndevs = avdevice_list_input_sources(indev_fmt, NULL, device_options,
                                            &device_list);
    for(int i=0;i<device_list->nb_devices;i++) {
        AVDeviceInfo* dev = device_list->devices[i];
        VidCapDevice newdev;
        newdev.api = ACE_TEXT("v4l2");
        newdev.devicename = dev->device_description;
        newdev.deviceid = dev->device_name;

        int fd = open(newdev.deviceid.c_str(), O_RDONLY);
        if(fd >= 0)
        {
            FillVidCapDevice(fd, newdev);
            close(fd);
        }

        devs.push_back(newdev);
    }
    return devs;
}

void FillVidCapDevice(int fd, VidCapDevice& dev)
{
    v4l2_fmtdesc pixel_format = {};
    pixel_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (ioctl(fd, VIDIOC_ENUM_FMT, &pixel_format) == 0)
    {
        media::VideoFormat fmt;
        switch(pixel_format.pixelformat) {
        case V4L2_PIX_FMT_YUV420:
            fmt.fourcc = FOURCC_I420;
            break;
        case V4L2_PIX_FMT_YUYV:
            fmt.fourcc = FOURCC_YUY2;
            break;
        case V4L2_PIX_FMT_MJPEG:
            MYTRACE(ACE_TEXT("MJPEG not supported\n"));
            break;
        case V4L2_PIX_FMT_JPEG:
            MYTRACE(ACE_TEXT("JPEG not supported\n"));
            break;
        case V4L2_PIX_FMT_RGB32 :
            fmt.fourcc = FOURCC_RGB32;
            break;
        default :
            MYTRACE(ACE_TEXT("Unknown pixel format: %X\n"), pixel_format.pixelformat);
            break;
        }

        v4l2_frmsizeenum frame_size = {};
        frame_size.pixel_format = pixel_format.pixelformat;
        while (fmt.fourcc != FOURCC_NONE &&
               ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frame_size) == 0)
        {
            switch(frame_size.type) {
            case V4L2_FRMSIZE_TYPE_DISCRETE :
                fmt.width = frame_size.discrete.width;
                fmt.height = frame_size.discrete.height;
                break;
            case V4L2_FRMSIZE_TYPE_STEPWISE :
                MYTRACE(ACE_TEXT("Stepwise frame size not supported\n"));
                break;
            case V4L2_FRMSIZE_TYPE_CONTINUOUS :
                MYTRACE(ACE_TEXT("Continuous frame size not supported\n"));
                break;
            }

            v4l2_frmivalenum frame_interval = {};
            frame_interval.pixel_format = pixel_format.pixelformat;
            frame_interval.width = frame_size.discrete.width;
            frame_interval.height = frame_size.discrete.height;
            while (fmt.width && fmt.height &&
                   ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frame_interval) == 0)
            {
                switch(frame_interval.type)
                {
                case V4L2_FRMIVAL_TYPE_DISCRETE :
                    if (frame_interval.discrete.numerator != 0) {
                        fmt.fps_numerator = frame_interval.discrete.denominator;
                        fmt.fps_denominator = frame_interval.discrete.numerator;

                        dev.vidcapformats.push_back(fmt);
                        MYTRACE("Added format\n");
                    }
                    break;
                case V4L2_FRMIVAL_TYPE_STEPWISE :
                    MYTRACE(ACE_TEXT("Stepwise frame ival not supported\n"));
                    break;
                case V4L2_FRMIVAL_TYPE_CONTINUOUS :
                    MYTRACE(ACE_TEXT("Continuous frame ival not supported\n"));
                    break;
                }
                ++frame_interval.index;
            }
            ++frame_size.index;
        }
        ++pixel_format.index;
    }
}
