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

#include "AVFCapture.h"

#include <memory>
#include <sstream>
#include <assert.h>

using namespace vidcap;

FFMpeg3Capture::FFMpeg3Capture()
{
}

FFMpeg3Capture::~FFMpeg3Capture()
{
}

bool FFMpeg3Capture::InitVideoCapture(const ACE_TString& deviceid,
                                      const media::VideoFormat& vidfmt)
{
    vidcap_devices_t devs = GetDevices();
    
    VidCapDevice dev;
    for(size_t i=0;i<devs.size();i++)
    {
        if(devs[i].deviceid == deviceid)
            dev = devs[i];
    }

    ffmpegvideoinput_t streamer = createStreamer(this, dev, vidfmt);
    assert(streamer.get());

    MediaFileProp in_prop;
    in_prop.video = vidfmt;
    in_prop.filename = deviceid;

    MediaStreamOutput out_prop;
    out_prop.video = vidfmt;

    if(!streamer->OpenFile(in_prop, out_prop))
        return false;

    m_videoinput.swap(streamer);
    
    return true;
}

bool FFMpeg3Capture::StartVideoCapture()
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

void FFMpeg3Capture::StopVideoCapture()
{
    m_videoinput.reset();
    m_callback = {};
}

media::VideoFormat FFMpeg3Capture::GetVideoCaptureFormat()
{
    if (m_videoinput)
        return m_videoinput->GetVideoFormat();
    return media::VideoFormat();
}

bool FFMpeg3Capture::RegisterVideoFormat(VideoCaptureCallback callback, media::FourCC fcc)
{
    if (m_videoinput && m_videoinput->GetVideoFormat().fourcc == fcc)
    {
        m_callback = callback;
        return true;
    }
    return false;
}

void FFMpeg3Capture::UnregisterVideoFormat(media::FourCC fcc)
{
    if (m_videoinput && m_videoinput->GetVideoFormat().fourcc == fcc)
    {
        m_callback = {};
    }
}

bool FFMpeg3Capture::MediaStreamVideoCallback(MediaStreamer* streamer,
                                              media::VideoFrame& video_frame,
                                              ACE_Message_Block* mb_video)
{
    assert(m_videoinput.get());
    
    if (m_callback)
        return m_callback(video_frame, mb_video);
    
    return false;
}

bool FFMpeg3Capture::MediaStreamAudioCallback(MediaStreamer* streamer,
                                              media::AudioFrame& audio_frame,
                                              ACE_Message_Block* mb_audio)
{
    return false;
}

void FFMpeg3Capture::MediaStreamStatusCallback(MediaStreamer* streamer,
                                               const MediaFileProp& mfp,
                                               MediaStreamStatus status)
{
}
