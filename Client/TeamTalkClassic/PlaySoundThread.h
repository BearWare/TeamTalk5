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


