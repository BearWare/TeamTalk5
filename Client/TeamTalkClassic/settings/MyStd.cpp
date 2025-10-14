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

#include "stdafx.h"
#include "MyStd.h"
#include <stdio.h>
#include <algorithm>
#include <assert.h>
#include <sstream>
using namespace std;

/*******************************************************************/
/************************* Helper functions ************************/
/*******************************************************************/

void replace_all(string& target, const string& to_find, const string& replacement )
{
    if (to_find.empty())
        return;

    size_t replace_inc = replacement.length();
    for (size_t loc = target.find( to_find );
        loc != string::npos;
        loc = target.find( to_find, loc+replace_inc ) )
    {
        target.replace( loc, to_find.length(), replacement );
    }
}

string i2str(int i)
{
    char buf[20] = {};
    sprintf(buf, "%d", i);
    return string(buf);
}

int str2i(const string& szInt)
{
    return atoi(szInt.c_str());
}

std::string i2str(int64_t i)
{
    ostringstream is;
    is << i;
    return is.str();
}

int64_t str2i64(const std::string& szInt)
{
    int64_t ret = 0;
    istringstream is(szInt);
    is >> ret;
    return ret;
}

std::string str2lower(const std::string& str)
{
  string sstr = str;
  std::transform(sstr.begin(), sstr.end(), sstr.begin(), (int (*)(int))tolower);
  return sstr;
}

bool strcmpnocase(const string& str1, const string& str2)
{
    string tmp1 = str1;
    string tmp2 = str2;
    std::transform(tmp1.begin(), tmp1.end(), tmp1.begin(), (int (*)(int))tolower);
    std::transform(tmp2.begin(), tmp2.end(), tmp2.begin(), (int (*)(int))tolower);
    return tmp1 == tmp2;
}

stdstrings_t stdtokenize(const string& source, const string& delimeters) 
{ 
    vector<string> tokens; 
    string::size_type start = 0; 
    string::size_type end = 0; 

    while ((start = source.find_first_not_of(delimeters, start)) != string::npos)
    { 
        end = source.find_first_of(delimeters, start); 
        tokens.push_back(source.substr(start, end - start)); 
        start = end; 
    } 

    return tokens; 
}
