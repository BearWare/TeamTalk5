#ifdef WIN32
#include <windows.h>
#endif
#include <sys/stat.h>
#include <TeamTalk.h>
#include <iostream>
#include <string>
#include <sstream>
#include <assert.h>
#include <stdlib.h>
using namespace std;

TTInstance* ttInst;

#ifdef WIN32
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
#define DEFAULT_USERNAME    _T("")
#define DEFAULT_PASSWORD    _T("")

#define DEFAULT_AUDIO_STORAGE _T("")

string audio_dir;

void processTTMessage(const TTMessage& msg);
string get_str(const string& input);
int get_int(int value);

int main(int argc, TTCHAR* argv[])
{
    string ipaddr;
    string username, password;
    int tcpport, udpport;
    bool encrypted = false;
    TTMessage msg;
    int wait_ms, cmd_id;
    struct stat s_file;

    ttInst = TT_InitTeamTalkPoll();

    cout << "TeamTalk 5 server logger." << endl;
    cout << endl << "TeamTalk 5 server login information." << endl;
    cout << "Specify IP-address of server to connect to: ";
    ipaddr = get_str(DEFAULT_IPADDR);
    cout << "Specify TCP port: ";
    tcpport = get_int(DEFAULT_TCPPORT);
    cout << "Specify UDP port: ";
    udpport = get_int(DEFAULT_UDPPORT);
    cout << "Is server encrypted? ";
    encrypted = get_str(_T("n")) == _T("y");
    cout << endl << "Administrator account information." << endl;
    cout << "Specify username: ";
    username = get_str(DEFAULT_USERNAME);
    cout << "Specify password: ";
    password = get_str(DEFAULT_PASSWORD);
    cout << "Specify directory where to store audio: " << endl;
    audio_dir = get_str(DEFAULT_AUDIO_STORAGE);

    int i = stat ( "lame_enc.dll", &s_file );
    if ( i != 0 )
        cout << "lame_enc.dll not found, so audio will be stored to .wav instead of .mp3" << endl;

    //now that we got all the information we needed we can connect and logon
    if(!TT_Connect(ttInst, ipaddr.c_str(), tcpport, udpport, 0, 0, false))
        goto error_connect;
    
    //wait for connect event
    wait_ms = 10000;
    if(!TT_GetMessage(ttInst, &msg, &wait_ms) || 
        msg.nClientEvent == CLIENTEVENT_CON_FAILED)
        goto error_connect;
    assert(msg.nClientEvent == CLIENTEVENT_CON_SUCCESS);
    assert(TT_GetFlags(ttInst) & CLIENT_CONNECTED);
    cout << "Now Connected..." << endl;

    //now that we're connected log on
    cmd_id = TT_DoLogin(ttInst, _T(""), username.c_str(), password.c_str());
    if(cmd_id < 0)
        goto error_login;

    cout << "Login command got cmd ID #" << cmd_id << endl;

    //wait for server reply
    if(!TT_GetMessage(ttInst, &msg, &wait_ms) ||
        msg.nClientEvent != CLIENTEVENT_CMD_PROCESSING)
        goto error_login;
    //get response
    if(!TT_GetMessage(ttInst, &msg, &wait_ms) ||
        msg.nClientEvent == CLIENTEVENT_CMD_ERROR)
        goto error_login;

    //wait for login command to complete
    //client will now post all the server information
    while(TT_GetMessage(ttInst, &msg, &wait_ms) &&
        msg.nClientEvent != CLIENTEVENT_CMD_PROCESSING)
    {
        processTTMessage(msg);
    }
    assert(msg.bActive == FALSE); //command processing complete
    assert(TT_GetFlags(ttInst) & CLIENT_AUTHORIZED); //we're authorized
    //ensure account we used is administrator
    assert(TT_GetMyUserType(ttInst) & USERTYPE_ADMIN);

    //now just process events forever
    while(TT_GetMessage(ttInst, &msg, NULL))
        processTTMessage(msg);

	return EXIT_SUCCESS;
error_connect:
    cout << "Failed to connect to server." << endl;
    return EXIT_FAILURE;
error_login:
    cout << "Failed to log on to server." << endl;
    return EXIT_FAILURE;
}

