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

#if !defined(AVFCAPTURE_H)
#define AVFCAPTURE_H

#include "FFMpeg3Capture.h"

namespace vidcap {

    class AVFCapture : public FFMpeg3Capture
    {
        friend class ACE_Singleton<vidcap::AVFCapture, ACE_Null_Mutex>;

    protected:
        FFMpegVideoInput* createStreamer(MediaStreamListener* listener,
                                         const VidCapDevice& viddevice,
                                         const media::VideoFormat& fmt);

    public:
        AVFCapture();
        virtual ~AVFCapture();

        // VideoCapture interface
        vidcap_devices_t GetDevices();
    };

}

typedef ACE_Singleton<vidcap::AVFCapture, ACE_Null_Mutex> AVFSingleton;

#endif

