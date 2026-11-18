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
#include "mystd/MyStd.h"

#include <ace/ACE.h>
#include <ace/INET_Addr.h>

#include <map>
#include <set>
#include <vector>

constexpr auto TEAMTALK_PROTOCOL_VERSION = TTEXT("5.14");

/* parameter names */
constexpr auto TT_USERID = TTEXT("userid");
constexpr auto TT_NICKNAME = TTEXT("nickname");
constexpr auto TT_MSGCONTENT = TTEXT("content");
constexpr auto TT_ERRORMSG = TTEXT("message");
constexpr auto TT_PARAMNAME = TTEXT("param");
constexpr auto TT_PASSWORD = TTEXT("password");
constexpr auto TT_TOPIC = TTEXT("topic");
constexpr auto TT_OPERATORS = TTEXT("operators");
constexpr auto TT_STATUSMODE = TTEXT("statusmode");
constexpr auto TT_STATUSMESSAGE = TTEXT("statusmsg");
constexpr auto TT_IPADDR = TTEXT("ipaddr");
constexpr auto TT_TCPPORT = TTEXT("tcpport");
constexpr auto TT_UDPPORT = TTEXT("udpport");
constexpr auto TT_VERSION = TTEXT("version");
constexpr auto TT_SERVERNAME = TTEXT("servername");
constexpr auto TT_USERRIGHTS = TTEXT("userrights");
constexpr auto TT_CHANNELID = TTEXT("chanid");
constexpr auto TT_PARENTID = TTEXT("parentid");
constexpr auto TT_CHANNAME = TTEXT("name");
constexpr auto TT_CHANNEL = TTEXT("channel");
constexpr auto TT_INITCHANNEL = TTEXT("initchan");
constexpr auto TT_REQPASSWORD = TTEXT("protected");
constexpr auto TT_ERRORNUM = TTEXT("number");
constexpr auto TT_PROTOCOL = TTEXT("protocol");
constexpr auto TT_SRCUSERID = TTEXT("srcuserid");
constexpr auto TT_DESTUSERID = TTEXT("destuserid");
constexpr auto TT_KICKERID = TTEXT("kickerid");
constexpr auto TT_MSGTYPE = TTEXT("type");
constexpr auto TT_USERNAME = TTEXT("username");
constexpr auto TT_FILENAME = TTEXT("filename");
constexpr auto TT_FILESIZE = TTEXT("filesize");
constexpr auto TT_FILEDATE = TTEXT("filedate");
constexpr auto TT_FILEID = TTEXT("fileid");
constexpr auto TT_FILEOWNER = TTEXT("owner");
constexpr auto TT_TRANSFERID = TTEXT("transferid");
constexpr auto TT_PACKETPROTOCOL = TTEXT("packetprotocol");
constexpr auto TT_MOTD = TTEXT("motd");
constexpr auto TT_BANTIME = TTEXT("bantime");
constexpr auto TT_CRYPTKEY = TTEXT("cryptkey");
constexpr auto TT_DISKQUOTA = TTEXT("diskquota");
constexpr auto TT_FILESROOT = TTEXT("filesroot");
constexpr auto TT_MAXUSERS = TTEXT("maxusers");
constexpr auto TT_AUTOSAVE = TTEXT("autosave");
constexpr auto TT_MAXDISKUSAGE = TTEXT("maxdiskusage");
constexpr auto TT_OPPASSWORD = TTEXT("oppassword");
constexpr auto TT_OPERATORSTATUS = TTEXT("opstatus");
constexpr auto TT_LOCALSUBSCRIPTIONS = TTEXT("sublocal");
constexpr auto TT_PEERSUBSCRIPTIONS = TTEXT("subpeer");
constexpr auto TT_USERTIMEOUT = TTEXT("usertimeout");
constexpr auto TT_SUBSCRIBERS = TTEXT("subscribers");
constexpr auto TT_AUDIOCODEC = TTEXT("audiocodec");
constexpr auto TT_CMDID = TTEXT("id");
constexpr auto TT_CHANNELTYPE = TTEXT("type");
constexpr auto TT_USERTYPE = TTEXT("usertype");
constexpr auto TT_INDEX = TTEXT("index");
constexpr auto TT_COUNT = TTEXT("count");
constexpr auto TT_NOTEFIELD = TTEXT("note");
constexpr auto TT_USERDATA = TTEXT("userdata");
constexpr auto TT_VOICEUSERS = TTEXT("voiceusers");
constexpr auto TT_VIDEOUSERS = TTEXT("videousers");
constexpr auto TT_MEDIAFILEUSERS = TTEXT("mediafileusers");
constexpr auto TT_TOTALTX = TTEXT("totaltx");
constexpr auto TT_TOTALRX = TTEXT("totalrx");
constexpr auto TT_VOICETX = TTEXT("voicetx");
constexpr auto TT_VOICERX = TTEXT("voicerx");
constexpr auto TT_VIDEOCAPTX = TTEXT("videocaptx");
constexpr auto TT_VIDEOCAPRX = TTEXT("videocaprx");
constexpr auto TT_MEDIAFILETX = TTEXT("mediafiletx");
constexpr auto TT_MEDIAFILERX = TTEXT("mediafilerx");
constexpr auto TT_AUDIOCFG = TTEXT("audiocfg");
constexpr auto TT_AUDIOBPSLIMIT = TTEXT("audiocodeclimit");
constexpr auto TT_VOICETXLIMIT = TTEXT("voicetxlimit");
constexpr auto TT_VIDEOTXLIMIT = TTEXT("videotxlimit");
constexpr auto TT_MEDIAFILETXLIMIT = TTEXT("mediafiletxlimit");
constexpr auto TT_DESKTOPTXLIMIT = TTEXT("desktoptxlimit");
constexpr auto TT_TOTALTXLIMIT = TTEXT("totaltxlimit");
constexpr auto TT_UPTIME = TTEXT("uptime");
constexpr auto TT_USERSSERVED = TTEXT("usersserved");
constexpr auto TT_USERSPEAK = TTEXT("userspeak");
constexpr auto TT_FILESTX = TTEXT("filestx");
constexpr auto TT_FILESRX = TTEXT("filesrx");
constexpr auto TT_DESKTOPUSERS = TTEXT("desktopusers");
constexpr auto TT_DESKTOPTX = TTEXT("desktoptx");
constexpr auto TT_DESKTOPRX = TTEXT("desktoprx");
constexpr auto TT_AUTOOPCHANNELS = TTEXT("opchannels");
constexpr auto TT_MOTDRAW = TTEXT("motdraw");
constexpr auto TT_MAXLOGINSPERIP = TTEXT("maxiplogins");
constexpr auto TT_MAXLOGINATTEMPTS = TTEXT("maxloginattempts");
constexpr auto TT_CLIENTNAME = TTEXT("clientname"); // v5.1
constexpr auto TT_TRANSMITQUEUE = TTEXT("transmitqueue"); // v5.2
constexpr auto TT_CMDFLOOD = TTEXT("cmdflood"); // v5.3
constexpr auto TT_BANTYPE = TTEXT("type"); // v5.3
constexpr auto TT_LOGINDELAY = TTEXT("logindelay"); // v5.5
constexpr auto TT_ACCESSTOKEN = TTEXT("accesstoken"); // v5.6
constexpr auto TT_CHANMSGUSERS = TTEXT("chanmsgusers"); // v5.8
constexpr auto TT_UPLOADTIME = TTEXT("uploadtime"); // v5.9
constexpr auto TT_MODIFIEDTIME = TTEXT("modifiedtime"); // v5.9
constexpr auto TT_TRANSMITSWITCHDELAY = TTEXT("switchdelay"); // v5.10
constexpr auto TT_LOGEVENTS = TTEXT("logevents"); // v5.10
constexpr auto TT_TEXTMSG_MORE = TTEXT("more"); // v5.10
constexpr auto TT_TOTVOICE = TTEXT("voicetot"); // v5.12
constexpr auto TT_TOTMEDIAFILE = TTEXT("mediafiletot"); // v5.12
constexpr auto TT_TRANSFERKEY = TTEXT("filetxkey"); // v5.13
constexpr auto TT_BANOWNER = TTEXT("owner"); // v5.13
constexpr auto TT_LASTLOGINTIME = TTEXT("lastlogin"); // v5.14

