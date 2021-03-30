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

#include <ace/OS.h> // undef CreateFile() on Windows

#include "ServerConfig.h"
#include "ServerUtil.h"

#include "AppInfo.h"

#include <teamtalk/Common.h>
#include <teamtalk/Log.h>

#include <sstream>
#include <queue>
#include <iostream>

#if defined(UNICODE)
typedef std::wostringstream tostringstream;
#else
typedef std::ostringstream tostringstream;
#endif

using namespace std;
using namespace teamtalk;

bool LoadConfig(teamtalk::ServerXML& xmlSettings, const ACE_TString& cfgfile)
{
    ACE_TString settings_path;
    ACE_TCHAR buf[1024] = {};

    if(cfgfile.empty())
    {
        ACE_TString workdir = ACE_OS::getcwd(buf, 1024);
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
                ACE_TString errmsg = Utf8ToUnicode(xmlSettings.GetError().c_str());
                osErr << ACE_TEXT("Unable to read ") << settings_path.c_str() << ACE_TEXT(". ") << errmsg.c_str() << endl;
            }
            else
                osErr << ACE_TEXT("Unable to open file ") << settings_path.c_str() << ACE_TEXT(".");
            TT_SYSLOG(osErr.str().c_str());

            return false;
        }
        return true;
    }
    else
    {
        if(!xmlSettings.CreateFile(UnicodeToUtf8(settings_path).c_str()))
        {
            tostringstream osErr;
            osErr << ACE_TEXT("Unable to create settings file ") << settings_path.c_str() << ACE_TEXT(".");
            TT_SYSLOG(osErr.str().c_str());
            return false;
        }
        return true;
    }
}

