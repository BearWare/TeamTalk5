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

#if !defined(QTVIDCAP_H)
#define QTVIDCAP_H

#include <ace/Bound_Ptr.h>
#include <ace/Null_Mutex.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Singleton.h>

#include "VideoCapture.h"

#include <map>

struct QTVidCapSession
{
    media::VideoFormat vidfmt;
};

class QTVidCap : public vidcap::VideoCapture
{
    QTVidCap();
    friend class ACE_Singleton<QTVidCap, ACE_Null_Mutex>;
public:
    vidcap::vidcap_devices_t GetDevices();
    bool StartVideoCapture(const ACE_TString& deviceid,
                           const media::VideoFormat& vidfmt,
                           vidcap::VideoCaptureListener* listener);

    bool StopVideoCapture(vidcap::VideoCaptureListener* listener);

    bool GetVideoCaptureFormat(vidcap::VideoCaptureListener* listener,
                               media::VideoFormat& vidfmt);

private:
    typedef ACE_Strong_Bound_Ptr< QTVidCapSession, ACE_Null_Mutex > vidcap_session_t;
    typedef std::map<vidcap::VideoCaptureListener*, vidcap_session_t> vidcaplisteners_t;
    vidcaplisteners_t m_mListeners;

    ACE_Recursive_Thread_Mutex m_mutex;
};

void CocoaEventLoop();

typedef ACE_Singleton<QTVidCap, ACE_Null_Mutex> QTKitInst;

#endif /* QTVIDCAP_H */
