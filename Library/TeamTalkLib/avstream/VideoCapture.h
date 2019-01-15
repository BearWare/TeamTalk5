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

#if !defined(VIDEOCAPTURE_H)
#define VIDEOCAPTURE_H

#include <ace/SString.h>

#include <vector>

#include <codec/MediaUtil.h>

namespace vidcap {

    struct VidCapDevice
    {
        ACE_TString api;
        ACE_TString devicename;
        ACE_TString deviceid;

        std::vector<media::VideoFormat> vidcapformats;
    };

    typedef std::vector<VidCapDevice> vidcap_devices_t;

    class VideoCaptureListener
    {
    public:
        virtual ~VideoCaptureListener(){}
        // return true to take overship of 'mb_video'. 'mb_video' may be NULL
        virtual bool OnVideoCaptureCallback(media::VideoFrame& video_frame,
                                            ACE_Message_Block* mb_video) = 0;
    };

    class VideoCapture
    {
    public:
        virtual ~VideoCapture(){}
        virtual vidcap_devices_t GetDevices() = 0;
    
        virtual bool StartVideoCapture(const ACE_TString& deviceid,
                                       const media::VideoFormat& vidfmt,
                                       VideoCaptureListener* listener) = 0;

        virtual bool StopVideoCapture(VideoCaptureListener* listener) = 0;

        virtual media::VideoFormat GetVideoCaptureFormat(VideoCaptureListener* listener) = 0;

        virtual bool RegisterVideoFormat(VideoCaptureListener* listener, media::FourCC fcc, bool enable = true) { return false; }
    };
}

vidcap::VideoCapture* GetVideoCapture();

#define VIDCAP GetVideoCapture()

#endif /* VIDEOCAPTURE_H */