void RunWizard(teamtalk::ServerXML& xmlSettings)
{
    cout << TEAMTALK_NAME << " " << TEAMTALK_VERSION_FRIENDLY << " configurator" << endl;
    cout << endl;

    cout << "Do you want to configure your " TEAMTALK_NAME "? ";
    if(!printGetBool(true))
        return;

    cout << endl;
    cout << "Configuring file: " << xmlSettings.GetFileName() << endl;

    ACE_TString servername, motd, filesroot;
    std::vector<std::string> bindips;
    ACE_TString certfile, keyfile, cafile, cadir;
    int maxusers, max_logins_per_ip = 0;
    bool autosave = true, certverifypeer, certverifyonce, certdepth;
    int64_t diskquota = 0, maxdiskusage = 0, log_maxsize = 0;
    int tcpport = DEFAULT_TCPPORT, udpport = DEFAULT_UDPPORT, max_login_attempts = 0, logindelay = 0;

    servername = Utf8ToUnicode(xmlSettings.GetServerName().c_str());
    motd = Utf8ToUnicode(xmlSettings.GetMessageOfTheDay().c_str());
    filesroot = Utf8ToUnicode(xmlSettings.GetFilesRoot().c_str());
    bindips = xmlSettings.GetBindIPs();
    tcpport = xmlSettings.GetHostTcpPort()==UNDEFINED?DEFAULT_TCPPORT:xmlSettings.GetHostTcpPort();
    udpport = xmlSettings.GetHostUdpPort()==UNDEFINED?DEFAULT_UDPPORT:xmlSettings.GetHostUdpPort();

    maxusers = xmlSettings.GetMaxUsers() == UNDEFINED?MAX_USERS:xmlSettings.GetMaxUsers();
    log_maxsize = xmlSettings.GetServerLogMaxSize();
    autosave = xmlSettings.GetAutoSave();
    diskquota = xmlSettings.GetDefaultDiskQuota();
    maxdiskusage = xmlSettings.GetMaxDiskUsage();
    max_login_attempts = xmlSettings.GetMaxLoginAttempts();
    max_logins_per_ip = xmlSettings.GetMaxLoginsPerIP();
    logindelay = xmlSettings.GetLoginDelay();

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
    ACE_TCHAR *s;
    int utf8_mode = 0;

    if (((s = getenv("LC_ALL"))   && *s) ||
            ((s = getenv("LC_CTYPE")) && *s) ||
            ((s = getenv("LANG"))     && *s)) {
        if (strstr(s, "UTF-8"))
            utf8_mode = 1;
    }
    if(!utf8_mode)
        cout << "Warning: UTF-8 not enabled. Please stick to English characters!" << endl;
#endif
    //cout << "Value in parantesis will be used if no input is specified." << endl;
    cout << endl;
    cout << TEAMTALK_NAME << " name: ";
    servername = LocalToUnicode(printGetString(UnicodeToLocal(servername).c_str()).c_str());
    cout << "Message of the Day: ";
    motd = LocalToUnicode(printGetString(UnicodeToLocal(motd).c_str()).c_str());
    cout << "Maximum users allowed on server: ";
    maxusers = printGetInt(maxusers);
    if(maxusers > MAX_USERS)
    {
        cout << "Maximum allowed users is " << MAX_USERS << " and the value has been reduced accordingly." << endl;
        maxusers = MAX_USERS;
    }
    cout << "Server should automatically save changes: ";
    autosave = printGetBool(autosave);

    cout << "Enable file sharing: ";
    if(printGetBool(filesroot.length()))
    {
        ACE_TCHAR buff[1024] = {};
        ACE_OS::getcwd(buff, 1024);
#ifdef WIN32
        cout << "Directory for file storage, e.g. C:\\MyServerFiles: ";
#else
        cout << "Directory for file storage, e.g. /home/bill/srv1/files: ";
#endif
        filesroot = LocalToUnicode(printGetString(UnicodeToLocal(filesroot).c_str()).c_str());
        if(!filesroot.empty() && ACE_OS::chdir(filesroot.c_str()) == 0)
        {
            cout << "Disk quota (in KBytes) per channel, 0 = disabled: ";
            diskquota = printGetInt64(diskquota/1024)*1024;
            cout << "Maximum disk usage (in KBytes) for storing files: ";
            maxdiskusage = printGetInt64(maxdiskusage/1024)*1024;
        }
        else
        {
            cout << "Directory not found for file storage. File sharing disabled." << endl;
            filesroot.clear();
            diskquota = 0;
        }
        ACE_OS::chdir(buff);
    }
    else
    {
        filesroot.clear();
        diskquota = 0;
    }

    cout << "Log server activity: ";
    log_maxsize = printGetBool(log_maxsize != 0)? (log_maxsize!=0)? log_maxsize : -1 : 0;

    cout << "Server should bind to the following TCP port: ";
    tcpport = printGetInt(tcpport);
    cout << "Server should bind to the following UDP port: ";
    udpport = printGetInt(udpport);
    cout << "Bind to specific IP-addresses? (required for IPv6) ";
    if (printGetBool(bindips.size()))
    {
        while (true)
        {
            if (bindips.size())
            {
                cout << "Currently binding to IP-addresses:" << endl;
                for (auto ip : bindips)
                {
                    cout << "\t- " << (ip.empty()? "0.0.0.0" : ip) << endl;
                }

                cout << "Specify additional IP-addresses? ";
                if (!printGetBool(bindips.empty()))
                    break;
            }

            cout << "Specify the IP-address to bind to (IPv6 type \"::\" for all interfaces): " << endl;
            std::string ip = printGetString("0.0.0.0");
            bindips.push_back(ip);
        }
    }
    else
    {
        bindips.clear();
    }

#if defined(ENABLE_TEAMTALKPRO)
    cout << "Should server run in encrypted mode? ";
    if(printGetBool(certfile.length() && keyfile.length()))
    {
        while (true)
        {
            cout << "Server certificate file (in PEM format) for encryption: ";
            certfile = LocalToUnicode(printGetString(UnicodeToLocal(certfile).c_str()).c_str());
            if (ACE_OS::filesize(certfile.c_str()) <= 0)
                cerr << "File " << certfile << " not found!" << endl;
            else break;
        }

        while (true)
        {
            cout << "Server private key file (in PEM format) for encryption: ";
            keyfile = LocalToUnicode(printGetString(UnicodeToLocal(keyfile).c_str()).c_str());
            if (ACE_OS::filesize(keyfile.c_str()) <= 0)
                cerr << "File " << keyfile << " not found!" << endl;
            else break;
        }

        cout << "Should server verify client's certificate with provided Certificate Authority (CA) certificate? ";
        if ((certverifypeer = printGetBool(certverifypeer)))
        {
            cout << "File containing Certificate Authority (CA) certificate (in PEM format): ";
            cafile = LocalToUnicode(printGetString(UnicodeToLocal(cafile).c_str()).c_str());

            cout << "Directory containing Certificate Authority (CA) certificates (leave blank to only use single CA file): ";
            cadir = LocalToUnicode(printGetString(UnicodeToLocal(cadir).c_str()).c_str());

            cout << "Should client's certificate only be verified initially? ";
            certverifyonce = printGetBool(certverifyonce);

            cout << "Max depth in certificate chain during the verification process? ";
            certdepth = printGetInt(certdepth);
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

    bool encrypted = certfile.length() && keyfile.length();
#endif

    cout << endl << "User authentication." << endl;

    cout << endl;
    cout << "User account administration." << endl;
    int input = 0;
    enum UserAccountOptions {LIST_USERACCOUNTS = 1, CREATE_USERACCOUNT,
                     #if defined(ENABLE_TEAMTALKPRO)
                             CREATE_USERACCOUNT_BEARWARE,
                     #endif
                             DELETE_USERACCOUNT, QUIT_USERACCOUNTS};
#if defined(ENABLE_TEAMTALKPRO)
    ACE_CString url = WEBLOGIN_URL;
#endif
    std::string xml;

    while(input != QUIT_USERACCOUNTS)
    {
        UserAccount user;
        std::string tmp;
        int count = 0;
        while(xmlSettings.GetNextUser(count, user))count++;
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
        switch( (input = printGetInt(QUIT_USERACCOUNTS)) )
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
            user.username = LocalToUnicode(printGetString("").c_str());
            cout << "Type password: ";
            user.passwd = LocalToUnicode(printGetString("").c_str());
            goto useraccountcfg;
#if defined(ENABLE_TEAMTALKPRO)
        case CREATE_USERACCOUNT_BEARWARE :

            if (!encrypted)
            {
                cout << "BearWare.dk web-login can only be used in encrypted mode." << endl;
                break;
            }

            cout << "Creating BearWare.dk web-login account." << endl;
            user.username = ACE_TEXT( WEBLOGIN_BEARWARE_USERNAME );
            user.passwd = ACE_TEXT("");
            cout << "Testing BearWare.dk web-login service..." << endl;

            url += "client=" TEAMTALK_LIB_NAME;
            url += "&version=" TEAMTALK_VERSION;
            url += "&ping=true";
            switch(HttpRequest(url, xml))
            {
            case -1 :
                cout << "Failed to query " << WEBLOGIN_URL;
                break;
            case 0 :
                cout << "Invalid response from BearWare.dk login service" << endl;
                break;
            case 1 :
                cout << "Got valid response from BearWare.dk login service. Continuing..." << endl;
                break;
            }
            goto useraccountcfg;
#endif /* ENABLE_TEAMTALKPRO */
useraccountcfg:
            cout << "Available user types:" << endl;
            cout << "\t1. Default user." << endl;
            cout << "\t2. Administrator." << endl;
            cout << "Select user type:";
            switch(printGetInt(1))
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
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_MULTI_LOGIN?true:false)?
                            userrights | USERRIGHT_MULTI_LOGIN : userrights & ~USERRIGHT_MULTI_LOGIN;

                cout << "User can change nickname: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_LOCKED_NICKNAME?false:true)?
                            (userrights & ~USERRIGHT_LOCKED_NICKNAME) : (userrights | USERRIGHT_LOCKED_NICKNAME);

                cout << "User can see all other users on server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_VIEW_ALL_USERS?true:false)?
                            (userrights | USERRIGHT_VIEW_ALL_USERS) : (userrights & ~USERRIGHT_VIEW_ALL_USERS);

                cout << "User can create temporary channels: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_CREATE_TEMPORARY_CHANNEL?true:false)?
                            (userrights | USERRIGHT_CREATE_TEMPORARY_CHANNEL) : (userrights & ~USERRIGHT_CREATE_TEMPORARY_CHANNEL);

                cout << "User can create/modify all channels: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_MODIFY_CHANNELS?true:false)?
                            (userrights | USERRIGHT_MODIFY_CHANNELS) : (userrights & ~USERRIGHT_MODIFY_CHANNELS);

                cout << "User can broadcast text message to all users: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TEXTMESSAGE_BROADCAST?true:false)?
                            (userrights | USERRIGHT_TEXTMESSAGE_BROADCAST) : (userrights & ~USERRIGHT_TEXTMESSAGE_BROADCAST);

                cout << "User can kick users off the server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_KICK_USERS?true:false)?
                            (userrights | USERRIGHT_KICK_USERS) : (userrights & ~USERRIGHT_KICK_USERS);

                cout << "User can ban users from the server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_BAN_USERS?true:false)?
                            (userrights | USERRIGHT_BAN_USERS) : (userrights & ~USERRIGHT_BAN_USERS);

                cout << "User can move users between channels: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_MOVE_USERS?true:false)?
                            (userrights | USERRIGHT_MOVE_USERS) : (userrights & ~USERRIGHT_MOVE_USERS);

                cout << "User can make other users channel operator: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_OPERATOR_ENABLE?true:false)?
                            (userrights | USERRIGHT_OPERATOR_ENABLE) : (userrights & ~USERRIGHT_OPERATOR_ENABLE);

                cout << "User can upload files to channels: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_UPLOAD_FILES?true:false)?
                            (userrights | USERRIGHT_UPLOAD_FILES) : (userrights & ~USERRIGHT_UPLOAD_FILES);

                cout << "User can download files from channels: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_DOWNLOAD_FILES?true:false)?
                            (userrights | USERRIGHT_DOWNLOAD_FILES) : (userrights & ~USERRIGHT_DOWNLOAD_FILES);

                cout << "User can record conversations in channels that don't allow it: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_RECORD_VOICE?true:false)?
                            (userrights | USERRIGHT_RECORD_VOICE) : (userrights & ~USERRIGHT_RECORD_VOICE);

                cout << "User can update server properties: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_UPDATE_SERVERPROPERTIES?true:false)?
                            (userrights | USERRIGHT_UPDATE_SERVERPROPERTIES) : (userrights & ~USERRIGHT_UPDATE_SERVERPROPERTIES);

                cout << "User can transmit voice (microphone input) packets through server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_VOICE?true:false)?
                            (userrights | USERRIGHT_TRANSMIT_VOICE) : (userrights & ~USERRIGHT_TRANSMIT_VOICE);

                cout << "User can transmit video (webcam) packets through server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_VIDEOCAPTURE?true:false)?
                            (userrights | USERRIGHT_TRANSMIT_VIDEOCAPTURE) : (userrights & ~USERRIGHT_TRANSMIT_VIDEOCAPTURE);

                cout << "User can transmit desktop sharing packets through server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_DESKTOP?true:false)?
                            (userrights | USERRIGHT_TRANSMIT_DESKTOP) : (userrights & ~USERRIGHT_TRANSMIT_DESKTOP);

                cout << "User can transmit remote desktop access packets through server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_DESKTOPINPUT?true:false)?
                            (userrights | USERRIGHT_TRANSMIT_DESKTOPINPUT) : (userrights & ~USERRIGHT_TRANSMIT_DESKTOPINPUT);

                cout << "User can transmit audio file (wav, mp3 files) packets through server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO?true:false)?
                            (userrights | USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO) : (userrights & ~USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO);

                cout << "User can transmit video file (avi, wmv files) packets through server: ";
                userrights = printGetBool(USERRIGHT_DEFAULT & USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO?true:false)?
                            (userrights | USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO) : (userrights & ~USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO);

                user.userrights = userrights;

                cout << "Limit user's audio codec to a specific bit rate (in kbps), 0 = no limit: ";
                user.audiobpslimit = printGetInt(0)*1000;

                cout << "Limit number of commands user can issue (to prevent flooding)?";
                if(printGetBool(user.abuse.n_cmds && user.abuse.cmd_msec))
                {
                    cout << "Number of commands to allow within specified time frame: ";
                    user.abuse.n_cmds = printGetInt(user.abuse.n_cmds);
                    cout << "User can enter " << user.abuse.n_cmds << " commands within this number of msec: ";
                    user.abuse.cmd_msec = printGetInt(user.abuse.cmd_msec);
                }
            }
                break;
            }

            if(user.username.empty())
            {
                cout << "User account has no username. Create anonymous account? ";
                if(!printGetBool(false))
                    break;
            }

            xmlSettings.RemoveUser(UnicodeToUtf8(user.username).c_str());
            xmlSettings.AddNewUser(user);
            break;
        case DELETE_USERACCOUNT :
        {
            cout << "Type the username of the account to delete: ";
            tmp = printGetString("");
            ACE_TString utf8 = LocalToUnicode(tmp.c_str());
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
                cout << "Are you sure you want to exit user account";
                if(!printGetBool(false))
                    input = 0;
            }
            break;
        }
    }

    cout << endl;

    cout << "Maximum number of invalid login attempts before banning user, 0 = disabled: ";
    max_login_attempts = printGetInt(max_login_attempts);

    cout << "Maximum number of logins per IP-address, 0 = disabled: ";
    max_logins_per_ip = printGetInt(max_logins_per_ip);

    cout << "Delay in milliseconds before an IP-address can make another login, 0 = disabled: ";
    logindelay = printGetInt(logindelay);

    cout << endl << endl;
    cout << "Your " << TEAMTALK_NAME << " is now configured with the following settings:" << endl;
    cout << endl;
    cout << TEAMTALK_NAME << " name: " << servername << endl;
    cout << "Message of the Day: " << motd << endl;
    cout << "Maximum users allowed: " << maxusers << endl;
    cout << "Server will" << (autosave? "" : " not") << " automatically save changes." << endl;

    if(filesroot.length())
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
    cout << "Server will bind to IP-address: " << endl;
    for (auto ip : bindips)
        cout << "\t- " << ip << endl;

    cout << endl;
    if(max_login_attempts)
        cout << "Max incorrect login attempts before banning user: " << max_login_attempts << endl;
    else
        cout << "Max incorrect login attempts before banning user: " << "disabled" << endl;
    if(max_logins_per_ip)
        cout << "Max logins per IP-address: " << max_logins_per_ip << endl;
    else
        cout << "Max logins per IP-address: " << "disabled" << endl;

    cout << "Users wait for " << logindelay << " msec before attempting login again." << endl;

