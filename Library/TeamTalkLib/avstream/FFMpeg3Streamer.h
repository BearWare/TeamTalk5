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

#ifndef AVCONVWRAPPER_H
#define AVCONVWRAPPER_H

#include <ace/Task.h>
#include <ace/Future.h>
#include <ace/Semaphore.h>

#include "MediaStreamer.h"

#include <stdint.h>

// compatible with ffmpeg tag n1.1.13

void InitAVConv();

bool GetAVMediaFileProp(const ACE_TString& filename, MediaFileProp& out_prop);

class FFMpegStreamer : public MediaStreamer
{
public:
    FFMpegStreamer();
    virtual ~FFMpegStreamer();
    
    virtual bool AddStartTime() const { return true; }

protected:
    virtual bool SetupInput(struct AVInputFormat *iformat,
                            struct AVDictionary *options,
                            struct AVFormatContext*& fmt_ctx,
                            struct AVCodecContext*& aud_dec_ctx,
                            struct AVCodecContext*& vid_dec_ctx,
                            int& audio_stream_index,
                            int& video_stream_index);

private:
    void Run();

    int64_t ProcessAudioBuffer(struct AVFilterContext* aud_buffersink_ctx,
                               struct AVFrame* filt_frame,
                               struct AVStream* aud_stream,
                               ACE_UINT32 start_time,
                               ACE_UINT32& start_offset);
    int64_t ProcessVideoBuffer(struct AVFilterContext* vid_buffersink_ctx,
                               struct AVFrame* filt_frame,
                               struct AVStream* vid_stream,
                               ACE_UINT32 start_time,
                               ACE_UINT32& start_offset);
};

bool OpenInput(const ACE_TString& filename,
               AVInputFormat *iformat,
               AVDictionary *options,
               AVFormatContext*& fmt_ctx,
               AVCodecContext*& aud_dec_ctx,
               AVCodecContext*& vid_dec_ctx,
               int& audio_stream_index,
               int& video_stream_index);

struct AVFilterGraph* createAudioFilterGraph(AVFormatContext *fmt_ctx,
                                             AVCodecContext* aud_dec_ctx,
                                             AVFilterContext*& aud_buffersink_ctx,
                                             AVFilterContext*& aud_buffersrc_ctx,
                                             int audio_stream_index,
                                             int out_channels,
                                             int out_samplerate);

#endif
