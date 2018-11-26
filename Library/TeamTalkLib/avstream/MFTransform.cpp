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

media::FourCC ConvertNativeType(const GUID& native_subtype)
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
        return media::FOURCC_NONE;
    }
    else if(native_subtype == MFVideoFormat_NV12)
    {
        return media::FOURCC_NONE;
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

class MFTransformImpl : public MFTransform
{
    IMFActivate** m_pMFTs;
    CComPtr<IMFTransform> m_pMFT;
    UINT32 m_cMFTs = 0;
    DWORD m_dwInputID = 0, m_dwOutputID = 0;
    bool m_ready = false;

public:
    MFTransformImpl(IMFMediaType* pInputType, const GUID& dest_videoformat)
    {
        HRESULT hr;
        MFT_REGISTER_TYPE_INFO tininfo, toutinfo;
        tininfo.guidMajorType = toutinfo.guidMajorType = MFMediaType_Video;
        UINT32 w = 0, h = 0;
        hr = MFGetAttributeSize(pInputType, MF_MT_FRAME_SIZE, &w, &h);
        UINT32 numerator = 0, denominator = 0;
        hr = MFGetAttributeRatio(pInputType, MF_MT_FRAME_RATE, &numerator, &denominator);
        GUID native_subtype = { 0 };
        hr = pInputType->GetGUID(MF_MT_SUBTYPE, &native_subtype);
        tininfo.guidSubtype = native_subtype;

        toutinfo.guidSubtype = dest_videoformat;

        hr = MFTEnumEx(MFT_CATEGORY_VIDEO_PROCESSOR, 0, &tininfo, &toutinfo, &m_pMFTs, &m_cMFTs);
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

        LONG stride = 0;
        UINT32 size = 0;
        hr = MFGetStrideForBitmapInfoHeader(toutinfo.guidSubtype.Data1, w, &stride);
        if(FAILED(hr))
            return;

        hr = MFCalculateImageSize(toutinfo.guidSubtype, w, h, &size);
        if(FAILED(hr))
            return;

        CComPtr<IMFMediaType> pOutputType;
        hr = MFCreateMediaType(&pOutputType);
        if(FAILED(hr))
            return;

        hr = pInputType->CopyAllItems(pOutputType);
        if(FAILED(hr))
            return;

        hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, w, h);
        if(FAILED(hr))
            return;

        hr = pOutputType->SetGUID(MF_MT_SUBTYPE, dest_videoformat);
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

        return pOutSample;
    }

};

std::unique_ptr<MFTransform> MFTransform::Create(IMFMediaType* pInputType, const GUID& dest_videoformat)
{
    std::unique_ptr<MFTransformImpl> result;
    result.reset(new MFTransformImpl(pInputType, dest_videoformat));
    if (!result->Ready())
        result.reset();
    return result;
}
