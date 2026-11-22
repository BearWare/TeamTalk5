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

#include "Commands.h"

#include "TTAssert.h"
#include "TeamTalkDefs.h"
#include "mystd/MyStd.h"
#include "myace/MyINet.h"

#include <array>
#include <ctime>
#include <chrono>

namespace teamtalk {

    tstring GetErrorDescription(int nError)
    {
        tstring errmsg;
        switch(nError)
        {
            // command errors
        case TT_CMDERR_SYNTAX_ERROR :
            errmsg = TTEXT("Syntax error in command");break;
        case TT_CMDERR_UNKNOWN_COMMAND :
            errmsg = TTEXT("Command not recognized");break;
        case TT_CMDERR_INCOMPATIBLE_PROTOCOLS :
            errmsg = TTEXT("Incompatible protocols"); break;
        case TT_CMDERR_MISSING_PARAMETER :
            errmsg = TTEXT("Command is missing a parameter"); break;
        case TT_CMDERR_UNKNOWN_AUDIOCODEC :
            errmsg = TTEXT("Audio codec is not supported by the server"); break;
        case TT_CMDERR_INVALID_USERNAME :
            errmsg = TTEXT("Invalid username for user account"); break;

            // rights
        case TT_CMDERR_INVALID_ACCOUNT :
            errmsg = TTEXT("Invalid user account");  break;
        case TT_CMDERR_INCORRECT_CHANNEL_PASSWORD :
            errmsg = TTEXT("Incorrect channel password");    break;
        case TT_CMDERR_SERVER_BANNED :
            errmsg = TTEXT("Banned from server"); break;
        case TT_CMDERR_NOT_AUTHORIZED :
            errmsg = TTEXT("Command not authorized");    break;
        case TT_CMDERR_MAX_SERVER_USERS_EXCEEDED :
            errmsg = TTEXT("Maximum number of users on server exceeded"); break;
        case TT_CMDERR_MAX_DISKUSAGE_EXCEEDED :
            errmsg = TTEXT("Maximum disk usage exceeded"); break;
        case TT_CMDERR_MAX_CHANNEL_USERS_EXCEEDED :
            errmsg = TTEXT("Maximum number of users in channel exceeded"); break;
        case TT_CMDERR_INCORRECT_OP_PASSWORD :
            errmsg = TTEXT("Incorrect channel operator password");    break;
        case TT_CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED :
            errmsg = TTEXT("Maximum bitrate for audio codec exceeded"); break;
        case TT_CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED :
            errmsg = TTEXT("Maximum number of logins per IP-address exceeded"); break;
        case TT_CMDERR_MAX_CHANNELS_EXCEEDED :
            errmsg = TTEXT("Maximum number of channels exceeded"); break;
        case TT_CMDERR_COMMAND_FLOOD :
            errmsg = TTEXT("Commands are issued too fast. Flood protection."); break;
        case TT_CMDERR_CHANNEL_BANNED:
            errmsg = TTEXT("Banned from channel"); break;
        case TT_CMDERR_MAX_FILETRANSFERS_EXCEEDED :
            errmsg = TTEXT("Maximum number of file transfers exceeded"); break;

            // state errors
        case TT_CMDERR_NOT_LOGGEDIN :
            errmsg = TTEXT("Not logged in");break;
        case TT_CMDERR_ALREADY_LOGGEDIN :
            errmsg = TTEXT("Already logged in");break;
        case TT_CMDERR_NOT_IN_CHANNEL :
            errmsg = TTEXT("Not in channel");break;
        case TT_CMDERR_ALREADY_IN_CHANNEL :
            errmsg = TTEXT("Cannot join the same channel twice");break;
        case TT_CMDERR_CHANNEL_ALREADY_EXISTS :
            errmsg = TTEXT("Channel already exists");break;
        case TT_CMDERR_CHANNEL_NOT_FOUND :
            errmsg = TTEXT("Channel not found");break;
        case TT_CMDERR_USER_NOT_FOUND :
            errmsg = TTEXT("User not found");    break;
        case TT_CMDERR_BAN_NOT_FOUND :
            errmsg = TTEXT("Ban no found"); break;
        case TT_CMDERR_FILETRANSFER_NOT_FOUND :
            errmsg = TTEXT("File transfer not found"); break;
        case TT_CMDERR_OPENFILE_FAILED :
            errmsg = TTEXT("Server failed to create file"); break;            
        case TT_CMDERR_ACCOUNT_NOT_FOUND :
            errmsg = TTEXT("User account not found"); break;
        case TT_CMDERR_FILE_NOT_FOUND :
            errmsg = TTEXT("File not found"); break;
        case TT_CMDERR_FILE_ALREADY_EXISTS :
            errmsg = TTEXT("File already exists"); break;
        case TT_CMDERR_FILESHARING_DISABLED :
            errmsg = TTEXT("File sharing is disabled"); break;
        case TT_CMDERR_CHANNEL_HAS_USERS :
            errmsg = TTEXT("Channel has active users"); break;
        case TT_CMDERR_LOGINSERVICE_UNAVAILABLE :
            errmsg = TTEXT("Login service unavailable"); break;
        case TT_CMDERR_CHANNEL_CANNOT_BE_HIDDEN :
            errmsg = TTEXT("Cannot apply hidden property to channel"); break;

            /***** Internal TeamTalk errors not related to commands ********/
        case TT_INTERR_SNDINPUT_FAILURE :
            errmsg = TTEXT("Sound input device failure");break;
        case TT_INTERR_SNDOUTPUT_FAILURE :
            errmsg = TTEXT("Sound output device failure");break;
        case TT_INTERR_AUDIOCODEC_INIT_FAILED :
            errmsg = TTEXT("Audio codec failed to initialize");break;
        case TT_INTERR_AUDIOPREPROCESSOR_INIT_FAILED :
            errmsg = TTEXT("Audio preprocessor failed to initialize");break;
        case TT_INTERR_MSGQUEUE_OVERFLOW :
            errmsg = TTEXT("The internal message queue overflowed");break;
        case TT_INTERR_SNDEFFECT_FAILURE :
            errmsg = TTEXT("An audio effect could not be applied on the sound device");break;
        default :
            errmsg = TTEXT("Unknown error");break;
        }

        return errmsg;
    }

