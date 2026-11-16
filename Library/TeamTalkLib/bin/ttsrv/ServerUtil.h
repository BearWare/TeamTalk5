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

#include "mystd/MyStd.h"

#include <cstdint>
#include <fstream>
#include <string>

std::string PrintGetString(const std::string& input);
std::string PrintGetPassword(const std::string& input);
bool PrintGetBool(bool value);
int PrintGetInt(int value);
int64_t PrintGetInt64(int64_t value);

void RotateLogfile(const tstring& cwd, const tstring& logname, std::ofstream& logfile);

#if defined(ENABLE_TEAMTALKPRO)
enum WebLoginResult
{
    WEBLOGIN_SERVER_INCOMPATIBLE = -2,
    WEBLOGIN_SERVER_UNAVAILABLE = -1,
    WEBLOGIN_FAILED = 0,
    WEBLOGIN_SUCCESS = 1,
};
WebLoginResult LoginBearWareAccount(const tstring& username, const tstring& passwd, tstring& token, tstring& loginid);
WebLoginResult AuthBearWareAccount(const tstring& username, const tstring& token);
#endif
