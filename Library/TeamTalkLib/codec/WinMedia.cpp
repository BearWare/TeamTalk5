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

#include "WinMedia.h"

#include <atlbase.h>
#include <DShow.h>
#include <initguid.h>
#include <streams.h>
#include <assert.h>
#include <codec/AudioResampler.h>
#include <codec/MediaUtil.h>
#include <codec/WaveFile.h>

bool GetDSMediaFileProp(const ACE_TString& filename, MediaFileProp& fileprop)
{
    DSWrapperThread ds_thread(NULL);

    MediaStreamOutput output;
    //just put in any audio output format
    output.audio = true;
    output.audio_channels = 2;
    output.audio_samplerate = 44100;
    output.audio_samples = 22050;
    output.video = true;

    if(!ds_thread.OpenFile(filename, output))
        return false;

    fileprop = ds_thread.GetMediaInput();
    fileprop.filename = filename;

    ds_thread.Close();

    return true;
}

HRESULT ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pFirst, IBaseFilter *pSecond);
HRESULT GetNextFilter(IBaseFilter *pFilter, PIN_DIRECTION Dir, IBaseFilter **ppNext);
HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin);

DEFINE_GUID(CLSID_TTVideoFilter,
0x129a7065, 0x314d, 0x49b4, 0x89, 0xbf, 0x8b, 0x82, 0xea, 0x79, 0xe9, 0xe4);

DEFINE_GUID(CLSID_TTAudioFilter,
0x129a7065, 0x314d, 0x49b4, 0x89, 0xbf, 0x8b, 0x82, 0xea, 0x79, 0xe9, 0xe5);

class VideoGrabberCallback : public CBaseRenderer
{
public:
    VideoGrabberCallback(LPUNKNOWN pUnk, HRESULT *phr,
                         const MediaFileProp& media_in,
                         const MediaStreamOutput& media_out,
                         msg_queue_t& video_queue)
    : CBaseRenderer(CLSID_TTVideoFilter, L"TT Video Filter", pUnk, phr)
    , m_media_in(media_in)
    , m_media_out(media_out)
    , m_video_queue(video_queue)
    { }

    HRESULT DoRenderSample(IMediaSample *pMediaSample)
    {
        REFERENCE_TIME tStart, tStop;
        pMediaSample->GetTime(&tStart, &tStop);

        tStart = tStart / 10000; //turn to msec
        double SampleTime = (double)tStart / 1000.0;
        long nBufferLen = pMediaSample->GetActualDataLength();
        PBYTE pBuffer;
        HRESULT hr = pMediaSample->GetPointer(&pBuffer);
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            return hr;

        media::VideoFrame media_frame(reinterpret_cast<char*>(pBuffer),
                                      nBufferLen,
                                      m_media_in.video_width,
                                      m_media_in.video_height,
                                      media::FOURCC_RGB32, false);
        media_frame.timestamp = (ACE_UINT32)(SampleTime * 1000.0);

        ACE_Message_Block* mb = VideoFrameToMsgBlock(media_frame);

        if(m_video_queue.enqueue(mb) < 0)
        {
            mb->release();
            MYTRACE(ACE_TEXT("DSWrapper video buffer closed\n"));
        }

        return S_OK;
    }

    HRESULT CheckMediaType(const CMediaType *mt)
    {
        // Set the media type
        if(mt->formattype == FORMAT_VideoInfo &&
            mt->majortype == MEDIATYPE_Video &&
            mt->subtype == MEDIASUBTYPE_RGB32)
            return S_OK;

        return S_FALSE;
    }

    HRESULT SetMediaType(const CMediaType *pmt)
    {
        VIDEOINFOHEADER *pVih;
        if (pmt->formattype == FORMAT_VideoInfo) 
            pVih = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
        m_media_type = *pmt;
        return S_OK;
    }

    HRESULT GetConnectedMediaType(CMediaType& mt)
    {
        if(m_media_type.IsValid())
        {
            mt = m_media_type;
            return S_OK;
        }
        return S_FALSE;
    }

protected:
    CMediaType m_media_type;
    msg_queue_t& m_video_queue;
    const MediaFileProp& m_media_in;
    const MediaStreamOutput& m_media_out;
};

