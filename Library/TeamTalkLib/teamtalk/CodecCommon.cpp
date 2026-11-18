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

#include "CodecCommon.h"
#include "PacketLayout.h"
#include "myace/MyACE.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>

constexpr auto SPEEX_NB_MODE = 0;
constexpr auto SPEEX_WB_MODE = 1;
constexpr auto SPEEX_UWB_MODE = 2;
constexpr auto SPEEX_NB_SAMPLERATE = 8000;
constexpr auto SPEEX_WB_SAMPLERATE = 16000;
constexpr auto SPEEX_UWB_SAMPLERATE = 32000;
constexpr auto SPEEX_NB_FRAMESIZE = 160;
constexpr auto SPEEX_WB_FRAMESIZE = 320;
constexpr auto SPEEX_UWB_FRAMESIZE = 640;
constexpr auto SPEEX_QUALITY_MIN = 0;
constexpr auto SPEEX_QUALITY_MAX = 10;
constexpr auto SPEEX_FRAME_MSEC_MIN = 20;
constexpr auto SPEEX_FRAME_MSEC_MAX = 1000;

constexpr auto OPUS_FRAME_MSEC_MIN = 2;
constexpr auto OPUS_FRAME_MSEC_MAX = 120;
constexpr auto OPUS_SAMPLERATE_MIN = 8000;
constexpr auto OPUS_SAMPLERATE_MAX = 48000;
constexpr auto OPUS_BITRATE_MIN = 6000     /* Remember to updated DLL header file when modifying this */;
constexpr auto OPUS_BITRATE_MAX = 512000   /* Remember to updated DLL header file when modifying this */;

constexpr auto WAV_TXINTERVAL_MSEC_MIN = 2;
constexpr std::array<int, 15> WAV_SAMPLERATES = {8000, 11025, 12000, 16000, 22050,
                                                 24000, 32000, 44100, 48000, 88200,
                                                 96000, 176400, 192000, 352800, 384000};

constexpr auto FLAC_COMPRESSIONLEVEL_MIN = 0;
constexpr auto FLAC_COMPRESSIONLEVEL_MAX = 8;
constexpr auto FLAC_FRAME_OVERHEAD_MAX = 32;

constexpr auto AUDIOPACKET_DURATION_MSEC_MAX = 1000;

namespace teamtalk
{

    bool ValidAudioCodec(const AudioCodec& codec)
    {
        switch(codec.codec)
        {
        case CODEC_NO_CODEC :
            return true;
        case CODEC_SPEEX:
        case CODEC_SPEEX_VBR :
            if(GetAudioCodecSampleRate(codec) == 0 ||
               GetAudioCodecCbMillis(codec) < SPEEX_FRAME_MSEC_MIN ||
               GetAudioCodecCbMillis(codec) > AUDIOPACKET_DURATION_MSEC_MAX ||
               codec.speex.quality < SPEEX_QUALITY_MIN ||
               codec.speex.quality > SPEEX_QUALITY_MAX ||
               GetAudioCodecBitRate(codec) > GetAudioCodecMaxPacketBitrate(codec))
                return false;
            return true;
        case CODEC_OPUS :
            if (GetAudioCodecCbMillis(codec) < OPUS_FRAME_MSEC_MIN ||
                GetAudioCodecCbMillis(codec) > AUDIOPACKET_DURATION_MSEC_MAX ||
                GetAudioCodecSampleRate(codec) < OPUS_SAMPLERATE_MIN ||
                GetAudioCodecSampleRate(codec) > OPUS_SAMPLERATE_MAX ||
                GetAudioCodecFrameSize(codec) > GetAudioCodecSampleRate(codec) * .12 /*OPUS_FRAME_MSEC_MAX*/ ||
                GetAudioCodecBitRate(codec) < OPUS_BITRATE_MIN ||
                GetAudioCodecBitRate(codec) > OPUS_BITRATE_MAX ||
                GetAudioCodecBitRate(codec) > GetAudioCodecMaxPacketBitrate(codec) ||
                GetAudioCodecChannels(codec) < 1 ||
                GetAudioCodecChannels(codec) > 2)
                return false;
            return true;
        case CODEC_WAV :
            if (!ValidWavSampleRate(GetAudioCodecSampleRate(codec)) ||
                GetAudioCodecChannels(codec) < 1 ||
                GetAudioCodecChannels(codec) > 2 ||
                GetWavSampleFormatBytes(codec.wav.sampleformat) == 0 ||
                GetAudioCodecCbMillis(codec) < WAV_TXINTERVAL_MSEC_MIN ||
                GetAudioCodecCbMillis(codec) > AUDIOPACKET_DURATION_MSEC_MAX ||
                GetWavPacketSize(codec) > GetAudioCodecMaxEncFrameSize(codec))
                return false;
            return true;
        case CODEC_FLAC :
            if (!ValidWavSampleRate(GetAudioCodecSampleRate(codec)) ||
                GetAudioCodecChannels(codec) < 1 ||
                GetAudioCodecChannels(codec) > 2 ||
                codec.flac.compressionlevel < FLAC_COMPRESSIONLEVEL_MIN ||
                codec.flac.compressionlevel > FLAC_COMPRESSIONLEVEL_MAX ||
                GetAudioCodecCbMillis(codec) < WAV_TXINTERVAL_MSEC_MIN ||
                GetAudioCodecCbMillis(codec) > AUDIOPACKET_DURATION_MSEC_MAX ||
                GetFlacMaxPacketSize(codec) > GetAudioCodecMaxEncFrameSize(codec))
                return false;
            return true;
        case CODEC_WEBM_VP8 :
            break;
        }
        return false;
    }