void processTTMessage(const TTMessage& msg)
{
    switch (msg.nClientEvent)
    {
    case CLIENTEVENT_CMD_MYSELF_LOGGEDIN:
        cout << "Logged in successfully..." << endl;
        cout << "Got user ID #" << msg.user.nUserID << endl;
        break;
    case CLIENTEVENT_CMD_SERVER_UPDATE:
            cout << "Got new server properties:" << endl;
            cout << "Server password: " << msg.serverproperties.szServerName << endl;
            cout << "MOTD: " << msg.serverproperties.szMOTD << endl;
            cout << "Server version: " << msg.serverproperties.szServerVersion << endl;
        break;
    case CLIENTEVENT_CMD_CHANNEL_NEW:
    {
        TTCHAR buf[TT_STRLEN];
        if (!TT_GetChannelPath(ttInst, msg.channel.nChannelID, buf))
            break;
        cout << "Added channel " << buf << endl;
        break;
    }
    case CLIENTEVENT_CMD_CHANNEL_UPDATE:
    {
        TTCHAR buf[TT_STRLEN];
        if (!TT_GetChannelPath(ttInst, msg.channel.nChannelID, buf))
            break;
        cout << "Updated channel " << buf << endl;
        break;
    }
    case CLIENTEVENT_CMD_CHANNEL_REMOVE:
        //cannot retrieve channel since it no longer exists.
        cout << "Removed channel #" << msg.channel.nChannelID << endl;
        break;
    case CLIENTEVENT_CMD_USER_LOGGEDIN:
    {
        cout << "User #" << msg.user.nUserID << " " << msg.user.szNickname << " logged in" << endl;
        cout << "Setting audio storage: " << audio_dir << endl;
        TT_SetUserMediaStorageDir(ttInst, msg.user.nUserID, audio_dir.c_str(), NULL, AFF_WAVE_FORMAT);
        break;
    }
    case CLIENTEVENT_CMD_USER_LOGGEDOUT:
        cout << "User #" << msg.user.nUserID << " " << msg.user.szNickname << " logged out" << endl;
        break;
    case CLIENTEVENT_CMD_USER_UPDATE:
    {
        TTCHAR buf[TT_STRLEN];
        if (TT_GetChannelPath(ttInst, msg.user.nChannelID, buf))
            cout << "User #" << msg.user.nUserID << " " << msg.user.szNickname << " updated " << buf << endl;
        break;
    }
    case CLIENTEVENT_CMD_USER_JOINED:
    {
        TTCHAR buf[TT_STRLEN];
        if (TT_GetChannelPath(ttInst, msg.user.nChannelID, buf))
        {
            cout << "User #" << msg.user.nUserID << " " << msg.user.szNickname << " joined " << buf << endl;
            int cmd_id = TT_DoSubscribe(ttInst, msg.user.nUserID,
                SUBSCRIBE_INTERCEPT_USER_MSG | SUBSCRIBE_INTERCEPT_CHANNEL_MSG |
                SUBSCRIBE_INTERCEPT_VOICE);
            if (cmd_id > 0)
                cout << "Subscribing to text and audio events from #" << msg.user.nUserID << endl;
            else
                cout << "Failed to issue subscribe command" << endl;
        }
        break;
    }
    case CLIENTEVENT_CMD_USER_LEFT:
    {
        TTCHAR buf[TT_STRLEN];
        if (TT_GetChannelPath(ttInst, msg.user.nChannelID, buf))
            cout << "User #" << msg.user.nUserID << " " << msg.user.szNickname << " left " << buf << endl;
        break;
    }
    case CLIENTEVENT_CMD_FILE_NEW:
    {
        TTCHAR buf[TT_STRLEN];
        const RemoteFile& file = msg.remotefile;
        if (TT_GetChannelPath(ttInst, msg.nSource, buf))
            cout << "File #" << file.nFileID << " " << file.szFileName << " added to channel " << buf << endl;
        break;
    }
    case CLIENTEVENT_CMD_FILE_REMOVE:
    {
        TTCHAR buf[TT_STRLEN];
        const RemoteFile& file = msg.remotefile;
        //we can't extract it since it no longer exists
        if (TT_GetChannelPath(ttInst, msg.nSource, buf))
            cout << "File #" << file.nFileID << " removed from channel " << buf << endl;
        break;
    }
    case CLIENTEVENT_CMD_USER_TEXTMSG :
    {
        TTCHAR buf[TT_STRLEN];
        User user;
        const TextMessage& textmsg = msg.textmessage;
        User touser;
        if (!TT_GetUser(ttInst, textmsg.nFromUserID, &user))
            break;
        TT_GetUser(ttInst, textmsg.nToUserID, &touser);
        TT_GetChannelPath(ttInst, textmsg.nChannelID, buf);
        switch(textmsg.nMsgType)
        {
        case MSGTYPE_USER :
            cout << "Text message from user #" << user.nUserID << " ";
            cout << user.szNickname << " to user #" << touser.nUserID << " ";
            cout << touser.szNickname << " ";
            cout << "content: " << textmsg.szMessage << endl;
            break;
        case MSGTYPE_CHANNEL :
            cout << "Text message from user #" << user.nUserID << " ";
            cout << user.szNickname << " to channel " << buf << " ";
            cout << "content: " << textmsg.szMessage << endl;
            break;
        case MSGTYPE_BROADCAST :
            cout << "Text message from user #" << user.nUserID << " ";
            cout << user.szNickname << " to entire server ";
            cout << "content: " << textmsg.szMessage << endl;
            break;
        }
    }
    break;
    case CLIENTEVENT_CMD_PROCESSING :
        if(msg.bActive == TRUE)
            cout << "Started processing cmd ID #" << msg.nSource << endl;
        else
            cout << "Finished processing cmd ID #" << msg.nSource << endl;
        break;
    case CLIENTEVENT_USER_RECORD_MEDIAFILE :
    {
        User user;
        if(TT_GetUser(ttInst, msg.nSource, &user))
        {
            switch(msg.mediafileinfo.nStatus)
            {
            case MFS_ERROR :
                cout << "Failed to store audio file from #" << user.nUserID << " ";
                cout << user.szNickname << endl;
                break;
            case MFS_STARTED :
                cout << "Storing audio from #" << user.nUserID << " ";
                cout << user.szNickname << " to file." << endl;
                break;
            case MFS_FINISHED :
                cout << "Finished storing audio from #" << user.nUserID << " ";
                cout << user.szNickname << " to file." << endl;
                break;
            case MFS_ABORTED :
                cout << "Aborted storing audio from #" << user.nUserID << " ";
                cout << user.szNickname << " to file." << endl;
                break;
            }
        }
        break;
    }
    default :
        cout << "Unknown message #" << msg.nClientEvent << " nSource " << msg.nSource << " ttType " << msg.ttType << endl;
    }
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
