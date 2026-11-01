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

#include "Common.h"

#include <ace/ACE.h>
#include <ace/INET_Addr.h>
#include <ace/SString.h>

#include <map>
#include <set>
#include <vector>

constexpr auto TEAMTALK_PROTOCOL_VERSION = ACE_TEXT("5.14");

/* parameter names */
constexpr auto TT_USERID = ACE_TEXT("userid");
constexpr auto TT_NICKNAME = ACE_TEXT("nickname");
constexpr auto TT_MSGCONTENT = ACE_TEXT("content");
constexpr auto TT_ERRORMSG = ACE_TEXT("message");
constexpr auto TT_PARAMNAME = ACE_TEXT("param");
constexpr auto TT_PASSWORD = ACE_TEXT("password");
constexpr auto TT_TOPIC = ACE_TEXT("topic");
constexpr auto TT_OPERATORS = ACE_TEXT("operators");
constexpr auto TT_STATUSMODE = ACE_TEXT("statusmode");
constexpr auto TT_STATUSMESSAGE = ACE_TEXT("statusmsg");
constexpr auto TT_IPADDR = ACE_TEXT("ipaddr");
constexpr auto TT_TCPPORT = ACE_TEXT("tcpport");
constexpr auto TT_UDPPORT = ACE_TEXT("udpport");
constexpr auto TT_VERSION = ACE_TEXT("version");
constexpr auto TT_SERVERNAME = ACE_TEXT("servername");
constexpr auto TT_USERRIGHTS = ACE_TEXT("userrights");
constexpr auto TT_CHANNELID = ACE_TEXT("chanid");
constexpr auto TT_PARENTID = ACE_TEXT("parentid");
constexpr auto TT_CHANNAME = ACE_TEXT("name");
constexpr auto TT_CHANNEL = ACE_TEXT("channel");
constexpr auto TT_INITCHANNEL = ACE_TEXT("initchan");
constexpr auto TT_REQPASSWORD = ACE_TEXT("protected");
constexpr auto TT_ERRORNUM = ACE_TEXT("number");
constexpr auto TT_PROTOCOL = ACE_TEXT("protocol");
constexpr auto TT_SRCUSERID = ACE_TEXT("srcuserid");
constexpr auto TT_DESTUSERID = ACE_TEXT("destuserid");
constexpr auto TT_KICKERID = ACE_TEXT("kickerid");
constexpr auto TT_MSGTYPE = ACE_TEXT("type");
constexpr auto TT_USERNAME = ACE_TEXT("username");
constexpr auto TT_FILENAME = ACE_TEXT("filename");
constexpr auto TT_FILESIZE = ACE_TEXT("filesize");
constexpr auto TT_FILEDATE = ACE_TEXT("filedate");
constexpr auto TT_FILEID = ACE_TEXT("fileid");
constexpr auto TT_FILEOWNER = ACE_TEXT("owner");
constexpr auto TT_TRANSFERID = ACE_TEXT("transferid");
constexpr auto TT_PACKETPROTOCOL = ACE_TEXT("packetprotocol");
constexpr auto TT_MOTD = ACE_TEXT("motd");
constexpr auto TT_BANTIME = ACE_TEXT("bantime");
constexpr auto TT_CRYPTKEY = ACE_TEXT("cryptkey");
constexpr auto TT_DISKQUOTA = ACE_TEXT("diskquota");
constexpr auto TT_FILESROOT = ACE_TEXT("filesroot");
constexpr auto TT_MAXUSERS = ACE_TEXT("maxusers");
constexpr auto TT_AUTOSAVE = ACE_TEXT("autosave");
constexpr auto TT_MAXDISKUSAGE = ACE_TEXT("maxdiskusage");
constexpr auto TT_OPPASSWORD = ACE_TEXT("oppassword");
constexpr auto TT_OPERATORSTATUS = ACE_TEXT("opstatus");
constexpr auto TT_LOCALSUBSCRIPTIONS = ACE_TEXT("sublocal");
constexpr auto TT_PEERSUBSCRIPTIONS = ACE_TEXT("subpeer");
constexpr auto TT_USERTIMEOUT = ACE_TEXT("usertimeout");
constexpr auto TT_SUBSCRIBERS = ACE_TEXT("subscribers");
constexpr auto TT_AUDIOCODEC = ACE_TEXT("audiocodec");
constexpr auto TT_CMDID = ACE_TEXT("id");
constexpr auto TT_CHANNELTYPE = ACE_TEXT("type");
constexpr auto TT_USERTYPE = ACE_TEXT("usertype");
constexpr auto TT_INDEX = ACE_TEXT("index");
constexpr auto TT_COUNT = ACE_TEXT("count");
constexpr auto TT_NOTEFIELD = ACE_TEXT("note");
constexpr auto TT_USERDATA = ACE_TEXT("userdata");
constexpr auto TT_VOICEUSERS = ACE_TEXT("voiceusers");
constexpr auto TT_VIDEOUSERS = ACE_TEXT("videousers");
constexpr auto TT_MEDIAFILEUSERS = ACE_TEXT("mediafileusers");
constexpr auto TT_TOTALTX = ACE_TEXT("totaltx");
constexpr auto TT_TOTALRX = ACE_TEXT("totalrx");
constexpr auto TT_VOICETX = ACE_TEXT("voicetx");
constexpr auto TT_VOICERX = ACE_TEXT("voicerx");
constexpr auto TT_VIDEOCAPTX = ACE_TEXT("videocaptx");
constexpr auto TT_VIDEOCAPRX = ACE_TEXT("videocaprx");
constexpr auto TT_MEDIAFILETX = ACE_TEXT("mediafiletx");
constexpr auto TT_MEDIAFILERX = ACE_TEXT("mediafilerx");
constexpr auto TT_AUDIOCFG = ACE_TEXT("audiocfg");
constexpr auto TT_AUDIOBPSLIMIT = ACE_TEXT("audiocodeclimit");
constexpr auto TT_VOICETXLIMIT = ACE_TEXT("voicetxlimit");
constexpr auto TT_VIDEOTXLIMIT = ACE_TEXT("videotxlimit");
constexpr auto TT_MEDIAFILETXLIMIT = ACE_TEXT("mediafiletxlimit");
constexpr auto TT_DESKTOPTXLIMIT = ACE_TEXT("desktoptxlimit");
constexpr auto TT_TOTALTXLIMIT = ACE_TEXT("totaltxlimit");
constexpr auto TT_UPTIME = ACE_TEXT("uptime");
constexpr auto TT_USERSSERVED = ACE_TEXT("usersserved");
constexpr auto TT_USERSPEAK = ACE_TEXT("userspeak");
constexpr auto TT_FILESTX = ACE_TEXT("filestx");
constexpr auto TT_FILESRX = ACE_TEXT("filesrx");
constexpr auto TT_DESKTOPUSERS = ACE_TEXT("desktopusers");
constexpr auto TT_DESKTOPTX = ACE_TEXT("desktoptx");
constexpr auto TT_DESKTOPRX = ACE_TEXT("desktoprx");
constexpr auto TT_AUTOOPCHANNELS = ACE_TEXT("opchannels");
constexpr auto TT_MOTDRAW = ACE_TEXT("motdraw");
constexpr auto TT_MAXLOGINSPERIP = ACE_TEXT("maxiplogins");
constexpr auto TT_MAXLOGINATTEMPTS = ACE_TEXT("maxloginattempts");
constexpr auto TT_CLIENTNAME = ACE_TEXT("clientname"); // v5.1
constexpr auto TT_TRANSMITQUEUE = ACE_TEXT("transmitqueue"); // v5.2
constexpr auto TT_CMDFLOOD = ACE_TEXT("cmdflood"); // v5.3
constexpr auto TT_BANTYPE = ACE_TEXT("type"); // v5.3
constexpr auto TT_LOGINDELAY = ACE_TEXT("logindelay"); // v5.5
constexpr auto TT_ACCESSTOKEN = ACE_TEXT("accesstoken"); // v5.6
constexpr auto TT_CHANMSGUSERS = ACE_TEXT("chanmsgusers"); // v5.8
constexpr auto TT_UPLOADTIME = ACE_TEXT("uploadtime"); // v5.9
constexpr auto TT_MODIFIEDTIME = ACE_TEXT("modifiedtime"); // v5.9
constexpr auto TT_TRANSMITSWITCHDELAY = ACE_TEXT("switchdelay"); // v5.10
constexpr auto TT_LOGEVENTS = ACE_TEXT("logevents"); // v5.10
constexpr auto TT_TEXTMSG_MORE = ACE_TEXT("more"); // v5.10
constexpr auto TT_TOTVOICE = ACE_TEXT("voicetot"); // v5.12
constexpr auto TT_TOTMEDIAFILE = ACE_TEXT("mediafiletot"); // v5.12
constexpr auto TT_TRANSFERKEY = ACE_TEXT("filetxkey"); // v5.13
constexpr auto TT_BANOWNER = ACE_TEXT("owner"); // v5.13
constexpr auto TT_LASTLOGINTIME = ACE_TEXT("lastlogin"); // v5.14