    int GetAudioCodecSampleRate(const AudioCodec& codec)
    {
        switch(codec.codec)
        {
        case CODEC_SPEEX :
            return GetSpeexBandModeSampleRate(codec.speex.bandmode);
        case CODEC_SPEEX_VBR :
            return GetSpeexBandModeSampleRate(codec.speex_vbr.bandmode);
        case CODEC_OPUS :
            return codec.opus.samplerate;
        case CODEC_WAV :
            return codec.wav.samplerate;
        case CODEC_FLAC :
            return codec.flac.samplerate;
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
            break;
        }
        return 0;
    }

    int GetAudioCodecCbSamples(const AudioCodec& codec)
    {
        switch(codec.codec)
        {
        case CODEC_SPEEX :
            return GetSpeexSamplesCount(codec.speex.bandmode, 
                codec.speex.frames_per_packet);
        case CODEC_SPEEX_VBR :
            return GetSpeexSamplesCount(codec.speex_vbr.bandmode, 
                codec.speex_vbr.frames_per_packet);
        case CODEC_OPUS :
            return codec.opus.frame_size * codec.opus.frames_per_packet;
        case CODEC_WAV :
            return static_cast<int>(PCM16_DURATION_SAMPLES(codec.wav.txinterval_msec,
                                                           codec.wav.samplerate));
        case CODEC_FLAC :
            return static_cast<int>(PCM16_DURATION_SAMPLES(codec.flac.txinterval_msec,
                                                           codec.flac.samplerate));
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
            break;
        }
        return 0;
    }

    int GetAudioCodecCbBytes(const AudioCodec& codec)
    {
        return GetAudioCodecCbTotalSamples(codec) * sizeof(short);
    }

    int GetAudioCodecCbMillis(const AudioCodec& codec)
    {
        if (codec.codec == CODEC_WAV)
            return codec.wav.txinterval_msec;
        if (codec.codec == CODEC_FLAC)
            return codec.flac.txinterval_msec;

        int const samplerate = GetAudioCodecSampleRate(codec);
        if(samplerate == 0)
            return 0;

        int const cb_samples = GetAudioCodecCbSamples(codec);
        return cb_samples == 0? 0 : PCM16_SAMPLES_DURATION(cb_samples, samplerate);
    }

    int GetAudioCodecCbTotalSamples(const AudioCodec& codec)
    {
        int channels = GetAudioCodecChannels(codec);
        if(GetAudioCodecSimulateStereo(codec))
            channels = 2;
        return GetAudioCodecCbSamples(codec) * channels;
    }

    int GetAudioCodecChannels(const AudioCodec& codec)
    {
        switch(codec.codec)
        {
        case CODEC_SPEEX :
        case CODEC_SPEEX_VBR :
            return 1; //speex doesn't allow stereo
        case CODEC_OPUS :
            return codec.opus.channels;
        case CODEC_WAV :
            return codec.wav.channels;
        case CODEC_FLAC :
            return codec.flac.channels;
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
            break;
        }
        return 0;
    }

