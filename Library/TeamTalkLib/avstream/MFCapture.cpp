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

#include "MFCapture.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <atlbase.h>

#include <assert.h>

using namespace vidcap;

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

MFCapture::~MFCapture()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    while (m_sessions.size())
    {
        auto h = m_sessions.begin();
        m_mutex.unlock();
        StopVideoCapture(h->first);
        m_mutex.lock();
    }
}

vidcap_devices_t MFCapture::GetDevices()
{
    vidcap_devices_t devs;

    HRESULT hr;
    CComPtr<IMFAttributes> pAttributes;
    hr = MFCreateAttributes(&pAttributes, 1);
    assert(SUCCEEDED(hr));
    hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                              MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    assert(SUCCEEDED(hr));

    // Enumerate devices.
    UINT32      cDevices = 0;
    IMFActivate **ppDevices;
    hr = MFEnumDeviceSources(pAttributes, &ppDevices, &cDevices);
    for(UINT i = 0; i<cDevices; i++)
    {
        CComPtr<IMFActivate> pDevice(ppDevices[i]);
        VidCapDevice dev;

        // open source to get capture formats
        CComPtr<IMFMediaSource> pSource;
        hr = pDevice->ActivateObject(IID_PPV_ARGS(&pSource));
        assert(SUCCEEDED(hr));

        CComPtr<IMFAttributes> pReaderAttributes;
        hr = MFCreateAttributes(&pReaderAttributes, 2);
        assert(SUCCEEDED(hr));

        CComPtr<IMFSourceReader> pReader;
        hr = MFCreateSourceReaderFromMediaSource(pSource, pReaderAttributes, &pReader);
        assert(SUCCEEDED(hr));

        // Get native media type of device
        CComPtr<IMFMediaType> pInputType;
        DWORD dwMediaTypeIndex = 0;
        while(SUCCEEDED(pReader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                        dwMediaTypeIndex, &pInputType)))
        {
            media::VideoFormat fmt;

            UINT32 w = 0, h = 0;
            hr = MFGetAttributeSize(pInputType, MF_MT_FRAME_SIZE, &w, &h);
            if (SUCCEEDED(hr))
            {
                fmt.width = w;
                fmt.height = h;
            }

            UINT32 numerator = 0, denominator = 0;
            hr = MFGetAttributeRatio(pInputType, MF_MT_FRAME_RATE, &numerator, &denominator);
            if (SUCCEEDED(hr))
            {
                fmt.fps_numerator = numerator;
                fmt.fps_denominator = denominator;
            }

            GUID native_subtype = { 0 };
            hr = pInputType->GetGUID(MF_MT_SUBTYPE, &native_subtype);
            if (SUCCEEDED(hr))
            {
                fmt.fourcc = ConvertNativeType(native_subtype);
                if(fmt.fourcc != media::FOURCC_NONE)
                {
                    dev.vidcapformats.push_back(fmt);
                }
            }
            dwMediaTypeIndex++;
            pInputType.Release();
        }

        ACE_TCHAR* lpszName;
        if (SUCCEEDED(ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                      &lpszName, NULL)))
        {
            dev.devicename = lpszName;
            dev.api = ACE_TEXT("Microsoft Media Foundation");
            ACE_TCHAR buff[20];
            ACE_OS::snprintf(buff, 20, ACE_TEXT("%d"), i);
            dev.deviceid = buff;
            devs.push_back(dev);
            CoTaskMemFree(lpszName);
        }
    }

    if (cDevices)
        CoTaskMemFree(ppDevices);
    return devs;
}

bool MFCapture::StartVideoCapture(const ACE_TString& deviceid,
                                  const media::VideoFormat& vidfmt,
                                  VideoCaptureListener* listener)
{
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        if (m_sessions.find(listener) != m_sessions.end())
            return false;
    }

    std::shared_ptr<CaptureSession> session(new CaptureSession(deviceid, vidfmt));

    session->capturethread.reset(new std::thread(&MFCapture::Run, this, session.get(), listener));

    bool ret = false;
    session->opened.get(ret);

    if (ret)
    {
        std::lock_guard<std::mutex> m(m_mutex);
        m_sessions[listener] = session;
    }
    else
    {
        session->capturethread->join();
    }
    return ret;
}

bool MFCapture::StopVideoCapture(VideoCaptureListener* listener)
{
    std::shared_ptr<CaptureSession> session;
    {
        std::lock_guard<std::mutex> m(m_mutex);
        if(m_sessions.find(listener) == m_sessions.end())
            return false;

        session = m_sessions[listener];
        m_sessions.erase(listener);
    }
    
    session->stop = true;
    session->capturethread->join();

    return true;
}

