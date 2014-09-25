#pragma once

#include <queue>

#define MAX_SENTENCES 3

class CSpeechThread : public CWinThread 
{
public:
    CSpeechThread();

    void AddSentence(LPCTSTR szSentence);
    void KillThread();
    int Run();

private:
    ~CSpeechThread();
    CMutex m_mutex;

    std::queue<CString> m_Sentences;
    enum {KILL_EVENT, DATA_SEMAPHORE};
    HANDLE handles[2];

public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
};

void Speak(LPCTSTR szSentence);

