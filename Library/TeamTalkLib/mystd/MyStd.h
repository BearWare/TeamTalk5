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

#if !defined(MYSTD_H)
#define MYSTD_H

#include <string>
#include <vector>

#if !defined(_MSC_VER)
#include <stdint.h>
#endif

typedef std::vector<std::string> stdstrings_t;

#if defined(_MSC_VER)
std::string i2str(__int64 i);
__int64 str2i(const std::string& str);
#else
std::string i2str(int64_t i);
int64_t str2i(const std::string& str);
#endif

std::string str2lower(const std::string& str);

bool strcmpnocase(const std::string& str1, const std::string& str2);
stdstrings_t stdtokenize(const std::string& source, const std::string& delimeters);

uint32_t GETTIMESTAMP();

class NonCopyable
{
protected:
    NonCopyable() {}
    ~NonCopyable() {}
private: 
    NonCopyable(const NonCopyable &);
    NonCopyable& operator=(const NonCopyable &);
};

#endif

