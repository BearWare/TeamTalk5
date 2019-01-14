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

#include "LibVidCap.h"
#include <assert.h>

using namespace std;
using namespace media;
using namespace vidcap;

void Convert(const struct vidcap_sapi_info& sapi,
             const struct vidcap_src_info& src,
             VidCapDevice& device)
{
    device.api = sapi.identifier;
    device.devicename = src.description;
    device.deviceid = src.identifier;
}

void Convert(const struct vidcap_fmt_info& fmt_info,
             media::VideoFormat& vidfmt)
{
    vidfmt.width = fmt_info.width;
    vidfmt.height = fmt_info.height;
    vidfmt.fourcc = (media::FourCC)fmt_info.fourcc;
    vidfmt.fps_numerator = fmt_info.fps_numerator;
    vidfmt.fps_denominator = fmt_info.fps_denominator;
}

LibVidCap::LibVidCap()
: m_vc_state(NULL)
{
}

LibVidCap::~LibVidCap()
{
    if(m_vc_state)
        vidcap_destroy(m_vc_state);
}

void LibVidCap::Initialize()
{
    if(!m_vc_state)
    {
        m_vc_state = vidcap_initialize();
        assert(m_vc_state);

    }
}

vidcap_devices_t LibVidCap::GetDevices()
{
    Initialize();

    int ret;
    vidcap_devices_t devices;
    if(!m_vc_state)
        return devices;    

    std::vector<struct vidcap_sapi_info> sapi_infos;
    struct vidcap_sapi_info api;
    size_t i=0;
    while(vidcap_sapi_enumerate(m_vc_state, (int)i++, &api))
        sapi_infos.push_back(api);

    for(i=0;i<sapi_infos.size();i++)
    {
        vidcap_sapi* sapi;
        bool release = true;
        active_apis_t::iterator ite = m_mActiveAPIs.find(sapi_infos[i].identifier);
        if(ite == m_mActiveAPIs.end())
            sapi = vidcap_sapi_acquire(m_vc_state, &sapi_infos[i]);
        else
        {
            sapi = ite->second;
            release = false;
        }

        int n_devices = vidcap_src_list_update(sapi);
        if(n_devices>0)
        {
            std::vector<struct vidcap_src_info> src_infos;
            src_infos.resize(n_devices);
            ret = vidcap_src_list_get(sapi, n_devices, &src_infos[0]);
            assert(ret == 0);

            for(int j=0;j<n_devices;j++)
            {
                bool release_src;
                vidcap_src* vc_src;
                active_srcs_t::iterator ite = m_mActiveSources.find(src_infos[j].identifier);
                if(ite == m_mActiveSources.end())
                {
                    vc_src = vidcap_src_acquire(sapi, &src_infos[j]);
                    if(!vc_src)
                        continue;
                    release_src = true;
                }
                else
                {
                    vc_src = ite->second;
                    release_src = false;
                }

                VidCapDevice device;
                Convert(sapi_infos[i], src_infos[j], device);

                //get formats supported by device
                int k=0;
                struct vidcap_fmt_info fmt_info;
                while(vidcap_format_enumerate(vc_src, k++, &fmt_info))
                {
                    VideoFormat vidfmt;
                    Convert(fmt_info, vidfmt);
                    device.vidcapformats.push_back(vidfmt);
                }

                devices.push_back(device);

                if(release_src)
                    vidcap_src_release(vc_src);
            }
        }
        if(release)
        {
            ret = vidcap_sapi_release(sapi);
            assert(ret == 0);
        }
    }
    return devices;
}

