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

#include "ServerUtil.h"

#include "AppInfo.h"
#include "TeamTalkDefs.h"
#include "myace/MyACE.h"
#include "myace/MyINet.h"
#include "mystd/MyStd.h"
#include "settings/Settings.h"
#include "teamtalk/TTAssert.h"

#include <ace/Dirent_Selector.h>
#include <ace/INet/HTTP_Status.h>
#include <ace/Log_Msg.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#if defined(WIN32)
#include <windows.h>
#else
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#endif

#if defined(UNICODE)
using tostringstream = std::wostringstream;
#else
using tostringstream = std::ostringstream;
#endif

using namespace std;
using namespace teamtalk;

static bool InputTrue(const std::string& input)
{
    std::string const tmp = String2Lower(input);
    return tmp == "y" || tmp == "yes";
}

std::string PrintGetString(const std::string& input)
{
    cout << "(\"" << input << "\") ";
    std::string tmp;
    std::getline(cin, tmp);
    return tmp.empty()? input : tmp;
}

std::string PrintGetPassword(const std::string& input)
{
    cout << "(\"" << input << "\") ";

#if defined(WIN32)
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD dwMode;
    if (GetConsoleMode(hStdin, &dwMode))
    {
        dwMode &= ~ENABLE_ECHO_INPUT;
        SetConsoleMode(hStdin, dwMode);
    }
#else
    struct termios tty;
    if (tcgetattr(STDIN_FILENO, &tty) == 0)
    {
        tty.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    }
#endif

    std::string tmp;
    std::getline(cin, tmp);
    tmp =  tmp.empty()? input : tmp;

#if defined(WIN32)
    dwMode |= ENABLE_ECHO_INPUT;
    SetConsoleMode(hStdin, dwMode);
#else
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
    cout << endl;

    return tmp;
}

bool PrintGetBool(bool value)
{
    cout << "[Y/N] ";
    std::string const input = PrintGetString(value?"Y":"N");
    return InputTrue(input);
}

int PrintGetInt(int value)
{
    ostringstream os;
    os << value;
    std::string const sval = PrintGetString(os.str());
    istringstream is(sval.c_str());
    is >> value;
    return value;
}

int64_t PrintGetInt64(int64_t value)
{
    ostringstream os;
    os << value;
    std::string const sval = PrintGetString(os.str());
    istringstream is(sval.c_str());
    is >> value;
    return value;
}

void RotateLogfile(const ACE_TString& cwd, const ACE_TString& logname,
                   std::ofstream& logfile)
{
    logfile.close();

    ACE_Dirent_Selector ds;
    if(ds.open(cwd.c_str())<0)
        return;

    int max = 0;
    for(int i=0;i<ds.length();i++)
    {
        ACE_TString const curfile = ds[i]->d_name;
        if(curfile.length()>logname.length() &&
           curfile.substr(0, logname.length()) == logname)
        {
            strings_t tokens = Tokenize(curfile, ACE_TEXT("."));
            if(tokens.empty())
                continue;
            int const val = String2I(tokens[tokens.size()-1]);
            max = std::max(val, max);
        }
    }

    max++;

    ACE_TString const tmp_logname = logname + ACE_TEXT(".") + I2String(max);

    //rename from LogFile.log to LogFile.log.1
    int const ren = ACE_OS::rename(logname.c_str(), tmp_logname.c_str());
    TTASSERT(ren>=0);

    logfile.open(logname.c_str());
    if(!logfile.good())
        logfile.clear();
}

#if defined(ENABLE_TEAMTALKPRO)

