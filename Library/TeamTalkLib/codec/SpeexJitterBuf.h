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

#if !defined(SPEEXJITTER_H)
#define SPEEXJITTER_H

#include <speex/speex.h>
#include <speex/speex_jitter.h>

class Speex_Jitter
{
public:
    Speex_Jitter(int step_size);
    ~Speex_Jitter();

    void PutPacket(const JitterBufferPacket& packet);
    bool GetPacket(JitterBufferPacket& packet, 
        spx_int32_t timespan, spx_int32_t* start_offset);
    void Tick();
    int BufferSize() const;
    void Reset();
    int GetFramesLost() const;

private:
    JitterBuffer* m_jitter;
};

#endif
