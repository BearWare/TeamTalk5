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

#include "Common.h"
#include "Commands.h"
#include <myace/MyINet.h>

#include <time.h>
#include <ace/Date_Time.h>
#include <regex>

namespace teamtalk
{

    ServerProperties::ServerProperties()
    {
        systemid = SERVER_WELCOME;
    }

    RemoteFile::RemoteFile()
    {
        uploadtime = ACE_OS::gettimeofday();
    }

    UserAccount::UserAccount()
    {
        lastupdated = ACE_OS::gettimeofday();
    }

    bool UserAccount::IsWebLogin() const
    {
#if defined(ENABLE_TEAMTALKPRO)
        ACE_TString bwregex = ACE_TEXT(WEBLOGIN_BEARWARE_POSTFIX) + ACE_TString(ACE_TEXT("$"));
        return std::regex_search(username.c_str(), buildregex(bwregex.c_str()));
#else
        return false;
#endif
    }

    bool BannedUser::Same(const BannedUser& user) const
    {
        bool same = user.bantype == this->bantype;
        if (bantype & BANTYPE_IPADDR)
            same &= user.ipaddr == this->ipaddr;
        if (bantype & BANTYPE_CHANNEL)
            same &= user.chanpath == this->chanpath;
        if (bantype & BANTYPE_USERNAME)
            same &= user.username == this->username;
        return same;
    }

    bool BannedUser::Match(const BannedUser& user) const
    {
        bool match = true;
        if((bantype & BANTYPE_IPADDR) && ipaddr.length())
        {
            const ACE_TString rgxsubnet = ACE_TEXT("^") ACE_TEXT("(.*)/(\\d+)") ACE_TEXT("$");
#if defined(UNICODE)
            std::wsmatch sm;
            std::wstring bannedip = ipaddr.c_str();
#else
            std::smatch sm;
            std::string bannedip = ipaddr.c_str();
#endif
            // check if network ban
            if (std::regex_search(bannedip, sm, buildregex(rgxsubnet.c_str())) && sm.size() == 3)
            {
                ACE_TString net = sm[1].str().c_str();
                uint32_t prefix = string2i(sm[2].str().c_str());
                // match &= INetAddrNetwork(user.ipaddr, prefix) == net;
                // to prevent invalid network (192.168.1.0/23 = 192.168.0.0/23) ?
                match &= INetAddrNetwork(user.ipaddr, prefix) == INetAddrNetwork(net, prefix);
            }
            else
            {
                ACE_TString rgx = ACE_TEXT("^") + ipaddr + ACE_TEXT("$");
                match &= std::regex_search(user.ipaddr.c_str(), buildregex(rgx.c_str()));
            }
        }
        if((bantype & BANTYPE_USERNAME))
            match &= username == user.username;
        if((bantype & BANTYPE_CHANNEL))
            match &= chanpath == user.chanpath;
        match &= bantype != BANTYPE_NONE;
        return match;
    }


    ACE_TString DateToString(const ACE_Time_Value& tv)
    {
        ACE_Date_Time date(tv);
        ACE_TCHAR buf[200];
        ACE_OS::sprintf(buf, ACE_TEXT("%d/%.2d/%.2d %.2d:%.2d"), 
                        (int)date.year(), (int)date.month(), (int)date.day(), 
                        (int)date.hour(), (int)date.minute());
        return buf;
    }

    std::vector<int> ConvertFrameSizes(const std::vector<uint16_t>& in)
    {
        std::vector<int> out(in.size());
        for(size_t i=0;i<in.size();i++)
            out[i] = in[i];
        return out;
    }

    int SumFrameSizes(const std::vector<uint16_t>& in)
    {
        int result = 0;
        for(size_t i=0;i<in.size();i++)
            result += in[i];
        return result;
    }
    std::vector<uint16_t> ConvertFrameSizes(const std::vector<int>& in)
    {
        std::vector<uint16_t> out(in.size());
        for(size_t i=0;i<in.size();i++)
            out[i] = in[i];
        return out;
    }
    int SumFrameSizes(const std::vector<int>& in)
    {
        int result = 0;
        for(size_t i=0;i<in.size();i++)
            result += in[i];
        return result;
    }

    int AFFToMP3Bitrate(AudioFileFormat aff)
    {
        switch(aff)
        {
        case AFF_MP3_16KBIT_FORMAT: return 16000;
        case AFF_MP3_32KBIT_FORMAT: return 32000;
        case AFF_MP3_64KBIT_FORMAT: return 64000;
        case AFF_MP3_128KBIT_FORMAT: return 128000;
        case AFF_MP3_256KBIT_FORMAT: return 256000;
        case AFF_MP3_320KBIT_FORMAT: return 320000;
        default: return 0;
        }
    }
}
