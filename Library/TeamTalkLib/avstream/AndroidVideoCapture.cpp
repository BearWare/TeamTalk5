/*
 * Copyright (c) 2005-2026, BearWare.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 */

#include "AndroidVideoCapture.h"

#include <cstddef>
#include <utility>

namespace vidcap {

    namespace {
        void AddDefaultFormats(VidCapDevice& dev)
        {
            dev.vidcapformats.emplace_back(320, 240, 15, 1, media::FOURCC_I420);
            dev.vidcapformats.emplace_back(480, 640, 15, 1, media::FOURCC_I420);
            dev.vidcapformats.emplace_back(640, 480, 15, 1, media::FOURCC_I420);
            dev.vidcapformats.emplace_back(640, 480, 30, 1, media::FOURCC_I420);
            dev.vidcapformats.emplace_back(720, 1280, 15, 1, media::FOURCC_I420);
            dev.vidcapformats.emplace_back(1280, 720, 15, 1, media::FOURCC_I420);
        }
    } // namespace

    AndroidVideoCapture::AndroidVideoCapture()
    {
        VidCapDevice front;
        front.api = TTEXT("Android Camera2");
        front.deviceid = TTEXT("android:front");
        front.devicename = TTEXT("Front Camera");
        AddDefaultFormats(front);
        m_devices.push_back(std::move(front));

        VidCapDevice back;
        back.api = TTEXT("Android Camera2");
        back.deviceid = TTEXT("android:back");
        back.devicename = TTEXT("Back Camera");
        AddDefaultFormats(back);
        m_devices.push_back(std::move(back));
    }

    AndroidVideoCapture::~AndroidVideoCapture()
    {
        StopVideoCapture();
    }

    vidcap_devices_t AndroidVideoCapture::GetDevices()
    {
        return m_devices;
    }

    bool AndroidVideoCapture::InitVideoCapture(const tstring& deviceid,
                                               const media::VideoFormat& vidfmt)
    {
        std::lock_guard<std::mutex> const lock(m_mutex);

        if (deviceid.empty())
            return false;

        if (!vidfmt.IsValid() || (vidfmt.width % 2 != 0) || (vidfmt.height % 2 != 0))
            return false;

        if (vidfmt.fourcc != media::FOURCC_I420)
            return false;

        if (m_curfmt.IsValid())
            return false;

        m_curfmt = vidfmt;
        if (m_curfmt.fps_numerator <= 0 || m_curfmt.fps_denominator <= 0)
        {
            m_curfmt.fps_numerator = 15;
            m_curfmt.fps_denominator = 1;
        }
        return true;
    }

    bool AndroidVideoCapture::StartVideoCapture()
    {
        std::lock_guard<std::mutex> const lock(m_mutex);
        if (!m_curfmt.IsValid())
            return false;
        m_running = true;
        return true;
    }

    void AndroidVideoCapture::StopVideoCapture()
    {
        std::lock_guard<std::mutex> const lock(m_mutex);
        m_running = false;
        m_curfmt = media::VideoFormat();
        m_callback = {};
    }

    media::VideoFormat AndroidVideoCapture::GetVideoCaptureFormat()
    {
        std::lock_guard<std::mutex> const lock(m_mutex);
        return m_curfmt;
    }

    bool AndroidVideoCapture::RegisterVideoFormat(VideoCaptureCallback callback, media::FourCC fcc)
    {
        std::lock_guard<std::mutex> const lock(m_mutex);
        if (fcc != media::FOURCC_I420)
            return false;
        m_callback = std::move(callback);
        return true;
    }

    void AndroidVideoCapture::UnregisterVideoFormat(media::FourCC /*fcc*/)
    {
        std::lock_guard<std::mutex> const lock(m_mutex);
        m_callback = {};
    }

    bool AndroidVideoCapture::PushVideoFrame(const char* frame_data, int frame_len,
                                             int width, int height)
    {
        std::lock_guard<std::mutex> const lock(m_mutex);
        if (!m_running || !m_callback || !m_curfmt.IsValid() || frame_data == nullptr)
            return false;

        if (width != m_curfmt.width || height != m_curfmt.height)
            return false;

        int const expected_len = (width * height * 3) / 2;
        if (frame_len != expected_len)
            return false;

        // VideoThread passes bottom_up = !vid.top_down to VpxEncoder::Encode(),
        // which calls vpx_img_flip() when !bottom_up. Use top_down = false
        // (matching MFCapture) so upright Camera2 I420 frames are not flipped.
        media::VideoFrame frm(const_cast<char*>(frame_data), frame_len,
                              width, height, media::FOURCC_I420, false);
        m_callback(frm, nullptr);
        return true;
    }

} // namespace vidcap
