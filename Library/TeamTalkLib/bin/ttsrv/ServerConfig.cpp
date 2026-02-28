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


#include "ServerConfig.h"
#include "ServerUtil.h"

#include "AppInfo.h"
#include "ServerXML.h"
#include "mystd/MyStd.h"
#include "myace/MyACE.h"
#include "myace/MyINet.h"
#include "TeamTalkDefs.h"
#include "teamtalk/Commands.h"
#include "teamtalk/Common.h"
#include "teamtalk/Log.h"
#include "teamtalk/TTAssert.h"
#include "teamtalk/server/ServerChannel.h"

#include <ace/INET_Addr.h>
#include <ace/OS.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <queue>
#include <regex>
#include <sstream>
#include <thread>
#include <vector>

#if defined(UNICODE)
typedef std::wostringstream tostringstream;
#else
using tostringstream = std::ostringstream;
#endif

using namespace std;
using namespace teamtalk;

#if defined(WIN32) && defined(CreateFile)
#undef CreateFile
#endif

bool LoadConfig(teamtalk::ServerXML& xmlSettings, const ACE_TString& cfgfile)
{
    ACE_TString settings_path;
    std::array<ACE_TCHAR, 1024> buf;

    if(cfgfile.empty())
    {
        ACE_TString workdir = ACE_OS::getcwd(buf.data(), buf.size());
        workdir += ACE_DIRECTORY_SEPARATOR_STR;
        settings_path = workdir + ACE_TEXT(TEAMTALK_SETTINGSFILE);
    }
    else settings_path = cfgfile;

    if(ACE_OS::filesize(settings_path.c_str())>0)
    {
        if(!xmlSettings.LoadFile(UnicodeToUtf8(settings_path).c_str()))
        {
            tostringstream osErr;
            if(xmlSettings.HasErrors())
            {
                ACE_TString const errmsg = Utf8ToUnicode(xmlSettings.GetError().c_str());
                osErr << ACE_TEXT("Unable to read ") << settings_path.c_str() << ACE_TEXT(". ") << errmsg.c_str() << endl;
            }
            else
                osErr << ACE_TEXT("Unable to open file ") << settings_path.c_str() << ACE_TEXT(".");
            TT_SYSLOG(osErr.str().c_str());

            return false;
        }
        return true;
    }
    
    if(!xmlSettings.CreateFile(UnicodeToUtf8(settings_path).c_str()))
    {
        tostringstream osErr;
        osErr << ACE_TEXT("Unable to create settings file ") << settings_path.c_str() << ACE_TEXT(".");
        TT_SYSLOG(osErr.str().c_str());
        return false;
    }
    return true;
   
}

void RemoveFacebookLogins(teamtalk::ServerXML& xmlSettings)
{
    // remove all facebook logins
    if (!VersionSameOrLater(Utf8ToUnicode(xmlSettings.GetFileVersion().c_str()), ACE_TEXT("5.2")))
    {
        bool removefb = false;
        bool fbfound = false;
        int index = 0;
        UserAccount ua;
        while (xmlSettings.GetNextUser(index, ua))
        {
            bool const fbpostfix = std::regex_search(ua.username.c_str(), BuildRegex(ACE_TEXT("@facebook.com")));
            if (ua.username == ACE_TEXT("facebook") || fbpostfix)
            {
                fbfound = true;
                if (!removefb)
                {
                    cout << "Facebook login is no longer supported. Remove all Facebook logins.";
                    removefb = PrintGetBool(true);
                    if (!removefb)
                        break;
                }
#if defined(UNICODE)
                std::string fbname = UnicodeToUtf8(ua.username.c_str()).c_str();
#else
                std::string const fbname = ua.username.c_str();
#endif
                cout << "Removed: " << Utf8ToLocal(fbname.c_str()) << endl;
                xmlSettings.RemoveUser(fbname);
            }
            else index++;

            ua = UserAccount();
        }

        // facebook accounts removed, save new version
        if (fbfound)
        {
            xmlSettings.SetFileVersion(TEAMTALK_XML_VERSION);
            xmlSettings.SaveFile();
        }
    }
}