class AudioGrabberCallback : public CBaseRenderer
{
public:
    AudioGrabberCallback(LPUNKNOWN pUnk, HRESULT *phr,
                         const MediaFileProp& media_in,
                         const MediaStreamOutput& media_out,
                         msg_queue_t& audio_queue,
                         audio_resampler_t& resampler)
    : CBaseRenderer(CLSID_TTAudioFilter, L"My Audio Filter", pUnk, phr)
    , m_media_in(media_in)
    , m_media_out(media_out)
    , m_audio_queue(audio_queue)
    , m_resampler(resampler)
    { }

    HRESULT DoRenderSample(IMediaSample *pMediaSample)
    {
        REFERENCE_TIME tStart, tStop;
        pMediaSample->GetTime(&tStart, &tStop);

        tStart = tStart / 10000; //turn to msec
        double SampleTime = (double)tStart / 1000.0;
        long nBufferLen = pMediaSample->GetActualDataLength();
        PBYTE pBuffer;
        HRESULT hr = pMediaSample->GetPointer(&pBuffer);
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            return hr;

        const short* input_buffer = reinterpret_cast<short*>(pBuffer);
        assert(m_media_out.audio_channels>0);
        int input_samples_total = (nBufferLen / sizeof(short)) / m_media_in.audio_channels;
        
        int output_samples_total = CalcSamples(m_media_in.audio_samplerate,
                                               input_samples_total,
                                               m_media_out.audio_samplerate);

        //Windows' DMO resampler only wants to work with even number of samples
        if(output_samples_total % 2)
            output_samples_total++;
        //MYTRACE(ACE_TEXT("Sample time: %g, buffer: %d\n"), SampleTime, nBufferLen);
        size_t bytes_total = PCM16_BYTES(output_samples_total,
                                         m_media_out.audio_channels);
        ACE_Message_Block* mb;
        ACE_NEW_RETURN(mb, ACE_Message_Block(bytes_total + sizeof(media::AudioFrame)),
                       S_OK);
        
        media::AudioFrame media_frame;
        media_frame.timestamp = (ACE_UINT32)(SampleTime * 1000.0);
        media_frame.input_buffer = reinterpret_cast<short*>(mb->wr_ptr() + sizeof(media_frame));
        media_frame.input_samples = output_samples_total; //overwrite later
        media_frame.input_channels = m_media_out.audio_channels;
        media_frame.input_samplerate = m_media_out.audio_samplerate;

        //static WaveFile ff_org;
        //static WaveFile ff_resam;
        //static int x = 0;
        //if(ff_org.FileName().length() == 0)
        //{
        //    ff_org.NewFile(ACE_TEXT("C:\\temp\\sample_org.wav"), m_media_in.audio_samplerate,
        //                   m_media_in.audio_channels);
        //}
        //if(ff_resam.FileName().length() == 0)
        //{
        //    ff_resam.NewFile(ACE_TEXT("C:\\temp\\sample_resam.wav"), m_media_out.audio_samplerate,
        //        m_media_out.audio_channels);
        //}
        //ff_org.AppendSamples(input_buffer, input_samples_total);

        int output_resampled = m_resampler->Resample(input_buffer, 
                                                     input_samples_total,
                                                     media_frame.input_buffer,
                                                     output_samples_total);

        //MYTRACE_COND(output_resampled != output_samples_total,
        //        ACE_TEXT("Missing %d samples after %d samples\n"), 
        //        output_samples_total - output_resampled,
        //        x + output_resampled);
        //x += output_samples_total;

        //ff_resam.AppendSamples(media_frame.input_buffer, output_resampled);

        size_t resampled_bytes = PCM16_BYTES(output_resampled,
                                             m_media_out.audio_channels);

        media_frame.input_samples = output_resampled;
        mb->copy(reinterpret_cast<const char*>(&media_frame), sizeof(media_frame));
        mb->wr_ptr(resampled_bytes); //advance past resampled output

        //store audio
        if(m_audio_queue.enqueue(mb) < 0)
        {
            MYTRACE(ACE_TEXT("DSWrapper audio buffer closed\n"));
            mb->release();
        }
        return S_OK;
    }