    bool HasProperty(const mstrings_t& properties, 
                     const tstring& prop)
    {
        return properties.contains(prop);
    }

    bool GetProperty(const mstrings_t& properties, 
        const tstring& prop, 
        tstring& value)
    {
        auto const ite = properties.find(prop);
        if( ite != properties.end() )
        {
            if (value.length() > MAX_STRING_LENGTH)
                value = (*ite).second.substr(0, MAX_STRING_LENGTH);
            else
                value = (*ite).second;
            return true;
        }
                    return false;
    }

#define INT_OR_RET(val)                                        \
    do {                                                       \
        size_t i = 0;                                          \
        if((val).length() == 0) return false;                    \
        if((val)[0] == '-') i++;                                 \
        for(;i<(val).length();i++)                               \
            if(ACE_OS::ace_isdigit((val)[i]) == 0) return false; \
    } while(0)

#define UINT_OR_RET(val)                                       \
    do {                                                       \
        size_t i = 0;                                          \
        if((val).length() == 0) return false;                    \
        for(;i<(val).length();i++)                               \
            if(ACE_OS::ace_isdigit((val)[i]) == 0) return false; \
    } while(0)

    bool GetProperty(const mstrings_t& properties, 
        const tstring& prop, int& value)
    {
        auto const ite = properties.find(prop);
        if( ite != properties.end())
        {
            INT_OR_RET((*ite).second);
            value = String2I((*ite).second);
            return true;
        }
        return false;
    }

