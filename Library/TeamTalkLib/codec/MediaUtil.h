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

#ifndef MEDIAUTIL_H
#define MEDIAUTIL_H

#include "mystd/MyStd.h"

#include <ace/Message_Block.h>

#include <cstdint>
#include <cstring>
#include <vector>

// Returns number of bytes from number of 'samples' with 'channels'
constexpr auto PCM16_BYTES(int samples, int channels)
{
    return samples * channels * sizeof(short);
}
// Returns number of msec from number of 'bytes' with 'channels' at given 'samplerate'
constexpr auto PCM16_BYTES_DURATION(int bytes, int channels, int samplerate)
{
    return ((bytes / channels / sizeof(short)) * 1000) / samplerate;
}
// Returns number of msec from number of 'samples' at given 'samplerate'
constexpr auto PCM16_SAMPLES_DURATION(int samples, int samplerate)
{
    return ((samples / samplerate) * 1000) + (((samples % samplerate) * 1000) / samplerate);
}
// Returns number of samples from 'duration' msec at given 'samplerate'
constexpr auto PCM16_DURATION_SAMPLES(int duration, int samplerate)
{
    return (duration / 1000) * samplerate + ((duration % 1000) ? ((samplerate * (duration % 1000)) / 1000) : 0);
}

constexpr auto RGB32_BYTES(int w, int h)
{
    return h * w * 4;
}

namespace media
{
/* Remember to updated DLL header file when modifying this */
    enum FourCC
    {
        /* input/output formats */
        FOURCC_NONE   = 0,
        FOURCC_I420   = 100,
        FOURCC_YUY2   = 101,
        FOURCC_RGB32  = 102,
        FOURCC_RGB24  = 103,
        FOURCC_NV12   = 104,

        // input only formats
        FOURCC_MJPEG = 200,
        FOURCC_H264  = 201,
    };

/* Remember to updated DLL header file when modifying this */
    struct VideoFormat
    {
        int width;
        int height;
        int fps_numerator;
        int fps_denominator;
        FourCC fourcc;

        VideoFormat(int w, int h, int fps_num, int fps_denom, FourCC cc)
        : width(w)
        , height(h)
        , fps_numerator(fps_num)
        , fps_denominator(fps_denom)
        , fourcc(cc) {}

        VideoFormat(int w, int h, FourCC cc)
            : VideoFormat(w, h, 0, 0, cc) {}

        VideoFormat() : fourcc(FOURCC_NONE)
        {
            width = height = fps_numerator = fps_denominator = 0;
            
        }
        bool operator==(const VideoFormat& fmt) const
        {
            return memcmp(&fmt, this, sizeof(*this)) == 0;
        }
        bool operator!=(const VideoFormat& fmt) const
        {
            return memcmp(&fmt, this, sizeof(*this)) != 0;
        }

        bool IsValid() const { return width > 0 && height > 0; }
    };

    struct AudioFormat
    {
        int samplerate = 0;
        int channels = 0;

        bool IsValid() const { return samplerate > 0 && channels > 0; }

        AudioFormat(int sr, int chans) : samplerate(sr), channels(chans) {}
        AudioFormat() = default;

        bool operator==(const AudioFormat& fmt) const
        {
            return memcmp(&fmt, this, sizeof(*this)) == 0;
        }
        bool operator!=(const AudioFormat& fmt) const
        {
            return memcmp(&fmt, this, sizeof(*this)) != 0;
        }
    };

    struct AudioInputFormat
    {
        AudioFormat fmt;
        int samples = 0;
        AudioInputFormat(const AudioFormat& f, int s) : fmt(f), samples(s) {}
        AudioInputFormat() = default;
        bool IsValid() const { return fmt.IsValid() && samples > 0; }
        int GetDurationMSec() const;
        int GetTotalSamples() const;
        int GetBytes() const;
        bool operator==(const AudioInputFormat& f) const
        {
            return fmt == f.fmt && samples == f.samples;
        }
        bool operator!=(const AudioInputFormat& f) const
        {
            return fmt != f.fmt || samples != f.samples;
        }
    };

    struct AudioFrame
    {
        short* input_buffer = nullptr; //for recorded frame
        const short* output_buffer = nullptr; //for echo cancel frame
        int input_samples;
        int output_samples;
        AudioFormat inputfmt;
        AudioFormat outputfmt;
        int streamid = 0;
        int soundgrpid;
        uint32_t userdata;
        bool force_enc; //force encoding of frame
        bool voiceact_enc; //encode if voice active
        uint32_t timestamp;
        uint32_t sample_no = 0;
        int duplex_callback_delay = 0; // The time between capture and expected rendering of the samples, taken from the PortAudio duplex callback timing
        Rational gain; // if != 1 then apply gain

        AudioFrame()
        : soundgrpid(0), userdata(0), timestamp(GETTIMESTAMP()), gain(1, 1)
        {
            input_samples = output_samples = 0;
            
            
            force_enc = voiceact_enc = false;
            
        }

        AudioFrame(const AudioFormat& infmt, short* input_buf, int insamples, ACE_UINT32 sampleindex = 0)
        : AudioFrame()
        {
            inputfmt = infmt;
            input_buffer = input_buf;
            input_samples = insamples;
            sample_no = sampleindex;
        }

        AudioFrame(ACE_Message_Block* mb)
        : gain(1, 1)
        {
            auto* frm = reinterpret_cast<AudioFrame*>(mb->base());
            *this = *frm;
        }

        uint32_t InputDurationMSec() const
        {
            if (!inputfmt.IsValid())
                return 0;
            return PCM16_SAMPLES_DURATION(input_samples, inputfmt.samplerate);
        }

