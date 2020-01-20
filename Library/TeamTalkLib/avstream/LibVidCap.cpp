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
    m_vc_state = vidcap_initialize();
    assert(m_vc_state);
}

LibVidCap::~LibVidCap()
{
    StopVideoCapture();

    if (m_vc_source)
    {
        int ret = vidcap_src_release(m_vc_source);
        assert(ret == 0);
    }

    if (m_vc_api)
    {
        int ret = vidcap_sapi_release(m_vc_api);
        assert(ret == 0);
    }

    if(m_vc_state)
        vidcap_destroy(m_vc_state);
}

vidcap_devices_t LibVidCap::GetDevices()
{
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
        vidcap_sapi* vc_api = vidcap_sapi_acquire(m_vc_state, &sapi_infos[i]);

        int n_devices = vidcap_src_list_update(vc_api);
        if(n_devices>0)
        {
            std::vector<struct vidcap_src_info> src_infos;
            src_infos.resize(n_devices);
            ret = vidcap_src_list_get(vc_api, n_devices, &src_infos[0]);
            assert(ret == 0);

            for(int j=0;j<n_devices;j++)
            {
                vidcap_src* vc_src = vidcap_src_acquire(vc_api, &src_infos[j]);
                if(!vc_src)
                    continue;

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

                vidcap_src_release(vc_src);
            }
        }

        ret = vidcap_sapi_release(vc_api);
        assert(ret == 0);
    }
    return devices;
}

bool LibVidCap::InitVideoCapture(const ACE_TString& deviceid,
                                  const media::VideoFormat& vidfmt)
{
    if(!m_vc_state)
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

    struct vidcap_sapi_info sapi_info;
    struct vidcap_src_info src_info;

    ACE_OS::strncpy(sapi_info.identifier, sapi_id.c_str(),
        VIDCAP_NAME_LENGTH);
    ACE_OS::strncpy(src_info.identifier, deviceid.c_str(),
        VIDCAP_NAME_LENGTH);

    m_vc_api = vidcap_sapi_acquire(m_vc_state, &sapi_info);

    if(!m_vc_api)
        return false;

    ret = vidcap_srcs_notify(m_vc_api, LibVidCap::video_sapi_notify, this);
    assert(ret == 0);

    m_vc_source = vidcap_src_acquire(m_vc_api, &src_info);
    if(!m_vc_source)
    {
        vidcap_sapi_release(m_vc_api);
        m_vc_api = nullptr;
        return false;
    }

    struct vidcap_fmt_info fmt_info;
    fmt_info.width = vidfmt.width;
    fmt_info.height = vidfmt.height;
    fmt_info.fourcc = vidfmt.fourcc;
    fmt_info.fps_numerator = vidfmt.fps_numerator;
    fmt_info.fps_denominator = vidfmt.fps_denominator;

    if(vidcap_format_bind(m_vc_source, &fmt_info) != 0)
    {
        ret = vidcap_src_release(m_vc_source);
        m_vc_source = nullptr;
        vidcap_sapi_release(m_vc_api);
        m_vc_api = nullptr;
        assert(ret == 0);
        return false;
    }

    return true;
}

bool LibVidCap::StartVideoCapture()
{
    if (!m_vc_source)
        return false;

    if(vidcap_src_capture_start(m_vc_source, LibVidCap::video_capture_callback, this) != 0)
        return false;

    return true;

}

void LibVidCap::StopVideoCapture()
{
    if (!m_vc_state)
        return;

    if (!m_vc_source)
        return;

    int ret = vidcap_src_capture_stop(m_vc_source);
    assert( ret == 0);

    m_vc_source = nullptr;
}

media::VideoFormat LibVidCap::GetVideoCaptureFormat()
{
    struct vidcap_fmt_info fmt_info;
    if (m_vc_source && vidcap_format_info_get(m_vc_source, &fmt_info) == 0)
    {
        media::VideoFormat vidfmt;
        Convert(fmt_info, vidfmt);
        return vidfmt;
    }
    return media::VideoFormat();
}

bool LibVidCap::RegisterVideoFormat(VideoCaptureCallback callback, media::FourCC fcc)
{
    if (!m_callback && GetVideoCaptureFormat().fourcc == fcc)
    {
        m_callback = callback;
        return true;
    }
    return false;
}

void LibVidCap::UnregisterVideoFormat(media::FourCC fcc)
{
    m_callback = {};
}

void LibVidCap::DoVideoCaptureCallback(media::VideoFrame& frame)
{
    if (m_callback)
        m_callback(frame, nullptr);
}

int LibVidCap::video_capture_callback(vidcap_src* vc_src, void* user_data, 
                                      struct vidcap_capture_info* cap_info)
{
    LibVidCap* lsn = static_cast<LibVidCap*>(user_data);

    media::VideoFormat vidfmt = lsn->GetVideoCaptureFormat();
    if(vidfmt.IsValid() && cap_info->video_data && cap_info->video_data_size)
    {
        ACE_Time_Value tm(cap_info->capture_time_sec, cap_info->capture_time_usec);
        ACE_Time_Value diff = ACE_OS::gettimeofday() - tm;
        ACE_UINT32 tm_msec = GETTIMESTAMP() - (ACE_UINT32)diff.msec();
        media::VideoFrame vid_frm(const_cast<char*>(cap_info->video_data), cap_info->video_data_size,
                                  vidfmt.width, vidfmt.height, vidfmt.fourcc, true);
        vid_frm.timestamp = tm_msec;
        lsn->DoVideoCaptureCallback(vid_frm);
    }
    return 0;
}

int LibVidCap::video_sapi_notify(vidcap_sapi *sapi, void * user_data)
{
    //TODO: should be possible to detect when web-cam is removed.
    return 0;
}