#if defined(ENABLE_TEAMTALKPRO)
    cout << "Server certificate file for encryption: " << certfile << endl;
    cout << "Server private key file for encryption: " << keyfile << endl;
    cout << "Server should verify client's certificate: " << (certverifypeer? "true" : "false") << endl;
    if (certverifypeer && cafile.length())
        cout << "Certificate Authority (CA) file: " << cafile << endl;
    if (certverifypeer && cadir.length())
        cout << "Certificate Authority (CA) directory: " << cadir << endl;
#endif

    int count = 0;
    UserAccount user;
    while(xmlSettings.GetNextUser(count, user))count++;
    cout << "Active user accounts: " << count << endl;

    cout << endl;

    cout << "Save these settings? ";
    if(printGetBool(true))
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
    properties.maxusers = xmlSettings.GetMaxUsers() == UNDEFINED? MAX_USERS : xmlSettings.GetMaxUsers();
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

    u_short tcpport = xmlSettings.GetHostTcpPort() == UNDEFINED? DEFAULT_TCPPORT : xmlSettings.GetHostTcpPort();
    u_short udpport = xmlSettings.GetHostUdpPort() == UNDEFINED? DEFAULT_UDPPORT : xmlSettings.GetHostUdpPort();
    std::vector<std::string> bindips = xmlSettings.GetBindIPs();
    if (bindips.empty())
        bindips.push_back("");
    for (auto ip : bindips)
    {
        if(ip.length())
        {
#if defined(WIN32)
            ACE_INET_Addr tcpaddr(tcpport, Utf8ToUnicode(ip.c_str()).c_str());
            ACE_INET_Addr udpaddr(udpport, Utf8ToUnicode(ip.c_str()).c_str());
#else
            ACE_INET_Addr tcpaddr(tcpport, Utf8ToUnicode(ip.c_str()));
            ACE_INET_Addr udpaddr(udpport, Utf8ToUnicode(ip.c_str()));
#endif
            properties.tcpaddrs.push_back(tcpaddr);
            properties.udpaddrs.push_back(udpaddr);
        }
        else
        {
            properties.tcpaddrs.push_back(ACE_INET_Addr(tcpport));
            properties.udpaddrs.push_back(ACE_INET_Addr(udpport));
        }
    }

    xmlSettings.GetStaticChannels(channels);

    return true;
}