void ConfigureUserAccount(UserAccount user, teamtalk::ServerXML& xmlSettings)
{
    if (user.username.empty())
    {
        cout << "User account has no username. Create anonymous account? ";
        if (!PrintGetBool(false))
            return;
    }

    cout << "Available user types:" << endl;
    cout << "\t1. Default user." << endl;
    cout << "\t2. Administrator." << endl;
    cout << "Select user type:";
    switch(PrintGetInt(1))
    {
    case 2 :
        user.usertype = USERTYPE_ADMIN;
        break;
    case 1 :
    default :
    {
        user.usertype = USERTYPE_DEFAULT;
        int userrights = USERRIGHT_NONE;
        cout << "Should multiple users be allowed to log in with this user account? ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_MULTI_LOGIN) != 0)?
                         userrights | USERRIGHT_MULTI_LOGIN : userrights & ~USERRIGHT_MULTI_LOGIN;

        cout << "User can change nickname: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_LOCKED_NICKNAME) == 0)?
                         (userrights & ~USERRIGHT_LOCKED_NICKNAME) : (userrights | USERRIGHT_LOCKED_NICKNAME);

        cout << "User can see all other users on server: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_VIEW_ALL_USERS) != 0)?
                         (userrights | USERRIGHT_VIEW_ALL_USERS) : (userrights & ~USERRIGHT_VIEW_ALL_USERS);

        cout << "User can create temporary channels: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_CREATE_TEMPORARY_CHANNEL) != 0)?
                         (userrights | USERRIGHT_CREATE_TEMPORARY_CHANNEL) : (userrights & ~USERRIGHT_CREATE_TEMPORARY_CHANNEL);

        cout << "User can create/modify all channels: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_MODIFY_CHANNELS) != 0)?
                         (userrights | USERRIGHT_MODIFY_CHANNELS) : (userrights & ~USERRIGHT_MODIFY_CHANNELS);

        cout << "User can sent private text messages: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_TEXTMESSAGE_USER) != 0)?
                         (userrights | USERRIGHT_TEXTMESSAGE_USER) : (userrights & ~USERRIGHT_TEXTMESSAGE_USER);

        cout << "User can sent channel text messages: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_TEXTMESSAGE_CHANNEL) != 0)?
                         (userrights | USERRIGHT_TEXTMESSAGE_CHANNEL) : (userrights & ~USERRIGHT_TEXTMESSAGE_CHANNEL);

        cout << "User can broadcast text message to all users: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_TEXTMESSAGE_BROADCAST) != 0)?
                         (userrights | USERRIGHT_TEXTMESSAGE_BROADCAST) : (userrights & ~USERRIGHT_TEXTMESSAGE_BROADCAST);

        cout << "User can kick users off the server: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_KICK_USERS) != 0)?
                         (userrights | USERRIGHT_KICK_USERS) : (userrights & ~USERRIGHT_KICK_USERS);

        cout << "User can ban users from the server: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_BAN_USERS) != 0)?
                         (userrights | USERRIGHT_BAN_USERS) : (userrights & ~USERRIGHT_BAN_USERS);

        cout << "User can move users between channels: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_MOVE_USERS) != 0)?
                         (userrights | USERRIGHT_MOVE_USERS) : (userrights & ~USERRIGHT_MOVE_USERS);

        cout << "User can make other users channel operator: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_OPERATOR_ENABLE) != 0)?
                         (userrights | USERRIGHT_OPERATOR_ENABLE) : (userrights & ~USERRIGHT_OPERATOR_ENABLE);

        cout << "User can upload files to channels: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_UPLOAD_FILES) != 0)?
                         (userrights | USERRIGHT_UPLOAD_FILES) : (userrights & ~USERRIGHT_UPLOAD_FILES);

        cout << "User can download files from channels: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_DOWNLOAD_FILES) != 0)?
                         (userrights | USERRIGHT_DOWNLOAD_FILES) : (userrights & ~USERRIGHT_DOWNLOAD_FILES);

        cout << "User can record conversations in channels that don't allow it: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_RECORD_VOICE) != 0)?
                         (userrights | USERRIGHT_RECORD_VOICE) : (userrights & ~USERRIGHT_RECORD_VOICE);

        cout << "User can update server properties: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_UPDATE_SERVERPROPERTIES) != 0)?
                         (userrights | USERRIGHT_UPDATE_SERVERPROPERTIES) : (userrights & ~USERRIGHT_UPDATE_SERVERPROPERTIES);

        cout << "User can transmit voice (microphone input) packets through server: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_VOICE) != 0)?
                         (userrights | USERRIGHT_TRANSMIT_VOICE) : (userrights & ~USERRIGHT_TRANSMIT_VOICE);

        cout << "User can transmit video (webcam) packets through server: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_VIDEOCAPTURE) != 0)?
                         (userrights | USERRIGHT_TRANSMIT_VIDEOCAPTURE) : (userrights & ~USERRIGHT_TRANSMIT_VIDEOCAPTURE);

        cout << "User can transmit desktop sharing packets through server: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_DESKTOP) != 0)?
                         (userrights | USERRIGHT_TRANSMIT_DESKTOP) : (userrights & ~USERRIGHT_TRANSMIT_DESKTOP);

        cout << "User can transmit remote desktop access packets through server: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_DESKTOPINPUT) != 0)?
                         (userrights | USERRIGHT_TRANSMIT_DESKTOPINPUT) : (userrights & ~USERRIGHT_TRANSMIT_DESKTOPINPUT);

        cout << "User can transmit audio file (wav, mp3 files) packets through server: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO) != 0)?
                         (userrights | USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO) : (userrights & ~USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO);

        cout << "User can transmit video file (avi, wmv files) packets through server: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO) != 0)?
                         (userrights | USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO) : (userrights & ~USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO);

        user.userrights = userrights;

        cout << "Limit user's audio codec to a specific bit rate (in kbps), 0 = no limit: ";
        user.audiobpslimit = PrintGetInt(0)*1000;

        cout << "Limit number of commands user can issue (to prevent flooding)?";
        if(PrintGetBool((user.abuse.n_cmds != 0) && (user.abuse.cmd_msec != 0)))
        {
            cout << "Number of commands to allow within specified time frame: ";
            user.abuse.n_cmds = PrintGetInt(user.abuse.n_cmds);
            cout << "User can enter " << user.abuse.n_cmds << " commands within this number of msec: ";
            user.abuse.cmd_msec = PrintGetInt(user.abuse.cmd_msec);
        }
    }
    break;
    }

    xmlSettings.RemoveUser(UnicodeToUtf8(user.username).c_str());
    xmlSettings.AddNewUser(user);
}

#if defined(ENABLE_TEAMTALKPRO)

static std::string GetMyIPAddress()
{
    std::string xml;
    switch (HttpGetRequest(WEBLOGIN_PING_URL, xml))
    {
    case 1 :
    {
        teamtalk::XMLDocument xmldoc("teamtalk", "1.0");
        if (xmldoc.Parse(xml))
        {
            return xmldoc.GetValue(false, "teamtalk/ipaddress", "");
        }
    }
    }
    return {};
}

static void SubmitSpamBotServer(teamtalk::ServerXML& xmlSettings)
{
    cout << "Do you wish to submit your TeamTalk Pro server for SpamBot monitoring? ";
    if (!PrintGetBool(true))
        return;

    while (!LoginBearWare(xmlSettings))
    {
        cout << "Try again? ";
        if (!PrintGetBool(true))
            break;
    }

    auto myipaddr = GetMyIPAddress();
    std::cout << "What is the public IP-address of your server? ";
    myipaddr = PrintGetString(myipaddr);
    std::cout << "What is the TCP port of your server? ";
    int const tcpport = PrintGetInt(xmlSettings.GetHostTcpPort(DEFAULT_TCPPORT));
    std::cout << "What is the UDP port of your server? ";
    int const udpport = PrintGetInt(xmlSettings.GetHostUdpPort(DEFAULT_UDPPORT));
    std::cout << "Is the server encrypted? ";
    bool const encrypted = PrintGetBool(!xmlSettings.GetCertificateFile().empty());

    std::string bearwareid;
    std::string token;
    xmlSettings.GetBearWareWebLogin(bearwareid, token);
    std::map<std::string, std::string> formdata;
    formdata["client"] = TEAMTALK_LIB_NAME;
    formdata["version"] = TEAMTALK_VERSION;
    formdata["action"] = "spambotsubmit"; // TODO: make endpoint
    formdata["username"] = bearwareid;
    formdata["token"] = token;
    // TODO: query user for input
    formdata["ipaddress"] = myipaddr;
    formdata["tcpport"] = std::to_string(tcpport);
    formdata["udpport"] = std::to_string(udpport);
    formdata["encrypted"] = encrypted ? "true" : "false";

    std::string xml;
    switch (HttpPostRequest(SPAMBOT_SUBMIT_URL, formdata, xml))
    {
    case -1 :
    case 0 :
    case 1 :
    }
}

static void RemoveSpamBotServer(teamtalk::ServerXML& xmlSettings)
{
    cout << "Do you wish to remove your TeamTalk Pro server from SpamBot monitoring? ";
    if (!PrintGetBool(true))
        return;

    while (!LoginBearWare(xmlSettings))
    {
        cout << "Try again? ";
        if (!PrintGetBool(true))
            break;
    }

    auto myipaddr = GetMyIPAddress();
    std::cout << "What is the registered IP-address of your server? ";
    myipaddr = PrintGetString(myipaddr);
    std::cout << "What is the TCP port of your server? ";
    int const tcpport = PrintGetInt(xmlSettings.GetHostTcpPort(DEFAULT_TCPPORT));

    std::string bearwareid;
    std::string token;
    xmlSettings.GetBearWareWebLogin(bearwareid, token);
    std::map<std::string, std::string> formdata;
    formdata["client"] = TEAMTALK_LIB_NAME;
    formdata["version"] = TEAMTALK_VERSION;
    formdata["action"] = "spambotremove"; // TODO: make endpoint
    formdata["username"] = bearwareid;
    formdata["token"] = token;
    formdata["ipaddress"] = myipaddr;
    formdata["tcpport"] = std::to_string(tcpport);

    std::string xml;
    switch (HttpPostRequest(SPAMBOT_SUBMIT_URL, formdata, xml))
    {
    case -1 :
    case 0 :
    case 1 :
    }
}

