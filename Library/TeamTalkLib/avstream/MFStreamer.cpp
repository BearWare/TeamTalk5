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

#include "MFStreamer.h"
#include "MFTransform.h"

#include <assert.h>
#include <shlwapi.h> //QITAB
#include <atlbase.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

bool GetMFMediaFileProp(const ACE_TString& filename, MediaFileProp& fileprop)
{
    MediaFileProp prop;
    prop.filename = filename;
    MFStreamer s(NULL);
    if (s.OpenFile(prop, MediaStreamOutput()))
    {
        fileprop = s.GetMediaInput();
        return true;
    }
    return false;
}

MFStreamer::MFStreamer(MediaStreamListener* listener)
    : MediaStreamer(listener)
{
}

MFStreamer::~MFStreamer()
{
    Close();
}

bool MFStreamer::OpenFile(const MediaFileProp& in_prop,
                          const MediaStreamOutput& out_prop)
{
    Close();

    m_media_in = in_prop;
    m_media_out = out_prop;

    m_thread.reset(new std::thread(&MFStreamer::Run, this));

    bool ret = false;
    m_open.get(ret);
    return ret;
}

void MFStreamer::Close()
{
    if (m_thread.get())
    {
        m_stop = true;
        m_start.set(false);

        m_thread->join();
        m_thread.reset();
    }
    Reset();

    m_open.cancel();
    m_start.cancel();
}

bool MFStreamer::StartStream()
{
    m_start.set(true);
    return true;
}