    bool GetProperty(const mstrings_t& properties, 
                     const tstring& prop, uint32_t& value)
    {
        tstring tmp;
        if(GetProperty(properties, prop, tmp))
        {
            UINT_OR_RET(tmp);
            value = static_cast<uint32_t>(String2I(tmp));
            return true;
        }
        return false;
    }

    bool GetProperty(const mstrings_t& properties, 
        const tstring& prop, bool& value)
    {
        int i = 0;
        bool const b = GetProperty(properties, prop, i);
        if(b)
            value = i != 0;
        return b;
    }

    bool GetProperty(const mstrings_t& properties, 
        const tstring& prop, int64_t& value)
    {
        tstring tmp;
        if(GetProperty(properties, prop, tmp))
        {
            INT_OR_RET(tmp);
            value = String2I(tmp);
            return true;
        }
        return false;
    }


    bool GetProperty(const mstrings_t& properties, 
        const tstring& prop, std::vector<int>& vec)
    {
        tstring value;
        auto const ite = properties.find(prop);
        if( ite != properties.end() )
        {
            value = (*ite).second;
            tstring token;
            size_t offset = 0;
            size_t i = value.find(',', offset);//Tokenize(TTEXT(","),offset);
            while(i != tstring::npos)
            {
                token = value.substr(offset, i-offset);
                offset = i+1;
                vec.push_back(static_cast<int>(String2I(token)));
                i = value.find(',', offset);

            }
            if( (!value.empty()) && (value.length() - 1) >= offset )
            {
                token = value.substr(offset, value.length()-offset);
                offset = i+1;
                vec.push_back(static_cast<int>(String2I(token)));
            }
            return true;
        }
        return false;
    }

    bool GetProperty(const mstrings_t& properties, 
        const tstring& prop, 
        std::set<int>& myset)
    {
        std::vector<int> vec;
        bool const b = GetProperty(properties, prop, vec);
        for(int i : vec)
            myset.insert(i);
        return b;
    }

    bool GetProperty(const mstrings_t& properties, const tstring& prop, 
                     AudioCodec& codec)
    {
        intvec_t codec_type;
        if(!GetProperty(properties, prop, codec_type) || codec_type.empty())
            return false;

        codec.codec = static_cast<Codec>(codec_type[0]);
        switch(codec.codec)
        {
        case CODEC_NO_CODEC :
            return true;
        case CODEC_SPEEX :
            if(codec_type.size() < 5)
                return false;
            /* do not change the order since it will break compatibility 
            * with older clients */
            codec.speex.bandmode = codec_type[1];
            codec.speex.quality = codec_type[2];
            codec.speex.frames_per_packet = codec_type[3];
            codec.speex.sim_stereo = (codec_type[4] != 0);
            return true;
        case CODEC_SPEEX_VBR :
            if(codec_type.size() < 8)
                return false;
            /* do not change the order since it will break compatibility 
            * with older clients */
            codec.speex_vbr.bandmode = codec_type[1];
            codec.speex_vbr.vbr_quality = codec_type[2];
            codec.speex_vbr.bitrate = codec_type[3];
            codec.speex_vbr.max_bitrate = codec_type[4];
            codec.speex_vbr.dtx = (codec_type[5] != 0);
            codec.speex_vbr.frames_per_packet = codec_type[6];
            codec.speex_vbr.sim_stereo = (codec_type[7] != 0);
            return true;
        case CODEC_OPUS :
            if(codec_type.size() < 11)
                return false;
            codec.opus.samplerate = codec_type[1];
            codec.opus.channels = codec_type[2];
            codec.opus.application = codec_type[3];
            codec.opus.complexity = codec_type[4];
            codec.opus.fec = (codec_type[5] != 0);
            codec.opus.dtx = (codec_type[6] != 0);
            codec.opus.bitrate = codec_type[7];
            codec.opus.vbr = (codec_type[8] != 0);
            codec.opus.vbr_constraint = (codec_type[9] != 0);
            codec.opus.frame_size = codec_type[10];
            if (codec_type.size() > 11)
                codec.opus.frames_per_packet = codec_type[11];
            else
                codec.opus.frames_per_packet = 1;
            return true;
        case CODEC_WEBM_VP8 :
            break;
        }
        return false;
    }

