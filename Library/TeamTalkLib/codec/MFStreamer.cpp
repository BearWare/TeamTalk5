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

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <shlwapi.h> //QITAB
#include <atlbase.h>
#include <assert.h>

#if 0
bool GetMFMediaFileProp(const ACE_TString& filename, MediaFileProp& fileprop)
{
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
        return false;

    CComPtr<IMFSourceResolver> pSourceResolver;
    MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;
    CComPtr<IUnknown> pSource;
    CComPtr<IMFMediaSource> pMediaSource;
    CComPtr<IMFSourceReader> pSourceReader;
    CComPtr<IMFAttributes> pAttributes;
    CComPtr<IMFMediaType> pType;

    hr = MFCreateSourceResolver(&pSourceResolver);
    if(FAILED(hr))
        return false;

    hr = pSourceResolver->CreateObjectFromURL(
        filename.c_str(), // URL of the source.
        MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
        NULL,                       // Optional property store.
        &ObjectType,        // Receives the created object type. 
        &pSource            // Receives a pointer to the media source.
    );

    // Get the IMFMediaSource interface from the media source.
    hr = pSource->QueryInterface(IID_PPV_ARGS(&pMediaSource));
    if (FAILED(hr))
        return false;

    hr = MFCreateAttributes(&pAttributes, 2);
    if (FAILED(hr))
        return false;

    hr = MFCreateSourceReaderFromMediaSource(pMediaSource, pAttributes, &pSourceReader);
    if (FAILED(hr))
        return false;

    // Get native media type of device
    DWORD dwMediaTypeIndex = 0;
    if (SUCCEEDED(pSourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                  dwMediaTypeIndex, &pType)))
    {
        UINT32 c = 0;
        if (c = MFGetAttributeUINT32(pType, MF_MT_AUDIO_NUM_CHANNELS, -1) >= 0)
            fileprop.audio_channels = c;
        if (c = MFGetAttributeUINT32(pType, MF_MT_AUDIO_SAMPLES_PER_SECOND, -1) >= 0)
            fileprop.audio_samplerate = c;
        fileprop.filename = filename;
    }

    dwMediaTypeIndex = 0;
    if(SUCCEEDED(pSourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                 dwMediaTypeIndex, &pType)))
    {
        UINT32 w = 0, h = 0;
        hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &w, &h);
        if (SUCCEEDED(hr))
        {
            fileprop.video_width = w;
            fileprop.video_height = h;
        }

        UINT32 numerator = 0, denominator = 0;
        hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &numerator, &denominator);
        if (SUCCEEDED(hr))
        {
            fileprop.video_fps_denominator = denominator;
            fileprop.video_fps_numerator = numerator;
        }
        fileprop.filename = filename;
    }
    
    return fileprop.audio_channels>0 || fileprop.video_width>0;
}
#endif

class MyStreamer : public IMFSourceReaderCallback
{
    long m_nRefCount = 1;
    MediaStreamListener* m_listener = nullptr;
    CComPtr<IMFSourceResolver> m_pSourceResolver;
    MF_OBJECT_TYPE m_objectType = MF_OBJECT_INVALID;
    CComPtr<IUnknown> m_pSource;
    CComPtr<IMFMediaSource> m_pMediaSource;
    CComPtr<IMFSourceReader> m_pSourceReader;
    CComPtr<IMFAttributes> m_pAttributes;
    CComPtr<IMFMediaType> m_pAudioType, m_pVideoType;
    MediaFileProp m_fileprop;
    ACE_Future<bool> m_completed;

    void Reset()
    {
        m_fileprop = MediaFileProp();
        m_pAudioType.Release();
        m_pVideoType.Release();
        m_pAttributes.Release();
        m_pSourceReader.Release();
        m_pMediaSource.Release();
        m_pSource.Release();
        m_objectType = MF_OBJECT_INVALID;
        m_pSourceResolver.Release();
    }

public:
    MyStreamer(MediaStreamListener* lsn = nullptr) : m_listener(lsn)
    {
        HRESULT hr = MFStartup(MF_VERSION);
        assert(SUCCEEDED(hr));
    }

    ~MyStreamer()
    {
        HRESULT hr = MFShutdown();
        assert(SUCCEEDED(hr));
    }
    
    bool LoadFile(const ACE_TString& filename)
    {
        HRESULT hr = MFCreateSourceResolver(&m_pSourceResolver);
        if(FAILED(hr))
            return false;

        hr = m_pSourceResolver->CreateObjectFromURL(
            filename.c_str(), // URL of the source.
            MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
            NULL,                       // Optional property store.
            &m_objectType,        // Receives the created object type. 
            &m_pSource            // Receives a pointer to the media source.
        );

        // Get the IMFMediaSource interface from the media source.
        hr = m_pSource->QueryInterface(IID_PPV_ARGS(&m_pMediaSource));
        if(FAILED(hr))
            return false;

        hr = MFCreateAttributes(&m_pAttributes, 2);
        if (FAILED(hr))
            return false;

        hr = m_pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);
        if (FAILED(hr))
            return false;

