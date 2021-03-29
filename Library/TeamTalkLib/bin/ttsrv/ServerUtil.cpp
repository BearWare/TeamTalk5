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

#include <TeamTalkDefs.h>
#include <teamtalk/ttassert.h>
#include <settings/Settings.h>

#include <mystd/MyStd.h>
#include <myace/MyACE.h>

#include <ace/Dirent_Selector.h>

#include <iostream>
#include <sstream>

#if defined(UNICODE)
typedef std::wostringstream tostringstream;
#else
typedef std::ostringstream tostringstream;
#endif

using namespace std;
using namespace teamtalk;

bool inputTrue(const std::string& input)
{
    std::string tmp = str2lower(input);
    return tmp == "y" || tmp == "yes";
}

std::string printGetString(const std::string& input)
{
    cout << "(\"" << input << "\") ";
    std::string tmp;
    std::getline(cin, tmp);
    return tmp.empty()? input : tmp.c_str();
}

bool printGetBool(bool value)
{
    cout << "[Y/N] ";
    std::string input = printGetString(value?"Y":"N");
    return inputTrue(input);
}

int printGetInt(int value)
{
    ostringstream os;
    os << value;
    std::string sval = printGetString(os.str().c_str());
    istringstream is(sval.c_str());
    is >> value;
    return value;
}

int64_t printGetInt64(int64_t value)
{
    ostringstream os;
    os << value;
    std::string sval = printGetString(os.str().c_str());
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
        ACE_TString curfile = ds[i]->d_name;
        if(curfile.length()>logname.length() &&
           curfile.substr(0, logname.length()) == logname)
        {
            strings_t tokens = tokenize(curfile, ACE_TEXT("."));
            if(tokens.empty())
                continue;
            int val = int(string2i(tokens[tokens.size()-1]));
            if(val > max)
                max = val;
        }
    }

    max++;

    ACE_TString tmp_logname = logname + ACE_TEXT(".") + i2string(max);

    //rename from LogFile.log to LogFile.log.1
    int ren = ACE_OS::rename(logname.c_str(), tmp_logname.c_str());
    TTASSERT(ren>=0);

    logfile.open(logname.c_str());
    if(!logfile.good())
        logfile.clear();
}

#if defined(ENABLE_TEAMTALKPRO)

int LoginBearWareAccount(const ACE_CString& username, const ACE_CString& passwd)
{
    std::string url = WEBLOGIN_URL;
    url += "client=" TEAMTALK_LIB_NAME;
    url += "&version=" TEAMTALK_VERSION;
    url += "&service=bearware";
    url += "&action=auth";
    url += "&username=" + URLEncode(username.c_str());
    url += "&password=" + URLEncode(passwd.c_str());
    std::string utf8;
    switch (HttpRequest(url.c_str(), utf8))
    {
    default :
    case -1 :
        return -1;
    case 0 :
        return 0;
    case 1 :
        teamtalk::XMLDocument xmldoc("teamtalk", "1.0");
        if (xmldoc.Parse(utf8))
        {
            std::string nickname = xmldoc.GetValue(false, "teamtalk/bearware/nickname", "");
            std::string username = xmldoc.GetValue(false, "teamtalk/bearware/username", "");
            std::string token = xmldoc.GetValue(false, "teamtalk/bearware/token", "");
            return token.size() > 0;
        }
        return 0;
    }
}

#endif
