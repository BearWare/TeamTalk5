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

#include "VideoCapture.h"

using namespace vidcap;

#if defined(ENABLE_MEDIAFOUNDATION)
#include "MFCapture.h"
typedef MFCapture videocapturedevice_t;
#elif defined(ENABLE_LIBVIDCAP)
#error "Not implemented yet"
#include "LibVidCap.h"
#elif defined(ENABLE_QTKIT)
#error "No longer supported"
#include "QTVidCap.h"
#elif defined(ENABLE_AVF)
#error "Not implemented yet"
#include "AVFCapture.h"
#elif defined(ENABLE_V4L2)
#error "Not implemented yet"
#include "V4L2Capture.h"
#else

class NullVideoCapture : public VideoCapture
{
public:
    vidcap_devices_t GetDevices() { return vidcap_devices_t(); }

    bool StartVideoCapture(const ACE_TString& deviceid,
        const media::VideoFormat& vidfmt,
        VideoCaptureListener* listener) { return false; }

    bool StopVideoCapture(VideoCaptureListener* listener) { return false; }


    bool GetVideoCaptureFormat(VideoCaptureListener* listener,
        media::VideoFormat& vidfmt) { return false; }

};
typedef NullVideoCapture videocapturedevice_t;
#endif

videocapture_t VideoCapture::Create()
{
    return videocapture_t(new videocapturedevice_t());
}
