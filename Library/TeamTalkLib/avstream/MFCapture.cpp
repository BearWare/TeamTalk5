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
        while(SUCCEEDED(pReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                        dwMediaTypeIndex, &pInputType)))
        {
            media::VideoFormat fmt = ConvertMediaType(pInputType);
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
        ACE_TString deviceid;
        media::VideoFormat vidfmt;
        CComPtr<IMFMediaType> pInputType;

        std::shared_ptr<std::thread> capturethread;
        ACE_Future<bool> opened;
        bool stop = false;

        std::map<media::FourCC, mftransform_t> transforms;
        std::mutex transforms_mtx;

        CaptureSession(const CaptureSession&) = delete;
        CaptureSession(ACE_TString id, const media::VideoFormat& fmt)
            : deviceid(id), vidfmt(fmt) {}

        bool CreateTransform(media::FourCC fcc)
        {
            std::lock_guard<std::mutex> lck(transforms_mtx);
            assert(pInputType.p);
            transforms[fcc] = MFTransform::Create(pInputType, ConvertFourCC(fcc));
            if (transforms[fcc] != nullptr)
                return true;
            transforms.erase(fcc);
            return false;
        }
        void RemoveTransform(media::FourCC fcc)
        {
            std::lock_guard<std::mutex> lck(transforms_mtx);
            transforms.erase(fcc);
        }
    };

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


    {
        std::lock_guard<std::mutex> m(m_mutex);
        m_sessions[listener] = session;
    }


    session->capturethread.reset(new std::thread(&MFCapture::Run, this, session.get(), listener));

    bool ret = false;
    session->opened.get(ret);

    if (!ret)
    {
        session->capturethread->join();

        m_sessions.erase(listener);
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
        session->stop = true;
    }
    
    {
        session->capturethread->join();

        std::lock_guard<std::mutex> m(m_mutex);
        m_sessions.erase(listener);
    }

    return true;
}

media::VideoFormat MFCapture::GetVideoCaptureFormat(VideoCaptureListener* listener)
{
    std::lock_guard<std::mutex> m(m_mutex);
    if(m_sessions.find(listener) != m_sessions.end())
       return m_sessions[listener]->vidfmt;
    return media::VideoFormat();
}

bool MFCapture::RegisterVideoFormat(VideoCaptureListener* listener, media::FourCC fcc, bool enable/* = true*/)
{
    std::lock_guard<std::mutex> m(m_mutex);

    auto is = m_sessions.find(listener);
    if(is == m_sessions.end())
        return false;

    if (enable)
        return is->second->CreateTransform(fcc);
    else
        is->second->RemoveTransform(fcc);
    
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
    DWORD dwMediaTypeIndex = 0, dwVideoStreamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
    CComPtr<IMFAttributes> pAttributes;
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

    hr = pReaderAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
    MYTRACE_COND(FAILED(hr), ACE_TEXT("Failed to enable video processing\n"));

    hr = MFCreateSourceReaderFromMediaSource(pSource, pReaderAttributes, &pReader);
    if(FAILED(hr))
        goto fail;

    // Get native media type of device
    while(SUCCEEDED(pReader->GetNativeMediaType(dwVideoStreamIndex, dwMediaTypeIndex, &pInputType)))
    {
        media::VideoFormat fmt = ConvertMediaType(pInputType);
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
                ACE_Message_Block* mb = VideoFrameToMsgBlock(media_frame);
                if (!listener->OnVideoCaptureCallback(media_frame, mb))
                    mb->release();
            }
            hr = pMediaBuffer->Unlock();
            assert(SUCCEEDED(hr));
        }

        std::lock_guard<std::mutex> lck(session->transforms_mtx);
        for (auto& transform : session->transforms)
        {
            if(dwBufCount && transform.second->SubmitSample(pSample))
            {
                ACE_Message_Block* mb = transform.second->RetrieveMBSample();
                if(mb)
                {
                    media::VideoFrame media_frame(mb);
                    media_frame.timestamp = uTimeStamp;
                    if(!listener->OnVideoCaptureCallback(media_frame, mb))
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
