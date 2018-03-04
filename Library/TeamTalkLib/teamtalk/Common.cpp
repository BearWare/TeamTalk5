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
#include <time.h>
#include "Commands.h"

namespace teamtalk
{

    ServerProp::ServerProp()
    {
        systemid = SERVER_WELCOME;
        autosave = false;
        maxusers = max_logins_per_ipaddr = 0;
        maxloginattempts = 0;
        usertimeout = 0;
        diskquota = 0;
        maxdiskusage = 0;
        voicetxlimit = videotxlimit = mediafiletxlimit = desktoptxlimit = totaltxlimit = 0;
    }

    ACE_Date_Time StringToDate(const ACE_TString& str_date)
    {
        long year = 0, month = 0, day = 0, hour = 0, minutes = 0;
        size_t i=0;
        while(i<str_date.length())
        {
            if(str_date[i] == '/')
            {
                year = string2i(str_date.substr(0, i));
                break;
            }
            else i++;
        }
        size_t begin = ++i;
        while(i<str_date.length())
        {
            if(str_date[i] == '/')
            {
                month = string2i(str_date.substr(begin, i));
                break;
            }
            else i++;
        }
        begin = ++i;
        while(i<str_date.length())
        {
            if(str_date[i] == ' ')
            {
                day = string2i(str_date.substr(begin, i));
                break;
            }
            else i++;
        }

        begin = ++i;
        while(i<str_date.length())
        {
            if(str_date[i] == ':')
            {
                hour = string2i(str_date.substr(begin, i));
                break;
            }
            else i++;
        }

        begin = ++i;

        if(str_date.length() - begin > 0)
            minutes = string2i(str_date.substr(begin, str_date.length()));
        return ACE_Date_Time(day, month, year, hour, minutes);
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
    int GetAudioFileFormatBitrate(teamtalk::AudioFileFormat aff)
    {
        switch(aff)
        {
        case AFF_MP3_16KBIT_FORMAT :
            return 16;
        case AFF_MP3_32KBIT_FORMAT :
            return 32;
        case AFF_MP3_64KBIT_FORMAT :
            return 64;
        case AFF_MP3_128KBIT_FORMAT :
            return 128;
        case AFF_MP3_256KBIT_FORMAT :
            return 256;
        default:
            return 0;
        }
    }
}
