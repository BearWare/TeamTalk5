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

#if !defined(LIBVIDCAP_H)
#define LIBVIDCAP_H

#include <map>
#include <ace/Singleton.h>
#include <vidcap/vidcap.h>

#include <myace/MyACE.h>

#include "VideoCapture.h"

class LibVidCap : public vidcap::VideoCapture
{
public:
    LibVidCap();
    ~LibVidCap();

    vidcap::vidcap_devices_t GetDevices();

    bool InitVideoCapture(const ACE_TString& deviceid,
                          const media::VideoFormat& vidfmt);

    bool StartVideoCapture();

    void StopVideoCapture();

    media::VideoFormat GetVideoCaptureFormat();

    bool RegisterVideoFormat(VideoCaptureCallback callback, media::FourCC fcc);
    void UnregisterVideoFormat(media::FourCC fcc);

    //Callbacks for libvidcap
    static int video_capture_callback(vidcap_src* vc_src, void* user_data, 
                                      struct vidcap_capture_info* cap_info);
    //Callbacks for libvidcap
    static int video_sapi_notify(vidcap_sapi *sapi, void * user_data);


    void DoVideoCaptureCallback(media::VideoFrame& frame);
private:
    vidcap_state* m_vc_state = nullptr;
    vidcap_src* m_vc_source = nullptr;
    vidcap_sapi* m_vc_api = nullptr;

    VideoCaptureCallback m_callback;
};

#endif
