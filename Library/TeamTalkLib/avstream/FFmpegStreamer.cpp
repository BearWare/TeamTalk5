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

#include "FFmpegStreamer.h"

#include <inttypes.h>
#include <myace/MyACE.h>
#include <codec/MediaUtil.h>

#include <assert.h>

#if defined(__APPLE__)
#include <mach/mach_time.h>
#endif

// FFmpeg type collides with AVFoundation, so keep in cpp file
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/rational.h>
}

#define DEBUG_FFMPEG 0

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
#if DEBUG_FFMPEG
            av_log_set_level(AV_LOG_MAX_OFFSET);
#else
            av_log_set_level(AV_LOG_QUIET);
#endif
            avdevice_register_all();
            ready = true;
        }
    }
}

bool OpenInput(const ACE_TString& filename,
               const AVInputFormat *iformat,
               AVDictionary *options,
               AVFormatContext*& fmt_ctx,
               AVCodecContext*& aud_dec_ctx,
               AVCodecContext*& vid_dec_ctx,
               int& audio_stream_index,
               int& video_stream_index)
{
    const AVCodec *aud_dec, *vid_dec;

    if (avformat_open_input(&fmt_ctx, filename.c_str(), iformat, &options) < 0)
    {
        MYTRACE(ACE_TEXT("FFmpeg opened %s\n"), filename.c_str());
        goto cleanup;
    }

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
    {
        MYTRACE(ACE_TEXT("FFmpeg found stream info\n"));
        goto cleanup;
    }

    /* select the audio stream */
    audio_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO,
                                             -1, -1, &aud_dec, 0);
    if (audio_stream_index >= 0) {

        const AVCodecParameters* audparms = fmt_ctx->streams[audio_stream_index]->codecpar;
        const AVCodec *audcodec = avcodec_find_decoder(audparms->codec_id);
        if (audcodec)
        {
            aud_dec_ctx = avcodec_alloc_context3(audcodec);
            /* transfer audio codec parameters to context */
            if (avcodec_parameters_to_context(aud_dec_ctx, audparms) < 0)
            {
                MYTRACE(ACE_TEXT("Failed to transfer audio codec properties to decoder context\n"));
                audio_stream_index = -1;
            }
            /* init the audio decoder */
            else if (avcodec_open2(aud_dec_ctx, aud_dec, NULL) < 0)
            {
                MYTRACE(ACE_TEXT("Failed to open FFmpeg audio decoder\n"));
                audio_stream_index = -1;
            }
        }
        else
        {
            audio_stream_index = -1;
        }
    }

    /* select the video stream */
    video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO,
                                             -1, -1, &vid_dec, 0);
    if (video_stream_index >= 0) {
        const AVStream* vidstream = fmt_ctx->streams[video_stream_index];
        const AVCodecParameters* vidparms = vidstream->codecpar;
        const AVCodec *vidcodec = avcodec_find_decoder(vidparms->codec_id);
        if (vidcodec)
        {
            vid_dec_ctx = avcodec_alloc_context3(vidcodec);
            /* transfer video codec parameters to context */
            if (avcodec_parameters_to_context(vid_dec_ctx, vidparms) < 0)
            {
                MYTRACE(ACE_TEXT("Failed to transfer video codec properties to decoder context\n"));
                video_stream_index = -1;
            }
            /* init the video decoder */
            else if (avcodec_open2(vid_dec_ctx, vid_dec, NULL) < 0)
            {
                video_stream_index = -1;
            }
        }
        else
        {
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
                       int video_stream_index,
                       MediaFileProp& out_prop)
{
    if (aud_dec_ctx)
    {
        out_prop.audio = media::AudioFormat(aud_dec_ctx->sample_rate, aud_dec_ctx->ch_layout.nb_channels);
    }

    if (vid_dec_ctx && video_stream_index >= 0)
    {
        // set frame rate
        const AVStream* vidstream = fmt_ctx->streams[video_stream_index];
        out_prop.video = media::VideoFormat(vid_dec_ctx->width, vid_dec_ctx->height,
                                            vidstream->avg_frame_rate.num,
                                            vidstream->avg_frame_rate.den,
                                            media::FOURCC_RGB32);
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

    FillMediaFileProp(fmt_ctx, aud_dec_ctx, vid_dec_ctx, video_stream_index, out_prop);
    out_prop.filename = filename;

    if (aud_dec_ctx)
        avcodec_free_context(&aud_dec_ctx);

    if (vid_dec_ctx)
        avcodec_free_context(&vid_dec_ctx);

    avformat_close_input(&fmt_ctx);

    return out_prop.IsValid();
}


FFmpegStreamer::FFmpegStreamer(const ACE_TString& filename, const MediaStreamOutput& out_prop)
    : MediaFileStreamer(filename, out_prop)
{
    InitAVConv();
}

FFmpegStreamer::~FFmpegStreamer()
{
    Close();
    MYTRACE(ACE_TEXT("~FFmpegStreamer()\n"));
}

bool FFmpegStreamer::SetupInput(const AVInputFormat *iformat,
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


void FFmpegStreamer::Run()
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

    FillMediaFileProp(fmt_ctx, aud_dec_ctx, vid_dec_ctx, video_stream_index, m_media_in);

    if (m_media_in.HasAudio() && !m_media_out.HasAudio() && m_media_out.audio_duration_ms)
    {
        int audio_samples = PCM16_DURATION_SAMPLES(m_media_out.audio_duration_ms, m_media_in.audio.samplerate);
        MediaStreamOutput newoutput(m_media_in.audio, audio_samples, m_media_out.video);
        m_media_out = newoutput;
    }

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
            MYTRACE("Failed to create video filter. Device: %s, fmt %dx%d@%d fourCC: %u\n",
                    m_media_in.filename.c_str(), m_media_out.video.width, m_media_out.video.height,
                    (m_media_out.video.fps_denominator ? m_media_out.video.fps_numerator / m_media_out.video.fps_denominator : -1),
                    m_media_out.video.fourcc);
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
    MYTRACE(ACE_TEXT("FFmpeg waiting to start streaming: %s\n"), m_media_in.filename.c_str());
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
    av_init_packet(&packet);

    while (!m_stop)
    {
        MYTRACE_COND(DEBUG_MEDIASTREAMER, ACE_TEXT("Sync. Audio %u, Video %u\n"), ACE_UINT32(curaudiotime),
                ACE_UINT32(curvideotime));

        if (curaudiotime >= 0)
            m_media_in.elapsed_ms = ACE_UINT32(curaudiotime);
        else if (curvideotime >= 0)
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

        // check if we should seek
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

        if (av_read_frame(fmt_ctx, &packet) < 0)
            break;

        if (packet.stream_index == audio_stream_index)
        {
            ret = avcodec_send_packet(aud_dec_ctx, &packet);
            if (ret < 0)
            {
                MYTRACE(ACE_TEXT("Error decoding audio\n"));
                av_packet_unref(&packet);
                continue;
            }

            ret = avcodec_receive_frame(aud_dec_ctx, aud_frame);
            if (ret == 0)
            {
                /* push the audio data from decoded frame into the filtergraph */
                if (av_buffersrc_add_frame(aud_buffersrc_ctx, aud_frame) < 0) {
                    MYTRACE(ACE_TEXT("Error while feeding the audio filtergraph\n"));
                    av_packet_unref(&packet);
                    break;
                }

                curaudiotime = ProcessAudioBuffer(aud_buffersink_ctx, filt_frame,
                                                  fmt_ctx->streams[audio_stream_index],
                                                  start_time, start_offset);
                if (curaudiotime < 0)
                {
                    goto fail;
                }
            }
            else if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
            {
                MYTRACE(ACE_TEXT("Error decoding audio stream.\n"));
            }
        }
        else if(packet.stream_index == video_stream_index)
        {
            ret = avcodec_send_packet(vid_dec_ctx, &packet);
            if (ret < 0) {
                MYTRACE(ACE_TEXT("Error decoding video\n"));
                av_packet_unref(&packet);
                break;
            }

            ret = avcodec_receive_frame(vid_dec_ctx, vid_frame);
            if (ret == 0)
            {
                /* push the decoded frame into the filtergraph */
                if (av_buffersrc_add_frame(vid_buffersrc_ctx, vid_frame) < 0)
                {
                    MYTRACE(ACE_TEXT("Error while feeding the filtergraph\n"));
                    av_packet_unref(&packet);
                    break;
                }

                curvideotime = ProcessVideoBuffer(vid_buffersink_ctx, filt_frame,
                                                  fmt_ctx->streams[video_stream_index],
                                                  start_time, start_offset);
                if (curvideotime < 0)
                {
                    goto fail;
                }

            }
            else if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
            {
                MYTRACE(ACE_TEXT("Error decoding video stream.\n"));
            }
        }
        av_packet_unref(&packet);

        if (m_statuscallback)
            m_statuscallback(m_media_in, MEDIASTREAM_PLAYING);

        while(!m_stop && ProcessAVQueues(start_time, GETTIMESTAMP() - totalpausetime, false));

    }

    while(!m_stop && ProcessAVQueues(start_time, GETTIMESTAMP() - totalpausetime, true));

    m_completed = true;

    if (m_statuscallback && !m_stop)
        m_statuscallback(m_media_in, MEDIASTREAM_FINISHED);

    MYTRACE(ACE_TEXT("FFmpeg finished streaming: %s\n"), m_media_in.filename.c_str());
    goto end;

fail:
    m_completed = true;

    if (m_statuscallback && !m_stop)
        m_statuscallback(m_media_in, MEDIASTREAM_ERROR);

end:
    if (audio_filter_graph)
        avfilter_graph_free(&audio_filter_graph);
    if (aud_dec_ctx)
        avcodec_free_context(&aud_dec_ctx);
    if (video_filter_graph)
        avfilter_graph_free(&video_filter_graph);
    if (vid_dec_ctx)
        avcodec_free_context(&vid_dec_ctx);
    if(fmt_ctx)
        avformat_close_input(&fmt_ctx);
    av_frame_free(&aud_frame);
    av_frame_free(&vid_frame);
    av_frame_free(&filt_frame);
    MYTRACE(ACE_TEXT("Quitting FFmpegStreamer thread\n"));
}

int64_t FFmpegStreamer::ProcessAudioBuffer(AVFilterContext* aud_buffersink_ctx,
                                           AVFrame* filt_frame,
                                           AVStream* aud_stream,
                                           ACE_UINT32 start_time,
                                           ACE_UINT32& start_offset)
{
    while (true)
    {
        int ret = av_buffersink_get_frame(aud_buffersink_ctx, filt_frame);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return 0;

        if(ret < 0)
            return -1;

        int64_t frame_tm = filt_frame->best_effort_timestamp;
        double frame_sec = frame_tm * av_q2d(aud_stream->time_base);
        frame_sec = std::max(0., frame_sec);
        ACE_UINT32 frame_timestamp = ACE_UINT32(frame_sec * 1000.0);

        if (!IsSystemTime())
        {
            if (start_offset == MEDIASTREAMER_OFFSET_IGNORE)
                start_offset = frame_timestamp;
            frame_timestamp -= start_offset;
        }

        int n_channels = filt_frame->ch_layout.nb_channels;
        short* audio_data = reinterpret_cast<short*>(filt_frame->data[0]);

        AudioFrame media_frame;
        media_frame.timestamp = frame_timestamp;
        media_frame.input_buffer = audio_data;
        media_frame.input_samples = filt_frame->nb_samples;
        assert(m_media_out.audio.channels == n_channels);
        media_frame.inputfmt = m_media_out.audio;
        QueueAudio(media_frame);

        av_frame_unref(filt_frame);

        return frame_timestamp;
    }
}

int64_t FFmpegStreamer::ProcessVideoBuffer(AVFilterContext* vid_buffersink_ctx,
                                           AVFrame* filt_frame,
                                           AVStream* vid_stream,
                                           ACE_UINT32 start_time,
                                           ACE_UINT32& start_offset)
{
    while(true)
    {
        int ret = av_buffersink_get_frame(vid_buffersink_ctx, filt_frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return 0;
        if (ret < 0)
            return -1;

        int64_t frame_tm = filt_frame->best_effort_timestamp;
        double frame_sec = frame_tm * av_q2d(vid_stream->time_base);
        frame_sec = std::max(0., frame_sec);
        ACE_UINT32 frame_timestamp = ACE_UINT32(frame_sec * 1000.0);

        if (!IsSystemTime())
        {
            if (start_offset == MEDIASTREAMER_OFFSET_IGNORE)
                start_offset = frame_timestamp;
            frame_timestamp -= start_offset;
        }
        else
        {
#if defined(__APPLE__)
            uint32_t monotonicDiffMSec = GETTIMESTAMP() - (mach_absolute_time() / 1000000);
            frame_timestamp += monotonicDiffMSec;
#endif
            if (start_offset == MEDIASTREAMER_OFFSET_IGNORE)
            {
                if (W32_LT(frame_timestamp, start_time))
                {
                    av_frame_unref(filt_frame);
                    continue;
                }
                start_offset = 0;
            }
            frame_timestamp -= start_time;
        }

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
}


AVFilterGraph* createAudioFilterGraph(AVFormatContext *fmt_ctx,
                                      AVCodecContext* aud_dec_ctx,
                                      AVFilterContext*& aud_buffersink_ctx,
                                      AVFilterContext*& aud_buffersrc_ctx,
                                      int audio_stream_index,
                                      int out_channels,
                                      int out_samplerate)
{
    AVFilterGraph *filter_graph = NULL;
    const AVFilter *abuffersrc  = avfilter_get_by_name("abuffer");
    const AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    int ret = 0;

    // Move variable declarations to the top to fix "goto bypasses initialization" error
    char args[512];
    char filter_descr[256];
    char in_ch_layout_str[128] = {0};
    char out_ch_layout_str[128] = {0};

    const enum AVSampleFormat out_sample_fmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
    int out_sample_rates[] = { out_samplerate, -1 };
    int64_t out_channel_layouts[2] = {0, -1}; // For av_opt_set_int_list

    AVRational time_base = fmt_ctx->streams[audio_stream_index]->time_base;
    AVChannelLayout in_ch_layout = aud_dec_ctx->ch_layout;
    AVChannelLayout out_ch_layout;

    av_channel_layout_default(&out_ch_layout, out_channels);
    out_channel_layouts[0] = out_ch_layout.u.mask;

    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto error;
    }

    /* buffer audio source: the decoded frames from the decoder will be inserted here. */
    if (in_ch_layout.order == AV_CHANNEL_ORDER_UNSPEC) {
        av_channel_layout_default(&in_ch_layout, aud_dec_ctx->ch_layout.nb_channels);
    }
    av_channel_layout_describe(&in_ch_layout, in_ch_layout_str, sizeof(in_ch_layout_str));

    snprintf(args, sizeof(args),
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=%s",
             time_base.num, time_base.den, aud_dec_ctx->sample_rate,
             av_get_sample_fmt_name(aud_dec_ctx->sample_fmt), in_ch_layout_str);

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

    // Use av_opt_set_int_list for older FFmpeg compatibility instead of av_opt_set_chlayout_list
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

    av_channel_layout_describe(&out_ch_layout, out_ch_layout_str, sizeof(out_ch_layout_str));
    snprintf(filter_descr, sizeof(filter_descr),
             "aresample=%d,aformat=sample_fmts=s16:channel_layouts=%s",
             out_samplerate, out_ch_layout_str);

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_descr,
                                        &inputs, &outputs, NULL)) < 0)
    {
        MYTRACE(ACE_TEXT("Failed to parse graph\n"));
        goto error;
    }

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto error;

    goto end;

error:
    avfilter_graph_free(&filter_graph);
    filter_graph = NULL;

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    av_channel_layout_uninit(&in_ch_layout);
    av_channel_layout_uninit(&out_ch_layout);

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
    const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVRational time_base = fmt_ctx->streams[video_stream_index]->time_base;
    const enum AVPixelFormat pix_fmts[] = { output_pixfmt, AV_PIX_FMT_NONE };
    char filters_descr[100];

    snprintf(filters_descr, sizeof(filters_descr), "scale=%d:%d",
             vid_dec_ctx->width, vid_dec_ctx->height);

    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto error;
    }


    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             vid_dec_ctx->width, vid_dec_ctx->height, vid_dec_ctx->pix_fmt,
             time_base.num, time_base.den,
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

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
                                    &inputs, &outputs, NULL)) < 0)
        goto error;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto error;

    goto end;

error:
    avfilter_graph_free(&filter_graph);
    filter_graph = NULL;

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return filter_graph;

}
