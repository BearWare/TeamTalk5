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

using namespace vidcap;

FFMpeg3Capture::FFMpeg3Capture()
{
    InitAVConv();
}

FFMpeg3Capture::~FFMpeg3Capture()
{
    for(auto i=m_streams.begin();i!=m_streams.end();++i)
    {
        i->first->Close();
        delete i->first;
    }
}

bool FFMpeg3Capture::StartVideoCapture(const ACE_TString& deviceid,
                                       const media::VideoFormat& vidfmt,
                                       VideoCaptureListener* listener)
{
    vidcap_devices_t devs = GetDevices();
    
    VidCapDevice dev;
    for(size_t i=0;i<devs.size();i++)
    {
        if(devs[i].deviceid == deviceid)
            dev = devs[i];
    }

    std::auto_ptr<FFMpegVideoInput> streamer(createStreamer(this, dev, vidfmt));

    MediaFileProp in_prop;
    in_prop.video = vidfmt;
    in_prop.filename = deviceid;

    MediaStreamOutput out_prop;
    out_prop.video = vidfmt;

    if(!streamer->OpenFile(in_prop, out_prop))
        return false;

    if (!streamer->StartStream())
        return false;

    m_streams[streamer.release()] = listener;
    return true;
}

bool FFMpeg3Capture::StopVideoCapture(VideoCaptureListener* listener)
{
    wguard_t g(m_mutex);
    for(auto i=m_streams.begin();i!=m_streams.end();++i)
    {
        if (i->second == listener)
        {
            i->first->Close();
            delete i->first;
            m_streams.erase(i);
            return true;
        }
    }
    return false;
}

bool FFMpeg3Capture::GetVideoCaptureFormat(VideoCaptureListener* listener,
                                           media::VideoFormat& vidfmt)
{
    wguard_t g(m_mutex);
    for(auto i=m_streams.begin();i!=m_streams.end();++i)
    {
        if (i->second == listener)
        {
            vidfmt = i->first->GetVideoFormat();
            return true;
        }
    }
    return false;
}

bool FFMpeg3Capture::MediaStreamVideoCallback(MediaStreamer* streamer,
                                              media::VideoFrame& video_frame,
                                              ACE_Message_Block* mb_video)
{
    wguard_t g(m_mutex);
    auto i=m_streams.find(dynamic_cast<FFMpegVideoInput*>(streamer));
    if (i != m_streams.end())
    {
        return i->second->OnVideoCaptureCallback(video_frame, mb_video);
    }
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
