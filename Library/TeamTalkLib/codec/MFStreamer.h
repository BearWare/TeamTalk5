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
 
#if !defined(MFSTREAMER_H)
#define MFSTREAMER_H

#include <codec/MediaStreamer.h>

#include <thread>
#include <ace/Future.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

bool GetMFMediaFileProp(const ACE_TString& filename, MediaFileProp& fileprop);

class MFStreamer : public MediaStreamer
                 , public IMFSourceReaderCallback
{
    long m_nRefCount = 1;

public:
    MFStreamer(MediaStreamListener* listener);
    ~MFStreamer();

    bool OpenFile(const MediaFileProp& in_prop,
                  const MediaStreamOutput& out_prop);
    void Close();

    bool StartStream();

    // IMFSourceReaderCallback
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP OnReadSample(HRESULT hrStatus,
                              DWORD dwStreamIndex,
                              DWORD dwStreamFlags,
                              LONGLONG llTimestamp,
                              IMFSample *pSample);
    STDMETHODIMP OnEvent(DWORD dwStreamIndex, IMFMediaEvent *pEvent);
    STDMETHODIMP OnFlush(DWORD dwStreamIndex);

private:
    void Run();

    std::shared_ptr< std::thread > m_thread;
    ACE_Future<bool> m_open, m_start;
    ACE_Future<DWORD> m_stream_callbacks;
    enum : DWORD
    {
        STREAMERROR = 0x8FFFFFFF,
        STREAMENDED = 0x9FFFFFFF,

    };
};
#endif