    bool GetProperty(const mstrings_t& properties, const tstring& prop, 
                     AudioConfig& audcfg)
    {
        intvec_t audcfg_prop;
        GetProperty(properties, prop, audcfg_prop);

        if(audcfg_prop.size()<2)
            return false;

        /* do not change the order since it will break compatibility 
        * with older clients */
        audcfg.enable_agc = (audcfg_prop[0] != 0);
        audcfg.gain_level = audcfg_prop[1];

        return true;
    }

    bool GetProperty(const mstrings_t& properties, const tstring& prop, 
                     ACE_INET_Addr& addr)
    {
        tstring str;
        if(GetProperty(properties, prop, str))
        {
            addr = ACE_INET_Addr(str.c_str());
            return true;
        }
        return false;
    }

    bool GetProperty(const mstrings_t& properties, const tstring& prop,
                     std::chrono::system_clock::time_point& tp)
    {
        int64_t gmttime = 0;
        if (GetProperty(properties, prop, gmttime))
        {
            auto const gmt_tm = static_cast<time_t>(gmttime);
            struct tm* local = std::localtime(&gmt_tm);
            if (local != nullptr)
            {
                time_t local_tm = std::mktime(local);
                local_tm -= ACE_OS::timezone();
                tp = std::chrono::system_clock::from_time_t(local_tm);
                return true;
            }
        }
        return false;
    }

    void GetProperties(const mstrings_t& properties, UserAccount& useraccount)
    {
        GetProperty(properties, TT_USERNAME, useraccount.username);
        GetProperty(properties, TT_PASSWORD, useraccount.passwd);
        GetProperty(properties, TT_USERTYPE, useraccount.usertype);
        GetProperty(properties, TT_USERRIGHTS, useraccount.userrights);
        GetProperty(properties, TT_USERDATA, useraccount.userdata);
        GetProperty(properties, TT_NOTEFIELD, useraccount.note);
        GetProperty(properties, TT_INITCHANNEL, useraccount.init_channel);
        GetProperty(properties, TT_AUTOOPCHANNELS, useraccount.auto_op_channels);
        GetProperty(properties, TT_AUDIOBPSLIMIT, useraccount.audiobpslimit);
        GetProperty(properties, TT_MODIFIEDTIME, useraccount.lastupdated);
        GetProperty(properties, TT_LASTLOGINTIME, useraccount.lastlogin);

        std::vector<int> flood;
        if(GetProperty(properties, TT_CMDFLOOD, flood))
            useraccount.abuse.FromParam(flood);
    }

    tstring PrepareString(const tstring& str)
    {
        tstring newstr;

#if defined(UNICODE)
        if (UnicodeToUtf8(str.c_str()).length() > MAX_STRING_LENGTH)
        {
            std::string utf8 = LimitUtf8(UnicodeToUtf8(str.c_str()), MAX_STRING_LENGTH);
            newstr = Utf8ToUnicode(utf8.c_str());
        }
        else
            newstr = str;
#else
        newstr = LimitUtf8(str, MAX_STRING_LENGTH);
#endif /* UNICODE */

        ReplaceAll(newstr, TTEXT("\\"), TTEXT("\\\\"));
        ReplaceAll(newstr, TTEXT("\""), TTEXT("\\\""));
        ReplaceAll(newstr, TTEXT("\r"), TTEXT("\\r"));
        ReplaceAll(newstr, TTEXT("\n"), TTEXT("\\n"));

        return newstr;
    }

