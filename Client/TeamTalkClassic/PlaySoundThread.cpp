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

extern TTInstance* ttInst;

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
    while (m_SoundQueue.size())m_SoundQueue.pop();
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

void CPlaySoundThread::AddSoundEvent(LPCTSTR szFilename, PlaybackMode mode, int sndVol)
{
    m_mutex.Lock();
    
    PlaybackFile pf = {};
    pf.mode = mode;
    pf.szFilename = szFilename;
    pf.sndVol = sndVol;

    m_SoundQueue.push(pf);

    if (m_SoundQueue.size()>5)
    {
        m_SoundQueue.pop();
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
            PlaybackFile pf = {};
            m_mutex.Lock();
            if (m_SoundQueue.size())
            {
                pf = m_SoundQueue.front();
                m_SoundQueue.pop();
            }
            m_mutex.Unlock();
            
            switch (pf.mode)
            {
            case PLAYBACKMODE_SYNC :
                PlayWaveFile(pf.szFilename, FALSE);
                break;
            case PLAYBACKMODE_ASYNC :
                PlayWaveFile(pf.szFilename, TRUE);
                break;
            case PLAYBACKMODE_TEAMTALK :
            {
                MediaFilePlayback mfp = {};
                mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
                mfp.bPaused = FALSE;
                mfp.audioPreprocessor = InitDefaultAudioPreprocessor(TEAMTALK_AUDIOPREPROCESSOR);
                mfp.audioPreprocessor.ttpreprocessor.nGainLevel = RefGain(pf.sndVol);
                auto inst = TT_InitLocalPlayback(ttInst, pf.szFilename, &mfp);
                break;
            }
            }
            break;
        }
        case KILL_EVENT:
        default:
            return ExitInstance();
        }
    }
}