//    Client ---> Server
//    -------------------------
constexpr auto CLIENT_LOGIN = ACE_TEXT("login");
constexpr auto CLIENT_LOGOUT = ACE_TEXT("logout");
constexpr auto CLIENT_CHANGENICK = ACE_TEXT("changenick");
constexpr auto CLIENT_CHANGESTATUS = ACE_TEXT("changestatus");
constexpr auto CLIENT_JOINCHANNEL = ACE_TEXT("join");
constexpr auto CLIENT_LEAVECHANNEL = ACE_TEXT("leave");
constexpr auto CLIENT_MESSAGE = ACE_TEXT("message");
constexpr auto CLIENT_KEEPALIVE = ACE_TEXT("ping");
constexpr auto CLIENT_KICK = ACE_TEXT("kick"); //for ops and admins
constexpr auto CLIENT_MAKECHANNEL = ACE_TEXT("makechannel");
constexpr auto CLIENT_UPDATECHANNEL = ACE_TEXT("updatechannel");
constexpr auto CLIENT_REMOVECHANNEL = ACE_TEXT("removechannel");
constexpr auto CLIENT_MOVEUSER = ACE_TEXT("moveuser");
constexpr auto CLIENT_UPDATESERVER = ACE_TEXT("updateserver");
constexpr auto CLIENT_SAVECONFIG = ACE_TEXT("saveconfig");
constexpr auto CLIENT_CHANNELOP = ACE_TEXT("op");
constexpr auto CLIENT_BAN = ACE_TEXT("ban");
constexpr auto CLIENT_UNBAN = ACE_TEXT("unban");
constexpr auto CLIENT_LISTBANS = ACE_TEXT("listbans");
constexpr auto CLIENT_LISTUSERACCOUNTS = ACE_TEXT("listaccounts");
constexpr auto CLIENT_NEWUSERACCOUNT = ACE_TEXT("newaccount");
constexpr auto CLIENT_DELUSERACCOUNT = ACE_TEXT("delaccount");
constexpr auto CLIENT_REGSENDFILE = ACE_TEXT("regsendfile");
constexpr auto CLIENT_REGRECVFILE = ACE_TEXT("regrecvfile");
constexpr auto CLIENT_SENDFILE = ACE_TEXT("sendfile");
constexpr auto CLIENT_RECVFILE = ACE_TEXT("recvfile");
constexpr auto CLIENT_DELIVERFILE = ACE_TEXT("filedeliver");
constexpr auto CLIENT_DELETEFILE = ACE_TEXT("deletefile");
constexpr auto CLIENT_QUIT = ACE_TEXT("quit");
constexpr auto CLIENT_SUBSCRIBE = ACE_TEXT("subscribe");
constexpr auto CLIENT_UNSUBSCRIBE = ACE_TEXT("unsubscribe");
constexpr auto CLIENT_QUERYSTATS = ACE_TEXT("querystats");

