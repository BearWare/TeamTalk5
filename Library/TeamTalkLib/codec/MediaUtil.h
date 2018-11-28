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

#include <myace/MyACE.h>

namespace media
{
/* Remember to updated DLL header file when modifying this */
    enum FourCC
    {
        FOURCC_NONE   = 0,
        FOURCC_I420   = 100,
        FOURCC_YUY2   = 101,
        FOURCC_RGB32  = 102,
        FOURCC_RGB24  = 103,
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

        VideoFormat()
        {
            width = height = fps_numerator = fps_denominator = 0;
            fourcc = FOURCC_NONE;
        }
        bool operator==(const VideoFormat& fmt)
            {
                return memcmp(&fmt, this, sizeof(*this)) == 0;
            }
    };

    struct AudioFrame
    {
        short* input_buffer; //for recorded frame
        const short* output_buffer; //for echo cancel frame
        int input_samples;
        int output_samples;
        int input_channels;
        int output_channels;
        int input_samplerate;
        int output_samplerate;
        int soundgrpid;
        ACE_UINT32 userdata;
        bool force_enc; //force encoding of frame
        bool voiceact_enc; //encode if voice active
        ACE_UINT32 timestamp;
        AudioFrame()
            : input_buffer(NULL)
            , output_buffer(NULL)
            {
                input_samples = output_samples = 0;
                input_channels = output_channels = 0;
                input_samplerate = output_samplerate = 0;
                soundgrpid = 0;
                userdata = 0;
                force_enc = voiceact_enc = false;
                timestamp = GETTIMESTAMP();
            }
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
        ACE_UINT32 timestamp;
        VideoFrame(char* frm_data, int frm_len,
                   int w, int h, FourCC pic_type, bool top_down_bmp)
        : frame(frm_data), frame_length(frm_len)
        , width(w), height(h)
        , fourcc(pic_type)
        , top_down(top_down_bmp)
        , key_frame(true), stream_id(0)
        {
            timestamp = GETTIMESTAMP();
        }
        VideoFrame(const VideoFormat& fmt, char* buf, int len)
        : VideoFrame(buf, len, fmt.width, fmt.height, fmt.fourcc, false) {}
    };

}

ACE_Message_Block* VideoFrameInMsgBlock(media::VideoFrame& frm,
                                        ACE_Message_Block::ACE_Message_Type mb_type = ACE_Message_Block::MB_DATA);
ACE_Message_Block* VideoFrameToMsgBlock(const media::VideoFrame& frm,
                                        ACE_Message_Block::ACE_Message_Type mb_type = ACE_Message_Block::MB_DATA);

void SplitStereo(const short* input_buffer, int input_samples,
                 std::vector<short>& left_chan, std::vector<short>& right_chan);

void MergeStereo(const std::vector<short>& left_chan, 
                 const std::vector<short>& right_chan,
                 short* output_buffer, int output_samples);

#define PCM16_BYTES(samples, channels) (samples * channels * sizeof(short))

#define RGB32_BYTES(w, h) (h * w * 4)

#define SOFTGAIN(samples, n_samples, channels, factor) do {     \
    int samples_total = channels*n_samples;                     \
    if(samples_total % 4 == 0)                                  \
    {                                                           \
        int v[4];                                               \
        for(int i=0;i<samples_total;i+=4)                       \
        {                                                       \
            v[0] = (int)(samples[i] * factor);                  \
            v[1] = (int)(samples[i+1] * factor);                \
            v[2] = (int)(samples[i+2] * factor);                \
            v[3] = (int)(samples[i+3] * factor);                \
            if(v[0] > 32767) v[0] = 32767;                      \
            else if (v[0] < -32768) v[0] = -32768;              \
            if(v[1] > 32767) v[1] = 32767;                      \
            else if (v[1] < -32768) v[1] = -32768;              \
            if(v[2] > 32767) v[2] = 32767;                      \
            else if (v[2] < -32768) v[2] = -32768;              \
            if(v[3] > 32767) v[3] = 32767;                      \
            else if (v[3] < -32768) v[3] = -32768;              \
            samples[i] = (short)v[0];                           \
            samples[i+1] = (short)v[1];                         \
            samples[i+2] = (short)v[2];                         \
            samples[i+3] = (short)v[3];                         \
        }                                                       \
    }                                                           \
    else                                                        \
    {                                                           \
        int v;                                                  \
        for(int i=0;i<samples_total;i++)                        \
        {                                                       \
            v = (int)(samples[i] * factor);                     \
            if(v > 32767) v = 32767;                            \
            else if (v < -32768) v = -32768;                    \
            samples[i] = (short)v;                              \
        }                                                       \
    }                                                           \
} while(0)


#endif
