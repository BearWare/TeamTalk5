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

// PlaySoundThread.cpp : implementation file
//

#include "stdafx.h"
#include "PlaySoundThread.h"
#include <Mmsystem.h>

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
    m_mutex.Lock();
    m_SoundQueue.RemoveAll();
    m_mutex.Unlock();

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
    
    m_SoundQueue.AddTail(szFilename);

    if(m_SoundQueue.GetCount()>5)
    {
        //Cancel currently played file
        //PlayWaveFile(NULL, TRUE);
        //::PlaySound(NULL, NULL, 0); // doesn't work...

        m_SoundQueue.RemoveHead();
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
            CString szFilename;

            m_mutex.Lock();
            if(m_SoundQueue.GetSize())
                szFilename = m_SoundQueue.RemoveHead();
            m_mutex.Unlock();
            
            if(szFilename.GetLength())
                PlayWaveFile(szFilename, FALSE);
            break;
        }
        case KILL_EVENT:
        default:
            return ExitInstance();
        }
    }
}
