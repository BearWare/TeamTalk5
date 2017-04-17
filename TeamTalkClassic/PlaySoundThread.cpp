// PlaySoundThread.cpp : implementation file
//

#include "stdafx.h"
#include "PlaySoundThread.h"

// CPlaySoundThread

IMPLEMENT_DYNCREATE(CPlaySoundThread, CWinThread)

CPlaySoundThread::CPlaySoundThread()
    : m_handles()
{
}

CPlaySoundThread::~CPlaySoundThread()
{
}

void CPlaySoundThread::KillThread()
{
    ::SetEvent(m_handles[KILL_EVENT]);
}

BOOL CPlaySoundThread::InitInstance()
{
    //semaphore
    m_handles[DATA_SEMAPHORE] = ::CreateSemaphore(NULL, 0, 25, NULL);
    m_handles[KILL_EVENT] = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	return TRUE;
}

int CPlaySoundThread::ExitInstance()
{
    ::CloseHandle(m_handles[KILL_EVENT]);
    ::CloseHandle(m_handles[DATA_SEMAPHORE]);

	return CWinThread::ExitInstance();
}

void CPlaySoundThread::AddSoundEvent(LPCTSTR szFilename)
{
    m_mutex.Lock();
    
    if(szFilename)
    {
        m_SoundQueue.AddTail(szFilename);
    }
    else
    {
        m_SoundQueue.RemoveAll();
        PlayWaveFile(NULL, TRUE);
    }
    
    m_mutex.Unlock();

    ::ReleaseSemaphore(m_handles[DATA_SEMAPHORE], 1, NULL);
}

BEGIN_MESSAGE_MAP(CPlaySoundThread, CWinThread)
END_MESSAGE_MAP()

// CPlaySoundThread message handlers

int CPlaySoundThread::Run()
{
    while(TRUE)
    {
        switch(::WaitForMultipleObjects(sizeof(m_handles)/sizeof(m_handles[0]), m_handles, FALSE, INFINITE))
        {
        case DATA_SEMAPHORE:
        {
            m_mutex.Lock();
            if(m_SoundQueue.GetSize())
                PlayWaveFile(m_SoundQueue.RemoveHead(), FALSE);
            m_mutex.Unlock();
            break;
        }
        case KILL_EVENT:
        default:
            return ExitInstance();
        }
    }
}
