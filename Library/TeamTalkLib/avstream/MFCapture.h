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
                               VideoCaptureListener* listener);

        bool StopVideoCapture(VideoCaptureListener* listener);


        bool GetVideoCaptureFormat(VideoCaptureListener* listener,
                                   media::VideoFormat& vidfmt);
    private:
        struct CaptureSession
        {
            CaptureSession(const CaptureSession&) = delete;
            CaptureSession(ACE_TString id, const media::VideoFormat& fmt)
            : deviceid(id), vidfmt(fmt) {}

            ACE_TString deviceid;
            media::VideoFormat vidfmt;
            std::shared_ptr<std::thread> capturethread;
            ACE_Future<bool> opened;
            bool stop = false;
        };

        void Run(CaptureSession* session, VideoCaptureListener* listener);

        std::mutex m_mutex;
        std::map<VideoCaptureListener*, std::shared_ptr<CaptureSession> > m_sessions;
    };

}

typedef ACE_Singleton<vidcap::MFCapture, ACE_Null_Mutex> MFCaptureSingleton;

#endif
