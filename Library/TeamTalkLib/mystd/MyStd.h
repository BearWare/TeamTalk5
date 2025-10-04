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

#include <cstdint>
#include <regex>
#include <string>
#include <vector>

using stdstrings_t = std::vector<std::string>;

std::string String2Lower(const std::string& str);

bool StringCmpNoCase(const std::string& str1, const std::string& str2);
stdstrings_t StringTokenize(const std::string& source, const std::string& delimeters);

#if defined(UNICODE)
std::wregex BuildRegex(const std::wstring& regexstr);
#else
std::regex BuildRegex(const std::string& regexstr);
#endif

uint32_t GETTIMESTAMP();

class NonCopyable
{
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
public:
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable& operator=(const NonCopyable &) = delete;
    NonCopyable(NonCopyable&&) = delete;
    NonCopyable& operator=(NonCopyable&&) = delete;
};

struct Rational
{
    int numerator;
    int denominator;
    Rational(int n, int d) : numerator(n), denominator(d) { }
    bool IsOne() const { return numerator == denominator; }
};

#endif

