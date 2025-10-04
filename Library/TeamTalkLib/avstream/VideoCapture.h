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

#include "codec/MediaUtil.h"

#include <ace/Message_Block.h>
#include <ace/SString.h>

#include <functional>
#include <memory>
#include <vector>

namespace vidcap {

    struct VidCapDevice
    {
        ACE_TString api;
        ACE_TString devicename;
        ACE_TString deviceid;

        std::vector<media::VideoFormat> vidcapformats;
    };

    using vidcap_devices_t = std::vector<VidCapDevice>;

    using videocapture_t = std::unique_ptr<class VideoCapture>;

    class VideoCapture
    {
    public:
        // return true to take overship of 'mb_video'. 'mb_video' may be NULL
        using VideoCaptureCallback = std::function<bool(media::VideoFrame& video_frame, ACE_Message_Block* mb_video)>;

        virtual ~VideoCapture() = default;

        static videocapture_t Create();

        virtual vidcap_devices_t GetDevices() = 0;

        virtual bool InitVideoCapture(const ACE_TString& deviceid,
                                      const media::VideoFormat& vidfmt) = 0;
    
        virtual bool StartVideoCapture() = 0;

        virtual void StopVideoCapture() = 0;

        virtual media::VideoFormat GetVideoCaptureFormat() = 0;

        virtual bool RegisterVideoFormat(VideoCaptureCallback callback, media::FourCC fcc) = 0;
        virtual void UnregisterVideoFormat(media::FourCC fcc) = 0;
    };
} // namespace vidcap

#endif /* VIDEOCAPTURE_H */
