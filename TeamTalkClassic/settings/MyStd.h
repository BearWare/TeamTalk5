/*
 * Copyright (c) 2005-2014, BearWare.dk
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
 * This source code is part of the TeamTalk Conferencing System
 * owned by BearWare.dk. Unauthorized use is prohibited. No changes
 * are allowed to this file without the copyright owner's written
 * consent. Using this file requires a source code license issued by
 * BearWare.dk.
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

std::string i2str(int i);
int str2i(const std::string& szInt);

#if defined(_MSC_VER)
std::string i2str(__int64 i);
__int64 str2i64(const std::string& szInt);
#else
std::string i2str(int64_t i);
int64_t str2i64(const std::string& szInt);
#endif

std::string str2lower(const std::string& str);

bool strcmpnocase(const std::string& str1, const std::string& str2);
stdstrings_t stdtokenize(const std::string& source, const std::string& delimeters);

#endif