    int GetAudioCodecFrameSize(const AudioCodec& codec)
    {
        switch(codec.codec)
        {
        case CODEC_SPEEX :
            return GetSpeexBandModeFrameSize(codec.speex.bandmode);
        case CODEC_SPEEX_VBR :
            return GetSpeexBandModeFrameSize(codec.speex_vbr.bandmode);
        case CODEC_OPUS :
            return codec.opus.frame_size;
        case CODEC_WAV :
        case CODEC_FLAC :
            return GetAudioCodecCbSamples(codec);
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
            break;
        }
        return 0;
    }

    int GetAudioCodecFramesPerPacket(const AudioCodec& codec)
    {
        switch(codec.codec)
        {
        case CODEC_SPEEX :
            return codec.speex.frames_per_packet;
        case CODEC_SPEEX_VBR :
            return codec.speex_vbr.frames_per_packet;
        case CODEC_OPUS :
            return codec.opus.frames_per_packet;
        case CODEC_WAV :
        case CODEC_FLAC :
            return 1;
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
            break;
        }
        return 0;
    }

    bool GetAudioCodecVariableFrameSizes(const AudioCodec& codec)
    {
        switch(codec.codec)
        {
        case CODEC_OPUS :
            return GetAudioCodecVBRMode(codec) || codec.opus.dtx;
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
        case CODEC_SPEEX_VBR :
        case CODEC_SPEEX :
        case CODEC_WAV :
        case CODEC_FLAC :
            break;
        }
        return GetAudioCodecVBRMode(codec);
    }

    bool GetAudioCodecVBRMode(const AudioCodec& codec)
    {
        switch(codec.codec)
        {
        case CODEC_SPEEX_VBR :
        case CODEC_FLAC :
            return true;
        case CODEC_OPUS :
            return codec.opus.vbr;
        case CODEC_SPEEX :
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
        case CODEC_WAV :
            break;
        }
        return false;
    }

    bool GetAudioCodecSimulateStereo(const AudioCodec& codec)
    {
        switch(codec.codec)
        {
        case CODEC_SPEEX :
            return codec.speex.sim_stereo;
        case CODEC_SPEEX_VBR :
            return codec.speex_vbr.sim_stereo;
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
        case CODEC_OPUS :
        case CODEC_WAV :
        case CODEC_FLAC :
            break;
        }
        return false;
    }

    int GetAudioCodecBitRate(const AudioCodec& codec)
    {
        switch(codec.codec)
        {
        case CODEC_SPEEX :
            switch(codec.speex.bandmode)
            {
            case SPEEX_NB_MODE :
                switch(codec.speex.quality)
                {
                case 0 :
                    return 2150;
                case 1 :
                    return 3950;
                case 2 :
                    return 5950;
                case 3 :
                case 4 :
                    return 8000;
                case 5 :
                case 6 :
                    return 11000;
                case 7 :
                case 8 :
                    return 15000;
                case 9 :
                    return 18200;
                case 10 :
                    return 24600;
                default :
                    return 0;
                }
            case SPEEX_WB_MODE :
                switch(codec.speex.quality)
                {
                case 0 :
                    return 3950;
                case 1 :
                    return 5750;
                case 2 :
                    return 7750;
                case 3 :
                    return 9800;
                case 4 :
                    return 12800;
                case 5 :
                    return 16800;
                case 6 :
                    return 20600;
                case 7 :
                    return 23800;
                case 8 :
                    return 27800;
                case 9 :
                    return 34200;
                case 10 :
                    return 42200;
                default :
                    return 0;
                }
            case SPEEX_UWB_MODE :
                switch(codec.speex.quality)
                {
                case 0 :
                    return 4150;
                case 1 :
                    return 7550;
                case 2 :
                    return 9550;
                case 3 :
                    return 11600;
                case 4 :
                    return 14600;
                case 5 :
                    return 18600;
                case 6 :
                    return 22400;
                case 7 :
                    return 25600;
                case 8 :
                    return 29600;
                case 9 :
                    return 36000;
                case 10 :
                    return 44000;
                default :
                    return 0;
                }
            }
        case CODEC_SPEEX_VBR :
            return (codec.speex_vbr.bitrate>codec.speex_vbr.max_bitrate)?
                codec.speex_vbr.bitrate : codec.speex_vbr.max_bitrate;
        case CODEC_OPUS :
            return codec.opus.bitrate;
        case CODEC_WAV :
            return GetAudioCodecSampleRate(codec) * GetAudioCodecChannels(codec) *
                GetWavSampleFormatBytes(codec.wav.sampleformat) * 8;
        case CODEC_FLAC :
            return GetAudioCodecSampleRate(codec) * GetAudioCodecChannels(codec) * 16;
        case CODEC_WEBM_VP8 :
        case CODEC_NO_CODEC :
            break;
        } /* codec switch */
        return 0;
    }

