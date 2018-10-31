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

#include <assert.h>
#include <shlwapi.h> //QITAB
#include <atlbase.h>


static class MFInit {
public:
    MFInit()
    {
        HRESULT hr = MFStartup(MF_VERSION);
        assert(SUCCEEDED(hr));
    }
    ~MFInit()
    {
        //HRESULT hr = MFShutdown();
        //assert(SUCCEEDED(hr));
    }
} init;

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
    InterlockedDecrement(&m_nRefCount);
    assert(m_nRefCount == 0);
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
        m_stream_callbacks.set(STREAMENDED);

        m_thread->join();
        m_thread.reset();
    }
    Reset();

    m_open.cancel();
    m_start.cancel();
    m_stream_callbacks.cancel();
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
    CComPtr<IMFMediaType> pAudioType, pVideoType;
    DWORD dwAudioTypeIndex = 0, dwVideoTypeIndex = 0;
    DWORD dwVideoStreamIndex, dwAudioStreamIndex;
    bool start = false;
    const int BUF_SECS = 3;

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

    // Get the IMFMediaSource interface from the media source.
    hr = pSource->QueryInterface(IID_PPV_ARGS(&pMediaSource));
    if(FAILED(hr))
        goto fail_open;

    hr = MFCreateAttributes(&pAttributes, 2);
    if(FAILED(hr))
        goto fail_open;

    hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);
    if(FAILED(hr))
        goto fail_open;

    hr = MFCreateSourceReaderFromMediaSource(pMediaSource, pAttributes, &pSourceReader);
    if(FAILED(hr))
        goto fail_open;

    // Get native media type of device
    if(SUCCEEDED(pSourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        dwAudioTypeIndex, &pAudioType)))
    {
        UINT32 c = 0;
        if ((c = MFGetAttributeUINT32(pAudioType, MF_MT_AUDIO_NUM_CHANNELS, -1)) >= 0)
            m_media_in.audio_channels = c;
        if((c = MFGetAttributeUINT32(pAudioType, MF_MT_AUDIO_SAMPLES_PER_SECOND, -1)) >= 0)
            m_media_in.audio_samplerate = c;
    }

    if (m_media_in.HasAudio() && m_media_out.audio)
    {
        hr = pAudioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
        if(FAILED(hr))
            goto fail_open;

        hr = pAudioType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, m_media_out.audio_channels);
        if(FAILED(hr))
            goto fail_open;

        hr = pAudioType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, m_media_out.audio_samplerate);
        if(FAILED(hr))
            goto fail_open;

        hr = pAudioType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
        if(FAILED(hr))
            goto fail_open;

        hr = pAudioType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_BLOCK, m_media_out.audio_samples);
        if(FAILED(hr))
            goto fail_open;

        hr = pSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, pAudioType);
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
            m_media_in.video_width = w;
            m_media_in.video_height = h;
        }

        UINT32 numerator = 0, denominator = 0;
        hr = MFGetAttributeRatio(pVideoType, MF_MT_FRAME_RATE, &numerator, &denominator);
        if(SUCCEEDED(hr))
        {
            m_media_in.video_fps_denominator = denominator;
            m_media_in.video_fps_numerator = numerator;
        }
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

    if (m_media_in.HasAudio() && m_media_out.audio)
    {
        dwAudioStreamIndex = MF_SOURCE_READER_FIRST_AUDIO_STREAM;
        hr = pSourceReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                                       0, &dwAudioStreamIndex, NULL, NULL, NULL);
        if (FAILED(hr))
        {
            if (m_listener)
                m_listener->MediaStreamStatusCallback(this, m_media_in, MEDIASTREAM_ERROR);
            return;
        }
    }

    if (m_media_in.HasVideo() && m_media_out.video)
    {
        hr = pSourceReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                       0, &dwVideoStreamIndex, NULL, NULL, NULL);
        if(FAILED(hr))
        {
            if (m_listener)
                m_listener->MediaStreamStatusCallback(this, m_media_in, MEDIASTREAM_ERROR);
            return;
        }
    }

    int timeout_msec = 1000;
    if (m_media_out.audio && m_media_out.audio_samples > 0 && m_media_out.audio_samplerate > 0)
        timeout_msec = m_media_out.audio_samples * 1000 / m_media_out.audio_samplerate;
    if (m_media_out.video)
    {
        double fps = m_media_in.video_fps_numerator / std::max(1, m_media_in.video_fps_numerator);
        timeout_msec = 1000. / fps;
    }

    bool error = false, complete = false;
    DWORD streamid;
    while(!m_stop && !error && !complete && m_stream_callbacks.get(streamid) >= 0)
    {
        //ensure we can reuse semaphore
        m_stream_callbacks.cancel();

        switch (streamid)
        {
        case STREAMERROR :
            error = true;
            break;
        case STREAMENDED :
            complete = true;
            Flush(start_time);
            break;
        default :
            while(!m_stop && !error)
            {
                if(ProcessAVQueues(start_time, 0, false))
                {
                    // keep processing as long as we have enough data
                }
                else
                {
                    if(ProcessAVQueues(start_time, 1, false) == 0)
                    {
                        hr = pSourceReader->ReadSample(streamid, 0, NULL, NULL, NULL, NULL);
                        error = FAILED(hr);
                        break;
                    }
                }
            }
            break;
        }
    }

    assert(m_audio_frames.message_length() == 0);

    if(m_listener && !m_stop)
        m_listener->MediaStreamStatusCallback(this, m_media_in, error? MEDIASTREAM_ERROR : MEDIASTREAM_FINISHED);

    return;

