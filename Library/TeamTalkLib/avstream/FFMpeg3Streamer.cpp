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

#include "FFMpeg3Streamer.h"

#include <inttypes.h>
#include <myace/MyACE.h>
#include <codec/MediaUtil.h>

#include <assert.h>

// FFMpeg type collides with AVFoundation, so keep in cpp file
extern "C" {
#include <libavutil/rational.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}

using namespace media;

void InitAVConv()
{
    static bool ready = false;
    if(!ready)
    {
        static ACE_Recursive_Thread_Mutex mtx;

        wguard_t g(mtx);

        if (!ready)
        {
#if defined(_DEBUG)
            av_log_set_level(AV_LOG_MAX_OFFSET);
#else
            av_log_set_level(AV_LOG_QUIET);
#endif
            avdevice_register_all();
            av_register_all();
            avfilter_register_all();
            ready = true;
        }
    }
}

bool OpenInput(const ACE_TString& filename,
               AVInputFormat *iformat,
               AVDictionary *options,
               AVFormatContext*& fmt_ctx,
               AVCodecContext*& aud_dec_ctx,
               AVCodecContext*& vid_dec_ctx,
               int& audio_stream_index,
               int& video_stream_index)
{
    AVCodec *aud_dec, *vid_dec;

    if (avformat_open_input(&fmt_ctx, filename.c_str(), iformat, &options) < 0)
    {
        MYTRACE(ACE_TEXT("FFMpeg opened %s\n"), filename.c_str());
        goto cleanup;
    }
    
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
    {
        MYTRACE(ACE_TEXT("FFMpeg found stream info\n"));
        goto cleanup;
    }

    /* select the audio stream */
    audio_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, 
                                             -1, -1, &aud_dec, 0);
    if (audio_stream_index >= 0) {
        aud_dec_ctx = fmt_ctx->streams[audio_stream_index]->codec;

        /* init the audio decoder */
        if (avcodec_open2(aud_dec_ctx, aud_dec, NULL) < 0) {
            audio_stream_index = -1;
        }
    }

    /* select the video stream */
    video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, 
                                             -1, -1, &vid_dec, 0);
    if (video_stream_index >= 0) {
        vid_dec_ctx = fmt_ctx->streams[video_stream_index]->codec;

        /* init the video decoder */
        if (avcodec_open2(vid_dec_ctx, vid_dec, NULL) < 0) {
            video_stream_index = -1;
        }
    }


cleanup:

    if(audio_stream_index < 0 && video_stream_index < 0)
    {
        avformat_close_input(&fmt_ctx);
        fmt_ctx = NULL;

        return false;
    }
    else
    {
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
                                      int video_stream_index,
                                      AVPixelFormat output_pixfmt);

void FillMediaFileProp(AVFormatContext *fmt_ctx,
                       AVCodecContext *aud_dec_ctx, 
                       AVCodecContext *vid_dec_ctx,
                       MediaFileProp& out_prop)
{
    if (aud_dec_ctx)
    {
        out_prop.audio = media::AudioFormat(aud_dec_ctx->sample_rate, aud_dec_ctx->channels);
    }

    if(vid_dec_ctx)
    {
        //frame rate
        double fps = 1.0 / av_q2d(vid_dec_ctx->time_base) / std::max(vid_dec_ctx->ticks_per_frame, 1);
        AVRational r_fps = av_d2q(fps, 1000);
        out_prop.video = media::VideoFormat(vid_dec_ctx->width, vid_dec_ctx->height,
                                            r_fps.num, r_fps.den, media::FOURCC_RGB32);
    }

    out_prop.duration_ms = (fmt_ctx->duration * av_q2d(AV_TIME_BASE_Q)) * 1000;
}