    // AudioPacket can contain a maximum of 0xfff bytes
    int GetAudioCodecMaxPacketBitrate(const AudioCodec& codec)
    {
        int const txinterval_msec = GetAudioCodecCbMillis(codec);
        if (txinterval_msec == 0)
            return 0;

        int const maxbitrate = 8 * ((GetAudioCodecMaxEncFrameSize(codec) * 1000) / txinterval_msec);
        MYTRACE_COND(GetAudioCodecBitRate(codec) > maxbitrate,
                     ACE_TEXT("Max packet bitrate exceeded: %d > %d\n"),
                     GetAudioCodecBitRate(codec), maxbitrate);
        return maxbitrate;
    }

    int GetAudioCodecMaxEncFrameSize(const AudioCodec& codec)
    {
        switch(codec.codec)
        {
        case CODEC_WAV :
        case CODEC_FLAC :
            return MAX_ENC_FRAGMENTED_FRAMESIZE;
        case CODEC_SPEEX :
        case CODEC_SPEEX_VBR :
        case CODEC_OPUS :
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
            break;
        }
        return MAX_ENC_FRAMESIZE;
    }

    media::AudioFormat GetAudioCodecAudioFormat(const AudioCodec& codec)
    {
        int channels = GetAudioCodecChannels(codec);
        if (GetAudioCodecSimulateStereo(codec))
            channels = 2;
        return {GetAudioCodecSampleRate(codec), channels};
    }

    media::AudioInputFormat GetAudioCodecAudioInputFormat(const AudioCodec& codec)
    {
        return media::AudioInputFormat(GetAudioCodecAudioFormat(codec),
                                       GetAudioCodecCbSamples(codec));
    }

    int GetSpeexBandMode(const AudioCodec& codec)
    {
        switch(codec.codec)
        {
        case CODEC_SPEEX :
            return codec.speex.bandmode;
        case CODEC_SPEEX_VBR :
            return codec.speex_vbr.bandmode;
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
        case CODEC_OPUS :
        case CODEC_WAV :
        case CODEC_FLAC :
            break;
        }
        return -1;
    }

    int GetSpeexQuality(const AudioCodec& codec)
    {
        switch(codec.codec)
        {
        case CODEC_SPEEX :
            return codec.speex.quality;
        case CODEC_SPEEX_VBR :
            return codec.speex_vbr.vbr_quality;
        case CODEC_NO_CODEC :
        case CODEC_WEBM_VP8 :
        case CODEC_OPUS :
        case CODEC_WAV :
        case CODEC_FLAC :
            break;
        }
        return -1;
    }


    int GetSpeexBandModeSampleRate(int bandmode)
    {
        switch(bandmode)
        {
        case SPEEX_NB_MODE :
            return SPEEX_NB_SAMPLERATE;
        case SPEEX_WB_MODE :
            return SPEEX_WB_SAMPLERATE;  
        case SPEEX_UWB_MODE :
            return SPEEX_UWB_SAMPLERATE; 
        }
        return 0;
    }

    int GetSpeexBandModeFrameSize(int bandmode)
    {
        switch(bandmode)
        {
        case SPEEX_NB_MODE :
            return SPEEX_NB_FRAMESIZE;
        case SPEEX_WB_MODE :
            return SPEEX_WB_FRAMESIZE;
        case SPEEX_UWB_MODE :
            return SPEEX_UWB_FRAMESIZE;
        default :
            return 0;
        }
    }

    int GetSpeexFramesDuration(int bandmode, int framecount)
    {
        int const nSamples = GetSpeexBandModeFrameSize(bandmode) * framecount;
        int const nSampleRate = GetSpeexBandModeSampleRate(bandmode);
        if(nSampleRate>0)
            return (nSamples * 1000) / nSampleRate;
        return 0;
    }

