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

#if !defined(COMMANDS_H)
#define COMMANDS_H

#include <vector>
#include <set>
#include <map>

#include <ace/ACE.h>
#include <ace/SString.h>
#include "Common.h"

#define TEAMTALK_PROTOCOL_VERSION ACE_TEXT("5.7")

/* parameter names */
#define TT_USERID ACE_TEXT("userid")
#define TT_NICKNAME ACE_TEXT("nickname")
#define TT_MSGCONTENT ACE_TEXT("content")
#define TT_ERRORMSG ACE_TEXT("message")
#define TT_PARAMNAME ACE_TEXT("param")
#define TT_PASSWORD ACE_TEXT("password")
#define TT_TOPIC ACE_TEXT("topic")
#define TT_OPERATORS ACE_TEXT("operators")
#define TT_STATUSMODE ACE_TEXT("statusmode")
#define TT_STATUSMESSAGE ACE_TEXT("statusmsg")
#define TT_IPADDR ACE_TEXT("ipaddr")
#define TT_TCPPORT ACE_TEXT("tcpport")
#define TT_UDPPORT ACE_TEXT("udpport")
#define TT_VERSION ACE_TEXT("version")
#define TT_SERVERNAME ACE_TEXT("servername")
#define TT_USERRIGHTS ACE_TEXT("userrights")
#define TT_CHANNELID ACE_TEXT("chanid")
#define TT_PARENTID ACE_TEXT("parentid")
#define TT_CHANNAME ACE_TEXT("name")
#define TT_CHANNEL ACE_TEXT("channel")
#define TT_INITCHANNEL ACE_TEXT("initchan")
#define TT_REQPASSWORD ACE_TEXT("protected")
#define TT_ERRORNUM ACE_TEXT("number")
#define TT_PROTOCOL ACE_TEXT("protocol")
#define TT_SRCUSERID ACE_TEXT("srcuserid")
#define TT_DESTUSERID ACE_TEXT("destuserid")
#define TT_KICKERID ACE_TEXT("kickerid")
#define TT_MSGTYPE ACE_TEXT("type")
#define TT_USERNAME ACE_TEXT("username")
#define TT_FILENAME ACE_TEXT("filename")
#define TT_FILESIZE ACE_TEXT("filesize")
#define TT_FILEDATE ACE_TEXT("filedate")
#define TT_FILEID ACE_TEXT("fileid")
#define TT_FILEOWNER ACE_TEXT("owner")
#define TT_TRANSFERID ACE_TEXT("transferid")
#define TT_PROTOCOL ACE_TEXT("protocol")
#define TT_PACKETPROTOCOL ACE_TEXT("packetprotocol")
#define TT_MOTD ACE_TEXT("motd")
#define TT_BANTIME ACE_TEXT("bantime")
#define TT_CRYPTKEY ACE_TEXT("cryptkey")
#define TT_TRANSFERID ACE_TEXT("transferid")
#define TT_DISKQUOTA ACE_TEXT("diskquota")
#define TT_FILESROOT ACE_TEXT("filesroot")
#define TT_MAXUSERS ACE_TEXT("maxusers")
#define TT_AUTOSAVE ACE_TEXT("autosave")
#define TT_MAXDISKUSAGE ACE_TEXT("maxdiskusage")
#define TT_OPPASSWORD ACE_TEXT("oppassword")
#define TT_OPERATORSTATUS ACE_TEXT("opstatus")
#define TT_LOCALSUBSCRIPTIONS ACE_TEXT("sublocal") 
#define TT_PEERSUBSCRIPTIONS ACE_TEXT("subpeer")
#define TT_USERTIMEOUT ACE_TEXT("usertimeout")
#define TT_SUBSCRIBERS ACE_TEXT("subscribers")
#define TT_AUDIOCODEC ACE_TEXT("audiocodec")
#define TT_CMDID ACE_TEXT("id")
#define TT_CHANNELTYPE ACE_TEXT("type")
#define TT_USERTYPE ACE_TEXT("usertype")
#define TT_INDEX ACE_TEXT("index")
#define TT_COUNT ACE_TEXT("count")
#define TT_NOTEFIELD ACE_TEXT("note")
#define TT_USERDATA ACE_TEXT("userdata")
#define TT_VOICEUSERS ACE_TEXT("voiceusers")
#define TT_VIDEOUSERS ACE_TEXT("videousers")
#define TT_MEDIAFILEUSERS ACE_TEXT("mediafileusers")
#define TT_TOTALTX ACE_TEXT("totaltx")
#define TT_TOTALRX ACE_TEXT("totalrx")
#define TT_VOICETX ACE_TEXT("voicetx")
#define TT_VOICERX ACE_TEXT("voicerx")
#define TT_VIDEOCAPTX ACE_TEXT("videocaptx")
#define TT_VIDEOCAPRX ACE_TEXT("videocaprx")
#define TT_MEDIAFILETX ACE_TEXT("mediafiletx")
#define TT_MEDIAFILERX ACE_TEXT("mediafilerx")
#define TT_AUDIOCFG ACE_TEXT("audiocfg")
#define TT_AUDIOBPSLIMIT ACE_TEXT("audiocodeclimit")
#define TT_VOICETXLIMIT ACE_TEXT("voicetxlimit")
#define TT_VIDEOTXLIMIT ACE_TEXT("videotxlimit")
#define TT_MEDIAFILETXLIMIT ACE_TEXT("mediafiletxlimit")
#define TT_DESKTOPTXLIMIT ACE_TEXT("desktoptxlimit")
#define TT_TOTALTXLIMIT ACE_TEXT("totaltxlimit")
#define TT_UPTIME ACE_TEXT("uptime")
#define TT_USERSSERVED ACE_TEXT("usersserved")
#define TT_USERSPEAK ACE_TEXT("userspeak")
#define TT_FILESTX ACE_TEXT("filestx")
#define TT_FILESRX ACE_TEXT("filesrx")
#define TT_DESKTOPUSERS ACE_TEXT("desktopusers")
#define TT_DESKTOPTX ACE_TEXT("desktoptx")
#define TT_DESKTOPRX ACE_TEXT("desktoprx")
#define TT_AUTOOPCHANNELS ACE_TEXT("opchannels")
#define TT_MOTDRAW ACE_TEXT("motdraw")
#define TT_MAXLOGINSPERIP ACE_TEXT("maxiplogins")
#define TT_MAXLOGINATTEMPTS ACE_TEXT("maxloginattempts")
#define TT_CLIENTNAME ACE_TEXT("clientname") // v5.1
#define TT_TRANSMITQUEUE ACE_TEXT("transmitqueue") // v5.2
#define TT_CMDFLOOD ACE_TEXT("cmdflood") // v5.3
#define TT_BANTYPE ACE_TEXT("type") // v5.3
#define TT_LOGINDELAY ACE_TEXT("logindelay") // v5.5
#define TT_ACCESSTOKEN ACE_TEXT("accesstoken") // v5.6

