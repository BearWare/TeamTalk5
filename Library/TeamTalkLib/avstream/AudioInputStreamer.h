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

#ifndef AUDIOINPUTSTREAMER_H
#define AUDIOINPUTSTREAMER_H

#include "MediaStreamer.h"
#include "AudioResampler.h"
#include "codec/MediaUtil.h"
#include "myace/MyACE.h"

#include <ace/Message_Block.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

struct AudioInputStatus
{
    AudioInputStatus(uint32_t qmsec, uint32_t elapmsec, int sid)
    : queueduration_msec(qmsec), elapsed_msec(elapmsec), streamid(sid) {}
    uint32_t queueduration_msec = 0;
    uint32_t elapsed_msec = 0;
    int streamid = 0;
};

using audioinput_statuscallback_t = std::function< void(const AudioInputStatus& ais) >;

class AudioInputStreamer : public MediaStreamer
{
public:
    AudioInputStreamer(int streamid, const MediaStreamOutput& out_prop);
    ~AudioInputStreamer() override;

    void RegisterAudioInputStatusCallback(audioinput_statuscallback_t cb, bool enable);

    bool InsertAudio(const media::AudioFrame& frame);

    bool Flush();
    
    int GetStreamID() const { return m_streamid; }

protected:
    void AudioProgress(uint32_t queuedmsec, uint32_t elapsedmsec) override;
    void Run() override;

private:
    audioinput_statuscallback_t m_statuscb;

    // @return True = Flush
    bool ProcessResample();
    msg_queue_t m_resample_frames;
    media::AudioFormat m_inputfmt;
    audio_resampler_t m_resampler;
    std::vector<short> m_resamplebuffer;

    bool Submit(const media::AudioFrame& frame);
    bool Submit(ACE_Message_Block* mb);
    void UpdateTimeStamp(media::AudioFrame& frame);
    int64_t m_sampleindex = 0;
    int m_streamid;
};

using audioinput_streamer_t = std::shared_ptr< AudioInputStreamer >;

#endif