static void ConfigureSpamBotUserAccount(UserAccount user, teamtalk::ServerXML& xmlSettings)
{
    cout << "Creating SpamBot web-login account." << endl;
    user.username = ACE_TEXT( WEBLOGIN_SPAMBOT_USERNAME );
    user.passwd = ACE_TEXT("");

    cout << "Available user types:" << endl;
    cout << "\t1. Default user." << endl;
    cout << "\t2. Administrator." << endl;
    cout << "Select user type:";
    switch(PrintGetInt(1))
    {
    case 2 :
        user.usertype = USERTYPE_ADMIN;
        break;
    case 1 :
    default :
    {
        user.usertype = USERTYPE_DEFAULT;
        int userrights = USERRIGHT_NONE;

        cout << "User can see all other users on server: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_VIEW_ALL_USERS) != 0)?
                         (userrights | USERRIGHT_VIEW_ALL_USERS) : (userrights & ~USERRIGHT_VIEW_ALL_USERS);

        cout << "User can kick users off the server: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_KICK_USERS) != 0)?
                         (userrights | USERRIGHT_KICK_USERS) : (userrights & ~USERRIGHT_KICK_USERS);

        cout << "User can ban users from the server: ";
        userrights = PrintGetBool((USERRIGHT_DEFAULT & USERRIGHT_BAN_USERS) != 0)?
                         (userrights | USERRIGHT_BAN_USERS) : (userrights & ~USERRIGHT_BAN_USERS);

        user.userrights = userrights;
    }
    break;
    }

    xmlSettings.RemoveUser(UnicodeToUtf8(user.username).c_str());
    xmlSettings.AddNewUser(user);

    SubmitSpamBotServer(xmlSettings);
}

void ConfigureEncryption(bool& certverifyonce, int& certdepth,
                         ACE_TString& cadir, ACE_TString& keyfile,
                         ACE_TString& certfile, ACE_TString& cafile,
                         bool& certverifypeer)
{
    cout << "Should server run in encrypted mode? ";
    if(PrintGetBool((!certfile.empty()) && (!keyfile.empty())))
    {
        while (true)
        {
            cout << "Server certificate file (in PEM format) for encryption: ";
            certfile = LocalToUnicode(PrintGetString(UnicodeToLocal(certfile).c_str()).c_str());
            if (ACE_OS::filesize(certfile.c_str()) <= 0)
                cerr << "File " << certfile << " not found!" << endl;
            else break;
        }

        while (true)
        {
            cout << "Server private key file (in PEM format) for encryption: ";
            keyfile = LocalToUnicode(PrintGetString(UnicodeToLocal(keyfile).c_str()).c_str());
            if (ACE_OS::filesize(keyfile.c_str()) <= 0)
                cerr << "File " << keyfile << " not found!" << endl;
            else break;
        }

        cout << "Should server verify client's certificate with provided Certificate Authority (CA) certificate? ";
        if ((certverifypeer = PrintGetBool(certverifypeer)))
        {
            cout << "File containing Certificate Authority (CA) certificate (in PEM format): ";
            cafile = LocalToUnicode(PrintGetString(UnicodeToLocal(cafile).c_str()).c_str());

            cout << "Directory containing Certificate Authority (CA) certificates (leave blank to only use single CA file): ";
            cadir = LocalToUnicode(PrintGetString(UnicodeToLocal(cadir).c_str()).c_str());

            cout << "Should client's certificate only be verified initially? ";
            certverifyonce = PrintGetBool(certverifyonce);

            cout << "Max depth in certificate chain during the verification process? ";
            certdepth = (PrintGetInt(static_cast<int>(certdepth)) != 0);
        }
        else
        {
            cafile.clear();
            cadir.clear();
            certverifypeer = false;
            certverifyonce = true;
            certdepth = 0;
        }
    }
    else
    {
        certfile.clear();
        keyfile.clear();
        cafile.clear();
        cadir.clear();
        certverifypeer = false;
        certverifyonce = true;
        certdepth = 0;
    }
}
#endif /* ENABLE_TEAMTALKPRO */