//    Client ---> Server
//    -------------------------
#define CLIENT_LOGIN ACE_TEXT("login") 
#define CLIENT_LOGOUT ACE_TEXT("logout") 
#define CLIENT_CHANGENICK ACE_TEXT("changenick")
#define CLIENT_CHANGESTATUS ACE_TEXT("changestatus")
#define CLIENT_JOINCHANNEL ACE_TEXT("join")
#define CLIENT_LEAVECHANNEL ACE_TEXT("leave")
#define CLIENT_MESSAGE ACE_TEXT("message")
#define CLIENT_KEEPALIVE ACE_TEXT("ping")
#define CLIENT_KICK ACE_TEXT("kick")    //for ops and admins
#define CLIENT_MAKECHANNEL ACE_TEXT("makechannel")
#define CLIENT_UPDATECHANNEL ACE_TEXT("updatechannel")
#define CLIENT_REMOVECHANNEL ACE_TEXT("removechannel")
#define CLIENT_MOVEUSER ACE_TEXT("moveuser")
#define CLIENT_UPDATESERVER ACE_TEXT("updateserver")
#define CLIENT_SAVECONFIG ACE_TEXT("saveconfig")
#define CLIENT_CHANNELOP ACE_TEXT("op") 
#define CLIENT_BAN ACE_TEXT("ban") 
#define CLIENT_UNBAN ACE_TEXT("unban")
#define CLIENT_LISTBANS ACE_TEXT("listbans")
#define CLIENT_LISTUSERACCOUNTS ACE_TEXT("listaccounts")
#define CLIENT_NEWUSERACCOUNT ACE_TEXT("newaccount")
#define CLIENT_DELUSERACCOUNT ACE_TEXT("delaccount")
#define CLIENT_REGSENDFILE ACE_TEXT("regsendfile")
#define CLIENT_REGRECVFILE ACE_TEXT("regrecvfile")
#define CLIENT_SENDFILE ACE_TEXT("sendfile")
#define CLIENT_RECVFILE ACE_TEXT("recvfile")
#define CLIENT_DELIVERFILE ACE_TEXT("filedeliver")
#define CLIENT_DELETEFILE ACE_TEXT("deletefile")
#define CLIENT_QUIT ACE_TEXT("quit")
#define CLIENT_SUBSCRIBE ACE_TEXT("subscribe")
#define CLIENT_UNSUBSCRIBE ACE_TEXT("unsubscribe")
#define CLIENT_QUERYSTATS ACE_TEXT("querystats")

