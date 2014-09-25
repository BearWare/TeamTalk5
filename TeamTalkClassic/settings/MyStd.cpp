/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk Conferencing System
 * owned by BearWare.dk. Unauthorized use is prohibited. No changes
 * are allowed to this file without the copyright owner's written
 * consent. Using this file requires a source code license issued by
 * BearWare.dk.
 *
 */

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
    char buf[20] = {0};
    sprintf(buf, "%d", i);
    return string(buf);
}

int str2i(const string& szInt)
{
    return atoi(szInt.c_str());
}

#if defined(_MSC_VER)
std::string i2str(__int64 i)
{
    ostringstream is;
    is << i;
    return is.str();
}

__int64 str2i64(const std::string& szInt)
{
    __int64 ret = 0;
    istringstream is(szInt);
    is >> ret;
    return ret;
}
#else
std::string i2str(int64_t i)
{
    ostringstream is;
    is << i;
    return is.str();
}

int64_t str2i64(const std::string& szInt)
{
    long long ret = 0;
    istringstream is(szInt);
    is >> ret;
    return ret;
}
#endif

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