//    Server ---> Client
//    -------------------------
constexpr auto SERVER_WELCOME = ACE_TEXT("teamtalk");
constexpr auto SERVER_LOGINACCEPTED = ACE_TEXT("accepted");
constexpr auto SERVER_SERVERUPDATE = ACE_TEXT("serverupdate");
constexpr auto SERVER_ERROR = ACE_TEXT("error");
constexpr auto SERVER_KEEPALIVE = ACE_TEXT("pong");
constexpr auto SERVER_ADDCHANNEL = ACE_TEXT("addchannel");
constexpr auto SERVER_UPDATECHANNEL = ACE_TEXT("updatechannel");
constexpr auto SERVER_REMOVECHANNEL = ACE_TEXT("removechannel");
constexpr auto SERVER_JOINED = ACE_TEXT("joined");
constexpr auto SERVER_LEFTCHANNEL = ACE_TEXT("left");
constexpr auto SERVER_LOGGEDIN = ACE_TEXT("loggedin");
constexpr auto SERVER_LOGGEDOUT = ACE_TEXT("loggedout");
constexpr auto SERVER_ADDUSER = ACE_TEXT("adduser");
constexpr auto SERVER_UPDATEUSER = ACE_TEXT("updateuser");
constexpr auto SERVER_REMOVEUSER = ACE_TEXT("removeuser");
constexpr auto SERVER_ADDFILE = ACE_TEXT("addfile");
constexpr auto SERVER_REMOVEFILE = ACE_TEXT("removefile");
constexpr auto SERVER_KICKED = ACE_TEXT("kicked");
constexpr auto SERVER_MESSAGE_DELIVER = ACE_TEXT("messagedeliver");
constexpr auto SERVER_BANNED = ACE_TEXT("userbanned");
constexpr auto SERVER_USERACCOUNT = ACE_TEXT("useraccount");
constexpr auto SERVER_ADDUSERACCOUNT = ACE_TEXT("adduseraccount"); //v5.11
constexpr auto SERVER_REMOVEUSERACCOUNT = ACE_TEXT("removeuseraccount"); //v5.11
constexpr auto SERVER_FILE_ACCEPTED = ACE_TEXT("fileaccepted");
constexpr auto SERVER_FILE_DELIVER = ACE_TEXT("filedeliver");
constexpr auto SERVER_FILE_COMPLETED = ACE_TEXT("filecompleted");
constexpr auto SERVER_FILE_READY = ACE_TEXT("fileready"); //response to ACE_TEXT("recvfile") to tell client that file is ready to be downloaded
constexpr auto SERVER_FILESHAREINFO = ACE_TEXT("fileshareinfo"); //response to ACE_TEXT("fileshare")
constexpr auto SERVER_BEGINCMD = ACE_TEXT("begin");
constexpr auto SERVER_ENDCMD = ACE_TEXT("end");
constexpr auto SERVER_QUIT = ACE_TEXT("quit");
constexpr auto SERVER_COMMAND_OK = ACE_TEXT("ok");
constexpr auto SERVER_STATS = ACE_TEXT("stats");

