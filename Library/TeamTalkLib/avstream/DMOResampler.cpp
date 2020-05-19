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

#include "DMOResampler.h"
#include <uuids.h>
#include <Mmreg.h>
#include <assert.h>
#include <new>

bool SetWaveMediaType(SampleFormat sampleFmt, int channels,
                      int samplerate, DMO_MEDIA_TYPE& mt)
{
    //TODO: idiot VS2010 gives linker error for 'MEDIATYPE_Audio', 'MEDIASUBTYPE_PCM', 'FORMAT_WaveFormatEx'
    mt.majortype = MEDIATYPE_Audio;
    //CLSIDFromString(OLESTR("{73647561-0000-0010-8000-00AA00389B71}"), &mt.majortype);
    mt.subtype = MEDIASUBTYPE_PCM;
    //CLSIDFromString(OLESTR("{00000001-0000-0010-8000-00AA00389B71}"), &mt.subtype);
    mt.lSampleSize = 0;
    mt.bFixedSizeSamples = TRUE;
    mt.bTemporalCompression = FALSE;
    mt.formattype = FORMAT_WaveFormatEx;
    //CLSIDFromString(OLESTR("{05589f81-c356-11ce-bf01-00aa0055595a}"), &mt.formattype);

    WAVEFORMATEX *pwav = reinterpret_cast<WAVEFORMATEX*>(mt.pbFormat);
    pwav->wFormatTag = WAVE_FORMAT_PCM;
    pwav->nChannels = channels;
    pwav->nSamplesPerSec = samplerate;
    switch(sampleFmt)
    {
    case SAMPLEFORMAT_INT16 :
        pwav->wBitsPerSample = 16;
        break;
    case SAMPLEFORMAT_INT24 :
        pwav->wBitsPerSample = 24;
        break;
    default :
        assert(0);
        return false;
    }
    pwav->nBlockAlign = pwav->nChannels * (pwav->wBitsPerSample / 8);
    pwav->nAvgBytesPerSec = pwav->nSamplesPerSec * pwav->nBlockAlign;

    pwav->cbSize = 0;

    return true;
}

DMOResampler::DMOResampler(const media::AudioFormat& informat, const media::AudioFormat& outformat,
                           int fixed_input_samples)
: AudioResampler(informat, outformat, fixed_input_samples)
, m_mt_input()
, m_mt_output()
{
}

DMOResampler::~DMOResampler()
{
    Close();
}

bool DMOResampler::Init(SampleFormat inputSampleFmt, SampleFormat outputSampleFmt)
{
    if(m_pDMO)
        return false;

    HRESULT hr;
    GUID gResampler = __uuidof(CResamplerMediaObject);
    GUID gMO = __uuidof(IMediaObject);
    static bool init = false;
    if(!init)
    {
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        init = true;
    }

    hr = CoCreateInstance(gResampler, NULL, CLSCTX_INPROC_SERVER, gMO, 
                          (void**)&m_pDMO);
    if(FAILED(hr))
        return false;

    hr = MoInitMediaType(&m_mt_input, sizeof(WAVEFORMATEX));
    if (FAILED(hr))
        goto fail;
    hr = MoInitMediaType(&m_mt_output, sizeof(WAVEFORMATEX));
    if (FAILED(hr))
        goto fail;

    if(!SetWaveMediaType(inputSampleFmt, GetInputFormat().channels, GetInputFormat().samplerate, m_mt_input))
        goto fail;
    if(!SetWaveMediaType(outputSampleFmt, GetOutputFormat().channels, GetOutputFormat().samplerate, m_mt_output))
        goto fail;

    hr = m_pDMO->SetInputType(0, &m_mt_input, 0);
    if (FAILED(hr))
        goto fail;

    hr = m_pDMO->SetOutputType(0, &m_mt_output, 0);
    if (FAILED(hr))
        goto fail;

    return true;

fail:
    Close();
    return false;
}

void DMOResampler::Close()
{
    if(m_mt_input.pbFormat)
    {
        MoFreeMediaType(&m_mt_input);
        ZeroMemory(&m_mt_input, sizeof(m_mt_input));
    }
    if(m_mt_output.pbFormat)
    {
        MoFreeMediaType(&m_mt_output);
        ZeroMemory(&m_mt_output, sizeof(m_mt_output));
    }

    if(m_pDMO)
        m_pDMO->Release();
    m_pDMO = NULL;
}

int DMOResampler::Resample(const short* input_samples, int input_samples_cnt,
                           short* output_samples, int output_samples_cnt)
{
    assert(m_pDMO);
    if(!m_pDMO)
        return 0;

    HRESULT hr;
    BYTE* pBuffer = NULL;
    DWORD dwLen = 0;
    DMO_OUTPUT_DATA_BUFFER dodb = {};
    WAVEFORMATEX *pInputWav = reinterpret_cast<WAVEFORMATEX*>(m_mt_input.pbFormat);
    WAVEFORMATEX *pOutputWav = reinterpret_cast<WAVEFORMATEX*>(m_mt_output.pbFormat);
    IMediaBuffer* input_mb = NULL, *output_mb = NULL;
    int ret = 0;

    long inbufsize = PCM16_BYTES(input_samples_cnt, pInputWav->nChannels);
    hr = CMediaBuffer::CreateBuffer((BYTE*)input_samples, inbufsize, inbufsize, (LPVOID*)&input_mb);
    assert(SUCCEEDED(hr));
    if(FAILED(hr))
        goto fail;

    long outbufsize = PCM16_BYTES(output_samples_cnt, pOutputWav->nChannels);
    hr = CMediaBuffer::CreateBuffer((BYTE*)output_samples, 0, outbufsize, (LPVOID*)&output_mb);
    assert(SUCCEEDED(hr));
    if(FAILED(hr))
        goto fail;

    dodb.pBuffer = output_mb;

    DWORD status = 0;
    hr = m_pDMO->GetInputStatus(0, &status);
    assert(SUCCEEDED(hr));
    //this should not happen but if there's data left in the media object then flush it
    MYTRACE_COND(status == 0, ACE_TEXT("Flushing audio resampler. %d Hz -> %d Hz. Frame size: %d -> %d\n"),
                 GetInputFormat().samplerate, GetOutputFormat().samplerate, input_samples_cnt, output_samples_cnt);
    if(status == 0)
        m_pDMO->Flush();

    hr = m_pDMO->ProcessInput(0, input_mb, 0, 0, 0);
    assert(SUCCEEDED(hr));
    if(FAILED(hr))
        goto fail;

    hr = m_pDMO->ProcessOutput(0, 1, &dodb, &dwLen);
    assert(SUCCEEDED(hr));
    if(FAILED(hr))
        goto fail;

    hr = output_mb->GetBufferAndLength(&pBuffer, &dwLen);
    assert(SUCCEEDED(hr));
    if(FAILED(hr))
        goto fail;

    ret = (dwLen / sizeof(short)) / pOutputWav->nChannels;
    assert(ret <= output_samples_cnt);

    if(ret < output_samples_cnt)
    {
        FillOutput(pOutputWav->nChannels, output_samples, ret, output_samples_cnt);
    }
    MYTRACE_COND(ret != output_samples_cnt,
                 ACE_TEXT("Unexpect number of samples %d!=%d, %d\n"), ret, output_samples_cnt, (int)output_samples[output_samples_cnt-1]);

fail:
    if(input_mb)
        input_mb->Release();
    if(output_mb)
        output_mb->Release();
    return ret;
}
