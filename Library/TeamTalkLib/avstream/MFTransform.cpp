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

#include "MFTransform.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <atlbase.h>

#include <assert.h>

media::FourCC ConvertSubType(const GUID& native_subtype)
{
    if(native_subtype == MFVideoFormat_RGB32)
    {
        return media::FOURCC_RGB32;
    }
    else if(native_subtype == MFVideoFormat_ARGB32)
    {
        return media::FOURCC_RGB32;
    }
    else if(native_subtype == MFVideoFormat_I420)
    {
        return media::FOURCC_I420;
    }
    else if(native_subtype == MFVideoFormat_YUY2)
    {
        return media::FOURCC_YUY2;
    }
    else if(native_subtype == MFVideoFormat_H264)
    {
        return media::FOURCC_NONE;
    }
    else if(native_subtype == MFVideoFormat_RGB24)
    {
        return media::FOURCC_RGB24;
    }
    else if(native_subtype == MFVideoFormat_NV12)
    {
        return media::FOURCC_NV12;
    }
    else if(native_subtype == MFVideoFormat_MJPG)
    {
        return media::FOURCC_NONE;
    }
    else
    {
        return media::FOURCC_NONE;
    }
}

const GUID& ConvertFourCC(media::FourCC fcc)
{
    switch(fcc)
    {
    case media::FOURCC_RGB32:
        return MFVideoFormat_ARGB32;
    case media::FOURCC_I420:
        return MFVideoFormat_I420;
    case media::FOURCC_YUY2:
        return MFVideoFormat_YUY2;
    case media::FOURCC_RGB24 :
        return MFVideoFormat_RGB24;
    case media::FOURCC_NV12 :
        return MFVideoFormat_NV12;
    default:
        return GUID_NULL;
    }
}

class MFTransformImpl : public MFTransform
{
    IMFActivate** m_pMFTs;
    CComPtr<IMFTransform> m_pMFT;
    UINT32 m_cMFTs = 0;
    DWORD m_dwInputID = 0, m_dwOutputID = 0;
    bool m_ready = false;
    mftransform_t m_endpoint;

public:
    MFTransformImpl(IMFMediaType* pInputType, const GUID& dest_videoformat, bool decoder, MFTransform* endpoint = nullptr)
    {
        m_endpoint.reset(endpoint);
        HRESULT hr;
        MFT_REGISTER_TYPE_INFO tininfo, toutinfo;
        tininfo.guidMajorType = toutinfo.guidMajorType = MFMediaType_Video;
        UINT32 w = 0, h = 0;
        hr = MFGetAttributeSize(pInputType, MF_MT_FRAME_SIZE, &w, &h);
        UINT32 numerator = 0, denominator = 0;
        hr = MFGetAttributeRatio(pInputType, MF_MT_FRAME_RATE, &numerator, &denominator);
        GUID majortype = {0}, native_subtype = { 0 };
        hr = pInputType->GetGUID(MF_MT_MAJOR_TYPE, &majortype);
        assert(majortype == MFMediaType_Video);
        hr = pInputType->GetGUID(MF_MT_SUBTYPE, &native_subtype);
        tininfo.guidSubtype = native_subtype;
        media::FourCC fcc = ConvertSubType(native_subtype);

        toutinfo.guidSubtype = dest_videoformat;

        // https://docs.microsoft.com/en-us/windows/desktop/medfound/video-processor-mft
        hr = MFTEnumEx(decoder? MFT_CATEGORY_VIDEO_DECODER : MFT_CATEGORY_VIDEO_PROCESSOR, 0, &tininfo, &toutinfo, &m_pMFTs, &m_cMFTs);
        if(FAILED(hr) || m_cMFTs == 0)
            return;

        hr = m_pMFTs[0]->ActivateObject(IID_PPV_ARGS(&m_pMFT));
        if(FAILED(hr))
            return;

        hr = m_pMFT->GetStreamIDs(1, &m_dwInputID, 1, &m_dwOutputID);
        if(hr == E_NOTIMPL)
        {
            // The stream identifiers are zero-based.
            m_dwInputID = 0;
            m_dwOutputID = 0;
            hr = S_OK;
        }
        if(FAILED(hr))
            return;

        hr = m_pMFT->SetInputType(m_dwInputID, pInputType, 0);
        if(FAILED(hr))
            return;

        CComPtr<IMFMediaType> pOutputType;
        hr = MFCreateMediaType(&pOutputType);
        if(FAILED(hr))
            return;

        //hr = pInputType->CopyAllItems(pOutputType);
        //if(FAILED(hr))
        //    return;

        hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, toutinfo.guidMajorType);
        if(FAILED(hr))
            return;

        hr = pOutputType->SetGUID(MF_MT_SUBTYPE, toutinfo.guidSubtype);
        if(FAILED(hr))
            return;

        UINT32 size = 0;
        hr = MFCalculateImageSize(toutinfo.guidSubtype, w, h, &size);
        if(FAILED(hr))
            return;

        hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, w, h);
        if(FAILED(hr))
            return;

        if (numerator > 0 && denominator > 0)
        {
            hr = MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE, numerator, denominator);
            if(FAILED(hr))
                return;
        }

        LONG stride = 0;
        hr = MFGetStrideForBitmapInfoHeader(toutinfo.guidSubtype.Data1, w, &stride);
        if(FAILED(hr))
            return;

        hr = pOutputType->SetUINT32(MF_MT_DEFAULT_STRIDE, stride);
        if(FAILED(hr))
            return;

        hr = pOutputType->SetUINT32(MF_MT_SAMPLE_SIZE, size);
        if(FAILED(hr))
            return;

        hr = m_pMFT->SetOutputType(m_dwOutputID, pOutputType, 0);
        if(FAILED(hr))
            return;

        m_ready = true;
    }

    ~MFTransformImpl()
    {
        if(m_cMFTs)
            CoTaskMemFree(m_pMFTs);
    }

    bool Ready() const { return m_ready; }

    bool SubmitSample(CComPtr<IMFSample>& pInSample)
    {
        assert(Ready());

        HRESULT hr;
        hr = m_pMFT->ProcessInput(m_dwInputID, pInSample, 0);

        return SUCCEEDED(hr);
    }

    bool SubmitSample(const media::VideoFrame& frame)
    {
        HRESULT hr;
        CComPtr<IMFSample> pSample;
        CComPtr<IMFMediaBuffer> pMediaBuffer;

        hr = MFCreateSample(&pSample);
        if(FAILED(hr))
            goto fail;

        hr = pSample->SetSampleTime(frame.timestamp * 10000);
        if(FAILED(hr))
            goto fail;

        //hr = pSample->SetSampleDuration((1000 / 30) * 10000);
        //if(FAILED(hr))
        //    goto fail;

        hr = MFCreateMemoryBuffer(frame.frame_length, &pMediaBuffer);
        if(FAILED(hr))
            goto fail;

        BYTE* pBuffer;
        DWORD dwCurLen, dwMaxSize;
        hr = pMediaBuffer->Lock(&pBuffer, &dwMaxSize, &dwCurLen);
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            goto fail;

        memcpy_s(pBuffer, dwMaxSize, frame.frame, frame.frame_length);
        hr = pMediaBuffer->SetCurrentLength(frame.frame_length);
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            goto fail;

        hr = pMediaBuffer->Unlock();
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            goto fail;

        hr = pSample->AddBuffer(pMediaBuffer);
        assert(SUCCEEDED(hr));
        if(FAILED(hr))
            goto fail;

        return SubmitSample(pSample);

    fail:
        return false;
    }

    CComPtr<IMFSample> RetrieveSample()
    {
        assert(Ready());

        HRESULT hr;
        CComPtr<IMFSample> pOutSample;
        MFT_OUTPUT_STREAM_INFO mftStreamInfo = { 0 };
        hr = m_pMFT->GetOutputStreamInfo(m_dwOutputID, &mftStreamInfo);
        if (FAILED(hr))
            return CComPtr<IMFSample>();

        hr = MFCreateSample(&pOutSample);
        if(FAILED(hr))
            return CComPtr<IMFSample>();

        CComPtr<IMFMediaBuffer> pBufferOut;
        hr = MFCreateMemoryBuffer(mftStreamInfo.cbSize, &pBufferOut);
        if(FAILED(hr))
            return CComPtr<IMFSample>();
        hr = pOutSample->AddBuffer(pBufferOut);
        if(FAILED(hr))
            return CComPtr<IMFSample>();

        MFT_OUTPUT_DATA_BUFFER mftOutputData = { 0 };
        mftOutputData.pSample = pOutSample;
        mftOutputData.dwStreamID = m_dwOutputID;

        DWORD dwStatus;
        hr = m_pMFT->ProcessOutput(0, 1, &mftOutputData, &dwStatus);
        if(FAILED(hr))
            return CComPtr<IMFSample>();

        hr = m_pMFT->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, m_dwInputID);
        if(FAILED(hr))
            return CComPtr<IMFSample>();

        if (m_endpoint.get())
        {
            if (!m_endpoint->SubmitSample(pOutSample))
                return CComPtr<IMFSample>();

            pOutSample = m_endpoint->RetrieveSample();
        }

        return pOutSample;
    }

    ACE_Message_Block* RetrieveSample(const media::VideoFormat& fmt)
    {
        HRESULT hr;
        ACE_Message_Block* mb = nullptr;
        CComPtr<IMFSample> pSample = RetrieveSample();
        if (!pSample.p)
            return nullptr;

        DWORD dwBufCount = 0;
        if(pSample)
        {
            hr = pSample->GetBufferCount(&dwBufCount);
            assert(SUCCEEDED(hr));
        }

        for(DWORD i = 0; i<dwBufCount && mb == nullptr; i++)
        {
            CComPtr<IMFMediaBuffer> pMediaBuffer;
            hr = pSample->GetBufferByIndex(i, &pMediaBuffer);
            assert(SUCCEEDED(hr));
            if(FAILED(hr))
                break;

            BYTE* pBuffer = NULL;
            DWORD dwCurLen, dwMaxSize;
            hr = pMediaBuffer->Lock(&pBuffer, &dwMaxSize, &dwCurLen);
            assert(SUCCEEDED(hr));
            if(SUCCEEDED(hr))
            {
                mb = VideoFrameToMsgBlock(media::VideoFrame(fmt, reinterpret_cast<char*>(pBuffer), dwCurLen));
            }
            hr = pMediaBuffer->Unlock();
            assert(SUCCEEDED(hr));
        }
        return mb;
    }

};