void RunWizard(teamtalk::ServerXML& xmlSettings)
{
    cout << TEAMTALK_NAME << " " << TEAMTALK_VERSION_FRIENDLY << " configurator" << endl;
    cout << endl;

    cout << "Do you want to configure your " TEAMTALK_NAME "? ";
    if(!PrintGetBool(true))
        return;

    cout << endl;
    cout << "Configuring file: " << xmlSettings.GetFileName() << endl;

    ACE_TString servername;
    ACE_TString motd;
    ACE_TString filesroot;
    std::vector<std::string> bindips;
#if defined(ENABLE_TEAMTALKPRO)
    ACE_TString certfile;
    ACE_TString keyfile;
    ACE_TString cafile;
    ACE_TString cadir;
    bool certverifypeer = false;
    bool certverifyonce = false;
    int certdepth = 0;
#endif
    int maxusers = 0;
    int max_logins_per_ip = 0;
    bool autosave = true;
    int64_t diskquota = 0;
    int64_t maxdiskusage = 0;
    int64_t log_maxsize = 0;
    int tcpport = DEFAULT_TCPPORT;
    int udpport = DEFAULT_UDPPORT;
    int max_login_attempts = 0;
    int logindelay = 0;
    bool upnp = false;

    servername = Utf8ToUnicode(xmlSettings.GetServerName().c_str());
    motd = Utf8ToUnicode(xmlSettings.GetMessageOfTheDay().c_str());
    filesroot = Utf8ToUnicode(xmlSettings.GetFilesRoot().c_str());
    bindips = xmlSettings.GetBindIPs();
    tcpport = xmlSettings.GetHostTcpPort(DEFAULT_TCPPORT);
    udpport = xmlSettings.GetHostUdpPort(DEFAULT_UDPPORT);

    maxusers = xmlSettings.GetMaxUsers(MAX_USERS);
    log_maxsize = xmlSettings.GetServerLogMaxSize();
    autosave = xmlSettings.GetAutoSave();
    diskquota = xmlSettings.GetDefaultDiskQuota();
    maxdiskusage = xmlSettings.GetMaxDiskUsage();
    max_login_attempts = xmlSettings.GetMaxLoginAttempts();
    max_logins_per_ip = xmlSettings.GetMaxLoginsPerIP();
    logindelay = xmlSettings.GetLoginDelay();
    upnp = xmlSettings.GetUPnP();

#if defined(ENABLE_TEAMTALKPRO)
    certfile = Utf8ToUnicode(xmlSettings.GetCertificateFile().c_str());
    keyfile = Utf8ToUnicode(xmlSettings.GetPrivateKeyFile().c_str());
    cafile = Utf8ToUnicode(xmlSettings.GetCertificateAuthFile().c_str());
    cadir = Utf8ToUnicode(xmlSettings.GetCertificateAuthDir().c_str());
    certverifypeer = xmlSettings.GetCertificateVerify(false);
    certverifyonce = xmlSettings.GetCertificateVerifyOnce(true);
    certdepth = xmlSettings.GetCertificateVerifyDepth(0);
#endif

    cout << endl;
    cout << "Ready to configure " << TEAMTALK_NAME << " settings." << endl;

#if !defined(WIN32)
    ACE_TCHAR *s = nullptr;
    int utf8_mode = 0;

    if ((((s = getenv("LC_ALL")) != nullptr)   && (*s != 0)) ||
            (((s = getenv("LC_CTYPE")) != nullptr) && (*s != 0)) ||
            (((s = getenv("LANG")) != nullptr)     && (*s != 0))) {
        if (strstr(s, "UTF-8") != nullptr)
            utf8_mode = 1;
    }
    if(utf8_mode == 0)
        cout << "Warning: UTF-8 not enabled. Please stick to English characters!" << endl;
#endif
    //cout << "Value in parantesis will be used if no input is specified." << endl;
    cout << endl;
    cout << TEAMTALK_NAME << " name: ";
    servername = LocalToUnicode(PrintGetString(UnicodeToLocal(servername).c_str()).c_str());
    cout << "Message of the Day: ";
    motd = LocalToUnicode(PrintGetString(UnicodeToLocal(motd).c_str()).c_str());
    cout << "Maximum users allowed on server: ";
    maxusers = PrintGetInt(maxusers);
    if(maxusers > MAX_USERS)
    {
        cout << "Maximum allowed users is " << MAX_USERS << " and the value has been reduced accordingly." << endl;
        maxusers = MAX_USERS;
    }
    cout << "Server should automatically save changes: ";
    autosave = PrintGetBool(autosave);

    cout << "Enable file sharing: ";
    if(PrintGetBool(!filesroot.empty()))
    {
        std::array<ACE_TCHAR, 1024> buff;
        ACE_OS::getcwd(buff.data(), buff.size());
#ifdef WIN32
        cout << "Directory for file storage, e.g. C:\\MyServerFiles: ";
#else
        cout << "Directory for file storage, e.g. /home/bill/srv1/files: ";
#endif
        filesroot = LocalToUnicode(PrintGetString(UnicodeToLocal(filesroot).c_str()).c_str());
        if(!filesroot.empty() && ACE_OS::chdir(filesroot.c_str()) == 0)
        {
            cout << "Disk quota (in KBytes) per channel, 0 = disabled: ";
            diskquota = PrintGetInt64(diskquota/1024)*1024;
            cout << "Maximum disk usage (in KBytes) for storing files: ";
            maxdiskusage = PrintGetInt64(maxdiskusage/1024)*1024;
        }
        else
        {
            cout << "Directory not found for file storage. File sharing disabled." << endl;
            filesroot.clear();
            diskquota = 0;
        }
        ACE_OS::chdir(buff.data());
    }
    else
    {
        filesroot.clear();
        diskquota = 0;
    }

    cout << "Log server activity: ";
    log_maxsize = PrintGetBool(log_maxsize != 0)? (log_maxsize!=0)? log_maxsize : -1 : 0;

    cout << "Server should bind to the following TCP port: ";
    tcpport = PrintGetInt(tcpport);
    cout << "Server should bind to the following UDP port: ";
    udpport = PrintGetInt(udpport);
    cout << "Enable UPnP port forwarding (automatically open TCP/UDP ports on router)? ";
    upnp = PrintGetBool(upnp);
    cout << "Bind to specific IP-addresses? (required for IPv6) ";
    if (PrintGetBool(!bindips.empty()))
    {
        while (true)
        {
            if (!bindips.empty())
            {
                cout << "Currently binding to IP-addresses:" << endl;
                for (const auto& ip : bindips)
                {
                    cout << "\t- " << (ip.empty()? "0.0.0.0" : ip) << endl;
                }

                cout << "Specify additional IP-addresses? ";
                if (!PrintGetBool(bindips.empty()))
                    break;
            }

            cout << "Specify the IP-address to bind to (IPv6 type \"::\" for all interfaces): " << endl;
            std::string const ip = PrintGetString("0.0.0.0");
            bindips.push_back(ip);
        }
    }
    else
    {
        bindips.clear();
    }

#if defined(ENABLE_TEAMTALKPRO)
    ConfigureEncryption(certverifyonce, certdepth, cadir, keyfile, certfile, cafile, certverifypeer);
#endif

    cout << endl << "User authentication." << endl;

    cout << endl;
    cout << "User account administration." << endl;
    int input = 0;
    enum UserAccountOptions
    {
        LIST_USERACCOUNTS = 1,
        CREATE_USERACCOUNT,
#if defined(ENABLE_TEAMTALKPRO)
        CREATE_USERACCOUNT_BEARWARE,
        CREATE_USERACCOUNT_SPAMBOT,
#endif
        DELETE_USERACCOUNT,
        QUIT_USERACCOUNTS
    };

#if defined(ENABLE_TEAMTALKPRO)
    ACE_CString url;
    std::string xml;
#endif

    while(input != QUIT_USERACCOUNTS)
    {
        UserAccount user;
        std::string tmp;
        int count = 0;
        while(xmlSettings.GetNextUser(count, user))count++;
        user = UserAccount();
        cout << endl;
        cout << "Currently there's " << count << " user accounts." << endl;
        cout << LIST_USERACCOUNTS << ") List user accounts." << endl;
        cout << CREATE_USERACCOUNT << ") Create new user account." << endl;
#if defined(ENABLE_TEAMTALKPRO)
        cout << CREATE_USERACCOUNT_BEARWARE << ") Create BearWare.dk web-login account." << endl;
#endif
        cout << DELETE_USERACCOUNT << ") Delete user account." << endl;
        cout << QUIT_USERACCOUNTS << ") Quit and proceed server configuration." << endl;
        cout << "Select option: ";
        switch( (input = PrintGetInt(QUIT_USERACCOUNTS)) )
        {
        case LIST_USERACCOUNTS :
            cout << endl;
            cout << "Active user accounts:" << endl << endl;
            count = 0;
            while(xmlSettings.GetNextUser(count, user))
            {
                cout << "User #" << count + 1 << endl;
                cout << "Username: " << user.username << endl;
                cout << "Password: " << user.passwd << endl;
                switch(user.usertype)
                {
                case USERTYPE_DEFAULT :
                    cout << "User type: Default user" << endl;
                    break;
                case USERTYPE_ADMIN :
                    cout << "User type: Administrator" << endl;
                    break;
                default :
                    cout << "User type: Unknown" << endl;
                }
                cout << endl;
                count++;
            }
            break;
        case CREATE_USERACCOUNT :
            cout << "Creating new user account." << endl;
            cout << "Type username: ";
            user.username = LocalToUnicode(PrintGetString("").c_str());
            cout << "Type password: ";
            user.passwd = LocalToUnicode(PrintGetString("").c_str());

            ConfigureUserAccount(user, xmlSettings);
            break;
#if defined(ENABLE_TEAMTALKPRO)
        case CREATE_USERACCOUNT_BEARWARE :
            cout << "Testing BearWare.dk web-login service..." << endl;

            url = WEBLOGIN_URL;
            url += "client=" TEAMTALK_LIB_NAME;
            url += "&version=" TEAMTALK_VERSION;
            url += "&ping=true";
            switch(HttpGetRequest(url, xml))
            {
            case -1 :
                cout << "Failed to query " << WEBLOGIN_URL << endl;
                break;
            case 0 :
                cout << "Invalid response from BearWare.dk login service" << endl;
                break;
            case 1 :
                cout << "Got valid response from BearWare.dk login service. Continuing..." << endl;
                break;
            }

            while (!LoginBearWare(xmlSettings))
            {
                cout << "Try again? ";
                if (!PrintGetBool(true))
                    break;
            }

            cout << "Creating BearWare.dk web-login account." << endl;
            user.username = ACE_TEXT( WEBLOGIN_BEARWARE_USERNAME );
            user.passwd = ACE_TEXT("");
            ConfigureUserAccount(user, xmlSettings);
            break;
        case CREATE_USERACCOUNT_SPAMBOT :
            ConfigureSpamBotUserAccount(user, xmlSettings);
            break;
#endif /* ENABLE_TEAMTALKPRO */
        case DELETE_USERACCOUNT :
        {
            cout << "Type the username of the account to delete: ";
            tmp = PrintGetString("");
            ACE_TString const utf8 = LocalToUnicode(tmp.c_str());
            if(xmlSettings.RemoveUser(UnicodeToUtf8(utf8).c_str()))
                cout << "1 user deleted." << endl;
            else
                cout << "User not found." << endl;
            break;
        }
        case QUIT_USERACCOUNTS :
            if(count == 0)
            {
                cout << "There's no active user account. No user will be able to log in!" << endl;
                cout << "Are you sure you want to exit user account? ";
                if(!PrintGetBool(false))
                    input = 0;
            }
            break;
        }
    }

    cout << endl;

    cout << "Maximum number of invalid login attempts before banning user, 0 = disabled: ";
    max_login_attempts = PrintGetInt(max_login_attempts);

    cout << "Maximum number of logins per IP-address, 0 = disabled: ";
    max_logins_per_ip = PrintGetInt(max_logins_per_ip);

    cout << "Delay in milliseconds before an IP-address can make another login, 0 = disabled: ";
    logindelay = PrintGetInt(logindelay);

    cout << endl << endl;
    cout << "Your " << TEAMTALK_NAME << " is now configured with the following settings:" << endl;
    cout << endl;
    cout << TEAMTALK_NAME << " name: " << servername << endl;
    cout << "Message of the Day: " << motd << endl;
    cout << "Maximum users allowed: " << maxusers << endl;
    cout << "Server will" << (autosave? "" : " not") << " automatically save changes." << endl;

    if(!filesroot.empty())
    {
        cout << "File sharing enabled." << endl;
        cout << "Root directory for root channel: " << filesroot << endl;
        cout << "Disk quota in bytes per channel: " << diskquota/1024 << " KBytes" << endl;
        cout << "Maximum disk usage for storing files: " << maxdiskusage/1024 << " KBytes" <<endl;
    }
    else
        cout << "File sharing disabled." << endl;

    if(log_maxsize != 0)
        cout << "Server logging enabled." << endl;
    else
        cout << "Server logging disabled." << endl;

    cout << "Server will bind to TCP port " << tcpport << endl;
    cout << "Server will bind to UDP port " << udpport << endl;
    cout << "UPnP port forwarding: " << (upnp ? "enabled" : "disabled") << endl;
    cout << "Server will bind to IP-address: " << endl;
    for (const auto& ip : bindips)
        cout << "\t- " << ip << endl;

    cout << endl;
    if(max_login_attempts != 0)
        cout << "Max incorrect login attempts before banning user: " << max_login_attempts << endl;
    else
        cout << "Max incorrect login attempts before banning user: " << "disabled" << endl;
    if(max_logins_per_ip != 0)
        cout << "Max logins per IP-address: " << max_logins_per_ip << endl;
    else
        cout << "Max logins per IP-address: " << "disabled" << endl;

    cout << "Users wait for " << logindelay << " msec before attempting login again." << endl;

#if defined(ENABLE_TEAMTALKPRO)
    cout << "Server certificate file for encryption: " << certfile << endl;
    cout << "Server private key file for encryption: " << keyfile << endl;
    cout << "Server should verify client's certificate: " << (certverifypeer? "true" : "false") << endl;
    if (certverifypeer && (!cafile.empty()))
        cout << "Certificate Authority (CA) file: " << cafile << endl;
    if (certverifypeer && (!cadir.empty()))
        cout << "Certificate Authority (CA) directory: " << cadir << endl;
#endif

    int count = 0;
    UserAccount user;
    while(xmlSettings.GetNextUser(count, user))count++;
    cout << "Active user accounts: " << count << endl;

    cout << endl;

    cout << "Save these settings? ";
    if(PrintGetBool(true))
    {
        xmlSettings.SetServerName(UnicodeToUtf8(servername).c_str());
        xmlSettings.SetMessageOfTheDay(UnicodeToUtf8(motd).c_str());
        xmlSettings.SetMaxUsers(maxusers);
        xmlSettings.SetAutoSave(autosave);
        xmlSettings.SetFilesRoot(UnicodeToUtf8(filesroot).c_str());
        xmlSettings.SetMaxDiskUsage(maxdiskusage);
        xmlSettings.SetDefaultDiskQuota(diskquota);
        xmlSettings.SetBindIPs(bindips);
        xmlSettings.SetHostTcpPort(tcpport);
        xmlSettings.SetHostUdpPort(udpport);
        xmlSettings.SetUPnP(upnp);
#if defined(ENABLE_TEAMTALKPRO)
        xmlSettings.SetCertificateFile(UnicodeToUtf8(certfile).c_str());
        xmlSettings.SetPrivateKeyFile(UnicodeToUtf8(keyfile).c_str());
        xmlSettings.SetCertificateAuthFile(UnicodeToUtf8(cafile).c_str());
        xmlSettings.SetCertificateAuthDir(UnicodeToUtf8(cadir).c_str());
        xmlSettings.SetCertificateVerify(certverifypeer);
        xmlSettings.SetCertificateVerifyOnce(certverifyonce);
        xmlSettings.SetCertificateVerifyDepth(certdepth);
#endif
        xmlSettings.SetServerLogMaxSize(log_maxsize);
        xmlSettings.SetMaxLoginAttempts(max_login_attempts);
        xmlSettings.SetMaxLoginsPerIP(max_logins_per_ip);
        xmlSettings.SetLoginDelay(logindelay);
        xmlSettings.SaveFile();

        cout << "Changes saved." << endl;
        cout << endl;
        cout << "If your " << TEAMTALK_NAME << " is currently running the settings will not take" << endl;
        cout << "effect until you restart the server." << endl;
        cout << endl;
    }
    else
        cout << "Changes discarded." << endl;
}