    HRESULT CheckMediaType(const CMediaType *mt)
    {
        if(mt->formattype == FORMAT_WaveFormatEx &&
           mt->majortype == MEDIATYPE_Audio &&
           mt->subtype == MEDIASUBTYPE_PCM)
        {
            WAVEFORMATEX *pwav;
            pwav = reinterpret_cast<WAVEFORMATEX*>(mt->pbFormat);
            return S_OK;
        }
        return S_FALSE;
    }


    HRESULT SetMediaType(const CMediaType *pmt)
    {
        WAVEFORMATEX *pwav;
        if(pmt->formattype == FORMAT_WaveFormatEx)
            pwav = reinterpret_cast<WAVEFORMATEX*>(pmt->pbFormat);
        m_media_type = *pmt;
        return S_OK;
    }

    HRESULT GetConnectedMediaType(CMediaType& mt)
    {
        if(m_media_type.IsValid())
        {
            mt = m_media_type;
            return S_OK;
        }
        return S_FALSE;
    }

protected:
    audio_resampler_t& m_resampler;
    msg_queue_t& m_audio_queue;
    const MediaFileProp& m_media_in;
    const MediaStreamOutput& m_media_out;
    CMediaType m_media_type;
};

DSWrapperThread::DSWrapperThread(MediaStreamListener* listener)
: MediaStreamer(listener)
{
}

DSWrapperThread::~DSWrapperThread()
{
    Close();
    MYTRACE(ACE_TEXT("~DSWrapperThread()\n"));
}

bool DSWrapperThread::OpenFile(const MediaFileProp& in_prop,
                               const MediaStreamOutput& out_prop)
{
    if(this->thr_count())
        return false;

    m_media_in = in_prop;
    m_media_out = out_prop;

    int ret = activate();
    if(ret<0)
        goto fail;

    bool success = false;
    ret = m_open.get(success);

    if(success)
    {
        return true;
    }

fail:
    Reset();
    return false;
}

bool DSWrapperThread::StartStream()
{
    if(m_media_in.filename.length())
    {
        m_start.set(true);
        return true;
    }
    return false;
}

void DSWrapperThread::Close()
{
    m_stop = true;

    m_start.set(false);

    this->wait();

    //reset semaphore
    m_open.cancel();
    m_start.cancel();
    
    Reset();
}

