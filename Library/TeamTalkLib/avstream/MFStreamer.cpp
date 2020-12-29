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
#include <mfidl.h>
#include <mfreadwrite.h>
#include <propvarutil.h>

bool GetMFMediaFileProp(const ACE_TString& filename, MediaFileProp& fileprop)
{
    MFStreamer s;
    if (s.OpenFile(filename, MediaStreamOutput()))
    {
        fileprop = s.GetMediaFile();
        return true;
    }
    return false;
}

MFStreamer::MFStreamer()
{
}

MFStreamer::~MFStreamer()
{
    Close();
}


LONGLONG SeekInStream(CComPtr<IMFSourceReader>& pSourceReader, DWORD dwStreamIndex, 
                      LONGLONG llOffset, const MediaFileProp& prop)
{
    LONGLONG llInitialTimestamp = -1, llTimestamp = 0;

    // advance to the new offset (apparently we have to do this manually)
    while(true)
    {
        CComPtr<IMFSample> pSample;
        DWORD dwStreamFlags = 0;
        HRESULT hr = pSourceReader->ReadSample(dwStreamIndex, 0, NULL, &dwStreamFlags, &llTimestamp, &pSample);
        if (FAILED(hr))
        {
            MYTRACE(ACE_TEXT("Failed to seek to new offset %u in %s\n"), UINT32(llOffset / 10000), prop.filename.c_str());
            llTimestamp = -1;
            break;
        }
        if (dwStreamFlags & (MF_SOURCE_READERF_ENDOFSTREAM | MF_SOURCE_READERF_ERROR))
        {
            MYTRACE(ACE_TEXT("Failed to seek to new offset %u in %s due to EOF or error\n"), UINT32(llOffset / 10000), prop.filename.c_str());
            llTimestamp = -1;
            break;
        }

        if(llInitialTimestamp == -1)
            llInitialTimestamp = llTimestamp;

        // forward
        if (llInitialTimestamp <= llOffset && llTimestamp >= llOffset)
            break;

        //rewind
        if (llInitialTimestamp >= llOffset && llTimestamp <= llOffset)
            break;
    }

    return llTimestamp;
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

    // find duration
    PROPVARIANT var;
    hr = pSourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var);
    if (SUCCEEDED(hr))
    {
        LONGLONG nsDuration = 0;
        hr = PropVariantToInt64(var, &nsDuration);
        PropVariantClear(&var);
        m_media_in.duration_ms = ACE_UINT32(nsDuration / 10000);
    }

    if (m_media_in.IsValid())
    {
        m_open.set(m_media_in.IsValid());
    }
    else
    {
        goto fail_open;
    }

    // wait for semaphore to be notified from ::StartStream()
    m_run.get(start);

    if(!start)
        return;

    InitBuffers(); // requires m_media_out is specified

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

    ACE_UINT32 start_time = GETTIMESTAMP();

    // ensure we don't submit MEDIASTREAM_STARTED twice (also for pause/seek)
    MediaStreamStatus status = MEDIASTREAM_STARTED;

    ACE_UINT32 totalpausetime = 0, offset = 0;
    bool error = false, initialread = true;
    while(!m_stop && !error && (llAudioTimestamp >= 0 || llVideoTimestamp >= 0))
    {
        MYTRACE(ACE_TEXT("Sync. Audio %u, Video %u\n"), unsigned(llAudioTimestamp/10000), unsigned(llVideoTimestamp/10000));

        if (llAudioTimestamp >= 0)
            m_media_in.elapsed_ms = ACE_UINT32(llAudioTimestamp / 10000);
        else if (llVideoTimestamp >= 0)
            m_media_in.elapsed_ms = ACE_UINT32(llVideoTimestamp / 10000);

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
            MYTRACE_COND(pausetime > 0, ACE_TEXT("Pause %s for %u msec\n"), GetMediaFile().filename.c_str(), pausetime);
            totalpausetime += pausetime;
        }

        ACE_UINT32 newoffset = SetOffset(MEDIASTREAMER_OFFSET_IGNORE);

        // check if we should forward/rewind
        if (newoffset == 0 && initialread)
        {
            // There's a weird bug in Windows Media Foundation. If you seek to 0 in a very short audio file (<500 msec)
            // then it actually seeks a few milliseconds into the file.
            MYTRACE(ACE_TEXT("Skipping seek to 0 in %s because already at 0 msec\n"), GetMediaFile().filename.c_str());
        }
        else if (newoffset != MEDIASTREAMER_OFFSET_IGNORE)
        {
            PROPVARIANT var;
            HRESULT hrprop = InitPropVariantFromInt64(newoffset * 10000, &var);
            assert(SUCCEEDED(hrprop));
            hr = pSourceReader->SetCurrentPosition(GUID_NULL, var);
            assert(SUCCEEDED(hr));
            if(SUCCEEDED(hr))
            {
                // advance to the new offset (apparently we have to do this manually)
                if (llAudioTimestamp >= 0)
                    SeekInStream(pSourceReader, dwAudioStreamIndex, LONGLONG(newoffset) * 10000, GetMediaFile());
                if (llVideoTimestamp >= 0)
                    SeekInStream(pSourceReader, dwVideoStreamIndex, LONGLONG(newoffset) * 10000, GetMediaFile());

                m_media_in.elapsed_ms = newoffset;
                start_time = GETTIMESTAMP();
                totalpausetime = 0;
                offset = newoffset;
                ClearBuffers();

                // ensure we don't submit MEDIASTREAM_STARTED twice (also for pause)
                status = MEDIASTREAM_STARTED;
            }

            if(SUCCEEDED(hrprop))
                PropVariantClear(&var);

            MYTRACE(ACE_TEXT("Media file %s starting from %u\n"), GetMediaFile().filename.c_str(), newoffset);
        }

        if (status != MEDIASTREAM_NONE)
        {
            if(m_statuscallback)
                m_statuscallback(m_media_in, status);

            status = MEDIASTREAM_NONE;
        }

        // first process audio
        if (llVideoTimestamp < 0 || (llAudioTimestamp >= 0 && llAudioTimestamp <= llVideoTimestamp))
        {
            CComPtr<IMFSample> pSample;
            DWORD dwStreamFlags = 0, dwActualStreamIndex = 0;
            hr = pSourceReader->ReadSample(dwAudioStreamIndex, 0, &dwActualStreamIndex, &dwStreamFlags, &llAudioTimestamp, &pSample);
            error = FAILED(hr);
            initialread = false;
            
            //if(dwStreamFlags & MF_SOURCE_READERF_NEWSTREAM)
            //{
            //}
            if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
            {
                llAudioTimestamp = -1;
            }
            error |= (dwStreamFlags & MF_SOURCE_READERF_ERROR);

            if(error || llAudioTimestamp < 0)
                continue;

            QueueAudioSample(pSample, llAudioTimestamp);
        }

        // now process video
        if (llAudioTimestamp < 0 || (llVideoTimestamp >= 0 && llVideoTimestamp <= llAudioTimestamp))
        {
            imfsamples_t samples;
            CComPtr<IMFSample> pSample;
            DWORD dwStreamFlags = 0;
            hr = pSourceReader->ReadSample(dwVideoStreamIndex, 0, NULL, &dwStreamFlags, &llVideoTimestamp, &pSample);
            error = FAILED(hr);
            initialread = false;

            if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
            {
                llVideoTimestamp = -1;
            }

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
                vframes += QueueVideoSample(sample, llVideoTimestamp);
            }
        }

        if (m_statuscallback)
            m_statuscallback(this->GetMediaFile(), MEDIASTREAM_PLAYING);

        while(!m_stop && !error && ProcessAVQueues(start_time, GETTIMESTAMP() - totalpausetime + offset, false));
    }

    while(!m_stop && !error && ProcessAVQueues(start_time, GETTIMESTAMP() - totalpausetime + offset, true));

    if (!error && !m_stop)
    {
        assert(GetQueuedAudioDataSize() == 0);
    }

    assert(m_stop || GetQueuedAudioDataSize() == 0);

    if (m_statuscallback && !m_stop)
        m_statuscallback(m_media_in, error? MEDIASTREAM_ERROR : MEDIASTREAM_FINISHED);

    return;

