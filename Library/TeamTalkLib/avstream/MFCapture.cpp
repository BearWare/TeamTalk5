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
#include "MFTransform.h"
#include <codec/BmpFile.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <atlbase.h>

#include <assert.h>

using namespace vidcap;

MFCapture::~MFCapture()
{
    MYTRACE(ACE_TEXT("~MFCapture()\n"));

    StopVideoCapture();
}

vidcap_devices_t MFCapture::GetDevices()
{
    vidcap_devices_t devs;

    HRESULT hr;
    CComPtr<IMFAttributes> pAttributes;
    hr = MFCreateAttributes(&pAttributes, 1);
    if (FAILED(hr))
        return devs;

    hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                              MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr))
        return devs;

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
        if (FAILED(hr))
            continue;

        CComPtr<IMFAttributes> pReaderAttributes;
        hr = MFCreateAttributes(&pReaderAttributes, 2);
        if (FAILED(hr))
            continue;

        CComPtr<IMFSourceReader> pReader;
        hr = MFCreateSourceReaderFromMediaSource(pSource, pReaderAttributes, &pReader);
        if (FAILED(hr))
            continue;

        // Get native media type of device
        CComPtr<IMFMediaType> pInputType;
        DWORD dwMediaTypeIndex = 0;
        while(SUCCEEDED(pReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                        dwMediaTypeIndex, &pInputType)))
        {
            media::VideoFormat fmt = ConvertVideoMediaType(pInputType);
            if(fmt.fourcc != media::FOURCC_NONE)
            {
                dev.vidcapformats.push_back(fmt);
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

namespace vidcap {

    struct CaptureSession
    {
        media::VideoFormat vidfmt;
        CComPtr<IMFMediaType> pInputType;

        std::unique_ptr<std::thread> capturethread;
        ACE_Future<bool> opened, started;
        bool stop = false;

        std::map<media::FourCC, mftransform_t> transforms;

        std::map<media::FourCC, VideoCapture::VideoCaptureCallback> callbacks;

        CaptureSession(const CaptureSession&) = delete;
        CaptureSession(const media::VideoFormat& fmt)
            : vidfmt(fmt)
        {
        }

        ~CaptureSession()
        {
        }

        bool PerformCallback(media::VideoFrame& video_frame, ACE_Message_Block* mb)
        {
            if (callbacks.find(video_frame.fourcc) != callbacks.end())
            {
                auto m = callbacks[video_frame.fourcc];
                return m(video_frame, mb);
            }
            return false;
        }
        bool CreateCallback(media::FourCC fcc, VideoCapture::VideoCaptureCallback callback)
        {
            assert(callbacks.find(fcc) == callbacks.end());
            if(callbacks.find(fcc) != callbacks.end())
                return false;
            
            // don't hold mutex while we create transform since it's a slow process
            if(fcc != vidfmt.fourcc)
            {
                auto transform = MFTransform::Create(pInputType, ConvertFourCC(fcc));
                if (transform)
                {
                    transforms[fcc].swap(transform);
                }
                else return false;
            }

            callbacks[fcc] = callback;
            return true;
        }
        void RemoveCallback(media::FourCC fcc)
        {
            transforms.erase(fcc);
            callbacks.erase(fcc);
        }
    };

}

bool MFCapture::InitVideoCapture(const ACE_TString& deviceid,
                                 const media::VideoFormat& vidfmt)
{
    if(m_session)
        return false;

    m_session.reset(new CaptureSession(vidfmt));

    m_session->capturethread.reset(new std::thread(&MFCapture::Run, this, m_session.get(), deviceid));

    bool ret = false;
    m_session->opened.get(ret);

    if(!ret)
    {
        StopVideoCapture();
    }
    return ret;
}

bool MFCapture::StartVideoCapture()
{
    if(!m_session)
        return false;

    m_session->started.set(true);

    return true;
}

void MFCapture::StopVideoCapture()
{
    if (m_session)
    {
        m_session->stop = true;
        m_session->started.set(false);

        m_session->capturethread->join();
    }
    m_session.reset();
}

media::VideoFormat MFCapture::GetVideoCaptureFormat()
{
    if(!m_session)
        return media::VideoFormat();
    return m_session->vidfmt;
}

bool MFCapture::RegisterVideoFormat(VideoCaptureCallback callback, media::FourCC fcc)
{
    if(!m_session)
        return false;

    return m_session->CreateCallback(fcc, callback);
}

void MFCapture::UnregisterVideoFormat(media::FourCC fcc)
{
    if(m_session)
        m_session->RemoveCallback(fcc);
}

void MFCapture::Run(CaptureSession* session, ACE_TString deviceid)
{
    HRESULT hr;
    UINT32 cDevices = 0;
    IMFActivate **ppDevices;
    CComPtr<IMFActivate> pDevice;
    CComPtr<IMFMediaSource> pSource;
    CComPtr<IMFAttributes> pReaderAttributes;
    CComPtr<IMFSourceReader> pReader;
    CComPtr<IMFMediaType> pInputType;
    DWORD dwMediaTypeIndex = 0, dwVideoStreamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
    CComPtr<IMFAttributes> pAttributes;
    unsigned devid = ACE_OS::atoi(deviceid.c_str());
    bool start = false;

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

    hr = pReaderAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
    MYTRACE_COND(FAILED(hr), ACE_TEXT("Failed to enable video processing\n"));

    hr = MFCreateSourceReaderFromMediaSource(pSource, pReaderAttributes, &pReader);
    if(FAILED(hr))
        goto fail;

    // Get native media type of device
    while(SUCCEEDED(pReader->GetNativeMediaType(dwVideoStreamIndex, dwMediaTypeIndex, &pInputType)))
    {
        media::VideoFormat fmt = ConvertVideoMediaType(pInputType);
        if (session->vidfmt == fmt)
        {
            session->pInputType = pInputType;
            break;
        }

        dwMediaTypeIndex++;
        pInputType.Release();
    }

    if (!pInputType.p)
        goto fail;

    hr = pReader->SetCurrentMediaType(dwVideoStreamIndex, NULL, pInputType);
    if(FAILED(hr))
        goto fail;

    //{
    //    GUID subType;
    //    UINT32 uStride = 0;
    //    LONG lStride = 0;
    //    hr = pInputType->GetUINT32(MF_MT_DEFAULT_STRIDE, &uStride);
    //    lStride = uStride;
    //    
    //    hr = pInputType->GetGUID(MF_MT_SUBTYPE, &subType);
    //    hr = MFGetStrideForBitmapInfoHeader(subType.Data1, session->vidfmt.width, &lStride);
    //    hr = hr;
    //}

    session->opened.set(true);

    session->started.get(start);

    if(!start)
        goto fail;

    bool error = false;
    while (!session->stop)
    {
        CComPtr<IMFSample> pSample;
        DWORD dwStreamFlags = 0, dwActualStreamIndex = 0;
        LONGLONG llVideoTimestamp = 0;
        ACE_UINT32 uTimeStamp;

        hr = pReader->ReadSample(dwVideoStreamIndex, 0, &dwActualStreamIndex, &dwStreamFlags, &llVideoTimestamp, &pSample);
        if (FAILED(hr))
            break;

        if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
            break;

        if (dwStreamFlags & MF_SOURCE_READERF_ERROR)
            break;

        uTimeStamp = ACE_UINT32(llVideoTimestamp / 10000);
        uTimeStamp = GETTIMESTAMP();

        DWORD dwBufCount = 0;
        if(pSample)
        {
            hr = pSample->GetBufferCount(&dwBufCount);
            assert(SUCCEEDED(hr));
        }

        // TODO: don't bother creating this frame if there's no callback to the media format
        for(DWORD i = 0; i<dwBufCount; i++)
        {
            CComPtr<IMFMediaBuffer> pMediaBuffer;
            hr = pSample->GetBufferByIndex(i, &pMediaBuffer);
            assert(SUCCEEDED(hr));
            if (FAILED(hr))
                break;

            BYTE* pBuffer = NULL;
            DWORD dwCurLen, dwMaxSize;
            hr = pMediaBuffer->Lock(&pBuffer, &dwMaxSize, &dwCurLen);
            assert(SUCCEEDED(hr));
            if(SUCCEEDED(hr))
            {
                media::VideoFrame media_frame(session->vidfmt, reinterpret_cast<char*>(pBuffer),
                                              dwCurLen);
                media_frame.timestamp = uTimeStamp;
                session->PerformCallback(media_frame, nullptr);
            }
            hr = pMediaBuffer->Unlock();
            assert(SUCCEEDED(hr));
        }

        for (auto& transform : session->transforms)
        {
            if(dwBufCount)
            {
                auto mbs = transform.second->ProcessMBSample(pSample);
                for (auto& mb : mbs)
                {
                    media::VideoFrame media_frame(mb);
                    assert(media_frame.frame_length);
                    media_frame.timestamp = uTimeStamp;
                    if(!session->PerformCallback(media_frame, mb))
                        mb->release();
                }
            }
        }
    }

fail:
    if(cDevices)
        CoTaskMemFree(ppDevices);

    session->opened.set(false);
}