bool ReadServerProperties(teamtalk::ServerXML& xmlSettings,
                          teamtalk::ServerSettings& properties,
                          teamtalk::statchannels_t& channels)
{
    properties.servername = Utf8ToUnicode(xmlSettings.GetServerName().c_str());
    properties.motd = Utf8ToUnicode(xmlSettings.GetMessageOfTheDay().c_str());
    properties.maxusers = xmlSettings.GetMaxUsers(MAX_USERS);
    properties.max_logins_per_ipaddr = xmlSettings.GetMaxLoginsPerIP();
    properties.maxloginattempts = xmlSettings.GetMaxLoginAttempts();
    properties.logindelay = xmlSettings.GetLoginDelay();
    properties.usertimeout = xmlSettings.GetUserTimeout();
    properties.filesroot = Utf8ToUnicode(xmlSettings.GetFilesRoot().c_str());
    properties.diskquota = xmlSettings.GetDefaultDiskQuota();
    properties.maxdiskusage = xmlSettings.GetMaxDiskUsage();
    properties.voicetxlimit = xmlSettings.GetVoiceTxLimit();
    properties.videotxlimit = xmlSettings.GetVideoCaptureTxLimit();
    properties.mediafiletxlimit = xmlSettings.GetMediaFileTxLimit();
    properties.desktoptxlimit = xmlSettings.GetDesktopTxLimit();
    properties.totaltxlimit = xmlSettings.GetTotalTxLimit();
    properties.autosave = xmlSettings.GetAutoSave();
    properties.logevents = xmlSettings.GetServerLogEvents(SERVERLOGEVENT_DEFAULT);

    u_short const tcpport = xmlSettings.GetHostTcpPort(DEFAULT_TCPPORT);
    u_short const udpport = xmlSettings.GetHostUdpPort(DEFAULT_UDPPORT);
    std::vector<std::string> bindips = xmlSettings.GetBindIPs();
    if (bindips.empty())
        bindips.emplace_back("");
    for (const auto& ip : bindips)
    {
        if(!ip.empty())
        {
#if defined(WIN32)
            ACE_INET_Addr tcpaddr(tcpport, Utf8ToUnicode(ip.c_str()).c_str());
            ACE_INET_Addr udpaddr(udpport, Utf8ToUnicode(ip.c_str()).c_str());
#else
            ACE_INET_Addr const tcpaddr(tcpport, Utf8ToUnicode(ip.c_str()));
            ACE_INET_Addr const udpaddr(udpport, Utf8ToUnicode(ip.c_str()));
#endif
            properties.tcpaddrs.push_back(tcpaddr);
            properties.udpaddrs.push_back(udpaddr);
        }
        else
        {
            properties.tcpaddrs.emplace_back(tcpport);
            properties.udpaddrs.emplace_back(udpport);
        }
    }

    xmlSettings.GetStaticChannels(channels);

    return true;
}

