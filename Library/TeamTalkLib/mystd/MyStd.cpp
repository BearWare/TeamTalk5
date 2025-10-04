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

#include "MyStd.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <regex>
#include <string>
#include <vector>

/*******************************************************************/
/************************* Helper functions ************************/
/*******************************************************************/

static void ReplaceAll(std::string& target, const std::string& to_find, const std::string& replacement )
{
    if (to_find.empty())
        return;

    std::size_t const REPLACE_INC = replacement.length();
    for (std::size_t loc = target.find( to_find );
        loc != std::string::npos;
        loc = target.find( to_find, loc+REPLACE_INC ) )
    {
        target.replace( loc, to_find.length(), replacement );
    }
}

std::string String2Lower(const std::string& str)
{
    std::string sstr = str;
    std::ranges::transform(sstr, sstr.begin(), (int (*)(int))tolower);
  return sstr;
}

bool StringCmpNoCase(const std::string& str1, const std::string& str2)
{
    std::string tmp1 = str1;
    std::string tmp2 = str2;
    std::ranges::transform(tmp1, tmp1.begin(), (int (*)(int))tolower);
    std::ranges::transform(tmp2, tmp2.begin(), (int (*)(int))tolower);
    return tmp1 == tmp2;
}

stdstrings_t StringTokenize(const std::string& source, const std::string& delimeters)
{ 
    std::vector<std::string> tokens;
    std::string::size_type start = 0;
    std::string::size_type end = 0;

    while ((start = source.find_first_not_of(delimeters, start)) != std::string::npos)
    { 
        end = source.find_first_of(delimeters, start); 
        tokens.push_back(source.substr(start, end - start)); 
        start = end;
    }

    return tokens; 
}

#if defined(UNICODE)
std::wregex BuildRegex(const std::wstring& regexstr)
{
    try
    {
        return std::wregex(regexstr);
    }
    catch (const std::regex_error& )
    {
        return std::wregex();
    }
}
#else
std::regex BuildRegex(const std::string& regexstr)
{
    try
    {
        return std::regex(regexstr);
    }
    catch (const std::regex_error& )
    {
        return {};
    }
}
#endif


uint32_t GETTIMESTAMP()
{
    using namespace std::chrono;
    steady_clock::time_point const NOW = steady_clock::now();
    auto now_ms = time_point_cast<milliseconds>(NOW);
    auto duration = now_ms.time_since_epoch();
    return uint32_t(duration.count());
}

