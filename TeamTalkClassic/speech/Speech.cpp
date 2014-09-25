#include "stdafx.h"
#include "Speech.h"

#include <sapi.h>   //voice speech


CSpeechThread::CSpeechThread()
{
}

CSpeechThread::~CSpeechThread()
{
}

void CSpeechThread::AddSentence(LPCTSTR szSentence)
{
    BOOL bPushed = FALSE;
    m_mutex.Lock();
    if(m_Sentences.size() <= MAX_SENTENCES)
    {
        m_Sentences.push(szSentence);
        bPushed = TRUE;
    }
    m_mutex.Unlock();

    if(bPushed)
    {
        VERIFY(::ReleaseSemaphore(handles[DATA_SEMAPHORE], 1, NULL));
    }
}

void CSpeechThread::KillThread()
{
    ::SetEvent(handles[KILL_EVENT]);
}

int CSpeechThread::Run()
{
    while(TRUE)
    {
        switch(::WaitForMultipleObjects(2, handles, FALSE, INFINITE))
        {
        case DATA_SEMAPHORE :
            {
                m_mutex.Lock();
                CString szSentence = m_Sentences.front();
                m_Sentences.pop();
                m_mutex.Unlock();

                Speak(szSentence);
                break;
            }
        case KILL_EVENT :
            {
                goto exit;
            }
        default:
            {
                ASSERT(0);
                break;
            }
        }
    }

exit:

    return ExitInstance();
}

BOOL CSpeechThread::InitInstance()
{
    //semaphore
    handles[DATA_SEMAPHORE] = ::CreateSemaphore(NULL,0,25,NULL);
    handles[KILL_EVENT] = ::CreateEvent(NULL, TRUE, FALSE, NULL);

    return TRUE;
}

int CSpeechThread::ExitInstance()
{
    ::CloseHandle(handles[KILL_EVENT]);
    ::CloseHandle(handles[DATA_SEMAPHORE]);

    return CWinThread::ExitInstance();
}


void Speak(LPCTSTR szSentence)
{
    ISpVoice * pVoice = NULL;

    if (FAILED(CoInitialize(NULL)))
    {
        AfxMessageBox(_T("Error to intiliaze COM"));
        return;
    }
    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
    if( SUCCEEDED( hr ) )
    {
#if defined(UNICODE) || defined(_UNICODE)
        hr = pVoice->Speak(szSentence, 0, NULL);
#else
        wchar_t buf[512] = {0};
        mbstowcs(buf, szSentence.GetBuffer(), szSentence.GetLength());
        hr = pVoice->Speak(buf, 0, NULL);
#endif
        pVoice->Release();
        pVoice = NULL;
    }

    CoUninitialize();
}