bool MFCapture::GetVideoCaptureFormat(VideoCaptureListener* listener,
                                      media::VideoFormat& vidfmt)
{
    std::lock_guard<std::mutex> m(m_mutex);
    if(m_sessions.find(listener) != m_sessions.end())
        return false;
    vidfmt = m_sessions[listener]->vidfmt;
    return true;
}

void MFCapture::Run(CaptureSession* session, VideoCaptureListener* listener)
{
    HRESULT hr;
    UINT32 cDevices = 0;
    IMFActivate **ppDevices;
    CComPtr<IMFActivate> pDevice;
    CComPtr<IMFMediaSource> pSource;
    CComPtr<IMFAttributes> pReaderAttributes;
    CComPtr<IMFSourceReader> pReader;
    CComPtr<IMFMediaType> pInputType;
    DWORD dwMediaTypeIndex = 0, dwVideoStreamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM, dwInputID = 0, dwOutputID = 0;
    CComPtr<IMFAttributes> pAttributes;
    IMFActivate **pMFTs;
    CComPtr<IMFTransform> pMFT;
    UINT32 cMFTs = 0;
    MFT_REGISTER_TYPE_INFO tininfo, toutinfo;
    unsigned devid = ACE_OS::atoi(session->deviceid.c_str());

    hr = MFCreateAttributes(&pAttributes, 1);
    if(FAILED(hr))
        goto fail;

    hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr))
        goto fail;

    // Enumerate devices.
    hr = MFEnumDeviceSources(pAttributes, &ppDevices, &cDevices);
    if (FAILED(hr))
        goto fail;

    if (devid >= cDevices)
        goto fail;
     
    pDevice = ppDevices[devid];

    // open source to get capture formats
    hr = pDevice->ActivateObject(IID_PPV_ARGS(&pSource));
    if(FAILED(hr))
        goto fail;

    hr = MFCreateAttributes(&pReaderAttributes, 2);
    if(FAILED(hr))
        goto fail;

    hr = MFCreateSourceReaderFromMediaSource(pSource, pReaderAttributes, &pReader);
    if(FAILED(hr))
        goto fail;

    // Get native media type of device
    while(SUCCEEDED(pReader->GetNativeMediaType(dwVideoStreamIndex, dwMediaTypeIndex, &pInputType)))
    {
        UINT32 w = 0, h = 0;
        hr = MFGetAttributeSize(pInputType, MF_MT_FRAME_SIZE, &w, &h);
        UINT32 numerator = 0, denominator = 0;
        hr = MFGetAttributeRatio(pInputType, MF_MT_FRAME_RATE, &numerator, &denominator);
        GUID native_subtype = { 0 };
        hr = pInputType->GetGUID(MF_MT_SUBTYPE, &native_subtype);
        if (SUCCEEDED(hr))
        {
            if (w == session->vidfmt.width && h == session->vidfmt.height &&
                session->vidfmt.fps_numerator == numerator && session->vidfmt.fps_denominator == denominator &&
                session->vidfmt.fourcc == ConvertNativeType(native_subtype))
            {
                tininfo.guidMajorType = MFMediaType_Video;
                tininfo.guidSubtype = native_subtype;
                break;
            }
        }
        dwMediaTypeIndex++;
        pInputType.Release();
    }

    if (!pInputType.p)
        goto fail;

    if (session->vidfmt.fourcc != media::FOURCC_RGB32)
    {
        toutinfo.guidMajorType = MFMediaType_Video;
        toutinfo.guidSubtype = MFVideoFormat_ARGB32;

        hr = MFTEnumEx(MFT_CATEGORY_VIDEO_PROCESSOR, 0, &tininfo, &toutinfo, &pMFTs, &cMFTs);
        if (FAILED(hr) || cMFTs == 0)
            goto fail;

        hr = pMFTs[0]->ActivateObject(IID_PPV_ARGS(&pMFT));
        if(FAILED(hr))
            goto fail;

        hr = pMFT->GetStreamIDs(1, &dwInputID, 1, &dwOutputID);
        if(hr == E_NOTIMPL)
        {
            // The stream identifiers are zero-based.
            dwInputID = 0;
            dwOutputID = 0;
            hr = S_OK;
        }
        if(FAILED(hr))
            goto fail;

        hr = pMFT->SetInputType(dwInputID, pInputType, 0);
        if(FAILED(hr))
            goto fail;

        LONG stride = 0;
        UINT32 size = 0;
        hr = MFGetStrideForBitmapInfoHeader(toutinfo.guidSubtype.Data1, session->vidfmt.width, &stride);
        if(FAILED(hr))
            goto fail;
        hr = MFCalculateImageSize(toutinfo.guidSubtype, session->vidfmt.width, session->vidfmt.height, &size);
        if(FAILED(hr))
            goto fail;

        CComPtr<IMFMediaType> pOutputType;
        hr = MFCreateMediaType(&pOutputType);
        if(FAILED(hr))
            goto fail;
        hr = pInputType->CopyAllItems(pOutputType);
        if(FAILED(hr))
            goto fail;
        hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, session->vidfmt.width, session->vidfmt.height);
        if(FAILED(hr))
            goto fail;
        hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
        if(FAILED(hr))
            goto fail;
        hr = pOutputType->SetUINT32(MF_MT_DEFAULT_STRIDE, stride);
        if(FAILED(hr))
            goto fail;
        hr = pOutputType->SetUINT32(MF_MT_SAMPLE_SIZE, size);
        if(FAILED(hr))
            goto fail;
        hr = pMFT->SetOutputType(dwOutputID, pOutputType, 0);
        if(FAILED(hr))
            goto fail;
    }
    else
    {
        toutinfo.guidMajorType = MFMediaType_Video;
        toutinfo.guidSubtype = MFVideoFormat_ARGB32;
    }

    session->opened.set(true);

    bool error = false;
    while (!session->stop)
    {
        CComPtr<IMFSample> pSample;
        DWORD dwStreamFlags = 0;
        LONGLONG llVideoTimestamp = 0;

        hr = pReader->ReadSample(dwVideoStreamIndex, 0, NULL, &dwStreamFlags, &llVideoTimestamp, &pSample);
        if (FAILED(hr))
            break;

        if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
            break;

        if (dwStreamFlags & MF_SOURCE_READERF_ERROR)
            break;

        DWORD dwBufCount = 0;
        if(pSample)
        {
            if (pMFT.p)
            {
                CComPtr<IMFSample> pTransformedSample;

                hr = pMFT->ProcessInput(dwInputID, pSample, 0);
                assert(SUCCEEDED(hr));

                //pSample.Release();

                MFT_OUTPUT_STREAM_INFO mftStreamInfo = { 0 };
                hr = pMFT->GetOutputStreamInfo(dwOutputID, &mftStreamInfo);
                assert(SUCCEEDED(hr));

                hr = MFCreateSample(&pTransformedSample);
                assert(SUCCEEDED(hr));
                CComPtr<IMFMediaBuffer> pBufferOut;
                hr = MFCreateMemoryBuffer(mftStreamInfo.cbSize, &pBufferOut);
                assert(SUCCEEDED(hr));
                hr = pTransformedSample->AddBuffer(pBufferOut);
                assert(SUCCEEDED(hr));

                MFT_OUTPUT_DATA_BUFFER mftOutputData = { 0 };
                mftOutputData.pSample = pTransformedSample;
                mftOutputData.dwStreamID = dwOutputID;

                DWORD dwStatus;
                hr = pMFT->ProcessOutput(0, 1, &mftOutputData, &dwStatus);
                assert(SUCCEEDED(hr));
                hr = pMFT->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, dwInputID);
                assert(SUCCEEDED(hr));

                pSample = pTransformedSample;
            }

            if (pSample.p)
            {
                hr = pSample->GetBufferCount(&dwBufCount);
                assert(SUCCEEDED(hr));
            }
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
                assert(dwCurLen == RGB32_BYTES(session->vidfmt.width, session->vidfmt.height));
                media::VideoFrame media_frame(reinterpret_cast<char*>(pBuffer),
                    dwCurLen, session->vidfmt.width, session->vidfmt.height,
                    ConvertNativeType(toutinfo.guidSubtype), false);
                media_frame.timestamp = ACE_UINT32(llVideoTimestamp / 10000);
                ACE_Message_Block* mb = VideoFrameToMsgBlock(media_frame);
                ACE_Time_Value tv;
                if (!listener->OnVideoCaptureCallback(media_frame, mb))
                    mb->release();
            }
            hr = pMediaBuffer->Unlock();
            assert(SUCCEEDED(hr));
        }
    }

fail:
    if(cDevices)
        CoTaskMemFree(ppDevices);
    if (cMFTs)
        CoTaskMemFree(pMFTs);

    session->opened.set(false);
}