//    Server ---> Client
//    -------------------------
#define SERVER_WELCOME ACE_TEXT("teamtalk")
#define SERVER_LOGINACCEPTED ACE_TEXT("accepted")
#define SERVER_SERVERUPDATE ACE_TEXT("serverupdate")
#define SERVER_ERROR ACE_TEXT("error")
#define SERVER_KEEPALIVE ACE_TEXT("pong")
#define SERVER_ADDCHANNEL ACE_TEXT("addchannel")
#define SERVER_UPDATECHANNEL ACE_TEXT("updatechannel")
#define SERVER_REMOVECHANNEL ACE_TEXT("removechannel")
#define SERVER_JOINED ACE_TEXT("joined")
#define SERVER_LEFTCHANNEL ACE_TEXT("left")
#define SERVER_LOGGEDIN ACE_TEXT("loggedin")
#define SERVER_LOGGEDOUT ACE_TEXT("loggedout")
#define SERVER_ADDUSER ACE_TEXT("adduser")
#define SERVER_UPDATEUSER ACE_TEXT("updateuser")
#define SERVER_REMOVEUSER ACE_TEXT("removeuser")
#define SERVER_ADDFILE ACE_TEXT("addfile")
#define SERVER_REMOVEFILE ACE_TEXT("removefile")
#define SERVER_KICKED ACE_TEXT("kicked")
#define SERVER_MESSAGE_DELIVER ACE_TEXT("messagedeliver")
#define SERVER_BANNED ACE_TEXT("userbanned")
#define SERVER_USERACCOUNT ACE_TEXT("useraccount")
#define SERVER_FILE_ACCEPTED ACE_TEXT("fileaccepted")
#define SERVER_FILE_DELIVER ACE_TEXT("filedeliver")
#define SERVER_FILE_COMPLETED ACE_TEXT("filecompleted")
#define SERVER_FILE_READY ACE_TEXT("fileready")    //response to ACE_TEXT("recvfile") to tell client that file is ready to be downloaded
#define SERVER_FILESHAREINFO ACE_TEXT("fileshareinfo") //response to ACE_TEXT("fileshare")
#define SERVER_BEGINCMD ACE_TEXT("begin")
#define SERVER_ENDCMD ACE_TEXT("end")
#define SERVER_QUIT ACE_TEXT("quit")
#define SERVER_COMMAND_OK ACE_TEXT("ok")
#define SERVER_STATS ACE_TEXT("stats")

//command termination. If changed change PrepareString
#define EOL ACE_TEXT("\r\n")

#define MAX_COMMAND_LENGTH (1024*1024)

namespace teamtalk {

    enum TTError
    {
        /****** Command errors ******/

        TT_CMDERR_IGNORE = -1,

        /* Remember to updated DLL header file when modifying this */

        TT_CMDERR_SUCCESS = 0,    //indicates success

        TT_CMDERR_SYNTAX_ERROR = 1000,
        TT_CMDERR_UNKNOWN_COMMAND = 1001,
        TT_CMDERR_MISSING_PARAMETER = 1002,
        TT_CMDERR_INCOMPATIBLE_PROTOCOLS = 1003,
        TT_CMDERR_UNKNOWN_AUDIOCODEC = 1004,
        TT_CMDERR_INVALID_USERNAME = 1005,

        /****** command errors due to rights ******/
        TT_CMDERR_INCORRECT_CHANNEL_PASSWORD = 2001,
        TT_CMDERR_INVALID_ACCOUNT = 2002,
        TT_CMDERR_MAX_SERVER_USERS_EXCEEDED = 2003,
        TT_CMDERR_MAX_CHANNEL_USERS_EXCEEDED = 2004,
        TT_CMDERR_SERVER_BANNED = 2005,
        TT_CMDERR_NOT_AUTHORIZED = 2006,
        TT_CMDERR_MAX_DISKUSAGE_EXCEEDED = 2008,
        TT_CMDERR_INCORRECT_OP_PASSWORD = 2010,
        TT_CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED = 2011,
        TT_CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED = 2012,
        TT_CMDERR_MAX_CHANNELS_EXCEEDED = 2013,
        TT_CMDERR_COMMAND_FLOOD = 2014,
        TT_CMDERR_CHANNEL_BANNED = 2015,

        /***** command errors due to invalid state *****/
        TT_CMDERR_NOT_LOGGEDIN = 3000,
        TT_CMDERR_ALREADY_LOGGEDIN = 3001,
        TT_CMDERR_NOT_IN_CHANNEL = 3002,
        TT_CMDERR_ALREADY_IN_CHANNEL = 3003,
        TT_CMDERR_CHANNEL_ALREADY_EXISTS = 3004,
        TT_CMDERR_CHANNEL_NOT_FOUND = 3005,
        TT_CMDERR_USER_NOT_FOUND = 3006,
        TT_CMDERR_BAN_NOT_FOUND = 3007,
        TT_CMDERR_FILETRANSFER_NOT_FOUND = 3008,
        TT_CMDERR_OPENFILE_FAILED = 3009,
        TT_CMDERR_ACCOUNT_NOT_FOUND = 3010,
        TT_CMDERR_FILE_NOT_FOUND = 3011,
        TT_CMDERR_FILE_ALREADY_EXISTS = 3012,
        TT_CMDERR_FILESHARING_DISABLED = 3013,
        TT_CMDERR_CHANNEL_HAS_USERS = 3015,
        TT_CMDERR_LOGINSERVICE_UNAVAILABLE = 3016,