mftransform_t MFTransform::Create(IMFMediaType* pInputType, const GUID& dest_videoformat)
{
    std::unique_ptr<MFTransformImpl> result;
    result.reset(new MFTransformImpl(pInputType, dest_videoformat, false));
    if (!result->Ready())
    {
        result.reset();

        // try creating an intermediate transformer
        HRESULT hr;
        MFT_REGISTER_TYPE_INFO tininfo;
        tininfo.guidMajorType = MFMediaType_Video;
        hr = pInputType->GetGUID(MF_MT_SUBTYPE, &tininfo.guidSubtype);
        if (FAILED(hr))
            return result;

        IMFActivate** pMFTs;
        UINT32 cMFTs = 0;
        hr = MFTEnumEx(MFT_CATEGORY_VIDEO_DECODER, 0, &tininfo, NULL, &pMFTs, &cMFTs);

        for (UINT32 i = 0; i < cMFTs; ++i)
        {
            CComPtr<IMFTransform> pMFT;

            hr = pMFTs[0]->ActivateObject(IID_PPV_ARGS(&pMFT));
            if (FAILED(hr))
                continue;
            
            DWORD dwInputID = 0, dwOutputID = 0;
            hr = pMFT->SetInputType(dwInputID, pInputType, 0);
            if(FAILED(hr))
                continue;

            DWORD dwIndex = 0;
            CComPtr<IMFMediaType> pOutputType;
            while(SUCCEEDED(pMFT->GetOutputAvailableType(dwOutputID, dwIndex, &pOutputType)))
            {
                GUID subType;
                hr = pOutputType->GetGUID(MF_MT_SUBTYPE, &subType);
                if (SUCCEEDED(hr))
                {
                    mftransform_t endpoint = MFTransform::Create(pOutputType, dest_videoformat);
                    if(endpoint.get())
                    {
                        result.reset(new MFTransformImpl(pInputType, subType, true, endpoint.release()));
                        if(result->Ready())
                            return result;
                    }
                }
                pOutputType.Release();
                dwIndex++;
            }
        }
    }
    return result;
}

mftransform_t MFTransform::Create(const media::VideoFormat& inputfmt, media::FourCC outputfmt)
{
    std::unique_ptr<MFTransform> result;

    HRESULT hr;
    CComPtr<IMFMediaType> pInputType;
    UINT32 size;
    LONG stride = 0;
    hr = MFCreateMediaType(&pInputType);
    if (FAILED(hr))
        goto fail;

    pInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    GUID subType = ConvertFourCC(inputfmt.fourcc);
    if(subType == GUID_NULL)
        goto fail;
    hr = pInputType->SetGUID(MF_MT_SUBTYPE, subType);
    if(FAILED(hr))
        goto fail;

    hr = MFGetStrideForBitmapInfoHeader(subType.Data1, inputfmt.width, &stride);
    if(FAILED(hr))
        goto fail;

    hr = MFSetAttributeRatio(pInputType, MF_MT_FRAME_RATE, inputfmt.fps_numerator, inputfmt.fps_denominator);
    if(FAILED(hr))
        goto fail;

    hr = MFSetAttributeSize(pInputType, MF_MT_FRAME_SIZE, inputfmt.width, inputfmt.height);
    if(FAILED(hr))
        goto fail;

    //hr = pInputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    //if(FAILED(hr))
    //    goto fail;

    //hr = pInputType->SetUINT32(MF_MT_INTERLACE_MODE, 2);
    //if(FAILED(hr))
    //    goto fail;

    //hr = MFSetAttributeRatio(pInputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    //if(FAILED(hr))
    //    goto fail;

    //hr = MFCalculateImageSize(subType, inputfmt.width, inputfmt.height, &size);
    //if(FAILED(hr))
    //    goto fail;

    //hr = pInputType->SetUINT32(MF_MT_SAMPLE_SIZE, size);
    //if(FAILED(hr))
    //    goto fail;

    result = MFTransform::Create(pInputType, ConvertFourCC(outputfmt));

    fail:
    return result;
}
