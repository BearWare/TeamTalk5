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

#include "FFMpeg1Streamer.h"

// compatible with ffmpeg tag n1.1.13

extern "C" {
#include <libavutil/rational.h>
#include <libavutil/log.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

#include <myace/MyACE.h>
#include "MediaUtil.h"

using namespace media;

void InitAVConv()
{
    static bool ready = false;
    if(!ready)
    {
        static ACE_Recursive_Thread_Mutex mtx;

        wguard_t g(mtx);
        
        av_log_set_level(AV_LOG_QUIET);
        avcodec_register_all();
        av_register_all();
        avfilter_register_all();
        ready = true;
    }
}

bool openMediaFile(const ACE_TString& filename,
                   AVFormatContext*& fmt_ctx,
                   AVCodecContext*& aud_dec_ctx,
                   AVCodecContext*& vid_dec_ctx,
                   int& audio_stream_index,
                   int& video_stream_index)
{
    AVFormatContext* _fmt_ctx = NULL;
    AVCodecContext* _aud_dec_ctx = NULL;
    AVCodecContext* _vid_dec_ctx = NULL;
    int _audio_stream_index = -1, _video_stream_index = -1;
    AVCodec *aud_dec, *vid_dec;

    if (avformat_open_input(&_fmt_ctx, filename.c_str(), NULL, NULL) < 0 ||
        avformat_find_stream_info(_fmt_ctx, NULL) < 0)
        goto cleanup;

    /* select the audio stream */
    _audio_stream_index = av_find_best_stream(_fmt_ctx, AVMEDIA_TYPE_AUDIO, 
                                              -1, -1, &aud_dec, 0);
    if (_audio_stream_index >= 0) {
        _aud_dec_ctx = _fmt_ctx->streams[_audio_stream_index]->codec;

        /* init the audio decoder */
        if (avcodec_open2(_aud_dec_ctx, aud_dec, NULL) < 0) {
            _audio_stream_index = -1;
        }
    }

    /* select the video stream */
    _video_stream_index = av_find_best_stream(_fmt_ctx, AVMEDIA_TYPE_VIDEO, 
                                              -1, -1, &vid_dec, 0);
    if (_video_stream_index >= 0) {
        _vid_dec_ctx = _fmt_ctx->streams[_video_stream_index]->codec;

        /* init the video decoder */
        if (avcodec_open2(_vid_dec_ctx, vid_dec, NULL) < 0) {
            _video_stream_index = -1;
        }
    }


cleanup:

    if(_audio_stream_index >= 0)
    {
        audio_stream_index = _audio_stream_index;
        aud_dec_ctx = _aud_dec_ctx;
    }

    if(_video_stream_index >= 0)
    {
        video_stream_index = _video_stream_index;
        vid_dec_ctx = _vid_dec_ctx;
    }

    if(_audio_stream_index < 0 && _video_stream_index < 0)
    {
        if(_fmt_ctx)
            avformat_close_input(&_fmt_ctx);
        return false;
    }
    else
    {
        fmt_ctx = _fmt_ctx;
        return true;
    }
}

AVFilterGraph* createAudioFilterGraph(AVFormatContext *fmt_ctx,
                                      AVCodecContext* aud_dec_ctx,
                                      AVFilterContext*& aud_buffersink_ctx,
                                      AVFilterContext*& aud_buffersrc_ctx,
                                      int audio_stream_index,
                                      int out_channels,
                                      int out_samplerate);

AVFilterGraph* createVideoFilterGraph(AVFormatContext *fmt_ctx,
                                      AVCodecContext* vid_dec_ctx,
                                      AVFilterContext*& vid_buffersink_ctx,
                                      AVFilterContext*& vid_buffersrc_ctx,
                                      int video_stream_index);

void FillMediaFileProp(AVFormatContext *fmt_ctx,
                       AVCodecContext *aud_dec_ctx, 
                       AVCodecContext *vid_dec_ctx,
                       MediaFileProp& out_prop)
{
    if (aud_dec_ctx)
    {
        out_prop.audio_channels = aud_dec_ctx->channels;
        out_prop.audio_samplerate = aud_dec_ctx->sample_rate;
    }

    if(vid_dec_ctx)
    {
        out_prop.video_width = vid_dec_ctx->width;
        out_prop.video_height = vid_dec_ctx->height;
 
        //frame rate
        double fps = 1.0 / av_q2d(vid_dec_ctx->time_base) / std::max(vid_dec_ctx->ticks_per_frame, 1);
        AVRational r_fps = av_d2q(fps, 1000);
        out_prop.video_fps_numerator = r_fps.num;
        out_prop.video_fps_denominator = r_fps.den;
    }

    out_prop.duration_ms = (fmt_ctx->duration * av_q2d(AV_TIME_BASE_Q)) * 1000;
}

bool GetAVMediaFileProp(const ACE_TString& filename, MediaFileProp& out_prop)
{
    InitAVConv();

    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *aud_dec_ctx = NULL, *vid_dec_ctx = NULL;
    int audio_stream_index = -1, video_stream_index = -1;

    if(!openMediaFile(filename, fmt_ctx, aud_dec_ctx, vid_dec_ctx, 
                      audio_stream_index, video_stream_index))
        return false;

    FillMediaFileProp(fmt_ctx, aud_dec_ctx, vid_dec_ctx, out_prop);
    out_prop.filename = filename;

    if (aud_dec_ctx)
        avcodec_close(aud_dec_ctx);

    if(vid_dec_ctx)
        avcodec_close(vid_dec_ctx);

    avformat_close_input(&fmt_ctx);

    return out_prop.video_width || out_prop.audio_channels;
}


FFMpegStreamer::FFMpegStreamer(MediaStreamListener* listener)
: MediaStreamer(listener)
{
}

FFMpegStreamer::~FFMpegStreamer()
{
    assert(thr_count() == 0);
    MYTRACE(ACE_TEXT("~FFMpegStreamer()\n"));
}

bool FFMpegStreamer::OpenFile(const MediaFileProp& in_prop,
                              const MediaStreamOutput& out_prop)
{
    if(this->thr_count())
        return false;

    if(!out_prop.audio && !out_prop.video)
        return false;
    
    m_media_in = in_prop;
    m_media_out = out_prop;

    bool success = false;
    int ret = activate();
    if(ret<0)
        goto fail;

    ret = m_open.get(success);

    if(success)
        return true;
fail:
    Reset();
    return false;
}

void FFMpegStreamer::Close()
{
    m_stop = true;

    m_start.set(true);

    this->wait();
    
    m_open.cancel();
    m_start.cancel();

    Reset();

    m_stop = false;
}

bool FFMpegStreamer::StartStream()
{
    if(m_media_in.filename.length())
    {
        m_start.set(true);
        return true;
    }
    return false;
}

int FFMpegStreamer::svc()
{
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *aud_dec_ctx= NULL, *vid_dec_ctx = NULL;
    AVFilterContext *aud_buffersink_ctx = NULL, *aud_buffersrc_ctx = NULL;
    AVFilterContext *vid_buffersink_ctx = NULL, *vid_buffersrc_ctx = NULL;
    AVFilterGraph *audio_filter_graph = NULL, *video_filter_graph = NULL;
    int audio_stream_index = -1, video_stream_index = -1;
    AVRational aud_time_base;
    AVRational vid_time_base;

    int audio_wait_ms = 0;
    int video_wait_ms = 0;
    const int BUF_SECS = 3;
    int ret;
    bool go = false;

    InitAVConv();

    if(!openMediaFile(m_media_in.filename, fmt_ctx, aud_dec_ctx, vid_dec_ctx, 
                      audio_stream_index, video_stream_index))
    {
        m_open.set(false);
        return 0;
    }

    FillMediaFileProp(fmt_ctx, aud_dec_ctx, vid_dec_ctx, m_media_in);

    if(audio_stream_index >= 0)
        aud_time_base = fmt_ctx->streams[audio_stream_index]->time_base;
    if(video_stream_index >= 0)
        vid_time_base = fmt_ctx->streams[video_stream_index]->time_base;

    if(m_media_out.audio && audio_stream_index >= 0)
    {
        audio_filter_graph = createAudioFilterGraph(fmt_ctx, aud_dec_ctx,
                                                    aud_buffersink_ctx,
                                                    aud_buffersrc_ctx,
                                                    audio_stream_index,
                                                    m_media_out.audio_channels,
                                                    m_media_out.audio_samplerate);

        if(!audio_filter_graph)
        {
            m_open.set(false);
            goto end;
        }
        
        if(m_media_out.audio_samplerate>0)
        {
            audio_wait_ms = (1000 * m_media_out.audio_samples) / m_media_out.audio_samplerate;
            
            int media_bytes = m_media_out.audio_samplerate * m_media_out.audio_channels * sizeof(short);
            media_bytes += sizeof(AudioFrame);
            media_bytes *= BUF_SECS;
            m_audio_frames.high_water_mark(media_bytes); //keep BUF_SECS seconds of audio data

            MYTRACE(ACE_TEXT("MediaStreamer audio buffer size %u\n"),
                    (unsigned int)m_audio_frames.high_water_mark());
        }
    }
    else
        audio_stream_index = -1; //disable audio processing

    if(m_media_out.video && video_stream_index >= 0)
    {
        video_filter_graph = createVideoFilterGraph(fmt_ctx, vid_dec_ctx,
                                                    vid_buffersink_ctx,
                                                    vid_buffersrc_ctx,
                                                    video_stream_index);
        if(!video_filter_graph)
        {
            m_open.set(false);
            goto end;
        }

        //resize buffer
        int video_frame_size = m_media_in.video_width * m_media_in.video_height * 4;
        double video_fps = (double)m_media_in.video_fps_numerator / (double)m_media_in.video_fps_denominator;
        if(video_fps > .0)
            video_wait_ms = 1000 / video_fps;

        int media_frame_size = video_frame_size + sizeof(VideoFrame);
        m_video_frames.high_water_mark(media_frame_size * video_fps * BUF_SECS); //keep BUF_SECS seconds of video data

        MYTRACE(ACE_TEXT("Media stream video buffer for dim. %dx%d %g. Size set to %d bytes\n"),
                m_media_in.video_width, m_media_in.video_height, video_fps,
                (int)m_video_frames.high_water_mark());

    }
    else
        video_stream_index = -1;

    //open and ready to go
    m_open.set(true);

    //wait for start signal
    m_start.get(go);
    if(!go)
        goto fail;

    if(!m_stop && m_listener)
        m_listener->MediaStreamStatusCallback(this, m_media_in, MEDIASTREAM_STARTED);

    ACE_UINT32 start_time, wait_ms;
    start_time = GETTIMESTAMP();
    if(audio_wait_ms && video_wait_ms)
        wait_ms = std::min(audio_wait_ms, video_wait_ms);
    else if(audio_wait_ms)
        wait_ms = audio_wait_ms;
    else if(video_wait_ms)
        wait_ms = video_wait_ms;

    /* read all packets */
    AVPacket packet;
    AVFrame frame;
    int got_frame;

    while (!m_stop)
    {
        AVFilterBufferRef *framebuf_ref;
        if ((ret = av_read_frame(fmt_ctx, &packet)) < 0)
            break;

        if (packet.stream_index == audio_stream_index)
        {
            avcodec_get_frame_defaults(&frame);
            got_frame = 0;
            ret = avcodec_decode_audio4(aud_dec_ctx, &frame, &got_frame, &packet);
            if (ret < 0)
            {
                MYTRACE(ACE_TEXT("Error decoding audio\n"));
                continue;
            }

            if (got_frame)
            {
                int64_t tm = av_frame_get_best_effort_timestamp(&frame);

//                 cout << "Audio frame " << n_audframe++ << " at time " << (tm * av_q2d(aud_time_base)) << endl;

                /* push the audio data from decoded frame into the filtergraph */
                if (av_buffersrc_add_frame(aud_buffersrc_ctx, &frame, 0) < 0) {
                    MYTRACE(ACE_TEXT("Error while feeding the audio filtergraph\n"));
                    break;
                }

                /* pull filtered audio from the filtergraph */
                while (!m_stop)
                {
                    ret = av_buffersink_get_buffer_ref(aud_buffersink_ctx, 
                                                       &framebuf_ref, 0);
                    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    if(ret < 0)
                        goto fail;
                    if (framebuf_ref)
                    {
                        const AVFilterBufferRefAudioProps *props = framebuf_ref->audio;
                        int n_channels = av_get_channel_layout_nb_channels(props->channel_layout);
                        int total_samples = props->nb_samples * n_channels;
                        short* audio_data = reinterpret_cast<short*>(framebuf_ref->data[0]);

                        ACE_Message_Block* mb;
                        ACE_NEW_NORETURN(mb, 
                                         ACE_Message_Block(sizeof(AudioFrame) + 
                                                           total_samples * 
                                                           sizeof(short)));
                        if(mb)
                        {
                            AudioFrame media_frame;
                            media_frame.timestamp = (ACE_UINT32)(tm * av_q2d(aud_time_base) * 1000.0);
                            media_frame.input_buffer = reinterpret_cast<short*>(mb->wr_ptr() + sizeof(media_frame));
                            media_frame.input_samples = props->nb_samples;
                            media_frame.input_channels = m_media_out.audio_channels;
                            media_frame.input_samplerate = m_media_out.audio_samplerate;

                            mb->copy(reinterpret_cast<const char*>(&media_frame), 
                                     sizeof(media_frame));
                            mb->copy(reinterpret_cast<const char*>(audio_data), 
                                     PCM16_BYTES(props->nb_samples, n_channels));

                            int push = -1;
                            ACE_Time_Value tm;
                            while(!m_stop && (push = m_audio_frames.enqueue(mb, &tm)) < 0)
                            {
//                                 MYTRACE(ACE_TEXT("Audio frame queue full. Waiting to insert frame %u\n"),
//                                         media_frame.timestamp);
                                ProcessAVQueues(start_time, wait_ms, false);
                            }

                            if(push < 0)
                            {
                                mb->release();
                                MYTRACE(ACE_TEXT("Dropped audio frame %u\n"), media_frame.timestamp);
                            }
//                             MYTRACE_COND(push >= 0, 
//                                          ACE_TEXT("Insert audio frame %u\n"), media_frame.timestamp);
                        }

                        avfilter_unref_bufferp(&framebuf_ref);
                    }
                }
            }
        }
        else if(packet.stream_index == video_stream_index)
        {
            avcodec_get_frame_defaults(&frame);
            got_frame = 0;
            ret = avcodec_decode_video2(vid_dec_ctx, &frame, &got_frame, &packet);
            if (ret < 0) {
                MYTRACE(ACE_TEXT("Error decoding video\n"));
                break;
            }

            if (got_frame)
            {
                int64_t tm = av_frame_get_best_effort_timestamp(&frame);

                /* push the decoded frame into the filtergraph */
                if (av_buffersrc_add_frame(vid_buffersrc_ctx, &frame, 0) < 0)
                {
                    MYTRACE(ACE_TEXT("Error while feeding the filtergraph\n"));
                    break;
                }

                /* pull filtered pictures from the filtergraph */
                while (!m_stop)
                {
                    ret = av_buffersink_get_buffer_ref(vid_buffersink_ctx, 
                                                       &framebuf_ref, 0);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    if (ret < 0)
                        goto fail;

                    if (framebuf_ref)
                    {
                        AVFilterBufferRefVideoProps *props = framebuf_ref->video;
                        ACE_Message_Block* mb;
                        int bmp_size = props->h * framebuf_ref->linesize[0];
                        int media_frame_size = bmp_size + sizeof(VideoFrame);
                        assert(props->w == m_media_in.video_width);
                        assert(props->h == m_media_in.video_height);
                        ACE_NEW_NORETURN(mb, ACE_Message_Block(media_frame_size));
                        if(mb)
                        {
                            VideoFrame media_frame;
                            media_frame.timestamp = (ACE_UINT32)(tm * av_q2d(vid_time_base) * 1000.0);
                            media_frame.frame_length = bmp_size;
                            media_frame.frame = mb->wr_ptr() + sizeof(media_frame);
                            media_frame.width = props->w;
                            media_frame.height = props->h;
                            media_frame.top_down = true;
                            mb->copy(reinterpret_cast<const char*>(&media_frame), sizeof(media_frame));
                            mb->copy(reinterpret_cast<const char*>(framebuf_ref->data[0]), bmp_size);

                            int push = -1;
                            ACE_Time_Value tm;
                            while(!m_stop && (push = m_video_frames.enqueue(mb, &tm)) < 0)
                            {
//                                 MYTRACE(ACE_TEXT("Video frame queue full. Waiting to insert frame %u\n"),
//                                         media_frame.timestamp);
                                ProcessAVQueues(start_time, wait_ms, false);
                            }

                            if(push < 0)
                            {
                                mb->release();
                                MYTRACE(ACE_TEXT("Dropped video frame %u\n"), media_frame.timestamp);
                            }
//                             MYTRACE_COND(push >= 0,
//                                          ACE_TEXT("Insert video frame %u\n"), media_frame.timestamp);
                        }
                        avfilter_unref_bufferp(&framebuf_ref);
                        //break loop if video buffer is full
                        if(mb == NULL)
                            break;
                    }
                }
            }
        }
        av_free_packet(&packet);

//         MYTRACE(ACE_TEXT("Video frame queue size %u usage %u/%u.\nAudio frame queue size %u usage %u/%u\n"),
//                 m_video_frames.message_count(),
//                 m_video_frames.message_bytes(), m_video_frames.high_water_mark(),
//                 m_audio_frames.message_count(),
//                 m_audio_frames.message_bytes(), m_audio_frames.high_water_mark());
    }

    while(!m_stop &&
          (m_video_frames.message_bytes() || m_audio_frames.message_bytes()))
        ProcessAVQueues(start_time, wait_ms, true);

    //don't do callback if thread is asked to quit
    if(m_listener && !m_stop)
        m_listener->MediaStreamStatusCallback(this, m_media_in, MEDIASTREAM_FINISHED);

    goto end;

fail:
    //don't do callback if thread is asked to quit
    if(m_listener && !m_stop)
        m_listener->MediaStreamStatusCallback(this, m_media_in, MEDIASTREAM_ERROR);

end:
    if(audio_filter_graph)
        avfilter_graph_free(&audio_filter_graph);
    if (aud_dec_ctx)
        avcodec_close(aud_dec_ctx);
    if(video_filter_graph)
        avfilter_graph_free(&video_filter_graph);
    if(vid_dec_ctx)
        avcodec_close(vid_dec_ctx);
    if(fmt_ctx)
        avformat_close_input(&fmt_ctx);

    MYTRACE(ACE_TEXT("Quitting FFMpegStreamer thread\n"));
    return 0;
}



AVFilterGraph* createAudioFilterGraph(AVFormatContext *fmt_ctx,
                                      AVCodecContext* aud_dec_ctx,
                                      AVFilterContext*& aud_buffersink_ctx,
                                      AVFilterContext*& aud_buffersrc_ctx,
                                      int audio_stream_index,
                                      int out_channels,
                                      int out_samplerate)
{
    //init filters
    AVFilterGraph *filter_graph;

    AVFilter *abuffersrc  = avfilter_get_by_name("abuffer");
    AVFilter *abuffersink = avfilter_get_by_name("ffabuffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc(); //TODO: Free??
    AVFilterInOut *inputs  = avfilter_inout_alloc(); //TODO: Free??
    const enum AVSampleFormat sample_fmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
    AVABufferSinkParams *abuffersink_params;
    const AVFilterLink *outlink;
    AVRational time_base = fmt_ctx->streams[audio_stream_index]->time_base;
    char args[512];
    char filter_descr[100];
    int ret;

    filter_graph = avfilter_graph_alloc();

    /* buffer audio source: the decoded frames from the decoder will be inserted here. */
    if (!aud_dec_ctx->channel_layout)
        aud_dec_ctx->channel_layout = av_get_default_channel_layout(aud_dec_ctx->channels);

    snprintf(args, sizeof(args),
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%X",
             time_base.num, time_base.den, aud_dec_ctx->sample_rate,
             av_get_sample_fmt_name(aud_dec_ctx->sample_fmt), (unsigned)aud_dec_ctx->channel_layout);

    ret = avfilter_graph_create_filter(&aud_buffersrc_ctx, abuffersrc, "in",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        MYTRACE(ACE_TEXT("Cannot create audio buffer source\n"));
        goto error;
    }

    /* buffer audio sink: to terminate the filter chain. */
    abuffersink_params = av_abuffersink_params_alloc();
    abuffersink_params->sample_fmts     = sample_fmts;
    ret = avfilter_graph_create_filter(&aud_buffersink_ctx, abuffersink, "out",
                                       NULL, abuffersink_params, filter_graph);
    av_free(abuffersink_params);
    if (ret < 0) {
        MYTRACE(ACE_TEXT("Cannot create audio buffer sink\n"));
        goto error;
    }

    /* Endpoints for the filter graph. */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = aud_buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    inputs->name       = av_strdup("out");
    inputs->filter_ctx = aud_buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    snprintf(filter_descr, sizeof(filter_descr),
             "aresample=%d,aconvert=s16:%s",
             out_samplerate, (out_channels == 2?"stereo":"mono"));

    if ((ret = avfilter_graph_parse(filter_graph, filter_descr,
                                    &inputs, &outputs, NULL)) < 0)
        goto error;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto error;

    /* Print summary of the sink buffer
     * Note: args buffer is reused to store channel layout string */
    outlink = aud_buffersink_ctx->inputs[0];
    // av_get_channel_layout_string(args, sizeof(args), -1, outlink->channel_layout);
    // av_log(NULL, AV_LOG_INFO, "Output: srate:%dHz fmt:%s chlayout:%s\n",
    //        (int)outlink->sample_rate,
    //        (char *)av_x_if_null(av_get_sample_fmt_name((AVSampleFormat)outlink->format), "?"),
    //        args);


    return filter_graph;

error:
    avfilter_graph_free(&filter_graph);
    return NULL;
}


AVFilterGraph* createVideoFilterGraph(AVFormatContext *fmt_ctx,
                                      AVCodecContext* vid_dec_ctx,
                                      AVFilterContext*& vid_buffersink_ctx,
                                      AVFilterContext*& vid_buffersrc_ctx,
                                      int video_stream_index)
{
    //init filters
    AVFilterGraph *filter_graph;
    char args[512];
    int ret;
    AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    AVFilter *buffersink = avfilter_get_by_name("ffbuffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    const enum PixelFormat pix_fmts[] = { AV_PIX_FMT_RGB32, AV_PIX_FMT_NONE };
    AVBufferSinkParams *buffersink_params;
    char filters_descr[100];

    snprintf(filters_descr, sizeof(filters_descr), "scale=%d:%d",
             vid_dec_ctx->width, vid_dec_ctx->height);

    filter_graph = avfilter_graph_alloc();

    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             vid_dec_ctx->width, vid_dec_ctx->height, vid_dec_ctx->pix_fmt,
             vid_dec_ctx->time_base.num, vid_dec_ctx->time_base.den,
             vid_dec_ctx->sample_aspect_ratio.num, vid_dec_ctx->sample_aspect_ratio.den);

    MYTRACE(ACE_TEXT("%s\n"), args);

    ret = avfilter_graph_create_filter(&vid_buffersrc_ctx, buffersrc, "in",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        MYTRACE(ACE_TEXT("Cannot create buffer source\n"));
        goto error;
    }

    /* buffer video sink: to terminate the filter chain. */
    buffersink_params = av_buffersink_params_alloc();
    buffersink_params->pixel_fmts = pix_fmts;
    ret = avfilter_graph_create_filter(&vid_buffersink_ctx, buffersink, "out",
                                       NULL, buffersink_params, filter_graph);
    av_free(buffersink_params);
    if (ret < 0) {
        MYTRACE(ACE_TEXT("Cannot create buffer sink\n"));
        goto error;
    }

    /* Endpoints for the filter graph. */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = vid_buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    inputs->name       = av_strdup("out");
    inputs->filter_ctx = vid_buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    if ((ret = avfilter_graph_parse(filter_graph, filters_descr,
                                    &inputs, &outputs, NULL)) < 0)
        goto error;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto error;

    return filter_graph;

error:
    avfilter_graph_free(&filter_graph);
    return NULL;
}

