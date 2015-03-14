#ifdef WIN32
#include <windows.h>
#endif
#include <sys/stat.h>
#include <TeamTalkSrv.h>
#include <iostream>
#include <string>
#include <sstream>
#include <assert.h>
#include <stdlib.h>
using namespace std;

TTSInstance* ttInst;

#ifdef UNICODE
#define cout wcout
#define cin wcin
#define string wstring
#define ostringstream wostringstream
#define istringstream wistringstream
#define _T(x) L ## x
#else
#define _T
#endif

#define DEFAULT_IPADDR      _T("")
#define DEFAULT_TCPPORT     10333
#define DEFAULT_UDPPORT     10333

string get_str(const string& input);
int get_int(int value);


INT32 MyLoginCallback(IN TTSInstance* lpTTSInstance, VOID* lpUserData,
                      IN const User* lpUser, IN OUT UserAccount* lpUserAccount)
{
    // User is trying to log onto our server. Now check username/password.
    cout << "Authenticating username \"" << lpUserAccount->szUsername << "\" "
         << "and password \"" << lpUserAccount->szPassword << "\" ..." << endl;

    lpUserAccount->uUserType = USERTYPE_ADMIN;
    //set user rights
    //lpUserAccount->uUserRights = USERRIGHT_CREATE_TEMPORARY_CHANNEL | USERRIGHT_VIEW_ALL_USERS;
    lpUserAccount->uUserRights = -1;

    lpUserAccount->nUserData = 333; //this could be a database ID
    string note = _T("Here's some message to the user");
    note.copy(lpUserAccount->szNote, TT_STRLEN);

    cout << "User #" << lpUser->nUserID << " username " << lpUserAccount->szUsername << " authenticated." << endl;

    return CMDERR_SUCCESS;
}

void LogUserConnected(IN TTSInstance* lpTTSInstance, IN VOID* lpUserData,
                      IN const User* lpUser)
{
    cout << "Log: User #" << lpUser->nUserID << " IP-Address: " << lpUser->szIPAddress << " connected" << endl;
}

void LogUserDisconnected(IN TTSInstance* lpTTSInstance, IN VOID* lpUserData,
                        IN const User* lpUser)
{
    cout << "Log: User #" << lpUser->nUserID << " IP-Address: " << lpUser->szIPAddress << " disconnected" << endl;
}

void LogUserLoggedInCallback(IN TTSInstance* lpTTSInstance,
                         IN VOID* lpUserData, IN const User* lpUser)
{
    cout << "Log: User #" << lpUser->nUserID << " username: " << lpUser->szUsername << " logged in" << endl;
}

void LogUserJoinedCallback(IN TTSInstance* lpTTSInstance,
                           IN VOID* lpUserData, IN const User* lpUser,
                           IN const Channel* lpChannel)
{
    cout << "Log: User #" << lpUser->nUserID << " username: " << lpUser->szUsername << " joined " << endl;

    Channel chan = {0};

    //TTS_MoveUser(lpTTSInstance, lpUser->nUserID, &chan);
}

int main(int argc, TTCHAR* argv[])
{
    string ipaddr;
    bool encrypted = false;
    int err;

    if(encrypted)
    {
        if(!TTS_SetEncryptionContext(0, _T("ttservercert.pem"), _T("ttserverkey.pem")))
        {
            cout << _T("Failed to set encryption context") << endl;
            return EXIT_FAILURE;
        }
    }

    TTSInstance* ttsInst = TTS_InitTeamTalk(FALSE);

    TTS_RegisterUserLoginCallback(ttsInst, MyLoginCallback, 0, TRUE);
    
    TTS_RegisterUserConnectedCallback(ttsInst, LogUserConnected, 0, TRUE);
    TTS_RegisterUserDisconnectedCallback(ttsInst, LogUserDisconnected, 0, TRUE);

    TTS_RegisterUserLoggedInCallback(ttsInst, LogUserLoggedInCallback, 0, TRUE);
    TTS_RegisterUserJoinedChannelCallback(ttsInst, LogUserJoinedCallback, 0, TRUE);

    ServerProperties srvprop = {0};
    string(_T("TeamTalk 5 Pro Server")).copy(srvprop.szServerName, TT_STRLEN);
    string(_T("This is my message of the day")).copy(srvprop.szMOTDRaw, TT_STRLEN);
    srvprop.nUserTimeout = 60;
    srvprop.nMaxUsers = 100;

    err = TTS_UpdateServer(ttsInst, &srvprop);

    Channel chan = {0};
    chan.nParentID = 0;
    chan.nChannelID = 1;
    chan.nMaxUsers = 100;
    chan.uChannelType = CHANNEL_PERMANENT;
    string(_T("This is the root channel")).copy(chan.szTopic, TT_STRLEN);

    err = TTS_MakeChannel(ttsInst, &chan);

    if(!TTS_StartServer(ttsInst, DEFAULT_IPADDR, DEFAULT_TCPPORT, DEFAULT_UDPPORT, encrypted))
    {
        cout << _T("Failed to start server") << endl;
        return EXIT_FAILURE;
    }

    while(TTS_RunEventLoop(ttsInst, 0))
    {
        cout << "Event" << endl;
    }

    TTS_CloseTeamTalk(ttsInst);
}

string get_str(const string& input)
{
    cout << "(\"" << input << "\") ";
    string tmp;
    std::getline(cin, tmp);
    return tmp.empty()? input : tmp;
}

int get_int(int value)
{
    ostringstream os;
    os << value;
    string sval = get_str(os.str());
    istringstream is(sval);
    is >> value;
    return value;
}
