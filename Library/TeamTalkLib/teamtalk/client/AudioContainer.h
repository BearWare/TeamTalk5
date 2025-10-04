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

#ifndef AUDIOCONTAINER_H
#define AUDIOCONTAINER_H

#include "avstream/AudioResampler.h"
#include "codec/MediaUtil.h"
#include "myace/MyACE.h"
#include "mystd/MyStd.h"
#include "teamtalk/Common.h"

#include <ace/Message_Block.h>

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>

constexpr auto AUDIOCONTAINER_MAXSIZE = PCM16_BYTES(48000, 2) * 3; // 3 seconds at 48KHz stereo

struct AudioEntry
{
    msg_queue_t mq;

    audio_resampler_t resampler;
    media::AudioFormat outfmt;

    AudioEntry(const media::AudioFormat& resamplefmt) : outfmt(resamplefmt)
    {
        mq.high_water_mark(AUDIOCONTAINER_MAXSIZE);
        mq.low_water_mark(AUDIOCONTAINER_MAXSIZE);
    }
};

using audioentry_t = std::shared_ptr<AudioEntry>;

class AudioContainer : public NonCopyable
{
public:
    AudioContainer() = default;
    void Reset();

    void AddAudioSource(int userid, teamtalk::StreamTypes sts, const media::AudioFormat& af);
    void RemoveAudioSource(int userid, teamtalk::StreamTypes sts);

    bool AddAudio(int userid, teamtalk::StreamTypes sts, const media::AudioFrame& frame);
    bool Exists(int userid, teamtalk::StreamTypes sts);
    bool IsEmpty(int userid, teamtalk::StreamTypes sts);

    ACE_Message_Block* AcquireAudioFrame(int userid, teamtalk::StreamTypes sts);
    void ReleaseAllAudio();

private:
    using audiostore_t = std::map< uint32_t, audioentry_t >;
    audiostore_t m_container;
    std::recursive_mutex m_store_mtx;
};

#endif