//    Client ---> Server
//    -------------------------
constexpr auto CLIENT_LOGIN = TTEXT("login");
constexpr auto CLIENT_LOGOUT = TTEXT("logout");
constexpr auto CLIENT_CHANGENICK = TTEXT("changenick");
constexpr auto CLIENT_CHANGESTATUS = TTEXT("changestatus");
constexpr auto CLIENT_JOINCHANNEL = TTEXT("join");
constexpr auto CLIENT_LEAVECHANNEL = TTEXT("leave");
constexpr auto CLIENT_MESSAGE = TTEXT("message");
constexpr auto CLIENT_KEEPALIVE = TTEXT("ping");
constexpr auto CLIENT_KICK = TTEXT("kick"); //for ops and admins
constexpr auto CLIENT_MAKECHANNEL = TTEXT("makechannel");
constexpr auto CLIENT_UPDATECHANNEL = TTEXT("updatechannel");
constexpr auto CLIENT_REMOVECHANNEL = TTEXT("removechannel");
constexpr auto CLIENT_MOVEUSER = TTEXT("moveuser");
constexpr auto CLIENT_UPDATESERVER = TTEXT("updateserver");
constexpr auto CLIENT_SAVECONFIG = TTEXT("saveconfig");
constexpr auto CLIENT_CHANNELOP = TTEXT("op");
constexpr auto CLIENT_BAN = TTEXT("ban");
constexpr auto CLIENT_UNBAN = TTEXT("unban");
constexpr auto CLIENT_LISTBANS = TTEXT("listbans");
constexpr auto CLIENT_LISTUSERACCOUNTS = TTEXT("listaccounts");
constexpr auto CLIENT_NEWUSERACCOUNT = TTEXT("newaccount");
constexpr auto CLIENT_DELUSERACCOUNT = TTEXT("delaccount");
constexpr auto CLIENT_REGSENDFILE = TTEXT("regsendfile");
constexpr auto CLIENT_REGRECVFILE = TTEXT("regrecvfile");
constexpr auto CLIENT_SENDFILE = TTEXT("sendfile");
constexpr auto CLIENT_RECVFILE = TTEXT("recvfile");
constexpr auto CLIENT_DELIVERFILE = TTEXT("filedeliver");
constexpr auto CLIENT_DELETEFILE = TTEXT("deletefile");
constexpr auto CLIENT_QUIT = TTEXT("quit");
constexpr auto CLIENT_SUBSCRIBE = TTEXT("subscribe");
constexpr auto CLIENT_UNSUBSCRIBE = TTEXT("unsubscribe");
constexpr auto CLIENT_QUERYSTATS = TTEXT("querystats");