bool GetAVMediaFileProp(const ACE_TString& filename, MediaFileProp& out_prop)
{
    InitAVConv();

    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *aud_dec_ctx = NULL, *vid_dec_ctx = NULL;
    int audio_stream_index = -1, video_stream_index = -1;

    if(!OpenInput(filename, NULL, NULL, fmt_ctx, aud_dec_ctx, vid_dec_ctx, 
                  audio_stream_index, video_stream_index))
        return false;

    FillMediaFileProp(fmt_ctx, aud_dec_ctx, vid_dec_ctx, out_prop);
    out_prop.filename = filename;

    if (aud_dec_ctx)
        avcodec_close(aud_dec_ctx);

    if(vid_dec_ctx)
        avcodec_close(vid_dec_ctx);

    avformat_close_input(&fmt_ctx);

    return out_prop.IsValid();
}


FFMpegStreamer::FFMpegStreamer()
{
    InitAVConv();
}

FFMpegStreamer::~FFMpegStreamer()
{
    Close();
    MYTRACE(ACE_TEXT("~FFMpegStreamer()\n"));
}

bool FFMpegStreamer::SetupInput(AVInputFormat *iformat,
                                AVDictionary *options,
                                AVFormatContext*& fmt_ctx,
                                AVCodecContext*& aud_dec_ctx,
                                AVCodecContext*& vid_dec_ctx,
                                int& audio_stream_index,
                                int& video_stream_index)
{
    return OpenInput(m_media_in.filename, iformat, options, fmt_ctx, aud_dec_ctx,
                     vid_dec_ctx, audio_stream_index, video_stream_index);
}


