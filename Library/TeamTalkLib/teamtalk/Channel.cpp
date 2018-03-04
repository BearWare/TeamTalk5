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

#include "Channel.h"

using namespace std;

namespace teamtalk {

    strings_t TokenizeChannelPath(const ACE_TString& str)
    {
        vector<ACE_TString> tokNames;
        size_t i=0;
        size_t last=0;
        while(i<str.length())
        {
            if(str[i]=='/')
            {
                if((i>0))
                {
                    if(i-last>0)
                        tokNames.push_back(str.substr(last, i-last));
                    last = i+1;
                }
                else
                    if(i==0)//want root
                    {
                        last = i+1;
                    }
            }
            else
                if(i==str.length()-1)
                {
                    tokNames.push_back(str.substr(last, i-last+1));
                    last = i;
                }

                i++;
        }
        return tokNames;
    }

    bool ChannelsEquals(const ACE_TString& szChanPath1, const ACE_TString& szChanPath2)
    {
        strings_t tok1 = TokenizeChannelPath(stringtolower(szChanPath1));
        strings_t tok2 = TokenizeChannelPath(stringtolower(szChanPath2));
        return tok1 == tok2;
    }

}

