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

using namespace vidcap;
using namespace media;


AVFCapture::AVFCapture()
{
}

AVFCapture::~AVFCapture()
{
}

ffmpegvideoinput_t AVFCapture::createStreamer(const VidCapDevice& viddevice,
                                              const media::VideoFormat& fmt)
{
    return ffmpegvideoinput_t(new AVFVideoInput(viddevice, fmt));
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