void FFMpegStreamer::Run()
{
    AVFormatContext *fmt_ctx = NULL;
    int audio_stream_index = -1, video_stream_index = -1;
    AVInputFormat *in_fmt = NULL;
    AVDictionary *options = NULL;
    AVCodecContext *aud_dec_ctx= NULL, *vid_dec_ctx = NULL;

    AVFilterContext *aud_buffersink_ctx = NULL, *aud_buffersrc_ctx = NULL;
    AVFilterContext *vid_buffersink_ctx = NULL, *vid_buffersrc_ctx = NULL;
    AVFilterGraph *audio_filter_graph = NULL, *video_filter_graph = NULL;
    AVFrame* aud_frame = av_frame_alloc();
    AVFrame* vid_frame = av_frame_alloc();
    AVFrame* filt_frame = av_frame_alloc();

    int ret;
    bool start = false;

    if(!SetupInput(in_fmt, options, fmt_ctx, aud_dec_ctx, vid_dec_ctx, 
                   audio_stream_index, video_stream_index))
    {
        MYTRACE("Failed to setup input: %s\n", m_media_in.filename.c_str());
        m_open.set(false);
        goto end;
    }

    FillMediaFileProp(fmt_ctx, aud_dec_ctx, vid_dec_ctx, m_media_in);

    if(m_media_out.HasAudio() && audio_stream_index >= 0)
    {
        audio_filter_graph = createAudioFilterGraph(fmt_ctx, aud_dec_ctx,
                                                    aud_buffersink_ctx,
                                                    aud_buffersrc_ctx,
                                                    audio_stream_index,
                                                    m_media_out.audio.channels,
                                                    m_media_out.audio.samplerate);

        if(!audio_filter_graph)
        {
            m_open.set(false);
            goto end;
        }
    }
    else
    {
        audio_stream_index = -1; //disable audio processing
        m_media_out.audio = media::AudioFormat();
    }
    
    if (m_media_out.video.fourcc != media::FOURCC_NONE && video_stream_index >= 0)
    {
        video_filter_graph = createVideoFilterGraph(fmt_ctx, vid_dec_ctx,
                                                    vid_buffersink_ctx,
                                                    vid_buffersrc_ctx,
                                                    video_stream_index,
                                                    AV_PIX_FMT_RGB32);
        if(!video_filter_graph)
        {
            m_open.set(false);
            goto end;
        }

        m_media_out.video = m_media_in.video;
        m_media_out.video.fourcc = media::FOURCC_RGB32;
    }
    else
    {
        video_stream_index = -1;
        m_media_out.video = media::VideoFormat();
    }

    //open and ready to start
    m_open.set(true);

    InitBuffers();
    
    //wait for start signal
    MYTRACE(ACE_TEXT("FFMpeg3 waiting to start streaming: %s\n"), m_media_in.filename.c_str());
    m_run.get(start);
    if(!start)
        goto fail;

    MediaStreamStatus status;
    ACE_UINT32 start_time, start_offset, totalpausetime;
    int64_t curaudiotime, curvideotime;
    
    status = MEDIASTREAM_STARTED;
    start_time = GETTIMESTAMP();
    start_offset = MEDIASTREAMER_OFFSET_IGNORE;
    totalpausetime = 0;
    curaudiotime = curvideotime = 0;

    /* read all packets */
    AVPacket packet;
    int got_frame;

    while (!m_stop)
    {
        MYTRACE(ACE_TEXT("Sync. Audio %u, Video %u\n"), ACE_UINT32(curaudiotime),
                ACE_UINT32(curvideotime));

        if (curaudiotime > 0)
            m_media_in.elapsed_ms = ACE_UINT32(curaudiotime);
        else if (curvideotime > 0)
            m_media_in.elapsed_ms = ACE_UINT32(curvideotime);

        if (start_offset != MEDIASTREAMER_OFFSET_IGNORE)
            m_media_in.elapsed_ms += start_offset;
        
        // check if we should pause
        if (m_pause)
        {
            if (m_statuscallback)
                m_statuscallback(m_media_in, MEDIASTREAM_PAUSED);

            ACE_UINT32 pausetime = GETTIMESTAMP();
            if ((m_run.get(start) >= 0 && !start) || m_stop)
            {
                MYTRACE(ACE_TEXT("Media playback aborted during pause\n"));
                break;
            }

            // ensure we don't submit MEDIASTREAM_STARTED twice (also for seek)
            status = MEDIASTREAM_STARTED;

            pausetime = GETTIMESTAMP() - pausetime;
            MYTRACE_COND(pausetime > 0, ACE_TEXT("Paused %s for %u msec\n"), m_media_in.filename.c_str(), pausetime);
            totalpausetime += pausetime;
        }

        auto newoffset = SetOffset(MEDIASTREAMER_OFFSET_IGNORE);

        if (newoffset != MEDIASTREAMER_OFFSET_IGNORE)
        {
            double offset_sec = newoffset;
            offset_sec /= 1000.0;

            bool success = true;
            
            if (audio_stream_index >= 0)
            {
                auto aud_stream = fmt_ctx->streams[audio_stream_index];
                double curaudio_sec = curaudiotime / 1000.0;
                double difftime_sec = (offset_sec > curaudio_sec)? offset_sec - curaudio_sec : curaudio_sec - offset_sec;
                    
                if (av_seek_frame(fmt_ctx, audio_stream_index, difftime_sec / av_q2d(aud_stream->time_base),
                                  (offset_sec > curaudio_sec? 0 : AVSEEK_FLAG_BACKWARD)) < 0)
                {
                    MYTRACE(ACE_TEXT("Failed to seek to audio position %u in %s\n"), ACE_UINT32(offset_sec * 1000), m_media_in.filename.c_str());
                    success = false;
                }
                else
                {
                    MYTRACE("Seeked to audio position %u in %s\n", ACE_UINT32(offset_sec * 1000), m_media_in.filename.c_str());
                }
            }

            if (video_stream_index >= 0)
            {
                auto vid_stream = fmt_ctx->streams[video_stream_index];
                double curvideo_sec = curvideotime / 1000.0;
                double difftime_sec = (offset_sec > curvideo_sec)? offset_sec - curvideo_sec : curvideo_sec - offset_sec;
                    
                if (av_seek_frame(fmt_ctx, video_stream_index, difftime_sec / av_q2d(vid_stream->time_base),
                                  (offset_sec > curvideo_sec? 0 : AVSEEK_FLAG_BACKWARD)) < 0)
                {
                    MYTRACE(ACE_TEXT("Failed to seek to video position %u in %s\n"), ACE_UINT32(offset_sec * 1000), m_media_in.filename.c_str());
                    success = false;
                }
                else
                {
                    MYTRACE("Seeked to video position %u in %s\n", ACE_UINT32(offset_sec * 1000), m_media_in.filename.c_str());
                }

            }

            if (success)
            {
                m_media_in.elapsed_ms = newoffset;
                start_time = GETTIMESTAMP();
                totalpausetime = 0;
                start_offset = MEDIASTREAMER_OFFSET_IGNORE;
                
                ClearBuffers();

                status = MEDIASTREAM_STARTED;
            }
        }
        
        if (status != MEDIASTREAM_NONE)
        {
            if (m_statuscallback)
                m_statuscallback(m_media_in, status);

            status = MEDIASTREAM_NONE;
        }
        
        if ((ret = av_read_frame(fmt_ctx, &packet)) < 0)
            break;

        if (packet.stream_index == audio_stream_index)
        {
            got_frame = 0;
            ret = avcodec_decode_audio4(aud_dec_ctx, aud_frame, &got_frame, &packet);
            if (ret < 0)
            {
                MYTRACE(ACE_TEXT("Error decoding audio\n"));
                continue;
            }

            if (got_frame)
            {
//                 cout << "Audio frame " << n_audframe++ << " at time " << (tm * av_q2d(aud_time_base)) << endl;

                /* push the audio data from decoded frame into the filtergraph */
                if (av_buffersrc_add_frame(aud_buffersrc_ctx, aud_frame) < 0) {
                    MYTRACE(ACE_TEXT("Error while feeding the audio filtergraph\n"));
                    break;
                }

                curaudiotime = ProcessAudioBuffer(aud_buffersink_ctx, filt_frame,
                                                  fmt_ctx->streams[audio_stream_index],
                                                  start_time, start_offset);
                if (curaudiotime < 0)
                {
                    goto fail;
                }
            } // got_frame
        }
        else if(packet.stream_index == video_stream_index)
        {
            got_frame = 0;
            ret = avcodec_decode_video2(vid_dec_ctx, vid_frame, &got_frame, &packet);
            if (ret < 0) {
                MYTRACE(ACE_TEXT("Error decoding video\n"));
                break;
            }

            if (got_frame)
            {
                // vid_frame->pts = av_frame_get_best_effort_timestamp(vid_frame);

                /* push the decoded frame into the filtergraph */
                if (av_buffersrc_add_frame(vid_buffersrc_ctx, vid_frame) < 0)
                {
                    MYTRACE(ACE_TEXT("Error while feeding the filtergraph\n"));
                    break;
                }

                curvideotime = ProcessVideoBuffer(vid_buffersink_ctx, filt_frame, 
                                                  fmt_ctx->streams[video_stream_index],
                                                  start_time, start_offset);
                if (curvideotime < 0)
                {
                    goto fail;
                }
                
            } // got_frame
        } // stream index
        av_packet_unref(&packet);

        if (m_statuscallback)
            m_statuscallback(m_media_in, MEDIASTREAM_PLAYING);

        while(!m_stop && ProcessAVQueues(start_time, GETTIMESTAMP() - totalpausetime, false));

    } // while

    while(!m_stop && ProcessAVQueues(start_time, GETTIMESTAMP() - totalpausetime, true));

    //don't do callback if thread is asked to quit
    if (m_statuscallback && !m_stop)
        m_statuscallback(m_media_in, MEDIASTREAM_FINISHED);

    MYTRACE(ACE_TEXT("FFMpeg3 finished streaming: %s\n"), m_media_in.filename.c_str());
    goto end;

fail:
    //don't do callback if thread is asked to quit
    if (m_statuscallback && !m_stop)
        m_statuscallback(m_media_in, MEDIASTREAM_ERROR);

end:
    if(audio_filter_graph)
        avfilter_graph_free(&audio_filter_graph);
    avcodec_close(aud_dec_ctx);
    if(video_filter_graph)
        avfilter_graph_free(&video_filter_graph);
    if(vid_dec_ctx)
        avcodec_close(vid_dec_ctx);
    // if(options)
    //     av_dict_free(&options); //causes crash...
    if(fmt_ctx)
        avformat_close_input(&fmt_ctx);
    av_frame_free(&aud_frame);
    av_frame_free(&vid_frame);
    av_frame_free(&filt_frame);
    MYTRACE(ACE_TEXT("Quitting FFMpegStreamer thread\n"));
}