//    Server ---> Client
//    -------------------------
constexpr auto SERVER_WELCOME = TTEXT("teamtalk");
constexpr auto SERVER_LOGINACCEPTED = TTEXT("accepted");
constexpr auto SERVER_SERVERUPDATE = TTEXT("serverupdate");
constexpr auto SERVER_ERROR = TTEXT("error");
constexpr auto SERVER_KEEPALIVE = TTEXT("pong");
constexpr auto SERVER_ADDCHANNEL = TTEXT("addchannel");
constexpr auto SERVER_UPDATECHANNEL = TTEXT("updatechannel");
constexpr auto SERVER_REMOVECHANNEL = TTEXT("removechannel");
constexpr auto SERVER_JOINED = TTEXT("joined");
constexpr auto SERVER_LEFTCHANNEL = TTEXT("left");
constexpr auto SERVER_LOGGEDIN = TTEXT("loggedin");
constexpr auto SERVER_LOGGEDOUT = TTEXT("loggedout");
constexpr auto SERVER_ADDUSER = TTEXT("adduser");
constexpr auto SERVER_UPDATEUSER = TTEXT("updateuser");
constexpr auto SERVER_REMOVEUSER = TTEXT("removeuser");
constexpr auto SERVER_ADDFILE = TTEXT("addfile");
constexpr auto SERVER_REMOVEFILE = TTEXT("removefile");
constexpr auto SERVER_KICKED = TTEXT("kicked");
constexpr auto SERVER_MESSAGE_DELIVER = TTEXT("messagedeliver");
constexpr auto SERVER_BANNED = TTEXT("userbanned");
constexpr auto SERVER_USERACCOUNT = TTEXT("useraccount");
constexpr auto SERVER_ADDUSERACCOUNT = TTEXT("adduseraccount"); //v5.11
constexpr auto SERVER_REMOVEUSERACCOUNT = TTEXT("removeuseraccount"); //v5.11
constexpr auto SERVER_FILE_ACCEPTED = TTEXT("fileaccepted");
constexpr auto SERVER_FILE_DELIVER = TTEXT("filedeliver");
constexpr auto SERVER_FILE_COMPLETED = TTEXT("filecompleted");
constexpr auto SERVER_FILE_READY = TTEXT("fileready"); //response to TTEXT("recvfile") to tell client that file is ready to be downloaded
constexpr auto SERVER_FILESHAREINFO = TTEXT("fileshareinfo"); //response to TTEXT("fileshare")
constexpr auto SERVER_BEGINCMD = TTEXT("begin");
constexpr auto SERVER_ENDCMD = TTEXT("end");
constexpr auto SERVER_QUIT = TTEXT("quit");
constexpr auto SERVER_COMMAND_OK = TTEXT("ok");
constexpr auto SERVER_STATS = TTEXT("stats");

