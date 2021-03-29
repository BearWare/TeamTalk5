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

#include <ace/SString.h>

#include <string>
#include <stdint.h>
#include <fstream>

std::string printGetString(const std::string& input);
bool printGetBool(bool value);
int printGetInt(int value);
int64_t printGetInt64(int64_t value);

void RotateLogfile(const ACE_TString& cwd, const ACE_TString& logname, std::ofstream& logfile);

#if defined(ENABLE_TEAMTALKPRO)
int LoginBearWareAccount(const ACE_CString& username, const ACE_CString& passwd);
#endif