#if defined(ENABLE_TEAMTALKPRO)
bool SetupEncryption(teamtalk::ServerNode& servernode, teamtalk::ServerXML& xmlSettings)
{
    ACE_TString certfile, privfile, cafile, cadir;

    certfile = Utf8ToUnicode(xmlSettings.GetCertificateFile().c_str());
    privfile = Utf8ToUnicode(xmlSettings.GetPrivateKeyFile().c_str());
    cafile = Utf8ToUnicode(xmlSettings.GetCertificateAuthFile().c_str());
    cadir = Utf8ToUnicode(xmlSettings.GetCertificateAuthDir().c_str());

    ACE_SSL_Context *context = servernode.SetupEncryptionContext();
    if (!context)
    {
        TT_SYSLOG("Failed to setup encryption context.");
        return false;
    }

    if (context->set_mode(ACE_SSL_Context::SSLv23) < 0)
        return false;

    if (certfile.length() && privfile.length())
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

    if (cafile.length() || cadir.length())
    {
        if (context->load_trusted_ca(cafile.length() ? UnicodeToLocal(cafile).c_str() :  nullptr,
                                     cadir.length() ? UnicodeToLocal(cadir).c_str() : nullptr, false) < 0)
        {
            TT_SYSLOG("Failed to load CA file. Check the settings file.");
            return false;
        }
    }

    context->set_verify_peer(xmlSettings.GetCertificateVerify(false),
                             xmlSettings.GetCertificateVerifyOnce(true),
                             xmlSettings.GetCertificateVerifyDepth(0));
    return true;
}

