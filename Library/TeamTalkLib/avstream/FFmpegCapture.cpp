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

#include "FFmpegCapture.h"

#include <ace/Message_Block.h>
#include <ace/SString.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <assert.h>

using namespace std::placeholders;
using namespace vidcap;

FFmpegVideoInput::FFmpegVideoInput(const VidCapDevice& viddevice,
                                   const media::VideoFormat& fmt)
    : FFmpegStreamer(viddevice.deviceid, MediaStreamOutput(fmt)), m_dev(viddevice), m_vidfmt(fmt)
{
}


FFmpegCapture::FFmpegCapture()
{
    InitAVConv();
}

FFmpegCapture::~FFmpegCapture() = default;

bool FFmpegCapture::InitVideoCapture(const ACE_TString& deviceid,
                                      const media::VideoFormat& vidfmt)
{
    vidcap_devices_t devs = GetDevices();
    
    VidCapDevice dev;
    for(auto & i : devs)
    {
        if(i.deviceid == deviceid)
            dev = i;
    }

    ffmpegvideoinput_t streamer = CreateStreamer(dev, vidfmt);
    assert(streamer.get());
    streamer->RegisterVideoCallback([this](auto && PH1, auto && PH2) { return MediaStreamVideoCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }, true);
    streamer->RegisterStatusCallback([this](auto && PH1, auto && PH2) { MediaStreamStatusCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }, true);
    if (!streamer->Open())
        return false;

    m_videoinput.swap(streamer);
    
    return true;
}

bool FFmpegCapture::StartVideoCapture()
{
    if (!m_videoinput)
        return false;

    if (!m_videoinput->StartStream())
    {
        StopVideoCapture();
        return false;
    }

    return true;
}

void FFmpegCapture::StopVideoCapture()
{
    m_videoinput.reset();
    m_callback = {};
}

media::VideoFormat FFmpegCapture::GetVideoCaptureFormat()
{
    if (m_videoinput)
        return m_videoinput->GetVideoFormat();
    return {};
}

bool FFmpegCapture::RegisterVideoFormat(VideoCaptureCallback callback, media::FourCC fcc)
{
    if (m_videoinput && m_videoinput->GetVideoFormat().fourcc == fcc)
    {
        m_callback = callback;
        return true;
    }
    return false;
}

void FFmpegCapture::UnregisterVideoFormat(media::FourCC fcc)
{
    if (m_videoinput && m_videoinput->GetVideoFormat().fourcc == fcc)
    {
        m_callback = {};
    }
}

bool FFmpegCapture::MediaStreamVideoCallback(media::VideoFrame& video_frame,
                                              ACE_Message_Block* mb_video)
{
    assert(m_videoinput.get());
    
    if (m_callback)
        return m_callback(video_frame, mb_video);
    
    return false;
}

void FFmpegCapture::MediaStreamStatusCallback(const MediaFileProp& /*mfp*/,
                                               MediaStreamStatus /*status*/)
{
}