        hr = MFCreateSourceReaderFromMediaSource(m_pMediaSource, m_pAttributes, &m_pSourceReader);
        if(FAILED(hr))
            return false;

        // Get native media type of device
        DWORD dwMediaTypeIndex = 0;
        if(SUCCEEDED(m_pSourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            dwMediaTypeIndex, &m_pAudioType)))
        {
            UINT32 c = 0;
            if(c = MFGetAttributeUINT32(m_pAudioType, MF_MT_AUDIO_NUM_CHANNELS, -1) >= 0)
                m_fileprop.audio_channels = c;
            if(c = MFGetAttributeUINT32(m_pAudioType, MF_MT_AUDIO_SAMPLES_PER_SECOND, -1) >= 0)
                m_fileprop.audio_samplerate = c;
            m_fileprop.filename = filename;
        }

        dwMediaTypeIndex = 0;
        if(SUCCEEDED(m_pSourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            dwMediaTypeIndex, &m_pVideoType)))
        {
            UINT32 w = 0, h = 0;
            hr = MFGetAttributeSize(m_pVideoType, MF_MT_FRAME_SIZE, &w, &h);
            if(SUCCEEDED(hr))
            {
                m_fileprop.video_width = w;
                m_fileprop.video_height = h;
            }

            UINT32 numerator = 0, denominator = 0;
            hr = MFGetAttributeRatio(m_pVideoType, MF_MT_FRAME_RATE, &numerator, &denominator);
            if(SUCCEEDED(hr))
            {
                m_fileprop.video_fps_denominator = denominator;
                m_fileprop.video_fps_numerator = numerator;
            }
            m_fileprop.filename = filename;
        }

        return m_fileprop.audio_channels>0 || m_fileprop.video_width>0;
    }

    const MediaFileProp& GetMediaFileProp() const
    {
        return m_fileprop;
    }

    bool StartStream(const MediaStreamOutput& out_prop)
    {
        int success = 0;
        if (out_prop.audio && m_pSourceReader)
            success += SUCCEEDED(m_pSourceReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                0, NULL, NULL, NULL, NULL))? 1 : 0;
        if (out_prop.video && m_pSourceReader)
            success += SUCCEEDED(m_pSourceReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                0, NULL, NULL, NULL, NULL))? 1 : 0;
        return success;
    }

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(MyStreamer, IMFSourceReaderCallback),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_nRefCount); }
    STDMETHODIMP_(ULONG) Release()
    {
        ULONG uCount = InterlockedDecrement(&m_nRefCount);
        if(uCount == 0)
            delete this;
        return uCount;
    }
    // IMFSourceReaderCallback methods
    STDMETHODIMP OnReadSample(HRESULT hrStatus,
                              DWORD dwStreamIndex,
                              DWORD dwStreamFlags,
                              LONGLONG llTimestamp,
                              IMFSample *pSample)
    {
        if (dwStreamFlags & MF_SOURCE_READERF_NEWSTREAM)
        {
        }

        if (dwStreamFlags & (MF_SOURCE_READERF_ENDOFSTREAM | MF_SOURCE_READERF_ERROR))
        {
            m_completed.set(true);
        }
        return S_OK;
    }
        
    STDMETHODIMP OnEvent(DWORD dwStreamIndex, IMFMediaEvent *pEvent)
    {
        return S_OK;
    }

    STDMETHODIMP OnFlush(DWORD dwStreamIndex)
    {
        return S_OK;
    }

    void WaitCompleted()
    {
        bool complete;
        m_completed.get(complete);
    }
};

bool GetMFMediaFileProp(const ACE_TString& filename, MediaFileProp& fileprop)
{
    MyStreamer s;
    if (s.LoadFile(filename))
    {
        fileprop = s.GetMediaFileProp();
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
    m_media_in = MediaFileProp();
    m_media_out = MediaStreamOutput();

    if (m_thread.get())
    {
        m_stop = true;
        m_thread->join();
    }
}

bool MFStreamer::StartStream()
{
    m_start.set(true);
    return true;
}

void MFStreamer::Run()
{
    MyStreamer s(m_listener);

    if (!s.LoadFile(m_media_in.filename))
    {
        m_open.set(false);
        return;
    }
    m_open.set(true);

    bool start = false;
    m_start.get(start);

    if (!start)
        return;

    s.StartStream(m_media_out);

    s.WaitCompleted();
}