bool HasBearWareWebLogin(teamtalk::ServerXML& xmlSettings)
{
    int i = 0;
    UserAccount ua;
    while (xmlSettings.GetNextUser(++i, ua))
    {
        if (ua.username == ACE_TEXT(WEBLOGIN_BEARWARE_USERNAME))
            return true;

        const ACE_TString BWREGEX = ACE_TEXT(WEBLOGIN_BEARWARE_POSTFIX) + ACE_TString(ACE_TEXT("$"));
#if defined(UNICODE)
        if (std::regex_search(ua.username.c_str(), std::wregex(BWREGEX.c_str())))
            return true;
#else
        if (std::regex_search(ua.username.c_str(), std::regex(BWREGEX.c_str())))
            return true;
#endif
    }
    return false;
}
#endif /* ENABLE_TEAMTALKPRO */

bool ConfigureServer(teamtalk::ServerNode& servernode,
                     const teamtalk::ServerSettings& properties,
                     const teamtalk::statchannels_t& channels)
{
    GUARD_OBJ(&servernode, servernode.lock());

    //load settings
    servernode.SetServerProperties(properties);

    servernode.SetAutoSaving(false);

    if(properties.filesroot.length() && !servernode.SetFileSharing(properties.filesroot))
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
        bool makeroot = servernode.MakeChannel(chanprop).errorno == TT_CMDERR_SUCCESS;
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
        serverchannel_t chan = sweeper.front();
        sweeper.pop();
        parentid = sweeperid.front();
        sweeperid.pop();

        ChannelProp schan = chan->GetChannelProp();
        channels[schan.channelid] = schan;

        ServerChannel::channels_t subs = chan->GetSubChannels();
        for(size_t i=0;i<subs.size();i++)
        {
            ACE_TString path = subs[i]->GetChannelPath().c_str();
            if(!onlystatic || (subs[i]->GetChannelType() & CHANNEL_PERMANENT))
            {
                sweeper.push(subs[i]);
                sweeperid.push(subs[i]->GetChannelID());
            }
        }
    }
}

