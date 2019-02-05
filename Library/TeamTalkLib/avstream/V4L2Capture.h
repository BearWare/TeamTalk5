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

#if !defined(V4L2CAPTURE_H)
#define V4L2CAPTURE_H

#include "FFMpeg3Capture.h"

namespace vidcap {

    class V4L2Capture : public FFMpeg3Capture
    {

    protected:
        ffmpegvideoinput_t createStreamer(MediaStreamListener* listener,
                                          const VidCapDevice& viddevice,
                                          const media::VideoFormat& fmt);
    public:
        V4L2Capture();
        virtual ~V4L2Capture();

        // VideoCapture interface
        vidcap_devices_t GetDevices();
    };

}

#endif