int DSWrapperThread::svc()
{
    HRESULT hr;
    CComPtr<IGraphBuilder> pGraph;
    CComPtr<IBaseFilter> pVideoGrabber, pAudioGrabber;
    CComPtr<IBaseFilter> pSrcFilter;
    CComPtr<IBaseFilter> pTmpFilter;
    CComPtr<IMediaControl> pMediaControl;
    CComPtr<IMediaSeeking> pMediaSeeking;
    CComPtr<IMediaEventEx> pMediaEvent;
    CComPtr<IMediaFilter> pMediaFilter;
    audio_resampler_t resampler;
    int audio_ms = 0, video_ms = 0;
    const int BUF_SECS = 3;

    MSG msg = {0};

    CMediaType mt_vid, mt_vid_con, mt_aud;

    hr = CoInitialize(NULL);
    if(FAILED(hr))
        goto fail_open;

    hr = pGraph.CoCreateInstance(CLSID_FilterGraph);
    if(FAILED(hr))
        goto fail_open;

    // Create the src filter
    hr = pGraph->AddSourceFilter(m_media_in.filename.c_str(), L"Source", &pSrcFilter);
    if(FAILED(hr))
        goto fail_open;


    if(!m_media_out.audio)
        goto no_audio;

    //*** Create audio grabber ***
    AudioGrabberCallback* aud_grab = new AudioGrabberCallback(NULL, &hr,
                                                              m_media_in,
                                                              m_media_out,
                                                              m_audio_frames,
                                                              resampler);
    pAudioGrabber = aud_grab; 
    if(FAILED(hr))
        goto no_audio;

    hr = pGraph->AddFilter(pAudioGrabber, L"Audio SampleGrabber");
    if(FAILED(hr))
        goto no_audio;

    // Connect the src and audio grabber
    hr = ConnectFilters(pGraph, pSrcFilter, pAudioGrabber);
    if(FAILED(hr)) /* VFW_E_CANNOT_CONNECT = 0x80040217*/
        goto no_audio;

    // Retrieve the actual media type for audio
    hr = aud_grab->GetConnectedMediaType(mt_aud);
    if(SUCCEEDED(hr) && *mt_aud.FormatType() == FORMAT_WaveFormatEx)
    {
        WAVEFORMATEX *pwav;
        pwav = reinterpret_cast<WAVEFORMATEX*>(mt_aud.Format());
        
        m_media_in.audio_channels = pwav->nChannels;
        m_media_in.audio_samplerate = pwav->nSamplesPerSec;

        int input_samples = CalcSamples(m_media_out.audio_samplerate,
                                        m_media_out.audio_samples,
                                        pwav->nSamplesPerSec);
        int output_samples = CalcSamples(pwav->nSamplesPerSec,
                                         input_samples,
                                         m_media_out.audio_samplerate);

        //MYTRACE_COND(output_samples != m_media_out.audio_samples,
        //             ACE_TEXT("WinMedia, unexpected samples output: %d, got %d\n"),
        //             m_media_out.audio_samplerate, output_samples);
        resampler = MakeAudioResampler(pwav->nChannels, pwav->nSamplesPerSec,
                                       m_media_out.audio_channels,
                                       m_media_out.audio_samplerate);

        if(m_media_out.audio_samplerate)
            audio_ms = m_media_out.audio_samples * 1000 / m_media_out.audio_samplerate;

        //make audio_queue and audio_time_queue the same size
        size_t buffer_size = PCM16_BYTES(m_media_out.audio_samplerate *
                                         BUF_SECS, m_media_out.audio_channels);
        m_audio_frames.low_water_mark(buffer_size);
        m_audio_frames.high_water_mark(buffer_size);
    }
    else goto no_audio;

    if(resampler.null())
        goto no_audio;

no_audio:

    // *** Create video grabber ***
    VideoGrabberCallback* vid_grab = new VideoGrabberCallback(NULL, &hr,
                                                              m_media_in,
                                                              m_media_out,
                                                              m_video_frames);
    pVideoGrabber = vid_grab;
    if(FAILED(hr))
        goto no_video;

    hr = pGraph->AddFilter(pVideoGrabber, L"Video SampleGrabber");
    if(FAILED(hr))
        goto no_video;

    pTmpFilter = pSrcFilter;
    // Connect the src and video grabber (enumerate filters for a place to connect)
    while((hr = ConnectFilters(pGraph, pTmpFilter, pVideoGrabber)) != S_OK)
    {
        CComPtr<IBaseFilter> pNextFilter;
        if(GetNextFilter(pTmpFilter, PINDIR_OUTPUT, &pNextFilter) != S_OK)
            break; //give up
        pTmpFilter = pNextFilter;
    }
    if(FAILED(hr))
        goto no_video;

    // Retrieve the actual media type for video
    hr = vid_grab->GetConnectedMediaType(mt_vid);

    if (SUCCEEDED(hr) && *mt_vid.FormatType() == FORMAT_VideoInfo) 
    {
        VIDEOINFOHEADER *pVih;
        pVih = reinterpret_cast<VIDEOINFOHEADER*>(mt_vid.Format());
        BITMAPINFOHEADER bmp_hdr;  // info header of frames of the video
        memcpy(&bmp_hdr, &pVih->bmiHeader, sizeof(bmp_hdr));
        video_ms = (int)(pVih->AvgTimePerFrame / 10000);
/* Example of connect/disconnect pins
        CComPtr<IPin> pVideoGrabPin, pVideoSrcPin;
        hr = GetPin(pVideoGrabberFilter, PINDIR_INPUT, &pVideoGrabPin);
        hr = pVideoGrabPin->ConnectedTo(&pVideoSrcPin);
        hr = pVideoSrcPin->Disconnect();
        hr = pVideoGrabPin->Disconnect();
        pVih->bmiHeader.biHeight *= -1;
        hr = pVideoGrabber->SetMediaType(&mt_vid);
        hr = pVideoSrcPin->Connect(pVideoGrabPin, &mt_vid);
*/
        m_media_in.video_width = bmp_hdr.biWidth;
        m_media_in.video_height = bmp_hdr.biHeight;
        if(video_ms > 0 && video_ms <= 1000)//TODO: support lower FPS
            m_media_in.video_fps_numerator = 1000 / video_ms;
        else
            m_media_in.video_fps_numerator = 1;
        m_media_in.video_fps_denominator = 1;

        int bmp_size = bmp_hdr.biWidth * bmp_hdr.biHeight * 4;
        int media_frame_size = bmp_size + sizeof(media::VideoFrame);
        size_t buffer_size = media_frame_size *
                             m_media_in.video_fps_numerator * BUF_SECS;
        m_video_frames.low_water_mark(buffer_size);
        m_video_frames.high_water_mark(buffer_size);
    }
    else
        goto no_video;

    if(FAILED(hr))
        goto no_video;

no_video:

    //remove audio filters from graph if audio failed
    if(m_media_in.audio_channels == 0)
    {
        if(pAudioGrabber.p)
        {
            hr = pGraph->RemoveFilter(pAudioGrabber);
            assert(SUCCEEDED(hr));
            pAudioGrabber.Release();
        }
    }

    //remove video filters from graph if video failed
    if(!m_media_in.video_width)
    {
        if(pVideoGrabber.p)
        {
            hr = pGraph->RemoveFilter(pVideoGrabber);
            assert(SUCCEEDED(hr));
            pVideoGrabber.Release();
        }
    }

    //Media control for playback
    hr = pGraph->QueryInterface(IID_IMediaControl, reinterpret_cast<void**>(&pMediaControl));
    if(FAILED(hr))
        goto fail_open;

    hr = pGraph->QueryInterface(IID_IMediaEvent, reinterpret_cast<void**>(&pMediaEvent));
    if(FAILED(hr))
        goto fail_open;

    hr = pGraph->QueryInterface(IID_IMediaSeeking, reinterpret_cast<void**>(&pMediaSeeking));
    MYTRACE_COND(FAILED(hr), ACE_TEXT("Failed to query IMediaSeeking\n"));
    if(SUCCEEDED(hr))
    {
        hr = pMediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
        assert(SUCCEEDED(hr));
        LONGLONG llTime = 0;
        hr = pMediaSeeking->GetDuration(&llTime);
        MYTRACE_COND(!SUCCEEDED(hr), ACE_TEXT("Failed to query duration of media file: %s\n"),
            m_media_in.filename.c_str());
        m_media_in.duration_ms = (ACE_UINT32)(llTime / 10000);
    }

    // Turn off the sync clock for max speed
    pGraph->QueryInterface(IID_IMediaFilter, reinterpret_cast<void**>(&pMediaFilter));
    pMediaFilter->SetSyncSource(NULL);

    //signal we're ready
    m_open.set(true);

    //wait for go signal
    bool go = true;
    m_start.get(go);
    if(!go)
        goto end;

    assert(m_audio_frames.state() == msg_queue_t::ACTIVATED);
    assert(m_video_frames.state() == msg_queue_t::ACTIVATED);

    if(!m_stop && m_listener)
        m_listener->MediaStreamStatusCallback(this, m_media_in, MEDIASTREAM_STARTED);

    ACE_UINT32 start_time = GETTIMESTAMP();

    //now run when we have the start time
    hr = pMediaControl->Run();
    if(FAILED(hr))
        goto fail_playback;

    ACE_UINT32 audio_time = 0;
    int audio_callbacks = 0;

    long timeoutMs = 1000;
    if(audio_ms)
        timeoutMs = audio_ms;
    if(video_ms && video_ms <= audio_ms)
        timeoutMs = video_ms;

    bool cancel = false;
    while(!m_stop && !cancel)
    {
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            DispatchMessage(&msg);

        LONG ev = 0;
        LONG_PTR p1 = 0, p2 = 0;
        while(pMediaEvent->GetEvent(&ev, &p1, &p2, timeoutMs) == S_OK)
        {
            switch(ev)
            {
            case EC_COMPLETE :
            case EC_USERABORT :
                cancel = true;
                break;
            case EC_ERRORABORT :
                cancel = true;
                pMediaControl->Stop();
                break;
            }
            hr = pMediaEvent->FreeEventParams(ev, p1, p2);
            assert(SUCCEEDED(hr));
        }

        while(!m_stop && ProcessAVQueues(start_time, timeoutMs, false));
    }

    if(m_stop)
    {
        m_audio_frames.close();
        m_video_frames.close();

        hr = pMediaControl->Stop();
        assert(SUCCEEDED(hr));
    }

    while(!m_stop &&
          (m_audio_frames.message_bytes() || m_video_frames.message_bytes()))
        ProcessAVQueues(start_time, timeoutMs, true);

    //probably not nescessary since callbacks should have ended
    m_audio_frames.close();
    m_video_frames.close();

    //check that we have not been stopped from the outside, since a callback
    //would then result in a deadlock
    if(!m_stop && m_listener)
        m_listener->MediaStreamStatusCallback(this, m_media_in, MEDIASTREAM_FINISHED);
    
    goto end;

fail_playback:
    if(!m_stop && m_listener)
        m_listener->MediaStreamStatusCallback(this, m_media_in, MEDIASTREAM_ERROR);

fail_open:
    m_open.set(false);
    
end:
    CoUninitialize();

    return 0;
}