//command termination. If changed change PrepareString
constexpr auto EOL = ACE_TEXT("\r\n");

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
        TT_CMDERR_MAX_FILETRANSFERS_EXCEEDED = 2016,

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
        TT_CMDERR_CHANNEL_CANNOT_BE_HIDDEN = 3017,

        TT_SRVERR_COMMAND_SUSPEND = 4000,

        /***** Internal TeamTalk errors not related to commands ********/
        TT_INTERR_SNDINPUT_FAILURE = 10000,
        TT_INTERR_SNDOUTPUT_FAILURE = 10001,
        TT_INTERR_AUDIOCODEC_INIT_FAILED = 10002,
        TT_INTERR_AUDIOPREPROCESSOR_INIT_FAILED = 10003,
        TT_INTERR_MSGQUEUE_OVERFLOW = 10004, // only used externally
        TT_INTERR_SNDEFFECT_FAILURE = 10005,
    };

    using mstrings_t = std::map<ACE_TString, ACE_TString>;

    //obtain error message to error number TT_CMDERR_*
    ACE_TString GetErrorDescription(int nError);

    struct ErrorMsg
    {
        int errorno = TT_CMDERR_SUCCESS;
        ACE_TString errmsg, paramname;
        ErrorMsg() = default;
        ErrorMsg(int cmderrno, const ACE_TString& param = ACE_TEXT(""))
            : errorno(cmderrno), paramname(param)
        {
            errmsg = GetErrorDescription(cmderrno);
        }

        bool Success() const
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
                     const ACE_TString& prop, uint32_t& value);

    bool GetProperty(const mstrings_t& properties, 
                     const ACE_TString& prop, std::vector<int>& vec);

    bool GetProperty(const mstrings_t& properties, 
                     const ACE_TString& prop, std::set<int>& myset);

    bool GetProperty(const mstrings_t& properties, 
                     const ACE_TString& prop, bool& value);

    bool GetProperty(const mstrings_t& properties, 
                     const ACE_TString& prop, int64_t& value);

    bool GetProperty(const mstrings_t& properties, const ACE_TString& prop, 
                     AudioCodec& codec);

    bool GetProperty(const mstrings_t& properties, const ACE_TString& prop, 
                     AudioConfig& audcfg);

    bool GetProperty(const mstrings_t& properties, const ACE_TString& prop, 
                     ACE_INET_Addr& addr);

    bool GetProperty(const mstrings_t& properties, const ACE_TString& prop,
                     ACE_Time_Value& tv);

    void GetProperties(const mstrings_t& properties, UserAccount& useraccount);

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
                        int64_t value, ACE_TString& dest_str);
    void AppendProperty(const ACE_TString& prop, const AudioCodec& codec, 
                        ACE_TString& dest_str);
    void AppendProperty(const ACE_TString& prop, const AudioConfig& audcfg, 
                        ACE_TString& dest_str);
    void AppendProperty(const ACE_TString& prop, 
                        const ACE_INET_Addr& addr, ACE_TString& dest_str);
    void AppendProperty(const ACE_TString& prop,
                        const ACE_Time_Value& tv, ACE_TString& dest_str);

    bool GetCmdLine(const ACE_CString& input, 
                    ACE_CString& cmd, ACE_CString& remain_input);
    bool GetCmd(const ACE_CString& input, ACE_CString& cmd);
    ACE_TString StripEol(const ACE_TString& input);
} // namespace teamtalk
#endif
