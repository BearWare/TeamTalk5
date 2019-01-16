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

#if !defined(MFCAPTURE_H)
#define MFCAPTURE_H

#include "VideoCapture.h"

#include <thread>
#include <mutex>
#include <map>

#include <ace/Future.h>

namespace vidcap {

    class MFCapture : public VideoCapture
    {
    public:
        MFCapture(const MFCapture&) = delete;
        MFCapture() {}
        virtual ~MFCapture();

        vidcap_devices_t GetDevices();

        bool StartVideoCapture(const ACE_TString& deviceid,
                               const media::VideoFormat& vidfmt,
                               VideoCaptureCallback callback);

        void StopVideoCapture();

        media::VideoFormat GetVideoCaptureFormat();

        bool RegisterVideoFormat(VideoCaptureCallback callback, media::FourCC fcc);
        void UnregisterVideoFormat(media::FourCC fcc);

    private:

        void Run(struct CaptureSession* session, ACE_TString deviceid);

        typedef std::shared_ptr<struct CaptureSession> capturesession_t;
        capturesession_t m_session;
    };

}

#endif