//command termination. If changed change PrepareString
constexpr auto EOL = TTEXT("\r\n");

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

    using mstrings_t = std::map<tstring, tstring>;

    //obtain error message to error number TT_CMDERR_*
    tstring GetErrorDescription(int nError);

    struct ErrorMsg
    {
        int errorno = TT_CMDERR_SUCCESS;
        tstring errmsg, paramname;
        ErrorMsg() = default;
        ErrorMsg(int cmderrno, const tstring& msg = TTEXT(""), const tstring& param = TTEXT(""))
            : errorno(cmderrno), paramname(param)
        {
            if (!msg.empty())
            {
                errmsg = msg;
            }
            else
            {
                errmsg = GetErrorDescription(cmderrno);
            }
        }

        bool Success() const
        {
            return errorno == TT_CMDERR_SUCCESS;
        }
    };

    bool HasProperty(const mstrings_t& properties, 
                     const tstring& prop);

    bool GetProperty(const mstrings_t& properties, 
                     const tstring& prop, tstring& value);

    bool GetProperty(const mstrings_t& properties, 
                     const tstring& prop, int& value);

    bool GetProperty(const mstrings_t& properties, 
                     const tstring& prop, uint32_t& value);

    bool GetProperty(const mstrings_t& properties, 
                     const tstring& prop, std::vector<int>& vec);

    bool GetProperty(const mstrings_t& properties, 
                     const tstring& prop, std::set<int>& myset);

    bool GetProperty(const mstrings_t& properties, 
                     const tstring& prop, bool& value);

    bool GetProperty(const mstrings_t& properties, 
                     const tstring& prop, int64_t& value);

    bool GetProperty(const mstrings_t& properties, const tstring& prop, 
                     AudioCodec& codec);

    bool GetProperty(const mstrings_t& properties, const tstring& prop, 
                     AudioConfig& audcfg);

    bool GetProperty(const mstrings_t& properties, const tstring& prop, 
                     ACE_INET_Addr& addr);

    bool GetProperty(const mstrings_t& properties, const tstring& prop,
                     ACE_Time_Value& tv);

    void GetProperties(const mstrings_t& properties, UserAccount& useraccount);

    tstring PrepareIntegerArray(const std::vector<int>& array);

    tstring PrepareIntegerSet(const std::set<int>& myset);

    tstring PrepareString(const tstring& str);

    tstring RebuildString(const tstring& str);

    //@return Number of properties found, -1 on error.
    int ExtractProperties(const tstring& input, mstrings_t& properties);

    //appends a property to a command tstring
    void AppendProperty(const tstring& prop, 
                        const tstring& value, tstring& dest_str);
    void AppendProperty(const tstring& prop, 
                        const std::vector<int>& vecValues, 
                        tstring& dest_str);
    void AppendProperty(const tstring& prop, 
                        const std::set<int>& setValues, 
                        tstring& dest_str);
    void AppendProperty(const tstring& prop, 
                        int64_t value, tstring& dest_str);
    void AppendProperty(const tstring& prop, const AudioCodec& codec, 
                        tstring& dest_str);
    void AppendProperty(const tstring& prop, const AudioConfig& audcfg, 
                        tstring& dest_str);
    void AppendProperty(const tstring& prop, 
                        const ACE_INET_Addr& addr, tstring& dest_str);
    void AppendProperty(const tstring& prop,
                        const ACE_Time_Value& tv, tstring& dest_str);

    bool GetCmdLine(const std::string& input, 
                    std::string& cmd, std::string& remain_input);
    bool GetCmd(const std::string& input, std::string& cmd);
    tstring StripEol(const tstring& input);
} // namespace teamtalk
#endif