void MFStreamer::Run()
{
    HRESULT hr;
    CComPtr<IMFSourceResolver> pSourceResolver;
    MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;
    CComPtr<IUnknown> pSource;
    CComPtr<IMFMediaSource> pMediaSource;
    CComPtr<IMFSourceReader> pSourceReader;
    CComPtr<IMFAttributes> pAttributes;
    CComPtr<IMFMediaType> pInputAudioType, pVideoType;
    DWORD dwAudioTypeIndex = 0, dwVideoTypeIndex = 0;
    DWORD dwVideoStreamIndex, dwAudioStreamIndex;
    LONGLONG llAudioTimestamp = 0, llVideoTimestamp = 0;
    bool start = false;
    mftransform_t transform;

    hr = MFCreateSourceResolver(&pSourceResolver);
    if(FAILED(hr))
        goto fail_open;

    hr = pSourceResolver->CreateObjectFromURL(
        m_media_in.filename.c_str(), // URL of the source.
        MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
        NULL,                       // Optional property store.
        &objectType,        // Receives the created object type. 
        &pSource            // Receives a pointer to the media source.
    );

    if(FAILED(hr))
        goto fail_open;

    // Get the IMFMediaSource interface from the media source.
    hr = pSource->QueryInterface(IID_PPV_ARGS(&pMediaSource));
    if(FAILED(hr))
        goto fail_open;

    hr = MFCreateAttributes(&pAttributes, 2);
    if(FAILED(hr))
        goto fail_open;

    hr = MFCreateSourceReaderFromMediaSource(pMediaSource, pAttributes, &pSourceReader);
    if(FAILED(hr))
        goto fail_open;

    // Get native media type of device
    if(SUCCEEDED(pSourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                                                   dwAudioTypeIndex, &pInputAudioType)))
    {
        UINT32 c = 0;
        if ((c = MFGetAttributeUINT32(pInputAudioType, MF_MT_AUDIO_NUM_CHANNELS, -1)) >= 0)
            m_media_in.audio.channels = c;
        if((c = MFGetAttributeUINT32(pInputAudioType, MF_MT_AUDIO_SAMPLES_PER_SECOND, -1)) >= 0)
            m_media_in.audio.samplerate = c;
    }
    else
    {
        m_media_out.audio = media::AudioFormat();
    }

    if (m_media_in.HasAudio() && m_media_out.HasAudio())
    {
        CComPtr<IMFMediaType> pAudioOutputType;
        hr = MFCreateMediaType(&pAudioOutputType);
        if(FAILED(hr))
            goto fail_open;

        hr = pAudioOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        if(FAILED(hr))
            goto fail_open;

        hr = pAudioOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
        if(FAILED(hr))
            goto fail_open;

        hr = pAudioOutputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, m_media_out.audio.channels);
        if(FAILED(hr))
            goto fail_open;

        hr = pAudioOutputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, m_media_out.audio.samplerate);
        if(FAILED(hr))
            goto fail_open;

        hr = pAudioOutputType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, PCM16_BYTES(1, m_media_out.audio.channels));
        if(FAILED(hr))
            goto fail_open;

        hr = pAudioOutputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, PCM16_BYTES(m_media_out.audio.samplerate, m_media_out.audio.channels));
        if(FAILED(hr))
            goto fail_open;

        hr = pAudioOutputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
        if(FAILED(hr))
            goto fail_open;

        hr = pAudioOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
        if(FAILED(hr))
            goto fail_open;

        hr = pAudioOutputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_BLOCK, m_media_out.audio_samples);
        if(FAILED(hr))
            goto fail_open;

        hr = pSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, pAudioOutputType);
        if(FAILED(hr))
            goto fail_open;
    }

    if(SUCCEEDED(pSourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                                   dwVideoTypeIndex, &pVideoType)))
    {
        UINT32 w = 0, h = 0;
        hr = MFGetAttributeSize(pVideoType, MF_MT_FRAME_SIZE, &w, &h);
        if(SUCCEEDED(hr))
        {
            m_media_in.video.width = w;
            m_media_in.video.height = h;
        }

        UINT32 numerator = 0, denominator = 0;
        hr = MFGetAttributeRatio(pVideoType, MF_MT_FRAME_RATE, &numerator, &denominator);
        if(SUCCEEDED(hr))
        {
            m_media_in.video.fps_numerator = numerator;
            m_media_in.video.fps_denominator = denominator;
        }
        else
        {
            m_media_in.video.fps_numerator = 30;
            m_media_in.video.fps_denominator = 1;
            MYTRACE(ACE_TEXT("No frame rate information found in %s\n"), m_media_in.filename.c_str());
        }

        GUID native_subtype = {};
        hr = pVideoType->GetGUID(MF_MT_SUBTYPE, &native_subtype);
        if(FAILED(hr))
            goto fail_open;

        // check whether user wants to transform video stream
        switch (m_media_out.video.fourcc)
        {
        case media::FOURCC_NONE :
            break;
        default :
            GUID oldSubType;
            hr = pVideoType->GetGUID(MF_MT_SUBTYPE, &oldSubType);
            assert(SUCCEEDED(hr));

            hr = pVideoType->SetGUID(MF_MT_SUBTYPE, ConvertFourCC(m_media_out.video.fourcc));
            if(FAILED(hr))
                goto fail_open;

            hr = pSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, pVideoType);
            if(FAILED(hr))
            {
                hr = pVideoType->SetGUID(MF_MT_SUBTYPE, oldSubType);
                assert(SUCCEEDED(hr));

                transform = MFTransform::Create(pVideoType, ConvertFourCC(m_media_out.video.fourcc));
                if (!transform.get())
                    goto fail_open;
            }

            m_media_out.video.width = int(w);
            m_media_out.video.height = int(h);
            m_media_out.video.fps_numerator = int(numerator);
            m_media_out.video.fps_denominator = int(denominator);
            break;
        }
    }
    else
    {
        m_media_out.video = media::VideoFormat();
    }

    if (m_media_in.IsValid())
    {
        m_open.set(m_media_in.IsValid());
    }
    else
    {
        goto fail_open;
    }

    m_start.get(start);

    if (!start)
        return;

    ACE_UINT32 start_time = GETTIMESTAMP();

    if (m_listener && !m_stop)
        m_listener->MediaStreamStatusCallback(this, m_media_in, MEDIASTREAM_STARTED);

    InitBuffers();

    assert(m_audio_frames.state() == msg_queue_t::ACTIVATED);
    assert(m_video_frames.state() == msg_queue_t::ACTIVATED);

    if (m_media_in.HasAudio() && m_media_out.HasAudio())
    {
        dwAudioStreamIndex = MF_SOURCE_READER_FIRST_AUDIO_STREAM;
    }
    else
    {
        llAudioTimestamp = -1;
    }

    if (m_media_in.HasVideo() && m_media_out.HasVideo())
    {
        dwVideoStreamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
    }
    else
    {
        llVideoTimestamp = -1;
    }

    bool error = false;
    while(!m_stop && !error && (llAudioTimestamp >= 0 || llVideoTimestamp >= 0))
    {
        MYTRACE(ACE_TEXT("Sync. Audio %u, Video %u\n"), unsigned(llAudioTimestamp/10000), unsigned(llVideoTimestamp/10000));
        // first process audio
        if (llVideoTimestamp < 0 || (llAudioTimestamp >= 0 && llAudioTimestamp <= llVideoTimestamp))
        {
            CComPtr<IMFSample> pSample;
            DWORD dwStreamFlags = 0;
            hr = pSourceReader->ReadSample(dwAudioStreamIndex, 0, NULL, &dwStreamFlags, &llAudioTimestamp, &pSample);
            error = FAILED(hr);
            
            //if(dwStreamFlags & MF_SOURCE_READERF_NEWSTREAM)
            //{
            //}
            if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
                llAudioTimestamp = -1;
            error |= (dwStreamFlags & MF_SOURCE_READERF_ERROR);

            if(error || llAudioTimestamp < 0)
                continue;

            DWORD dwBufCount = 0;
            if (pSample)
            {
                hr = pSample->GetBufferCount(&dwBufCount);
                assert(SUCCEEDED(hr));
            }

            for(DWORD i = 0; i<dwBufCount; i++)
            {
                CComPtr<IMFMediaBuffer> pMediaBuffer;
                hr = pSample->GetBufferByIndex(i, &pMediaBuffer);
                assert(SUCCEEDED(hr));
                BYTE* pBuffer = NULL;
                DWORD dwCurLen, dwMaxSize;
                hr = pMediaBuffer->Lock(&pBuffer, &dwMaxSize, &dwCurLen);
                assert(SUCCEEDED(hr));
                if(SUCCEEDED(hr))
                {
                    ACE_Message_Block* mb;
                    ACE_NEW_NORETURN(mb, ACE_Message_Block(dwCurLen + sizeof(media::AudioFrame)));

                    media::AudioFrame media_frame;
                    media_frame.timestamp = ACE_UINT32(llAudioTimestamp / 10000);
                    media_frame.input_buffer = reinterpret_cast<short*>(mb->wr_ptr() + sizeof(media_frame));
                    assert(m_media_out.audio.channels > 0);
                    media_frame.input_samples = dwCurLen / sizeof(uint16_t) / m_media_out.audio.channels;
                    media_frame.inputfmt = m_media_out.audio;
                    int ret = mb->copy(reinterpret_cast<const char*>(&media_frame), sizeof(media_frame));
                    assert(ret >= 0);
                    ret = mb->copy(reinterpret_cast<const char*>(pBuffer), dwCurLen);
                    assert(ret >= 0);
                    ACE_Time_Value tv;
                    ret = m_audio_frames.enqueue(mb, &tv);
                    MYTRACE_COND(ret < 0, ACE_TEXT("Skipped audio frame. Buffer full\n"));
                    if(ret < 0)
                    {
                        mb->release();
                    }
                }
                hr = pMediaBuffer->Unlock();
                assert(SUCCEEDED(hr));
            }
        }

        if (llAudioTimestamp < 0 || (llVideoTimestamp >= 0 && llVideoTimestamp <= llAudioTimestamp))
        {
            imfsamples_t samples;
            CComPtr<IMFSample> pSample;
            DWORD dwStreamFlags = 0;
            hr = pSourceReader->ReadSample(dwVideoStreamIndex, 0, NULL, &dwStreamFlags, &llVideoTimestamp, &pSample);
            error = FAILED(hr);

            if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
                llVideoTimestamp = -1;
            error |= (dwStreamFlags & MF_SOURCE_READERF_ERROR);
            assert(!error);
            MYTRACE_COND(error, ACE_TEXT("Error in video stream\n"));
            if(error || llVideoTimestamp < 0)
                continue;

            static int chunks = 0, vframes = 0;
            MYTRACE(ACE_TEXT("Video chunks %d, extracted: %d\n"), ++chunks, vframes);
            
            if(pSample)
            {
                if (transform)
                {
                    samples = transform->ProcessSample(pSample);
                }
                else
                {
                    samples.push_back(pSample);
                }
            }

            for (auto& sample : samples)
            {
                pSample = sample;

                DWORD dwBufCount = 0;
                if(pSample)
                {
                    hr = pSample->GetBufferCount(&dwBufCount);
                    assert(SUCCEEDED(hr));
                }

                for(DWORD i = 0; i<dwBufCount; i++)
                {
                    CComPtr<IMFMediaBuffer> pMediaBuffer;
                    hr = pSample->GetBufferByIndex(i, &pMediaBuffer);
                    LONGLONG llSampleTimeStamp = llVideoTimestamp;
                    hr = pSample->GetSampleTime(&llSampleTimeStamp);

                    assert(SUCCEEDED(hr));
                    BYTE* pBuffer = NULL;
                    DWORD dwCurLen, dwMaxSize;
                    hr = pMediaBuffer->Lock(&pBuffer, &dwMaxSize, &dwCurLen);
                    assert(SUCCEEDED(hr));
                    if(SUCCEEDED(hr))
                    {
                        media::VideoFrame media_frame(m_media_out.video,
                            reinterpret_cast<char*>(pBuffer), dwCurLen);
                        media_frame.timestamp = ACE_UINT32(llSampleTimeStamp / 10000);
                        ACE_Message_Block* mb = VideoFrameToMsgBlock(media_frame);
                        ACE_Time_Value tv;
                        int ret = m_video_frames.enqueue(mb, &tv);
                        MYTRACE_COND(ret < 0, ACE_TEXT("Skipped video frame. Buffer full\n"));
                        if(ret < 0)
                        {
                            mb->release();
                        }
                        vframes++;
                    }
                    hr = pMediaBuffer->Unlock();
                    assert(SUCCEEDED(hr));
                }
            }
        }

        while(!m_stop && !error && ProcessAVQueues(start_time, false));
    }

    while(!m_stop && !error && ProcessAVQueues(start_time, true));

    if (!error && !m_stop)
    {
        assert(m_audio_frames.message_count() == 0);
    }

    assert(m_stop || m_audio_frames.message_length() == 0);

    if(m_listener && !m_stop)
        m_listener->MediaStreamStatusCallback(this, m_media_in, error? MEDIASTREAM_ERROR : MEDIASTREAM_FINISHED);

    return;

fail_open:
    m_open.set(false);
}