bool LibVidCap::StartVideoCapture(const ACE_TString& deviceid,
                                  const media::VideoFormat& vidfmt,
                                  VideoCaptureListener* listener)
{
    wguard_t g(m_mutex);
    Initialize();

    if(m_mListeners.find(listener) != m_mListeners.end() || !m_vc_state)
        return false;

#if defined(WIN32)
    const ACE_TString sapi_id = ACE_TEXT("DirectShow"); //from sapi_dshow.cpp
#elif defined(__APPLE__)
    //look in 'sapi_qt.c' for Mac support and 'sapi_v4l.c' for Linux support
    const ACE_TString sapi_id = ACE_TEXT("quicktime"); //from sapi_dshow.cpp
#else
    //look in 'sapi_v4l.c' for Linux support
    const ACE_TString sapi_id = ACE_TEXT("video4linux"); //from sapi_v4l.cpp
#endif

    int ret;
    vidcap_sapi* vc_sapi;

    struct vidcap_sapi_info sapi_info;
    struct vidcap_src_info src_info;

    ACE_OS::strncpy(sapi_info.identifier, sapi_id.c_str(),
                    VIDCAP_NAME_LENGTH);
    ACE_OS::strncpy(src_info.identifier, deviceid.c_str(),
                    VIDCAP_NAME_LENGTH);

    active_apis_t::iterator ite = m_mActiveAPIs.find(sapi_info.identifier);
    if(ite != m_mActiveAPIs.end())
    {
        vc_sapi = ite->second;
        g.release();
    }
    else
    {
        g.release();
        vc_sapi = vidcap_sapi_acquire(m_vc_state, &sapi_info);
    }

    if(!vc_sapi)
        return false;
    else
    {
        g.acquire();
        m_mActiveAPIs[sapi_info.identifier] = vc_sapi;
        g.release();

        ret = vidcap_srcs_notify(vc_sapi, LibVidCap::video_sapi_notify, this);
        assert(ret == 0);
    }

    vidcap_src* vc_src = vidcap_src_acquire(vc_sapi, &src_info);
    if(!vc_src)
        return false;

    struct vidcap_fmt_info fmt_info;
    fmt_info.width = vidfmt.width;
    fmt_info.height = vidfmt.height;
    fmt_info.fourcc = vidfmt.fourcc;
    fmt_info.fps_numerator = vidfmt.fps_numerator;
    fmt_info.fps_denominator = vidfmt.fps_denominator;

    if(vidcap_format_bind(vc_src, &fmt_info) != 0)
    {
        ret = vidcap_src_release(vc_src);
        assert(ret == 0);
        return false;
    }

    if((ret = vidcap_src_capture_start(vc_src, LibVidCap::video_capture_callback, listener)) != 0)
    {
        ret = vidcap_src_release(vc_src);
        assert(ret == 0);
        return false;
    }

    g.acquire();
    m_mListeners[listener] = vc_src;
    m_mActiveSources[src_info.identifier] = vc_src;

    return true;
}

bool LibVidCap::StopVideoCapture(VideoCaptureListener* listener)
{
    int ret;
    wguard_t g(m_mutex);
    vidcaplisteners_t::iterator ite = m_mListeners.find(listener);
    if(ite == m_mListeners.end())
        return false;

    Initialize();

    assert(m_vc_state);

    vidcap_src* vc_src = ite->second;
    g.release();

    if(vidcap_src_capture_stop(vc_src) != 0)
        return false;

    ret = vidcap_src_release(vc_src);
    assert(ret == 0);

    //release API if all devices are stopped
    g.acquire();
    m_mListeners.erase(listener);

    active_srcs_t::iterator ite_src = m_mActiveSources.begin();
    while(ite_src != m_mActiveSources.end())
    {
        if(ite_src->second == vc_src)
        {
            m_mActiveSources.erase(ite_src);
            break;
        }
        ite_src++;
    }

    if(m_mListeners.empty())
    {
        while(m_mActiveAPIs.size())
        {
            ret = vidcap_sapi_release(m_mActiveAPIs.begin()->second);
            assert(ret == 0);
            m_mActiveAPIs.erase(m_mActiveAPIs.begin());
        }
    }
    return true;
}

media::VideoFormat LibVidCap::GetVideoCaptureFormat(vidcap::VideoCaptureListener* listener)
{
    wguard_t g(m_mutex);

    vidcaplisteners_t::iterator ii = m_mListeners.find(listener);
    if(ii == m_mListeners.end())
        return media::VideoFormat();

    struct vidcap_fmt_info fmt_info;
    if(vidcap_format_info_get(ii->second, &fmt_info) == 0)
    {
        media::VideoFormat vidfmt;
        Convert(fmt_info, vidfmt);
        return vidfmt;
    }
    return media::VideoFormat();
}

int LibVidCap::video_capture_callback(vidcap_src* vc_src, void* user_data, 
                                      struct vidcap_capture_info* cap_info)
{
    VideoCaptureListener* lsn = static_cast<VideoCaptureListener*>(user_data);
    media::VideoFormat vidfmt = VCSingleton::instance()->GetVideoCaptureFormat(lsn);
    if(vidfmt.IsValid() && cap_info->video_data && cap_info->video_data_size)
    {
        ACE_Time_Value tm(cap_info->capture_time_sec, cap_info->capture_time_usec);
        ACE_Time_Value diff = ACE_OS::gettimeofday() - tm;
        ACE_UINT32 tm_msec = GETTIMESTAMP() - (ACE_UINT32)diff.msec();
        media::VideoFrame vid_frm(const_cast<char*>(cap_info->video_data), cap_info->video_data_size,
                                  vidfmt.width, vidfmt.height, vidfmt.fourcc, true);
        vid_frm.timestamp = tm_msec;
        lsn->OnVideoCaptureCallback(vid_frm, NULL);
    }
    return 0;
}

int LibVidCap::video_sapi_notify(vidcap_sapi *sapi, void * user_data)
{
    //TODO: should be possible to detect when web-cam is removed.
    return 0;
}


