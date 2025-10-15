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

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#if defined(WIN32)
#include <windows.h>
#else
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

#endif
