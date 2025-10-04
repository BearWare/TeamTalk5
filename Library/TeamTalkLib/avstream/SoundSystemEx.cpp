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

#include "SoundSystemEx.h"

#include "codec/MediaUtil.h"

#include <cassert>
#include <cstring>
#include <mutex>
#include <vector>

namespace soundsystem {

void DuplexCallback(DuplexStreamer& dpxStream,
                    const short* recorded, short* playback,
                    int mastervol, bool mastermute)
{
    size_t const bytes = PCM16_BYTES(dpxStream.framesize, dpxStream.output_channels);

    //now mix all active players
    std::memset(playback, 0, bytes);
    {
        //lock 'players' so they're not removed during callback
        std::lock_guard<std::recursive_mutex> const g(dpxStream.players_mtx);
        assert(dpxStream.tmpOutputBuffer.size());
        MuxPlayers(dpxStream.players, dpxStream.tmpOutputBuffer.data(), playback, mastervol, mastermute);
        if (dpxStream.players.empty())
        {
            std::memcpy(dpxStream.tmpOutputBuffer.data(), playback, bytes);
        }
    }
    dpxStream.duplex->StreamDuplexCb(dpxStream, recorded, playback, dpxStream.framesize);
}

void MuxPlayers(const std::vector<OutputStreamer*>& players,
                short* tmp_buffer, short* playback,
                int mastervol, bool mastermute)
{
    for(auto i : players)
    {
        StreamPlayer* player = i->player;
        if (player->StreamPlayerCb(*i,
                                   tmp_buffer,
                                   i->framesize))
        {
            SoftVolume(*i, tmp_buffer, i->framesize, mastervol, mastermute);
            int const samples = i->framesize * i->channels;
            for(int p=0;p<samples;p++)
            {
                int const val = tmp_buffer[p] + playback[p];
                if(val>32767)
                    playback[p] = 32767;
                else if(val<-32768)
                    playback[p] = -32768;
                else
                    playback[p] = (short)val;
            }
        }
    }
}

void DuplexEnded(SoundSystem* sndsys, DuplexStreamer& dpxStream)
{
    size_t i = dpxStream.players.size();
    StreamPlayer* player = nullptr;
    while((i--) != 0u)
    {
        player = dpxStream.players[i]->player;
        sndsys->RemoveDuplexOutputStream(dpxStream.duplex, player);
    }
}

void SoftVolume(const OutputStreamer& streamer,
                short* buffer, int samples,
                int mastervol, bool mastermute)
{
    auto rational = streamer.GetMasterVolumeGain(mastermute, mastervol);
    if (rational.numerator == 0)
    {
        memset(buffer, 0, PCM16_BYTES(samples,streamer.channels));
    }
    else
    {
        SOFTGAIN(buffer, samples, streamer.channels, rational.numerator, rational.denominator);
    }
}

} // namespace soundsystem

