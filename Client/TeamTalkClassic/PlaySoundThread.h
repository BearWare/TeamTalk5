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

#pragma once

// CPlaySoundThread

class CPlaySoundThread : public CWinThread
{
	DECLARE_DYNCREATE(CPlaySoundThread)

public:
	CPlaySoundThread();           // protected constructor used by dynamic creation
	virtual ~CPlaySoundThread();

	virtual BOOL InitInstance();
	virtual int ExitInstance();

    void AddSoundEvent(LPCTSTR szFilename);
    void KillThread();

protected:
	DECLARE_MESSAGE_MAP()
    
    enum { KILL_EVENT, DATA_SEMAPHORE };
    HANDLE m_handles[2];
    CMutex m_mutex;
    CStringList m_SoundQueue;
public:
    virtual int Run();
};