int64_t FFMpegStreamer::ProcessAudioBuffer(AVFilterContext* aud_buffersink_ctx,
                                           AVFrame* filt_frame,
                                           AVStream* aud_stream,
                                           ACE_UINT32 start_time,
                                           ACE_UINT32& start_offset)
{

    /* pull filtered audio from the filtergraph */

    int ret = av_buffersink_get_frame(aud_buffersink_ctx, filt_frame);
    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        return 0;
    
    if(ret < 0)
        return -1;

    int64_t frame_tm = av_frame_get_best_effort_timestamp(filt_frame);
    double frame_sec = frame_tm * av_q2d(aud_stream->time_base);
    ACE_UINT32 frame_timestamp = ACE_UINT32(frame_sec * 1000.0); //msec

    if (AddStartTime())
    {
        // initial frame should be timestamp = 0 msec. HTTP streams
        // can start in the middle of a long session
        if (start_offset == MEDIASTREAMER_OFFSET_IGNORE)
            start_offset = frame_timestamp;
        frame_timestamp -= start_offset;
    }

    int n_channels = av_get_channel_layout_nb_channels(filt_frame->channel_layout);
    short* audio_data = reinterpret_cast<short*>(filt_frame->data[0]);

    AudioFrame media_frame;
    media_frame.timestamp = frame_timestamp;
    media_frame.input_buffer = audio_data;
    media_frame.input_samples = filt_frame->nb_samples;
    media_frame.inputfmt = m_media_out.audio;
    QueueAudio(media_frame);
        
    av_frame_unref(filt_frame);

    return frame_timestamp;
}

