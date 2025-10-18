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

#if !defined(MYINET_H)
#define MYINET_H

#include <ace/INET_Addr.h>
#include <ace/INet/HTTP_Status.h>
#include <ace/SString.h>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

std::vector<ACE_INET_Addr> DetermineHostAddress(const ACE_TString& host, int port);

int HttpGetRequest(const ACE_CString& url, std::string& result, ACE::HTTP::Status::Code* statusCode = nullptr);
int HttpPostRequest(const ACE_CString& url, const char* data, int len,
                    const std::map<std::string,std::string>& headers,
                    std::string& result, ACE::HTTP::Status::Code* statusCode = nullptr);
int HttpPostRequest(const ACE_CString& url, const std::map<std::string,std::string>& unencodedformdata,
                    std::string& result, ACE::HTTP::Status::Code* statusCode = nullptr);
std::string URLEncode(const std::string& utf8);

ACE_TString InetAddrToString(const ACE_INET_Addr& addr);
int InetAddrFamily(const ACE_TString& addr_str); //AF_INET AF_NET6
ACE_TString INetAddrNetwork(const ACE_TString& ipaddr, uint32_t prefix);

#endif
