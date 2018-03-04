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

#include "SpeexJitterBuf.h"
#include <assert.h>
#include <cstddef>

Speex_Jitter::Speex_Jitter(int step_size)
{
    m_jitter = jitter_buffer_init(step_size);
}

Speex_Jitter::~Speex_Jitter()
{
    jitter_buffer_destroy(m_jitter);
}

void Speex_Jitter::PutPacket(const JitterBufferPacket& packet)
{
    jitter_buffer_put(m_jitter, &packet);
}

bool Speex_Jitter::GetPacket(JitterBufferPacket& packet, 
                             spx_int32_t timespan, spx_int32_t* start_offset)
{
    int ret = jitter_buffer_get(m_jitter, &packet, timespan, start_offset);
    switch(ret)
    {
    case JITTER_BUFFER_OK :
    case JITTER_BUFFER_INSERTION :
        return true;
    case JITTER_BUFFER_MISSING :
        return false;
    default :
        return false;
    }
}

void Speex_Jitter::Tick()
{
    jitter_buffer_tick(m_jitter);
}

int Speex_Jitter::BufferSize() const
{
    int size = 0;
    int ret = jitter_buffer_ctl(m_jitter, JITTER_BUFFER_GET_AVAILABLE_COUNT, &size);
    assert(ret == 0);
    return size;
}

void Speex_Jitter::Reset()
{
    jitter_buffer_reset(m_jitter);
}

int Speex_Jitter::GetFramesLost() const
{
    //return m_jitter.lost_count;
    return 0;    //TODO: need new way to check this
}