        void ApplyGain();
    };

    struct VideoFrame
    {
        char* frame;
        int frame_length;
        int width;
        int height;
        FourCC fourcc;
        bool top_down;
        bool key_frame;
        int stream_id;
        uint32_t timestamp;

        VideoFrame(char* frm_data, int frm_len,
                   int w, int h, FourCC pic_type, bool top_down_bmp)
        : frame(frm_data), frame_length(frm_len)
        , width(w), height(h)
        , fourcc(pic_type)
        , top_down(top_down_bmp)
        , key_frame(true), stream_id(0), timestamp(GETTIMESTAMP())
        {
            
        }

        VideoFrame(const VideoFormat& fmt, char* buf, int len)
        : VideoFrame(buf, len, fmt.width, fmt.height, fmt.fourcc, false) {}

        VideoFrame() : VideoFrame(nullptr, 0, 0, 0, FOURCC_NONE, false) {}

        VideoFrame(ACE_Message_Block* mb)
        {
            auto* frm = reinterpret_cast<media::VideoFrame*>(mb->rd_ptr());
            frame = frm->frame;
            frame_length = frm->frame_length;
            width = frm->width;
            height = frm->height;
            fourcc = frm->fourcc;
            top_down = frm->top_down;
            key_frame = frm->key_frame;
            stream_id = frm->stream_id;
            timestamp = frm->timestamp;
        }

        bool IsValid() const { return (frame != nullptr) && (frame_length != 0) && GetVideoFormat().IsValid(); }

        VideoFormat GetVideoFormat() const 
        {
            return {width, height, fourcc};
        }
    };

} // namespace media

ACE_Message_Block* VideoFrameInMsgBlock(media::VideoFrame& frm,
                                        ACE_Message_Block::ACE_Message_Type mb_type = ACE_Message_Block::MB_DATA);
ACE_Message_Block* VideoFrameToMsgBlock(const media::VideoFrame& frm,
                                        ACE_Message_Block::ACE_Message_Type mb_type = ACE_Message_Block::MB_DATA);
media::VideoFrame* VideoFrameFromMsgBlock(ACE_Message_Block* mb);

ACE_Message_Block* AudioFrameToMsgBlock(const media::AudioFrame& frame, bool skip_copy = false);
media::AudioFrame* AudioFrameFromMsgBlock(ACE_Message_Block* mb);
ACE_Message_Block* AudioFramesMerge(const std::vector<ACE_Message_Block*>& mbs);
ACE_Message_Block* AudioFrameFromList(int samples_out, std::vector<ACE_Message_Block*>& mbs);

void SplitStereo(const short* input_buffer, int input_samples,
                 std::vector<short>& left_chan, std::vector<short>& right_chan);

void MergeStereo(const std::vector<short>& left_chan, 
                 const std::vector<short>& right_chan,
                 short* output_buffer, int output_samples);

enum Stereo
{
    STEREO_NONE     = 0x0,
    STEREO_LEFT     = 0x1,
    STEREO_RIGHT    = 0x2,
    STEREO_BOTH     = STEREO_LEFT | STEREO_RIGHT
};

using StereoMask = unsigned char;

StereoMask ToStereoMask(bool muteleft, bool muteright);

void SelectStereo(StereoMask stereo, short* buffer, int samples);

// returns new sample_index
int GenerateTone(media::AudioFrame& audblock, int sample_index, int tone_freq,
                 double volume = 8000, bool mute_left = false, bool mute_right = false);

constexpr int GAIN_MAX = 32000;
constexpr int GAIN_NORMAL = 1000;
constexpr int GAIN_MIN = 0;

#define SOFTGAIN(inputsamples, n_samples, channels, gain_numerator, gain_denominator) do { \
    if ((gain_numerator) == (gain_denominator))                     \
        break;                                                  \
    float factor = float(gain_numerator) / float(gain_denominator);   \
    int samples_total = (channels) * (n_samples);               \
    short* samplesbuffer = inputsamples;                        \
    if(samples_total % 4 == 0)                                  \
    {                                                           \
        int v[4];                                               \
        for(int i=0;i<samples_total;i+=4)                       \
        {                                                       \
            v[0] = (int)(samplesbuffer[i] * factor);            \
            v[1] = (int)(samplesbuffer[i+1] * factor);          \
            v[2] = (int)(samplesbuffer[i+2] * factor);          \
            v[3] = (int)(samplesbuffer[i+3] * factor);          \
            if(v[0] > 32767) v[0] = 32767;                      \
            else if (v[0] < -32768) v[0] = -32768;              \
            if(v[1] > 32767) v[1] = 32767;                      \
            else if (v[1] < -32768) v[1] = -32768;              \
            if(v[2] > 32767) v[2] = 32767;                      \
            else if (v[2] < -32768) v[2] = -32768;              \
            if(v[3] > 32767) v[3] = 32767;                      \
            else if (v[3] < -32768) v[3] = -32768;              \
            samplesbuffer[i] = (short)v[0];                     \
            samplesbuffer[i+1] = (short)v[1];                   \
            samplesbuffer[i+2] = (short)v[2];                   \
            samplesbuffer[i+3] = (short)v[3];                   \
        }                                                       \
    }                                                           \
    else                                                        \
    {                                                           \
        int v;                                                  \
        for(int i=0;i<samples_total;i++)                        \
        {                                                       \
            v = (int)(samplesbuffer[i] * factor);               \
            if(v > 32767) v = 32767;                            \
            else if (v < -32768) v = -32768;                    \
            samplesbuffer[i] = (short)v;                        \
        }                                                       \
    }                                                           \
} while(0)


#endif