fail_open:
    m_open.set(false);
}

int MFStreamer::QueueAudioSample(CComPtr<IMFSample>& pSample, int64_t sampletime)
{
    HRESULT hr;
    DWORD dwBufCount = 0;
    int aframes = 0;
    if(pSample)
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
            media::AudioFrame media_frame;
            media_frame.timestamp = ACE_UINT32(sampletime / 10000);
            media_frame.input_buffer = reinterpret_cast<short*>(pBuffer);
            assert(m_media_out.audio.channels > 0);
            media_frame.input_samples = dwCurLen / sizeof(uint16_t) / m_media_out.audio.channels;
            media_frame.inputfmt = m_media_out.audio;

            QueueAudio(media_frame);
            aframes++;
        }
        hr = pMediaBuffer->Unlock();
        assert(SUCCEEDED(hr));
    }
    return aframes;
}

int MFStreamer::QueueVideoSample(CComPtr<IMFSample>& pSample, int64_t sampletime)
{
    HRESULT hr;
    DWORD dwBufCount = 0;
    int vframes = 0;
    if(pSample)
    {
        hr = pSample->GetBufferCount(&dwBufCount);
        assert(SUCCEEDED(hr));
    }

    for(DWORD i = 0; i<dwBufCount; i++)
    {
        CComPtr<IMFMediaBuffer> pMediaBuffer;
        hr = pSample->GetBufferByIndex(i, &pMediaBuffer);
        LONGLONG llSampleTimeStamp = sampletime;
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
            QueueVideo(media_frame);
            vframes++;
        }
        hr = pMediaBuffer->Unlock();
        assert(SUCCEEDED(hr));
    }
    return vframes;
}