        TT_SRVERR_COMMAND_SUSPEND = 4000,

        /***** Internal TeamTalk errors not related to commands ********/
        TT_INTERR_SNDINPUT_FAILURE = 10000,
        TT_INTERR_SNDOUTPUT_FAILURE = 10001,
        TT_INTERR_AUDIOCODEC_INIT_FAILED = 10002,
        TT_INTERR_AUDIOPREPROCESSOR_INIT_FAILED = 10003,
        TT_INTERR_MSGQUEUE_OVERFLOW = 10004, // only used externally
        TT_INTERR_SNDEFFECT_FAILURE = 10005,
    };

    typedef std::map<ACE_TString, ACE_TString> mstrings_t;

    //obtain error message to error number TT_CMDERR_*
    ACE_TString GetErrorDescription(int nError);

    struct ErrorMsg
    {
        int errorno;
        ACE_TString errmsg, paramname;
        ErrorMsg() : errorno(TT_CMDERR_SUCCESS) { }
        ErrorMsg(int cmderrno, const ACE_TString& param = ACE_TEXT(""))
            : errorno(cmderrno), paramname(param)
        {
            errmsg = GetErrorDescription(cmderrno);
        }

        bool success() const
        {
            return errorno == TT_CMDERR_SUCCESS;
        }
    };

    bool HasProperty(const mstrings_t& properties, 
                     const ACE_TString& prop);

    bool GetProperty(const mstrings_t& properties, 
                     const ACE_TString& prop, ACE_TString& value);

    bool GetProperty(const mstrings_t& properties, 
                     const ACE_TString& prop, int& value);

    bool GetProperty(const mstrings_t& properties, 
                     const ACE_TString& prop, ACE_UINT32& value);

    bool GetProperty(const mstrings_t& properties, 
                     const ACE_TString& prop, std::vector<int>& vec);

    bool GetProperty(const mstrings_t& properties, 
                     const ACE_TString& prop, std::set<int>& myset);

    bool GetProperty(const mstrings_t& properties, 
                     const ACE_TString& prop, bool& value);

    bool GetProperty(const mstrings_t& properties, 
                     const ACE_TString& prop, ACE_INT64& value);

    bool GetProperty(const mstrings_t& properties, const ACE_TString& prop, 
                     AudioCodec& codec);

    bool GetProperty(const mstrings_t& properties, const ACE_TString& prop, 
                     AudioConfig& audcfg);

    bool GetProperty(const mstrings_t& properties, const ACE_TString& prop, 
                     ACE_INET_Addr& addr);

    ACE_TString PrepareIntegerArray(const std::vector<int>& array);

    ACE_TString PrepareIntegerSet(const std::set<int>& myset);

    ACE_TString PrepareString(const ACE_TString& str);

    ACE_TString RebuildString(const ACE_TString& str);

    //@return Number of properties found, -1 on error.
    int ExtractProperties(const ACE_TString& input, mstrings_t& properties);

    //appends a property to a command ACE_TString
    void AppendProperty(const ACE_TString& prop, 
                        const ACE_TString& value, ACE_TString& dest_str);
    void AppendProperty(const ACE_TString& prop, 
                        const std::vector<int>& vecValues, 
                        ACE_TString& dest_str);
    void AppendProperty(const ACE_TString& prop, 
                        const std::set<int>& setValues, 
                        ACE_TString& dest_str);
    void AppendProperty(const ACE_TString& prop, 
                        ACE_INT64 value, ACE_TString& dest_str);
    void AppendProperty(const ACE_TString& prop, const AudioCodec& codec, 
                        ACE_TString& dest_str);
    void AppendProperty(const ACE_TString& prop, const AudioConfig& audcfg, 
                        ACE_TString& dest_str);
    void AppendProperty(const ACE_TString& prop, 
                        const ACE_INET_Addr& addr, ACE_TString& dest_str);

    bool GetCmdLine(const ACE_CString& input, 
                    ACE_CString& cmd, ACE_CString& remain_input);
    bool GetCmd(const ACE_CString& input, ACE_CString& cmd);
    ACE_TString stripEOL(const ACE_TString& input);

    ACE_TString InetAddrToString(const ACE_INET_Addr& addr);
}
#endif