WebLoginResult LoginBearWareAccount(const ACE_TString& username, const ACE_TString& passwd, ACE_TString& token, ACE_TString& loginid)
{
    std::string const usernameUtf8 = UnicodeToUtf8(username).c_str();
    std::string const passwdUtf8 = UnicodeToUtf8(passwd).c_str();

    std::string url = WEBLOGIN_URL;
    url += "client=" TEAMTALK_LIB_NAME;
    url += "&version=" TEAMTALK_VERSION;
    url += "&service=bearware";
    url += "&action=auth";
    url += "&username=" + URLEncode(usernameUtf8);
    url += "&password=" + URLEncode(passwdUtf8);
    std::string utf8;
    switch (HttpGetRequest(url.c_str(), utf8))
    {
    default :
    case -1 :
        return WEBLOGIN_SERVER_UNAVAILABLE;
    case 0 :
        return WEBLOGIN_FAILED;
    case 1 :
        teamtalk::XMLDocument xmldoc("teamtalk", "1.0");
        if (xmldoc.Parse(utf8))
        {
            std::string const nickname = xmldoc.GetValue(false, "teamtalk/bearware/nickname", "");
            std::string const username = xmldoc.GetValue(false, "teamtalk/bearware/username", "");
            token = Utf8ToUnicode(xmldoc.GetValue(false, "teamtalk/bearware/token", "").c_str());
            loginid = Utf8ToUnicode(username.c_str());
            return !token.empty() ? WEBLOGIN_SUCCESS : WEBLOGIN_SERVER_INCOMPATIBLE;
        }
        return WEBLOGIN_FAILED;
    }
}

WebLoginResult AuthBearWareAccount(const ACE_TString& username, const ACE_TString& token)
{
    std::string const usernameUtf8 = UnicodeToUtf8(username).c_str();
    std::string const tokenUtf8 = UnicodeToUtf8(token).c_str();

    std::string url = WEBLOGIN_URL;
    url += "client=" TEAMTALK_LIB_NAME;
    url += "&version=" TEAMTALK_VERSION;
    url += "&service=bearware";
    url += "&action=clientauth";
    url += "&username=" + URLEncode(usernameUtf8);
    url += "&token=" + URLEncode(tokenUtf8);
    url += "&accesstoken=proserver";
    ACE::HTTP::Status::Code httpCode = ACE::HTTP::Status::INVALID;
    std::string utf8;
    switch (HttpGetRequest(url.c_str(), utf8, &httpCode))
    {
    default :
    case -1 :
        return WEBLOGIN_SERVER_UNAVAILABLE;
    case 0 :
        switch (httpCode)
        {
        case ACE::HTTP::Status::HTTP_UNAUTHORIZED :
        case ACE::HTTP::Status::HTTP_PAYMENT_REQUIRED :
            return WEBLOGIN_FAILED;
        default :
            return WEBLOGIN_SERVER_UNAVAILABLE;
        }
    case 1 :
        teamtalk::XMLDocument xmldoc("teamtalk", "1.0");
        if (xmldoc.Parse(utf8))
        {
            std::string const username = xmldoc.GetValue(false, "teamtalk/bearware/username", "");
            return !username.empty() ? WEBLOGIN_SUCCESS : WEBLOGIN_SERVER_INCOMPATIBLE;
        }
        return WEBLOGIN_FAILED;
    }
}

bool LoginBearWare(teamtalk::XMLDocument& xmlSettings)
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
            cout << "To avoid providing your credentials every time the application is" << endl;
            cout << "started, it is recommended to store your access token in the" << endl;
            cout << "configuration file." << endl << endl;
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

    ACE_TString msg = ACE_TEXT("Authenticating ") + bwid;
    ACE_DEBUG((LM_INFO, ACE_TEXT("%s\n"), msg.c_str()));
    switch (AuthBearWareAccount(bwid, token))
    {
    case WEBLOGIN_FAILED :
        msg = ACE_TEXT("Failed to authenticate BearWare.dk WebLogin: ") + bwid;
        ACE_DEBUG((LM_ERROR, ACE_TEXT("%s\n"), msg.c_str()));
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
        ACE_DEBUG((LM_WARNING, ACE_TEXT("BearWare.dk WebLogin is currently unavailable. Continuing...\n")));
        std::this_thread::sleep_for(std::chrono::seconds(sleepTimeSec *= 2));
        break;
    case WEBLOGIN_SUCCESS :
        break;
    }

    return true;
}

#endif