    tstring RebuildString(const tstring& str)
    {
        tstring resultstr = str;
        tstring tmpstr;
        std::array<tstring, 3> search = { TTEXT("\\n"), TTEXT("\\r"), TTEXT("\\\"") };
        std::array<tstring, 3> replace = { TTEXT("\n"), TTEXT("\r"), TTEXT("\"") };

        for(size_t j = 0; j < 3; j++)
        {
            size_t pos1 = 0;
            size_t oldpos = 0;
            tmpstr = resultstr;
            resultstr.clear();
            while( (pos1 = tmpstr.find(search[j], pos1)) != tstring::npos)
            {
                int i = static_cast<int>(pos1);
                int slashes = 0;
                while(i >= 0 && tmpstr[i--] == '\\')slashes++;
                if(slashes % 2 != 0) //odd number of \ means the character should be escaped
                {
                    resultstr += tmpstr.substr(oldpos, pos1 - oldpos) + replace[j];
                    oldpos = pos1 + 2;
                }
                pos1 += 2;
            }

            resultstr += tmpstr.substr(oldpos, tmpstr.length() - oldpos);
        }

        ReplaceAll(resultstr, TTEXT("\\\\"), TTEXT("\\"));
        return resultstr;
    }

    tstring PrepareIntegerArray(const std::vector<int>& array)
    {
        tstring s;
        for(int i=0;i<static_cast<int>(array.size())-1;i++)
        {
            s = s + I2String(array[i]) + TTEXT(",");
        }
        tstring res;
        if(!array.empty())
        {
            res = TTEXT("[") + s + I2String(array[array.size()-1]) + TTEXT("]");
        }
        else
        {
            res = TTEXT("[]");
        }

        return res;
    }

    tstring PrepareIntegerSet(const std::set<int>& myset)
    {
        tstring s;
        int index = 0;
        std::set<int>::const_iterator ite;
        for(ite=myset.begin();index++<static_cast<int>(myset.size())-1;ite++)
        {
            s = s + I2String(*ite) + TTEXT(",");
        }

        tstring res;
        if(myset.size()==1)
            res = TTEXT("[") + I2String(*ite) + TTEXT("]");
        else if(myset.size()>1)
            res = TTEXT("[") + s + I2String(*ite) + TTEXT("]");
        else
            res = TTEXT("[]");

        return res;
    }

    static size_t PastBlanks(size_t offset, const tstring& input)
    {
        while(offset<input.length() && 
            (input[offset] == ' ' ||
            input[offset] == '\r' ||
            input[offset] == '\n')) offset ++;
        return offset;
    }