int DSWrapperThread::handle_timeout(const ACE_Time_Value &current_time,
                                    const void *act)
{
    return 0;
}

HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
    IEnumPins  *pEnum;
    IPin       *pPin;
    pFilter->EnumPins(&pEnum);
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
        if (PinDir == PinDirThis)
        {
            pEnum->Release();
            *ppPin = pPin;
            return S_OK;
        }
        pPin->Release();
    }
    pEnum->Release();
    return E_FAIL;  
}

HRESULT ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pFirst, IBaseFilter *pSecond)
{
    IPin *pOut = NULL, *pIn = NULL;
    HRESULT hr = GetPin(pSecond, PINDIR_INPUT, &pIn);
    if (FAILED(hr)) return hr;
    // The previous filter may have multiple outputs, so try each one!
    IEnumPins  *pEnum = NULL;
    pFirst->EnumPins(&pEnum);
    while(pEnum->Next(1, &pOut, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pOut->QueryDirection(&PinDirThis);
        if (PINDIR_OUTPUT == PinDirThis)
        {
            hr = pGraph->Connect(pOut, pIn);
            if(!FAILED(hr))
            {
                break;
            }
        }
        pOut->Release();
        pOut = NULL;
    }
    if(pOut)
        pOut->Release();
    if(pEnum)
        pEnum->Release();
    if(pIn)
        pIn->Release();
    return hr;
}

// Get the first upstream or downstream filter
HRESULT GetNextFilter(
    IBaseFilter *pFilter, // Pointer to the starting filter
    PIN_DIRECTION Dir,    // Direction to search (upstream or downstream)
    IBaseFilter **ppNext) // Receives a pointer to the next filter.
{
    if (!pFilter || !ppNext) return E_POINTER;

    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr)) return hr;
    while (S_OK == pEnum->Next(1, &pPin, 0))
    {
        // See if this pin matches the specified direction.
        PIN_DIRECTION ThisPinDir;
        hr = pPin->QueryDirection(&ThisPinDir);
        if (FAILED(hr))
        {
            // Something strange happened.
            hr = E_UNEXPECTED;
            pPin->Release();
            break;
        }
        if (ThisPinDir == Dir)
        {
            // Check if the pin is connected to another pin.
            IPin *pPinNext = 0;
            hr = pPin->ConnectedTo(&pPinNext);
            if (SUCCEEDED(hr))
            {
                // Get the filter that owns that pin.
                PIN_INFO PinInfo;
                hr = pPinNext->QueryPinInfo(&PinInfo);
                pPinNext->Release();
                pPin->Release();
                pEnum->Release();
                if (FAILED(hr) || (PinInfo.pFilter == NULL))
                {
                    // Something strange happened.
                    return E_UNEXPECTED;
                }
                // This is the filter we're looking for.
                *ppNext = PinInfo.pFilter; // Client must release.
                return S_OK;
            }
        }
        pPin->Release();
    }
    pEnum->Release();
    // Did not find a matching filter.
    return E_FAIL;
}