bool SaveServerProperties(teamtalk::ServerXML& xmlSettings, const teamtalk::ServerSettings& properties,
                          const teamtalk::statchannels_t& channels)
{
    xmlSettings.SetServerName(UnicodeToUtf8(properties.servername).c_str());
    xmlSettings.SetMessageOfTheDay(UnicodeToUtf8(properties.motd).c_str());
    xmlSettings.SetAutoSave(properties.autosave);
    xmlSettings.SetMaxUsers(properties.maxusers);
    xmlSettings.SetMaxLoginAttempts(properties.maxloginattempts);
    xmlSettings.SetMaxLoginsPerIP(properties.max_logins_per_ipaddr);
    xmlSettings.SetLoginDelay(properties.logindelay);
    xmlSettings.SetUserTimeout(properties.usertimeout);
    xmlSettings.SetVoiceTxLimit(properties.voicetxlimit);
    xmlSettings.SetVideoCaptureTxLimit(properties.videotxlimit);
    xmlSettings.SetMediaFileTxLimit(properties.mediafiletxlimit);
    xmlSettings.SetDesktopTxLimit(properties.desktoptxlimit);
    xmlSettings.SetTotalTxLimit(properties.totaltxlimit);
    TTASSERT(!properties.tcpaddrs.empty());
    if (!properties.tcpaddrs.empty())
        xmlSettings.SetHostTcpPort(properties.tcpaddrs[0].get_port_number());
    TTASSERT(!properties.udpaddrs.empty());
    if (!properties.udpaddrs.empty())
        xmlSettings.SetHostUdpPort(properties.udpaddrs[0].get_port_number());

    xmlSettings.SetMaxDiskUsage(properties.maxdiskusage);
    xmlSettings.SetDefaultDiskQuota(properties.diskquota);
    xmlSettings.SetFilesRoot(UnicodeToUtf8(properties.filesroot).c_str());
    xmlSettings.SetServerLogEvents(properties.logevents);
    xmlSettings.SetStaticChannels(channels);

    return xmlSettings.SaveFile();
}


#if defined(ENABLE_TEAMTALKPRO)
bool SetupEncryption(teamtalk::ServerNode& servernode, teamtalk::ServerXML& xmlSettings)
{
    ACE_TString certfile;
    ACE_TString privfile;
    ACE_TString cafile;
    ACE_TString cadir;

    certfile = Utf8ToUnicode(xmlSettings.GetCertificateFile().c_str());
    privfile = Utf8ToUnicode(xmlSettings.GetPrivateKeyFile().c_str());
    cafile = Utf8ToUnicode(xmlSettings.GetCertificateAuthFile().c_str());
    cadir = Utf8ToUnicode(xmlSettings.GetCertificateAuthDir().c_str());

    ACE_SSL_Context *context = servernode.SetupEncryptionContext();
    if (context == nullptr)
    {
        TT_SYSLOG("Failed to setup encryption context.");
        return false;
    }

    if (context->set_mode(ACE_SSL_Context::SSLv23) < 0)
        return false;

    if ((!certfile.empty()) && (!privfile.empty()))
    {
        if (context->certificate(UnicodeToLocal(certfile).c_str(), SSL_FILETYPE_PEM) < 0)
        {
            TT_SYSLOG("Failed to load certificate file. Check the settings file.");
            return false;
        }

        if (context->private_key(UnicodeToLocal(privfile).c_str(), SSL_FILETYPE_PEM) < 0)
        {
            TT_SYSLOG("Failed to load private key file. Check the settings file.");
            return false;
        }
    }

    if ((!cafile.empty()) || (!cadir.empty()))
    {
        if (context->load_trusted_ca((!cafile.empty()) ? UnicodeToLocal(cafile).c_str() :  nullptr,
                                     (!cadir.empty()) ? UnicodeToLocal(cadir).c_str() : nullptr, false) < 0)
        {
            TT_SYSLOG("Failed to load CA file. Check the settings file.");
            return false;
        }
    }

    context->set_verify_peer(static_cast<int>(xmlSettings.GetCertificateVerify(false)),
                             static_cast<int>(xmlSettings.GetCertificateVerifyOnce(true)),
                             xmlSettings.GetCertificateVerifyDepth(0));
    return true;
}

