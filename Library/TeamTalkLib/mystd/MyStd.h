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

#include <cstddef>
#include <cstdint>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

//tstring is std::wstring on Windows and std::string on other platforms.
#if defined(UNICODE)
using tstring = std::wstring;
using tostringstream = std::wostringstream;
#else
using tstring = std::string;
using tostringstream = std::ostringstream;
#endif

using strings_t = std::vector<tstring>;

//directory separator used in tstring file paths
#if defined(WIN32)
constexpr tstring::value_type DIRECTORY_SEPARATOR_CHAR = '\\';
#else
constexpr tstring::value_type DIRECTORY_SEPARATOR_CHAR = '/';
#endif

tstring FixFilePath(const tstring& filepath);

void ReplaceAll(tstring& target, const tstring& to_find, const tstring& replacement);

tstring I2String(int64_t i);
int64_t String2I(const tstring& int_str, int base = 10, int64_t errorValue = 0);

bool StringCmpNoCase(const tstring& str1, const tstring& str2);
tstring StringToLower(const tstring& str);
strings_t Tokenize(const tstring& source, const tstring& delimeters);

tstring KeyToHexString(const unsigned char* key, std::size_t length);
void HexStringToKey(const tstring& crypt_key, uint8_t* key, std::size_t keylen);

bool VersionSameOrLater(const tstring& check, const tstring& against);

#if defined(UNICODE)
std::string UnicodeToUtf8(const wchar_t* unicode, int len = -1);
std::string UnicodeToUtf8(const tstring& str);
tstring Utf8ToUnicode(const char* utf8, int len = -1);
std::string UnicodeToLocal(const tstring& str);
tstring LocalToUnicode(const std::string& str);
std::string Utf8ToLocal(const char* local, int len = -1);
#else
#define UnicodeToUtf8(str)      (str)
#define Utf8ToUnicode(utf8)     (utf8)
#define UnicodeToLocal(str)     (str)
#define LocalToUnicode(str)     (str)
#define Utf8ToLocal(local)      (local)
#endif

bool ValidUtf8(const std::string& utf8_str);
std::string LimitUtf8(const std::string& utf8_str, size_t maxlen);

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

