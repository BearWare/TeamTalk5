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

#include <codec/MediaUtil.h>

#include <cstring>

namespace soundsystem {

void DuplexCallback(SoundSystem* sndsys, DuplexStreamer& dpxStream,
                    const short* recorded, short* playback)
{
    size_t bytes = PCM16_BYTES(dpxStream.framesize, dpxStream.output_channels);
    dpxStream.duplex->StreamDuplexEchoCb(dpxStream, recorded, &dpxStream.tmpOutputBuffer[0], dpxStream.framesize);

    //now mix all active players
    std::memset(playback, 0, bytes);
    {
        //lock 'players' so they're not removed during callback
        std::lock_guard<std::recursive_mutex> g(dpxStream.players_mtx);
        assert(dpxStream.tmpOutputBuffer.size());
        MuxPlayers(sndsys, dpxStream.players, &dpxStream.tmpOutputBuffer[0], playback);
        if (dpxStream.players.empty())
        {
            std::memcpy(&dpxStream.tmpOutputBuffer[0], playback, bytes);
        }
    }
    dpxStream.duplex->StreamDuplexCb(dpxStream, recorded, playback, dpxStream.framesize);
}

void MuxPlayers(SoundSystem* sndsys, const std::vector<OutputStreamer*>& players,
                short* tmp_buffer, short* playback)
{
    for(size_t i=0;i<players.size();i++)
    {
        StreamPlayer* player = players[i]->player;
        if (player->StreamPlayerCb(*players[i],
                                   tmp_buffer,
                                   players[i]->framesize))
        {
            SoftVolume(sndsys, *players[i], tmp_buffer, players[i]->framesize);
            int samples = players[i]->framesize * players[i]->channels;
            for(int p=0;p<samples;p++)
            {
                int val = tmp_buffer[p] + playback[p];
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
    StreamPlayer* player;
    while(i--)
    {
        player = dpxStream.players[i]->player;
        sndsys->RemoveDuplexOutputStream(dpxStream.duplex, player);
    }
}

void SoftVolume(SoundSystem* sndsys, const OutputStreamer& streamer,
                short* buffer, int samples)
{
    int mastervolume = sndsys->GetMasterVolume(streamer.sndgrpid);
    if (mastervolume == 0 || streamer.volume == 0 || streamer.mute ||
        sndsys->IsAllMute(streamer.sndgrpid))
    {
        memset(buffer, 0, PCM16_BYTES(samples,streamer.channels));
    }
    else
    {
        if(streamer.volume != VOLUME_DEFAULT || mastervolume != VOLUME_DEFAULT)
        {
            int volfac = streamer.volume * mastervolume;
            int powdef = VOLUME_DEFAULT * VOLUME_DEFAULT;
            SOFTGAIN(buffer, samples, streamer.channels, volfac, powdef);
        }
    }
}

} //namespace