    int ExtractProperties(const tstring& input, mstrings_t& properties)
    {
        TTASSERT(input.find('\n') == input.rfind('\n'));

        bool bSyntaxError = false;
        if( input.empty() )
            bSyntaxError = true;

        size_t offset = input.find(' ');//past command
        if(offset == tstring::npos)
            return 0;

        while(offset < input.length() && !bSyntaxError)
        {
            //past any spaces
            offset = PastBlanks(offset, input);
            if(offset == input.length())
            {
                break;
            }

            size_t const propBegin = offset;
            tstring prop;
            tstring value;
            while(offset < input.length()) //extract property name
            {
                if( input[offset] != ' ' && input[offset] != '=') offset ++;
                else break;
            }
            if(offset == input.length())
            {
                bSyntaxError = true; //no properties in tstring
                break;
            }

            prop = input.substr(propBegin, offset-propBegin); //set propertyname
            TTASSERT(properties.find(prop) == properties.end());
            offset = PastBlanks(offset, input); //past spaces
            if(offset == input.length())
            {
                bSyntaxError = true;
                break;
            }
            if(input[offset] != '=')
            {
                bSyntaxError = true;
                break;
            }
            offset ++; //past =

            offset = PastBlanks(offset, input); //past spaces
            if(offset == input.length())
            {
                bSyntaxError = true;
                break;
            }

            //determine whether it's a string or an integer
            if(input[offset] == '"') //a tstring
            {
                bool found = false;
                size_t const strBegin = ++offset; //past "
                while(!found && offset<input.length())
                {
                    /*
                    if(input[offset]==TTEXT('\"') && input[offset-1] != TTEXT('\\')) found = true;
                    offset++;
                    */
                    if(input[offset] == '\\')
                    {
                        offset += 2;
                    }
                    else if(input[offset] == '"')
                    {
                        found = true;
                        offset++;
                    }
                    else
                    {
                        offset++;
                    }
                }

                if(!found)
                {
                    bSyntaxError = true;
                    break;
                }

                value = input.substr(strBegin, offset-strBegin-1);
                offset ++; //past \"

                properties[prop] = RebuildString(value);
                //properties.SetAt(prop, RebuildString(value));
            }
            else if(input[offset] == '[') // an int list
            {
                bool found = false;
                size_t const listBegin = ++offset; //past "
                while(!found && offset<input.length())
                {
                    if(input[offset] == ']') found = true;
                    offset++;
                }

                if(!found)
                {
                    bSyntaxError = true;
                    break;
                }

                value = input.substr(listBegin, offset-listBegin-1);
                offset ++; //past ]

                properties[prop] = RebuildString(value);
                //properties.SetAt(prop, RebuildString(value));
            }
            else //eat what's left until space
            {
                size_t const intBegin = offset;
                while(offset<input.length() && 
                    input[offset] != ' ' && 
                    input[offset] != '\r' &&
                    input[offset] != '\n') offset ++; //past spaces
                value = input.substr(intBegin, offset-intBegin);

                properties[prop] = RebuildString(value);
                //properties.SetAt(prop, RebuildString(value));
            }
        }

        return bSyntaxError? -1 : static_cast<int>(properties.size());
    }

    void AppendProperty(const tstring& prop, 
        const tstring& value, tstring& dest_str)
    {
        tstring const newprop = tstring(TTEXT(" ")) + prop + tstring(TTEXT("=\"")) + PrepareString(value) + tstring(TTEXT("\""));
        dest_str += newprop;
    }

    void AppendProperty(const tstring& prop, 
        const std::vector<int>& vecValues, tstring& dest_str)
    {
        tstring const newprop = tstring(TTEXT(" ")) + prop + tstring(TTEXT("=")) + PrepareIntegerArray(vecValues);
        dest_str += newprop;
    }

    void AppendProperty(const tstring& prop, 
        const std::set<int>& setValues, tstring& dest_str)
    {
        tstring const newprop = tstring(TTEXT(" ")) + prop + tstring(TTEXT("=")) + PrepareIntegerSet(setValues);
        dest_str += newprop;
    }

    void AppendProperty(const tstring& prop, int64_t value, tstring& dest_str)
    {
        tstring const newprop = tstring(TTEXT(" ")) + prop + tstring(TTEXT("=")) + I2String(value);
        dest_str += newprop;
    }