int64_t FFMpegStreamer::ProcessVideoBuffer(AVFilterContext* vid_buffersink_ctx,
                                           AVFrame* filt_frame,
                                           AVStream* vid_stream,
                                           ACE_UINT32 start_time,
                                           ACE_UINT32& start_offset)
{
    assert(W32_LEQ(start_time, GETTIMESTAMP()));

    /* pull filtered pictures from the filtergraph */

    int ret = av_buffersink_get_frame(vid_buffersink_ctx, filt_frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        return 0;
    if (ret < 0)
        return -1;

    int64_t frame_tm = av_frame_get_best_effort_timestamp(filt_frame);
    double frame_sec = frame_tm * av_q2d(vid_stream->time_base);
    ACE_UINT32 frame_timestamp = ACE_UINT32(frame_sec * 1000.0); //msec

    if (AddStartTime())
    {
        // initial frame should be timestamp = 0 msec
        if (start_offset == -1)
            start_offset = frame_timestamp;
        frame_timestamp -= start_offset;
    }
    else
    {
        MYTRACE_COND(!W32_GEQ(frame_timestamp, start_time),
                     ACE_TEXT("Frame time: %u, start time: %u, diff: %d\n"),
                     frame_timestamp, start_time, int(frame_timestamp - start_time));
            
        if (start_offset == -1)
        {
            // the first couple of frames we get might be from before 'start_time'
            if (W32_LT(frame_timestamp, start_time))
            {
                MYTRACE(ACE_TEXT("Dropped video frame timestamped %u because it's before start time %u\n"),
                        frame_timestamp, start_time);
                av_frame_unref(filt_frame);
                return 1;
            }
            start_offset = 0;
        }
        assert(W32_GEQ(GETTIMESTAMP(), frame_timestamp));
        frame_timestamp -= start_time;
    }

    static int n_vidframe = 0;
    ACE_UINT32 ticks = GETTIMESTAMP();
    MYTRACE(ACE_TEXT("Video frame %d at tick %u, frame time: %u, diff: %u, pts: %.3f.\n"),
            n_vidframe++, ticks, frame_timestamp, ticks - frame_timestamp, frame_sec);

    int bmp_size = filt_frame->height * filt_frame->linesize[0];
    VideoFrame media_frame(reinterpret_cast<char*>(filt_frame->data[0]),
                           bmp_size, filt_frame->width, filt_frame->height,
                           m_media_out.video.fourcc, true);
    media_frame.timestamp = frame_timestamp;

    assert(filt_frame->width == m_media_in.video.width);
    assert(filt_frame->height == m_media_in.video.height);
    
    QueueVideo(media_frame);

    av_frame_unref(filt_frame);

    return frame_timestamp;
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
    AVFilterGraph *filter_graph = NULL;

    AVFilter *abuffersrc  = avfilter_get_by_name("abuffer");
    AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc(); //TODO: Free??
    AVFilterInOut *inputs  = avfilter_inout_alloc(); //TODO: Free??
    const enum AVSampleFormat out_sample_fmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
    int64_t out_channel_layouts[] = { (out_channels==1?AV_CH_LAYOUT_MONO:AV_CH_LAYOUT_STEREO), -1 };
    int out_sample_rates[] = { out_samplerate, -1 };
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
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%x",
             time_base.num, time_base.den, aud_dec_ctx->sample_rate,
             av_get_sample_fmt_name(aud_dec_ctx->sample_fmt), (unsigned)aud_dec_ctx->channel_layout);

    ret = avfilter_graph_create_filter(&aud_buffersrc_ctx, abuffersrc, "in",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        MYTRACE(ACE_TEXT("Cannot create audio buffer source\n"));
        goto error;
    }

    /* buffer audio sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&aud_buffersink_ctx, abuffersink, "out",
                                       NULL, NULL, filter_graph);
    if (ret < 0) {
        MYTRACE(ACE_TEXT("Cannot create audio buffer sink\n"));
        goto error;
    }

    ret = av_opt_set_int_list(aud_buffersink_ctx, "sample_fmts", out_sample_fmts, -1,
                              AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        MYTRACE(ACE_TEXT("Failed to set output sample fmt\n"));
        goto error;
    }

    ret = av_opt_set_int_list(aud_buffersink_ctx, "channel_layouts", out_channel_layouts, -1,
                              AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        MYTRACE(ACE_TEXT("Cannot set output channel layout\n"));
        goto error;
    }
    ret = av_opt_set_int_list(aud_buffersink_ctx, "sample_rates", out_sample_rates, -1,
                              AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        MYTRACE(ACE_TEXT("Cannot set output sample rate\n"));
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
             "aresample=%d,aformat=sample_fmts=s16:channel_layouts=%s",
             out_samplerate, (out_channels == 2?"stereo":"mono"));

    if ((ret = avfilter_graph_parse(filter_graph, filter_descr,
                                    inputs, outputs, NULL)) < 0)
    {
        MYTRACE(ACE_TEXT("Failed to parse graph\n"));
        goto error;
    }

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

    goto end;

error:
    avfilter_graph_free(&filter_graph);
    filter_graph = NULL;

end:
    // avfilter_inout_free(&inputs);
    // avfilter_inout_free(&outputs);

    return filter_graph;    
}


AVFilterGraph* createVideoFilterGraph(AVFormatContext *fmt_ctx,
                                      AVCodecContext* vid_dec_ctx,
                                      AVFilterContext*& vid_buffersink_ctx,
                                      AVFilterContext*& vid_buffersrc_ctx,
                                      int video_stream_index,
                                      AVPixelFormat output_pixfmt)
{
    //init filters
    AVFilterGraph *filter_graph;
    char args[512];
    int ret;
    AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    const enum AVPixelFormat pix_fmts[] = { output_pixfmt, AV_PIX_FMT_NONE };
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
    ret = avfilter_graph_create_filter(&vid_buffersink_ctx, buffersink, "out",
                                       NULL, NULL, filter_graph);
    if (ret < 0) {
        MYTRACE(ACE_TEXT("Cannot create buffer sink\n"));
        goto error;
    }

    ret = av_opt_set_int_list(vid_buffersink_ctx, "pix_fmts", pix_fmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        MYTRACE(ACE_TEXT("Cannot set output pixel format\n"));
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
                                    inputs, outputs, NULL)) < 0)
        goto error;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto error;

    goto end;

error:
    avfilter_graph_free(&filter_graph);
    filter_graph = NULL;

end:
    // avfilter_inout_free(&inputs);
    // avfilter_inout_free(&outputs);

    return filter_graph;

}