fail_open:
    m_open.set(false);
}

// IMFSourceReaderCallback
STDMETHODIMP MFStreamer::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(MFStreamer, IMFSourceReaderCallback),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) MFStreamer::AddRef() { return InterlockedIncrement(&m_nRefCount); }
STDMETHODIMP_(ULONG) MFStreamer::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    assert(uCount >= 0);
    return uCount;
}

STDMETHODIMP MFStreamer::OnReadSample(HRESULT hrStatus,
    DWORD dwStreamIndex,
    DWORD dwStreamFlags,
    LONGLONG llTimestamp,
    IMFSample *pSample)
{
    HRESULT hr;

    if(pSample)
    {
        DWORD dwBufCount;
        hr = pSample->GetBufferCount(&dwBufCount);
        assert(SUCCEEDED(hr));
        LONGLONG sampletime = 0;
        hr = pSample->GetSampleTime(&sampletime);
        assert(SUCCEEDED(hr));

        sampletime /= 10000;

        for(DWORD i = 0; i<dwBufCount; i++)
        {
            CComPtr<IMFMediaBuffer> pMediaBuffer;
            hr = pSample->GetBufferByIndex(i, &pMediaBuffer);
            assert(SUCCEEDED(hr));
            BYTE* pBuffer = NULL;
            DWORD dwCurLen, dwMaxSize;
            hr = pMediaBuffer->Lock(&pBuffer, &dwMaxSize, &dwCurLen);
            assert(SUCCEEDED(hr));
            if (SUCCEEDED(hr))
            {
                ACE_Message_Block* mb;
                ACE_NEW_NORETURN(mb, ACE_Message_Block(dwCurLen + sizeof(media::AudioFrame)));

                media::AudioFrame media_frame;
                media_frame.timestamp = ACE_UINT32(sampletime);
                media_frame.input_buffer = reinterpret_cast<short*>(mb->wr_ptr() + sizeof(media_frame));
                assert(m_media_out.audio_channels > 0);
                media_frame.input_samples = dwCurLen / sizeof(uint16_t) / m_media_out.audio_channels;
                media_frame.input_channels = m_media_out.audio_channels;
                media_frame.input_samplerate = m_media_out.audio_samplerate;
                int ret = mb->copy(reinterpret_cast<const char*>(&media_frame), sizeof(media_frame));
                assert(ret >= 0);
                ret = mb->copy(reinterpret_cast<const char*>(pBuffer), dwCurLen);
                assert(ret >= 0);
                MYTRACE(ACE_TEXT("Enqueued %u, size %u\n"), media_frame.timestamp, dwCurLen);
                ACE_Time_Value tv;
                if (m_audio_frames.enqueue(mb, &tv) < 0)
                    mb->release();
            }
            hr = pMediaBuffer->Unlock();
            assert(SUCCEEDED(hr));
        }
    }
    if(dwStreamFlags & MF_SOURCE_READERF_NEWSTREAM)
    {
    }

    if(dwStreamFlags & (MF_SOURCE_READERF_ENDOFSTREAM))
    {
        dwStreamIndex = STREAMENDED;
    }

    if (dwStreamFlags & MF_SOURCE_READERF_ERROR)
    {
        dwStreamIndex = STREAMERROR;
    }

    m_stream_callbacks.set(dwStreamIndex);

    return S_OK;
}

STDMETHODIMP MFStreamer::OnEvent(DWORD dwStreamIndex, IMFMediaEvent *pEvent)
{
    return S_OK;
}

STDMETHODIMP MFStreamer::OnFlush(DWORD dwStreamIndex)
{
    return S_OK;
}