void MakeStaticChannels(teamtalk::ServerNode& servernode, const teamtalk::statchannels_t& channels)
{
    //Make static channels
    int root_id = GetRootChannelID(channels);
    if(root_id>0)
    {
        int fileid = 0;

        statchannels_t::const_iterator ite = channels.find(root_id);
        ChannelProp chan = ite->second;
        std::queue< ChannelProp > sweeper;
        sweeper.push(chan);

        while(!sweeper.empty())
        {
            chan = sweeper.front();
            sweeper.pop();
            bool create_chan = servernode.MakeChannel(chan).errorno == TT_CMDERR_SUCCESS;
            TTASSERT(create_chan);
            if(!create_chan)
            {
                tostringstream os;
                os << ACE_TEXT("Failed to create channel #") << chan.channelid << ACE_TEXT(" with name ") << chan.name.c_str();
                TT_SYSLOG(os.str().c_str());
                continue;
            }
            ACE_TString filesroot = servernode.GetFilesRoot().c_str() + ACE_TString(ACE_DIRECTORY_SEPARATOR_STR);
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

                ACE_TString rootpath = filesroot + chan.files[i].internalname.c_str();
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

            std::for_each(chan.bans.begin(), chan.bans.end(), [&] (BannedUser ban)
            { servernode.AddBannedUserToChannel(ban); });

            statchannels_t subs = GetSubChannels(chan.channelid, channels);
            for(ite=subs.begin();ite!=subs.end();ite++)
                sweeper.push(ite->second);
        }
    }
}