    void AppendProperty(const tstring& prop, const AudioCodec& codec, 
                        tstring& dest_str)
    {
        intvec_t codec_prop;
        switch(codec.codec)
        {
        case CODEC_NO_CODEC :
            codec_prop.push_back(codec.codec);
            break;
        case CODEC_SPEEX :
            /* do not change the order since it will break compatibility 
            * with older clients */
            codec_prop.push_back(codec.codec);
            codec_prop.push_back(codec.speex.bandmode);
            codec_prop.push_back(codec.speex.quality);
            codec_prop.push_back(codec.speex.frames_per_packet);
            codec_prop.push_back(static_cast<int>(codec.speex.sim_stereo));
            break;
        case CODEC_SPEEX_VBR :
            /* do not change the order since it will break compatibility 
            * with older clients */
            codec_prop.push_back(codec.codec);
            codec_prop.push_back(codec.speex_vbr.bandmode);
            codec_prop.push_back(codec.speex_vbr.vbr_quality);
            codec_prop.push_back(codec.speex_vbr.bitrate);
            codec_prop.push_back(codec.speex_vbr.max_bitrate);
            codec_prop.push_back(static_cast<int>(codec.speex_vbr.dtx));
            codec_prop.push_back(codec.speex_vbr.frames_per_packet);
            codec_prop.push_back(static_cast<int>(codec.speex_vbr.sim_stereo));
            break;
        case CODEC_OPUS :
            codec_prop.push_back(codec.codec);
            codec_prop.push_back(codec.opus.samplerate);
            codec_prop.push_back(codec.opus.channels);
            codec_prop.push_back(codec.opus.application);
            codec_prop.push_back(codec.opus.complexity);
            codec_prop.push_back(static_cast<int>(codec.opus.fec));
            codec_prop.push_back(static_cast<int>(codec.opus.dtx));
            codec_prop.push_back(codec.opus.bitrate);
            codec_prop.push_back(static_cast<int>(codec.opus.vbr));
            codec_prop.push_back(static_cast<int>(codec.opus.vbr_constraint));
            codec_prop.push_back(codec.opus.frame_size);
            codec_prop.push_back(codec.opus.frames_per_packet);
            break;
        default :
            codec_prop.push_back(CODEC_NO_CODEC);
            TTASSERT(codec.codec != CODEC_NO_CODEC);
        }
        AppendProperty(prop, codec_prop, dest_str);
    }

    void AppendProperty(const tstring& prop, const AudioConfig& audcfg, 
                        tstring& dest_str)
    {
        intvec_t audcfg_prop;
        /* do not change the order since it will break compatibility 
        * with older clients */
        audcfg_prop.push_back(static_cast<int>(audcfg.enable_agc));
        audcfg_prop.push_back(audcfg.gain_level);

        AppendProperty(prop, audcfg_prop, dest_str);
    }

    void AppendProperty(const tstring& prop, 
                        const ACE_INET_Addr& addr, tstring& dest_str)
    {
        AppendProperty(prop, InetAddrToString(addr), dest_str);
    }

    void AppendProperty(const tstring& prop,
                        std::chrono::system_clock::time_point tp, tstring& dest_str)
    {
        time_t const local_tm = std::chrono::system_clock::to_time_t(tp);
        struct tm* gmt = std::gmtime(&local_tm);
        int64_t const utc_tm = (gmt != nullptr) ? std::mktime(gmt) : local_tm;
        AppendProperty(prop, utc_tm, dest_str);
    }


    bool GetCmdLine(const std::string& input, std::string& cmd, std::string& remain_input)
    {
        if(input.find('\n') != tstring::npos)
        {
            size_t const pos = input.find('\n');
            cmd = input.substr(0, pos+1);
            size_t const len = input.length();
            remain_input = input.substr(pos+1, len-pos+1);

            return true;
        }
        return false;
    }


    bool GetCmd(const std::string& input, std::string& cmd)
    {
        size_t nEndCommand = 0;
        while(nEndCommand < input.length() && 
            input[nEndCommand] != ' ' 
            && input[nEndCommand] != '\r' && 
            input[nEndCommand] != '\n')nEndCommand++;

        if(nEndCommand != 0u)
            cmd = input.substr(0,nEndCommand);
        return nEndCommand>0;
    }

    tstring stripEOL(const tstring& input)
    {
        size_t const len = ACE_OS::strlen(EOL);
        if(input.length()<len)
            return input;
        if(input.substr(input.length()-len,len) == EOL)
            return input.substr(0,input.length()-len);
        if(input.substr(input.length()-1,1) == TTEXT("\n"))
            return input.substr(0,input.length()-1);
        return input;
    }
} // namespace teamtalk