    int GetSpeexSamplesCount(int bandmode, int framecount)
    {
        switch(bandmode)
        {
        case SPEEX_NB_MODE :
            return SPEEX_NB_FRAMESIZE * framecount;
        case SPEEX_WB_MODE :
            return SPEEX_WB_FRAMESIZE * framecount;
        case SPEEX_UWB_MODE :
            return  SPEEX_UWB_FRAMESIZE * framecount;
        default :
            assert(0);
            return 0;
        }
    }

    bool ValidWavSampleRate(int samplerate)
    {
        return std::ranges::find(WAV_SAMPLERATES, samplerate) != WAV_SAMPLERATES.end();
    }

    int GetWavSampleFormatBytes(WavSampleFormat sampleformat)
    {
        switch(sampleformat)
        {
        case WAV_SAMPLEFORMAT_UINT8 :
            return 1;
        case WAV_SAMPLEFORMAT_INT16 :
            return 2;
        case WAV_SAMPLEFORMAT_NONE :
            break;
        }
        return 0;
    }

    int GetWavPacketSize(const AudioCodec& codec)
    {
        if (codec.codec != CODEC_WAV)
            return 0;
        return GetAudioCodecCbTotalSamples(codec) * GetWavSampleFormatBytes(codec.wav.sampleformat);
    }

    int GetFlacMaxPacketSize(const AudioCodec& codec)
    {
        if (codec.codec != CODEC_FLAC)
            return 0;
        return (GetAudioCodecCbTotalSamples(codec) * static_cast<int>(sizeof(short))) + FLAC_FRAME_OVERHEAD_MAX;
    }

    int EncodeWavSamples(WavSampleFormat sampleformat, const short* input_buffer,
                         int input_samples_total, char* output_buffer)
    {
        switch(sampleformat)
        {
        case WAV_SAMPLEFORMAT_UINT8 :
            for (int i=0;i<input_samples_total;i++)
                output_buffer[i] = static_cast<char>((input_buffer[i] >> 8) + 128);
            return input_samples_total;
        case WAV_SAMPLEFORMAT_INT16 :
            for (int i=0;i<input_samples_total;i++)
            {
                auto const sample = static_cast<uint16_t>(input_buffer[i]);
                output_buffer[i * 2] = static_cast<char>(sample & 0xFF);
                output_buffer[(i * 2) + 1] = static_cast<char>(sample >> 8);
            }
            return input_samples_total * 2;
        case WAV_SAMPLEFORMAT_NONE :
            break;
        }
        return 0;
    }

    int DecodeWavSamples(WavSampleFormat sampleformat, const char* enc_data,
                         int enc_length, short* output_buffer)
    {
        switch(sampleformat)
        {
        case WAV_SAMPLEFORMAT_UINT8 :
            for (int i=0;i<enc_length;i++)
                output_buffer[i] = static_cast<short>((static_cast<uint8_t>(enc_data[i]) - 128) * 256);
            return enc_length;
        case WAV_SAMPLEFORMAT_INT16 :
            for (int i=0;i<enc_length / 2;i++)
            {
                auto const lsb = static_cast<uint16_t>(static_cast<uint8_t>(enc_data[i * 2]));
                auto const msb = static_cast<uint16_t>(static_cast<uint8_t>(enc_data[(i * 2) + 1]));
                output_buffer[i] = static_cast<short>(lsb | (msb << 8));
            }
            return enc_length / 2;
        case WAV_SAMPLEFORMAT_NONE :
            break;
        }
        return 0;
    }

    bool AudioCodecConvertBug(const ACE_TString& streamprotocol, const AudioCodec& codec)
    {
        // Deprecated: Ignore in TeamTalk 6.
        // Handle crash issue in pre-protocol v5.7 (TeamTalk v5.4).
        // See git hash 02045445862fc62dec34701888eafebf0b5b5418.
        if (!VersionSameOrLater(streamprotocol, ACE_TEXT("5.7")))
        {
            switch(codec.codec)
            {
            case CODEC_SPEEX:
            case CODEC_SPEEX_VBR:
                if (GetAudioCodecCbMillis(codec) > 100)
                    return true;
                break;
            case CODEC_OPUS:
                if (GetAudioCodecCbMillis(codec) > 60)
                    return true;
                break;
            case CODEC_NO_CODEC :
            case CODEC_WEBM_VP8 :
            case CODEC_WAV :
            case CODEC_FLAC :
                break;
            }
        }
        return false;
    }

} // namespace teamtalk

