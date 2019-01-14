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
private:
    LibVidCap();
    ~LibVidCap();
    void Initialize();
    friend class ACE_Singleton<LibVidCap, ACE_Null_Mutex>;

public:
    vidcap::vidcap_devices_t GetDevices();

    bool StartVideoCapture(const ACE_TString& deviceid,
                           const media::VideoFormat& vidfmt,
                           vidcap::VideoCaptureListener* listener);

    bool StopVideoCapture(vidcap::VideoCaptureListener* listener);

    media::VideoFormat GetVideoCaptureFormat(vidcap::VideoCaptureListener* listener);

    //Callbacks for libvidcap
    static int video_capture_callback(vidcap_src* vc_src, void* user_data, 
                                      struct vidcap_capture_info* cap_info);
    //Callbacks for libvidcap
    static int video_sapi_notify(vidcap_sapi *sapi, void * user_data);


    void VideoCaptureCallback(vidcap::VideoCaptureListener* listener,
                              const char* video_data, int data_lenth, 
                              const ACE_Time_Value& tm);
private:
    vidcap_state* m_vc_state;

    typedef std::map<ACE_TString, vidcap_sapi*> active_apis_t;
    active_apis_t m_mActiveAPIs;
    typedef std::map<ACE_TString, vidcap_src*> active_srcs_t;
    active_srcs_t m_mActiveSources;

    typedef std::map<vidcap::VideoCaptureListener*, vidcap_src*> vidcaplisteners_t;
    vidcaplisteners_t m_mListeners;

    ACE_Recursive_Thread_Mutex m_mutex;
};

typedef ACE_Singleton<LibVidCap, ACE_Null_Mutex> VCSingleton;

#endif
