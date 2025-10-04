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
#include "LibVidCap.h"
typedef LibVidCap videocapturedevice_t;

#elif defined(ENABLE_AVF)
#include "AVFCapture.h"
using videocapturedevice_t = vidcap::AVFCapture;

#elif defined(ENABLE_V4L2)
#include "V4L2Capture.h"
typedef V4L2Capture videocapturedevice_t;

#else

#include "codec/MediaUtil.h"

#include "myace/MyACE.h"

#include <ace/Reactor.h>
#include <ace/Task.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

#define TEAMTALK_VIRTUAL_VIDEO_CAPTURE_DEVICE ACE_TEXT("1978") /* same as SOUND_DEVICEID_VIRTUAL */


class NullVideoCapture : public VideoCapture
        , private ACE_Task_Base
{
public:
    NullVideoCapture()
    {
        m_dev.api = ACE_TEXT("TeamTalk Virtual Video Capture");
        m_dev.deviceid = TEAMTALK_VIRTUAL_VIDEO_CAPTURE_DEVICE;
        m_dev.devicename = ACE_TEXT("TeamTalk Virtual Video Capture Device #0");
        m_dev.vidcapformats.emplace_back(320, 240, 30, 1, media::FOURCC_RGB32);
        m_dev.vidcapformats.emplace_back(640, 480, 15, 1, media::FOURCC_RGB32);
    }

    ~NullVideoCapture() override
    {
        StopVideoCapture();
    }

    vidcap_devices_t GetDevices() override
    {
        vidcap_devices_t devs;
        devs.push_back(m_dev);
        return devs;
    }

    bool InitVideoCapture(const ACE_TString& deviceid,
                          const media::VideoFormat& vidfmt) override
    {
        if (deviceid != m_dev.deviceid)
            return false;

        auto i = std::ranges::find(m_dev.vidcapformats, vidfmt);
        if (i == m_dev.vidcapformats.end())
            return false;

        if (m_curfmt.IsValid())
            return false;

        if (activate() < 0)
            return false;

        m_curfmt = vidfmt;
        return true;
    }
    
    bool StartVideoCapture() override
    {
        if (!m_curfmt.IsValid())
            return false;

        assert(m_curfmt.fps_denominator);
        auto msec = 1000. * (1 / (m_curfmt.fps_numerator / double(m_curfmt.fps_denominator)));
        auto tv = ToTimeValue(uint32_t(msec));
        return m_reactor.schedule_timer(this, nullptr, ACE_Time_Value(), tv) >= 0;
    }

    void StopVideoCapture() override
    {
        m_reactor.cancel_timer(this);
        m_reactor.end_reactor_event_loop();
        wait();

        m_curfmt = media::VideoFormat();
        UnregisterVideoFormat(media::FOURCC_NONE);
    }

    media::VideoFormat GetVideoCaptureFormat() override
    {
        return m_curfmt;
    }

    bool RegisterVideoFormat(VideoCaptureCallback callback, media::FourCC fcc) override
    {
        if (fcc != m_curfmt.fourcc)
            return false;

        m_callback = callback;
        return true;
    }

    void UnregisterVideoFormat(media::FourCC  /*fcc*/) override
    {
        m_callback = {};
    }

    int handle_timeout(const ACE_Time_Value&  /*tv*/, const void*  /*arg*/) override
    {
        uint32_t const c = (0xff << ((ACE_OS::gettimeofday().sec() % 4) * 8));

        int const rgbsize = RGB32_BYTES(m_curfmt.width, m_curfmt.height);
        std::vector<uint32_t> img(rgbsize / 4, c);

        if (m_curfmt.IsValid() && m_callback)
        {
            media::VideoFrame frm(m_curfmt, reinterpret_cast<char*>(img.data()), rgbsize);
            m_callback(frm, nullptr);
        }

        return 0;
    }

    int svc() override
    {
        m_reactor.owner (ACE_OS::thr_self ());
        m_reactor.run_reactor_event_loop();
        return 0;
    }

private:
    VidCapDevice m_dev;
    media::VideoFormat m_curfmt;
    ACE_Reactor m_reactor;
    VideoCaptureCallback m_callback;
};
using videocapturedevice_t = NullVideoCapture;
#endif

videocapture_t VideoCapture::Create()
{
    return videocapture_t(new videocapturedevice_t());
}