bool HasBearWareWebLogin(teamtalk::ServerXML& xmlSettings)
{
    int i = 0;
    UserAccount ua;
    while (xmlSettings.GetNextUser(i++, ua))
    {
        if (ua.username == ACE_TEXT(WEBLOGIN_BEARWARE_USERNAME))
            return true;

        const ACE_TString BWREGEX = ACE_TEXT(WEBLOGIN_BEARWARE_POSTFIX) + ACE_TString(ACE_TEXT("$"));
        if (std::regex_search(ua.username.c_str(), BuildRegex(BWREGEX.c_str())))
            return true;
    }
    return false;
}

bool LoginBearWare(teamtalk::ServerXML& xmlSettings)
{
    std::string bwidUtf8;
    std::string tokenUtf8;
    xmlSettings.GetBearWareWebLogin(bwidUtf8, tokenUtf8);
    ACE_TString bwid = Utf8ToUnicode(bwidUtf8.c_str());
    ACE_TString token = Utf8ToUnicode(tokenUtf8.c_str());
    static auto sleepTimeSec = 1ul;
    while (token.empty())
    {
        cout << TEAMTALK_NAME << " requires a BearWare.dk WebLogin" << endl;
        cout << "that is authorized for use." << endl;
        cout << endl;
        cout << "Please provide your credentials for BearWare.dk WebLogin." << endl;
        cout << endl;
        cout << "Type username: ";
        bwid = LocalToUnicode(PrintGetString(UnicodeToLocal(bwid).c_str()).c_str());
        cout << "Type password: ";
        ACE_TString const passwd = LocalToUnicode(PrintGetPassword("").c_str());
        ACE_TString newtoken;
        ACE_TString loginid;
        switch (LoginBearWareAccount(bwid, passwd, newtoken, loginid))
        {
        case WEBLOGIN_SUCCESS :
            cout << endl << "Login successful." << endl << endl;
            cout << "To avoid providing your credentials every time the server is started" << endl;
            cout << "it is recommended to store your access token in the server's configuration" << endl;
            cout << "file." << endl << endl;
            cout << "Store access token in " << xmlSettings.GetFileName() << "? ";
            if (PrintGetBool(true))
            {
                xmlSettings.SetBearWareWebLogin(UnicodeToUtf8(loginid).c_str(), UnicodeToUtf8(newtoken).c_str());
                xmlSettings.SaveFile();
#if !defined(WIN32)
                chmod(xmlSettings.GetFileName().c_str(), S_IRUSR | S_IWUSR);
                std::cout << "Changed file permissions to 600 on " << xmlSettings.GetFileName() << std::endl;
#endif
                std::cout << "The token will be valid as long as you do not change the password of your" << std::endl;
                std::cout << "BearWare.dk WebLogin account." << std::endl;
            }
            bwid = loginid;
            token = newtoken.c_str();
            break;
        case WEBLOGIN_SERVER_UNAVAILABLE :
        case WEBLOGIN_SERVER_INCOMPATIBLE :
            cout << "Unable to contact BearWare.dk WebLogin" << endl;
            std::this_thread::sleep_for(std::chrono::seconds(sleepTimeSec *= 2));
            break;
        case WEBLOGIN_FAILED :
            cout << "Login failed. Please try again." << endl;
            std::this_thread::sleep_for(std::chrono::seconds(sleepTimeSec *= 2));
            break;
        }
    }

    tostringstream os;
    os << "Authenticating " << UnicodeToLocal(bwid).c_str();
    TT_SYSLOG(os.str().c_str());
    switch (AuthBearWareAccount(bwid, token))
    {
    case WEBLOGIN_FAILED :
        os.str(ACE_TEXT(""));
        os << "Failed to authenticate BearWare.dk WebLogin: " << UnicodeToLocal(bwid).c_str();
        TT_SYSLOG(os.str().c_str());
        cout << "Reset BearWare.dk WebLogin credentials ";
        if (PrintGetBool(true))
        {
            xmlSettings.SetBearWareWebLogin(UnicodeToUtf8(bwid).c_str(), "");
            xmlSettings.SaveFile();
        }
        std::this_thread::sleep_for(std::chrono::seconds(sleepTimeSec *= 2));
        return false;
    case WEBLOGIN_SERVER_UNAVAILABLE :
    case WEBLOGIN_SERVER_INCOMPATIBLE :
        os.str(ACE_TEXT(""));
        os << "BearWare.dk WebLogin is currently unavailable. Continuing... ";
        TT_SYSLOG(os.str().c_str());
        std::this_thread::sleep_for(std::chrono::seconds(sleepTimeSec *= 2));
        break;
    case WEBLOGIN_SUCCESS :
        break;
    }

    return true;
}
#endif /* ENABLE_TEAMTALKPRO */

bool ConfigureServer(teamtalk::ServerNode& servernode,
                     const teamtalk::ServerSettings& properties,
                     const teamtalk::statchannels_t& channels)
{
    GUARD_OBJ(&servernode, servernode.Lock());

    //load settings
    servernode.SetServerProperties(properties);

    servernode.SetAutoSaving(false);

    if((!properties.filesroot.empty()) && !servernode.SetFileSharing(properties.filesroot))
        TT_SYSLOG("File sharing failed to initialize properly.");

    MakeStaticChannels(servernode, channels);

    if(!servernode.GetRootChannel())
    {
        ChannelProp chanprop;
        chanprop.channelid = 1;
        chanprop.chantype = CHANNEL_DEFAULT | CHANNEL_PERMANENT;
        chanprop.audiocodec.codec = CODEC_OPUS;
        chanprop.audiocodec.opus.samplerate = 48000;
        chanprop.audiocodec.opus.channels = 1;
        chanprop.audiocodec.opus.complexity = 10;
        chanprop.audiocodec.opus.fec = true;
        chanprop.audiocodec.opus.dtx = false;
        chanprop.audiocodec.opus.bitrate = 32000;
        chanprop.audiocodec.opus.vbr = true;
        chanprop.audiocodec.opus.vbr_constraint = false;
        chanprop.audiocodec.opus.frame_size = 1920;
        chanprop.audiocodec.opus.application = 2048; /* OPUS_APPLICATION_VOIP */
        chanprop.audiocodec.opus.frames_per_packet = 1;
#if 0
        chanprop.audiocodec.codec = CODEC_SPEEX;
        chanprop.audiocodec.speex.bandmode = 1;
        chanprop.audiocodec.speex.frames_per_packet = 2;
        chanprop.audiocodec.speex.quality = 4;
        chanprop.audiocodec.speex.sim_stereo = false;
#endif
        chanprop.diskquota = properties.diskquota;
        bool const makeroot = servernode.MakeChannel(chanprop).errorno == TT_CMDERR_SUCCESS;
        TTASSERT(makeroot);
    }

    //don't enable auto-save until everything has been set in the server
    servernode.SetAutoSaving(properties.autosave);

    return true;
}

