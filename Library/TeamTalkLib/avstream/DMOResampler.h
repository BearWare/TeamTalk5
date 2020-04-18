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

#ifndef DMORESAMPLER_H
#define DMORESAMPLER_H

#include <ace/OS.h>

#include <wmcodecdsp.h>
#include <dmort.h>

#include "AudioResampler.h"

enum SampleFormat
{
    SAMPLEFORMAT_INT16 = 0x00000008, //compatible with PaSampleFormat
    SAMPLEFORMAT_INT24 = 0x00000004,
};

class DMOResampler : public AudioResampler
{
public:
    DMOResampler(const DMOResampler&) = delete;
    DMOResampler(const media::AudioFormat& informat, const media::AudioFormat& outformat,
                 int fixed_input_samples = 0);
    virtual ~DMOResampler();

    bool Init(SampleFormat inputSampleFmt, SampleFormat outputSampleFmt);
    void Close();

    //returns no. of samples in output
    int Resample(const short* input_samples, int input_samples_cnt,
                 short* output_samples, int output_samples_cnt);
private:
    IMediaObject* m_pDMO = nullptr;
    DMO_MEDIA_TYPE m_mt_input, m_mt_output;
};

bool SetWaveMediaType(SampleFormat sampleFmt, int channels, int samplerate, DMO_MEDIA_TYPE& mt);

class CMediaBuffer : public IMediaBuffer
{
private:
    DWORD        m_cbLength;
    const DWORD  m_cbMaxLength;
    LONG         m_cRef;
    BYTE         *m_pbData;
    bool m_free_buf;

    ~CMediaBuffer()
    {
        if(m_free_buf) {
            delete[] m_pbData;
        }
    }

public:
    CMediaBuffer(DWORD cbMaxLength) :
        m_cRef(0),
        m_cbMaxLength(cbMaxLength),
        m_cbLength(0),
        m_pbData(NULL),
        m_free_buf(true)
    {
        m_pbData = new BYTE[cbMaxLength];
        if(!m_pbData) throw std::bad_alloc();
    }

    CMediaBuffer(BYTE* pInitBuf, DWORD cbInitLength, DWORD cbMaxLength) :
        m_cRef(0),
        m_cbMaxLength(cbMaxLength),
        m_cbLength(cbInitLength),
        m_pbData(pInitBuf),
        m_free_buf(false)
    {
    }

    // Function to create a new IMediaBuffer object and return 
    // an AddRef'd interface pointer.
    static HRESULT CreateBuffer(long cbMaxLen, void **ppUnk)
    {
        try {
            CMediaBuffer *pBuffer = new CMediaBuffer(cbMaxLen);
            return pBuffer->QueryInterface(__uuidof(IMediaBuffer), ppUnk);
        }
        catch(std::bad_alloc)
        {
            return E_OUTOFMEMORY;
        }
    }

    static HRESULT CreateBuffer(BYTE* pInitBuf, long cbInitLen, long cbMaxLen, void **ppUnk)
    {
        try {
            CMediaBuffer *pBuffer = new CMediaBuffer(pInitBuf, cbInitLen, cbMaxLen);
            return pBuffer->QueryInterface(__uuidof(IMediaBuffer), ppUnk);
        }
        catch(std::bad_alloc)
        {
            return E_OUTOFMEMORY;
        }
    }

    // IUnknown methods.
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        if(ppv == NULL) {
            return E_POINTER;
        }
        if(riid == __uuidof(IMediaBuffer) || riid == IID_IUnknown) {
            *ppv = static_cast<IMediaBuffer *>(this);
            AddRef();
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG) Release()
    {
        LONG lRef = InterlockedDecrement(&m_cRef);
        if(lRef == 0) {
            delete this;
            // m_cRef is no longer valid! Return lRef.
        }
        return lRef;
    }

    // IMediaBuffer methods.
    STDMETHODIMP SetLength(DWORD cbLength)
    {
        if(cbLength > m_cbMaxLength) {
            return E_INVALIDARG;
        }
        else {
            m_cbLength = cbLength;
            return S_OK;
        }
    }

    STDMETHODIMP GetMaxLength(DWORD *pcbMaxLength)
    {
        if(pcbMaxLength == NULL) {
            return E_POINTER;
        }
        *pcbMaxLength = m_cbMaxLength;
        return S_OK;
    }

    STDMETHODIMP GetBufferAndLength(BYTE **ppbBuffer, DWORD *pcbLength)
    {
        if(ppbBuffer == NULL || pcbLength == NULL) {
            return E_POINTER;
        }
        *ppbBuffer = m_pbData;
        *pcbLength = m_cbLength;
        return S_OK;
    }
};

#endif