void ConvertChannels(const teamtalk::serverchannel_t& root,
                     teamtalk::statchannels_t& channels,
                     bool onlystatic/* = false*/)
{
    std::queue< serverchannel_t > sweeper;
    std::queue< int > sweeperid;
    sweeper.push(root);
    sweeperid.push(0);

    int parentid = 0;
    while(!sweeper.empty())
    {
        serverchannel_t const chan = sweeper.front();
        sweeper.pop();
        parentid = sweeperid.front();
        sweeperid.pop();

        ChannelProp const schan = chan->GetChannelProp();
        channels[schan.channelid] = schan;

        ServerChannel::channels_t subs = chan->GetSubChannels();
        for(const auto & sub : subs)
        {
            ACE_TString const path = sub->GetChannelPath().c_str();
            if(!onlystatic || ((sub->GetChannelType() & CHANNEL_PERMANENT) != 0u))
            {
                sweeper.push(sub);
                sweeperid.push(sub->GetChannelID());
            }
        }
    }
}

void MakeStaticChannels(teamtalk::ServerNode& servernode, const teamtalk::statchannels_t& channels)
{
    //Make static channels
    int const root_id = GetRootChannelID(channels);
    if(root_id>0)
    {
        int fileid = 0;

        auto ite = channels.find(root_id);
        ChannelProp chan = ite->second;
        std::queue< ChannelProp > sweeper;
        sweeper.push(chan);

        while(!sweeper.empty())
        {
            chan = sweeper.front();
            sweeper.pop();
            bool const create_chan = servernode.MakeChannel(chan).errorno == TT_CMDERR_SUCCESS;
            TTASSERT(create_chan);
            if(!create_chan)
            {
                tostringstream os;
                os << ACE_TEXT("Failed to create channel #") << chan.channelid << ACE_TEXT(" with name ") << chan.name.c_str();
                TT_SYSLOG(os.str().c_str());
                continue;
            }
            ACE_TString const filesroot = servernode.GetFilesRoot().c_str() + ACE_TString(ACE_DIRECTORY_SEPARATOR_STR);
            for(size_t i=0;i<chan.files.size();i++)
            {
                // add file, even if it doesn't exists. Otherwise
                // a save operation will erase the files from the
                // xml file.
                chan.files[i].fileid = ++fileid;
                chan.files[i].channelid = chan.channelid;
                if(servernode.AddFileToChannel(chan.files[i]).errorno != TT_CMDERR_SUCCESS)
                {
                    tostringstream os;
                    os << ACE_TEXT("Failed to add file ") << chan.files[i].filename.c_str() << ACE_TEXT(" to channel #") << chan.files[i].channelid;
                    TT_SYSLOG(os.str().c_str());
                }

                ACE_TString const rootpath = filesroot + chan.files[i].internalname.c_str();
                if(ACE_OS::filesize(rootpath.c_str())<0)
                {
                    tostringstream os;
                    os << ACE_TEXT("Failed to open channel #") << chan.files[i].channelid << ACE_TEXT(" file: ") << rootpath.c_str();
                    TT_SYSLOG(os.str().c_str());
                }
                else
                {
                    TTASSERT(ACE_OS::filesize(rootpath.c_str()) == chan.files[i].filesize);
                }
            }

            std::ranges::for_each(chan.bans, [&] (const BannedUser& ban)
            { servernode.AddBannedUserToChannel(ban); });

            statchannels_t subs = GetSubChannels(chan.channelid, channels);
            for(ite=subs.begin();ite!=subs.end();ite++)
                sweeper.push(ite->second);
        }
    }
}

static std::vector<ACE_TString> GetFilesInFolder(const ACE_TString& rootdir)
{
    std::vector<ACE_TString> result;
    ACE_DIR* dir = ACE_OS::opendir(rootdir.c_str());
    if (dir == nullptr)
        return result;

    ACE_DIRENT* dirInfo = nullptr;
    while ((dirInfo = ACE_OS::readdir(dir)) != nullptr)
    {
#if !defined(WIN32)
        if (dirInfo->d_type != DT_REG)
            continue;
#endif
        result.emplace_back(dirInfo->d_name);
    }

    ACE_OS::closedir(dir);
    return result;
}

static std::vector<ACE_TString> GetChannelFiles(const teamtalk::statchannels_t& channels)
{
    std::vector<ACE_TString> channelfiles;
    for (const auto& c : channels)
    {
        for (const auto& f : c.second.files)
            channelfiles.push_back(f.internalname);
    }
    return channelfiles;
}

static void RemoveNonmatchingFiles(const ACE_TString& filesroot,
                            const std::vector<ACE_TString>& allfiles,
                            const std::vector<ACE_TString>& keepfiles)
{
    const ACE_TString EXT = CHANNELFILEEXTENSION;
    for (const auto& f : allfiles)
    {
        if (f.length() <= EXT.length() || f.substr(f.length() - EXT.length()) != EXT)
        {
            std::cout << "Skipping file " << UnicodeToUtf8(f).c_str() << std::endl;
            continue;
        }

        if (std::ranges::find(keepfiles, f) == keepfiles.end())
        {
            std::cout << "Unknown file: " << UnicodeToUtf8(f).c_str() << std::endl;
            std::cout << "Should it be removed? ";
            if (PrintGetBool(false))
            {
                const ACE_TString path = filesroot + ACE_DIRECTORY_SEPARATOR_STR + f;
                if (ACE_OS::unlink(path.c_str()) != 0)
                    std::cerr << "Failed to remove file: " << UnicodeToUtf8(path).c_str() << std::endl;
            }
        }
        else
            std::cout << "Keeping file: " << UnicodeToUtf8(f).c_str() << std::endl;
    }
}

void RemoveUnusedFiles(teamtalk::ServerXML& xmlSettings)
{
    std::cout << "Using configuration file: " << xmlSettings.GetFileName() << std::endl;

    ACE_TString const filesroot = Utf8ToUnicode(xmlSettings.GetFilesRoot().c_str());
    if (filesroot.is_empty())
    {
        std::cout << "File storage is currently not enabled. Skipping cleanup." << std::endl;
        return;
    }

    std::cout << "Cleaning files in directory: " << UnicodeToUtf8(filesroot).c_str() << std::endl;

    teamtalk::statchannels_t channels;
    if (!xmlSettings.GetStaticChannels(channels))
        return;

    auto dirfiles = GetFilesInFolder(filesroot);
    auto channelfiles = GetChannelFiles(channels);

    RemoveNonmatchingFiles(filesroot, dirfiles, channelfiles);
}
