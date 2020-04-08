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
#include <myace/MyACE.h>
#include "ttassert.h"

#include <ace/OS_NS_ctype.h> //isdigit

using namespace std;

namespace teamtalk {

    ACE_TString GetErrorDescription(int nError)
    {
        ACE_TString errmsg;
        switch(nError)
        {
            // command errors
        case TT_CMDERR_SYNTAX_ERROR :
            errmsg = ACE_TEXT("Syntax error in command");break;
        case TT_CMDERR_UNKNOWN_COMMAND :
            errmsg = ACE_TEXT("Command not recognized");break;
        case TT_CMDERR_INCOMPATIBLE_PROTOCOLS :
            errmsg = ACE_TEXT("Incompatible protocols"); break;
        case TT_CMDERR_MISSING_PARAMETER :
            errmsg = ACE_TEXT("Command is missing a parameter"); break;
        case TT_CMDERR_UNKNOWN_AUDIOCODEC :
            errmsg = ACE_TEXT("Audio codec is not supported by the server"); break;
        case TT_CMDERR_INVALID_USERNAME :
            errmsg = ACE_TEXT("Invalid username for user account"); break;

            // rights
        case TT_CMDERR_INVALID_ACCOUNT :
            errmsg = ACE_TEXT("Invalid user account");  break;
        case TT_CMDERR_INCORRECT_CHANNEL_PASSWORD :
            errmsg = ACE_TEXT("Incorrect channel password");    break;
        case TT_CMDERR_SERVER_BANNED :
            errmsg = ACE_TEXT("Banned from server"); break;
        case TT_CMDERR_NOT_AUTHORIZED :
            errmsg = ACE_TEXT("Command not authorized");    break;
        case TT_CMDERR_MAX_SERVER_USERS_EXCEEDED :
            errmsg = ACE_TEXT("Maximum number of users on server exceeded"); break;
        case TT_CMDERR_MAX_DISKUSAGE_EXCEEDED :
            errmsg = ACE_TEXT("Maximum disk usage exceeded"); break;
        case TT_CMDERR_MAX_CHANNEL_USERS_EXCEEDED :
            errmsg = ACE_TEXT("Maximum number of users in channel exceeded"); break;
        case TT_CMDERR_INCORRECT_OP_PASSWORD :
            errmsg = ACE_TEXT("Incorrect channel operator password");    break;
        case TT_CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED :
            errmsg = ACE_TEXT("Maximum bitrate for audio codec exceeded"); break;
        case TT_CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED :
            errmsg = ACE_TEXT("Maximum number of logins per IP-address exceeded"); break;
        case TT_CMDERR_MAX_CHANNELS_EXCEEDED :
            errmsg = ACE_TEXT("Maximum number of channels exceeded"); break;
        case TT_CMDERR_COMMAND_FLOOD :
            errmsg = ACE_TEXT("Commands are issued too fast. Flood protection."); break;
        case TT_CMDERR_CHANNEL_BANNED:
            errmsg = ACE_TEXT("Banned from channel"); break;

            // state errors
        case TT_CMDERR_NOT_LOGGEDIN :
            errmsg = ACE_TEXT("Not logged in");break;
        case TT_CMDERR_ALREADY_LOGGEDIN :
            errmsg = ACE_TEXT("Already logged in");break;
        case TT_CMDERR_NOT_IN_CHANNEL :
            errmsg = ACE_TEXT("Not in channel");break;
        case TT_CMDERR_ALREADY_IN_CHANNEL :
            errmsg = ACE_TEXT("Cannot join the same channel twice");break;
        case TT_CMDERR_CHANNEL_ALREADY_EXISTS :
            errmsg = ACE_TEXT("Channel already exists");break;
        case TT_CMDERR_CHANNEL_NOT_FOUND :
            errmsg = ACE_TEXT("Channel not found");break;
        case TT_CMDERR_USER_NOT_FOUND :
            errmsg = ACE_TEXT("User not found");    break;
        case TT_CMDERR_BAN_NOT_FOUND :
            errmsg = ACE_TEXT("Ban no found"); break;
        case TT_CMDERR_FILETRANSFER_NOT_FOUND :
            errmsg = ACE_TEXT("File transfer not found"); break;
        case TT_CMDERR_OPENFILE_FAILED :
            errmsg = ACE_TEXT("Server failed to create file"); break;            
        case TT_CMDERR_ACCOUNT_NOT_FOUND :
            errmsg = ACE_TEXT("User account not found"); break;
        case TT_CMDERR_FILE_NOT_FOUND :
            errmsg = ACE_TEXT("File not found"); break;
        case TT_CMDERR_FILE_ALREADY_EXISTS :
            errmsg = ACE_TEXT("File already exists"); break;
        case TT_CMDERR_FILESHARING_DISABLED :
            errmsg = ACE_TEXT("File sharing is disabled"); break;
        case TT_CMDERR_CHANNEL_HAS_USERS :
            errmsg = ACE_TEXT("Channel has active users"); break;
        case TT_CMDERR_LOGINSERVICE_UNAVAILABLE :
            errmsg = ACE_TEXT("Login service unavailable"); break;

            /***** Internal TeamTalk errors not related to commands ********/
        case TT_INTERR_SNDINPUT_FAILURE :
            errmsg = ACE_TEXT("Sound input device failure");break;
        case TT_INTERR_SNDOUTPUT_FAILURE :
            errmsg = ACE_TEXT("Sound output device failure");break;
        case TT_INTERR_AUDIOCODEC_INIT_FAILED :
            errmsg = ACE_TEXT("Audio codec failed to initialize");break;
        case TT_INTERR_AUDIOPREPROCESSOR_INIT_FAILED :
            errmsg = ACE_TEXT("Audio preprocessor failed to initialize");break;
        case TT_INTERR_MSGQUEUE_OVERFLOW :
            errmsg = ACE_TEXT("The internal message queue overflowed");break;
        case TT_INTERR_SNDEFFECT_FAILURE :
            errmsg = ACE_TEXT("An audio effect could not be applied on the sound device");break;
        default :
            errmsg = ACE_TEXT("Unknown error");break;
        }

        return errmsg;
    }

    bool HasProperty(const mstrings_t& properties, 
                     const ACE_TString& prop)
    {
        return properties.find(prop) != properties.end();
    }

    bool GetProperty(const mstrings_t& properties, 
        const ACE_TString& prop, 
        ACE_TString& value)
    {
        mstrings_t::const_iterator ite = properties.find(prop);
        if( ite != properties.end() )
        {
            value = (*ite).second;
            if(value.length()>MAX_STRING_LENGTH)
                value.resize(MAX_STRING_LENGTH);
            return true;
        }
        else
            return false;
    }

#define INT_OR_RET(val)                                        \
    do {                                                       \
        size_t i = 0;                                          \
        if(val.length() == 0) return false;                    \
        if(val[0] == '-') i++;                                 \
        for(;i<val.length();i++)                               \
            if(ACE_OS::ace_isdigit(val[i]) == 0) return false; \
    } while(0)

#define UINT_OR_RET(val)                                       \
    do {                                                       \
        size_t i = 0;                                          \
        if(val.length() == 0) return false;                    \
        for(;i<val.length();i++)                               \
            if(ACE_OS::ace_isdigit(val[i]) == 0) return false; \
    } while(0)

    bool GetProperty(const mstrings_t& properties, 
        const ACE_TString& prop, int& value)
    {
        mstrings_t::const_iterator ite = properties.find(prop);
        if( ite != properties.end())
        {
            INT_OR_RET((*ite).second);
            value = int(string2i((*ite).second));
            return true;
        }
        return false;
    }

    bool GetProperty(const mstrings_t& properties, 
                     const ACE_TString& prop, ACE_UINT32& value)
    {
        ACE_TString tmp;
        if(GetProperty(properties, prop, tmp))
        {
            UINT_OR_RET(tmp);
            value = ACE_UINT32(string2i(tmp));
            return true;
        }
        return false;
    }

    bool GetProperty(const mstrings_t& properties, 
        const ACE_TString& prop, bool& value)
    {
        int i = 0;
        bool b = GetProperty(properties, prop, i);
        if(b)
            value = i == 0? false : true;
        return b;
    }

    bool GetProperty(const mstrings_t& properties, 
        const ACE_TString& prop, ACE_INT64& value)
    {
        ACE_TString tmp;
        if(GetProperty(properties, prop, tmp))
        {
            INT_OR_RET(tmp);
            value = string2i(tmp);
            return true;
        }
        return false;
    }


    bool GetProperty(const mstrings_t& properties, 
        const ACE_TString& prop, std::vector<int>& vec)
    {
        ACE_TString value;
        mstrings_t::const_iterator ite = properties.find(prop);
        if( ite != properties.end() )
        {
            value = (*ite).second;
            ACE_TString token;
            size_t offset = 0;
            size_t i = value.find(',', offset);//Tokenize(ACE_TEXT(","),offset);
            while(i != ACE_TString::npos)
            {
                token = value.substr(offset, i-offset);
                offset = i+1;
                vec.push_back(int(string2i(token)));
                i = value.find(',', offset);

            }
            if( value.length() && (value.length() - 1) >= offset )
            {
                token = value.substr(offset, value.length()-offset);
                offset = i+1;
                vec.push_back(int(string2i(token)));
            }
            return true;
        }
        return false;
    }

    bool GetProperty(const mstrings_t& properties, 
        const ACE_TString& prop, 
        std::set<int>& myset)
    {
        std::vector<int> vec;
        bool b = GetProperty(properties, prop, vec);
        for(size_t i=0;i<vec.size();i++)
            myset.insert(vec[i]);
        return b;
    }

    bool GetProperty(const mstrings_t& properties, const ACE_TString& prop, 
                     AudioCodec& codec)
    {
        intvec_t codec_type;
        if(!GetProperty(properties, prop, codec_type) || codec_type.empty())
            return false;

        codec.codec = (Codec)codec_type[0];
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
            codec.speex.sim_stereo = codec_type[4];
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
            codec.speex_vbr.dtx = codec_type[5];
            codec.speex_vbr.frames_per_packet = codec_type[6];
            codec.speex_vbr.sim_stereo = codec_type[7];
            return true;
        case CODEC_OPUS :
            if(codec_type.size() < 11)
                return false;
            codec.opus.samplerate = codec_type[1];
            codec.opus.channels = codec_type[2];
            codec.opus.application = codec_type[3];
            codec.opus.complexity = codec_type[4];
            codec.opus.fec = codec_type[5];
            codec.opus.dtx = codec_type[6];
            codec.opus.bitrate = codec_type[7];
            codec.opus.vbr = codec_type[8];
            codec.opus.vbr_constraint = codec_type[9];
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

    bool GetProperty(const mstrings_t& properties, const ACE_TString& prop, 
                     AudioConfig& audcfg)
    {
        intvec_t audcfg_prop;
        GetProperty(properties, prop, audcfg_prop);

        if(audcfg_prop.size()<2)
            return false;

        /* do not change the order since it will break compatibility 
        * with older clients */
        audcfg.enable_agc = audcfg_prop[0];
        audcfg.gain_level = audcfg_prop[1];

        return true;
    }

    bool GetProperty(const mstrings_t& properties, const ACE_TString& prop, 
                     ACE_INET_Addr& addr)
    {
        ACE_TString str;
        if(GetProperty(properties, prop, str))
        {
            addr = ACE_INET_Addr(str.c_str());
            return true;
        }
        return false;
    }

    ACE_TString PrepareString(const ACE_TString& str)
    {
        ACE_TString newstr;
        if(str.length()>MAX_STRING_LENGTH)
            newstr = str.substr(0, MAX_STRING_LENGTH);
        else
            newstr = str;

        replace_all(newstr, ACE_TEXT("\\"), ACE_TEXT("\\\\"));
        replace_all(newstr, ACE_TEXT("\""), ACE_TEXT("\\\""));
        replace_all(newstr, ACE_TEXT("\r"), ACE_TEXT("\\r"));
        replace_all(newstr, ACE_TEXT("\n"), ACE_TEXT("\\n"));

        return newstr;
    }

    ACE_TString RebuildString(const ACE_TString& str)
    {
        ACE_TString resultstr = str, tmpstr;
        ACE_TCHAR search[3][3] = { ACE_TEXT("\\n"), ACE_TEXT("\\r"), ACE_TEXT("\\\"") };
        ACE_TCHAR replace[3][2] = { ACE_TEXT("\n"), ACE_TEXT("\r"), ACE_TEXT("\"") };

        for(size_t j = 0; j < 3; j++)
        {
            size_t pos1 = 0, oldpos = 0;
            tmpstr = resultstr;
            resultstr.clear();
            while( (pos1 = tmpstr.find(search[j], pos1)) != ACE_TString::npos)
            {
                int i = (int)pos1;
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

        replace_all(resultstr, ACE_TEXT("\\\\"), ACE_TEXT("\\"));
        return resultstr;
    }

    ACE_TString PrepareIntegerArray(const std::vector<int>& array)
    {
        ACE_TString s;
        for(int i=0;i<(int)array.size()-1;i++)
        {
            s = s + i2string(array[i]) + ACE_TEXT(",");
        }
        ACE_TString res;
        if(array.size()>0)
        {
            res = ACE_TEXT("[") + s + i2string(array[array.size()-1]) + ACE_TEXT("]");
        }
        else
            res = ACE_TEXT("[]");

        return res;
    }

    ACE_TString PrepareIntegerSet(const std::set<int>& myset)
    {
        ACE_TString s;
        int index = 0;
        std::set<int>::const_iterator ite;
        for(ite=myset.begin();index++<(int)myset.size()-1;ite++)
        {
            s = s + i2string(*ite) + ACE_TEXT(",");
        }

        ACE_TString res;
        if(myset.size()==1)
            res = ACE_TEXT("[") + i2string(*ite) + ACE_TEXT("]");
        else if(myset.size()>1)
            res = ACE_TEXT("[") + s + i2string(*ite) + ACE_TEXT("]");
        else
            res = ACE_TEXT("[]");

        return res;
    }

    size_t pastBlanks(size_t offset, const ACE_TString& input)
    {
        while(offset<input.length() && 
            (input[offset] == ' ' ||
            input[offset] == '\r' ||
            input[offset] == '\n')) offset ++;
        return offset;
    }

    int ExtractProperties(const ACE_TString& input, mstrings_t& properties)
    {
        TTASSERT(input.find('\n') == input.rfind('\n'));

        bool bSyntaxError = false;
        if( input.length() == 0 )
            bSyntaxError = true;

        size_t offset = input.find(' ');//past command
        if(offset == ACE_TString::npos)
            return 0;

        while(offset < input.length() && !bSyntaxError)
        {
            //past any spaces
            offset = pastBlanks(offset, input);
            if(offset == input.length())
            {
                break;
            }

            size_t propBegin = offset;
            ACE_TString prop;
            ACE_TString value;
            while(offset < input.length()) //extract property name
            {
                if( input[offset] != ' ' && input[offset] != '=') offset ++;
                else break;
            }
            if(offset == input.length())
            {
                bSyntaxError = true; //no properties in ACE_TString
                break;
            }

            prop = input.substr(propBegin, offset-propBegin); //set propertyname
            TTASSERT(properties.find(prop) == properties.end());
            offset = pastBlanks(offset, input); //past spaces
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
            else offset ++; //past =

            offset = pastBlanks(offset, input); //past spaces
            if(offset == input.length())
            {
                bSyntaxError = true;
                break;
            }

            //determine whether it's a string or an integer
            if(input[offset] == '"') //a ACE_TString
            {
                bool found = false;
                size_t strBegin = ++offset; //past "
                while(!found && offset<input.length())
                {
                    /*
                    if(input[offset]==ACE_TEXT('\"') && input[offset-1] != ACE_TEXT('\\')) found = true;
                    offset++;
                    */
                    if(input[offset] == '\\')
                        offset += 2;
                    else if(input[offset] == '"')
                    {
                        found = true;
                        offset++;
                    }
                    else
                        offset++;
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
                size_t listBegin = ++offset; //past "
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
                size_t intBegin = offset;
                while(offset<input.length() && 
                    input[offset] != ' ' && 
                    input[offset] != '\r' &&
                    input[offset] != '\n') offset ++; //past spaces
                value = input.substr(intBegin, offset-intBegin);

                properties[prop] = RebuildString(value);
                //properties.SetAt(prop, RebuildString(value));
            }
        }

        return bSyntaxError? -1 : (int)properties.size();
    }

    void AppendProperty(const ACE_TString& prop, 
        const ACE_TString& value, ACE_TString& dest_str)
    {
        ACE_TString newprop = ACE_TString(ACE_TEXT(" ")) + prop + ACE_TString(ACE_TEXT("=\"")) + PrepareString(value) + ACE_TString(ACE_TEXT("\""));
        dest_str += newprop;
    }

    void AppendProperty(const ACE_TString& prop, 
        const vector<int>& vecValues, ACE_TString& dest_str)
    {
        ACE_TString newprop = ACE_TString(ACE_TEXT(" ")) + prop + ACE_TString(ACE_TEXT("=")) + PrepareIntegerArray(vecValues);
        dest_str += newprop;
    }

    void AppendProperty(const ACE_TString& prop, 
        const set<int>& setValues, ACE_TString& dest_str)
    {
        ACE_TString newprop = ACE_TString(ACE_TEXT(" ")) + prop + ACE_TString(ACE_TEXT("=")) + PrepareIntegerSet(setValues);
        dest_str += newprop;
    }

    void AppendProperty(const ACE_TString& prop, ACE_INT64 value, ACE_TString& dest_str)
    {
        ACE_TString newprop = ACE_TString(ACE_TEXT(" ")) + prop + ACE_TString(ACE_TEXT("=")) + i2string(value);
        dest_str += newprop;
    }

    void AppendProperty(const ACE_TString& prop, const AudioCodec& codec, 
                        ACE_TString& dest_str)
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
            codec_prop.push_back(codec.speex.sim_stereo);
            break;
        case CODEC_SPEEX_VBR :
            /* do not change the order since it will break compatibility 
            * with older clients */
            codec_prop.push_back(codec.codec);
            codec_prop.push_back(codec.speex_vbr.bandmode);
            codec_prop.push_back(codec.speex_vbr.vbr_quality);
            codec_prop.push_back(codec.speex_vbr.bitrate);
            codec_prop.push_back(codec.speex_vbr.max_bitrate);
            codec_prop.push_back(codec.speex_vbr.dtx);
            codec_prop.push_back(codec.speex_vbr.frames_per_packet);
            codec_prop.push_back(codec.speex_vbr.sim_stereo);
            break;
        case CODEC_OPUS :
            codec_prop.push_back(codec.codec);
            codec_prop.push_back(codec.opus.samplerate);
            codec_prop.push_back(codec.opus.channels);
            codec_prop.push_back(codec.opus.application);
            codec_prop.push_back(codec.opus.complexity);
            codec_prop.push_back(codec.opus.fec);
            codec_prop.push_back(codec.opus.dtx);
            codec_prop.push_back(codec.opus.bitrate);
            codec_prop.push_back(codec.opus.vbr);
            codec_prop.push_back(codec.opus.vbr_constraint);
            codec_prop.push_back(codec.opus.frame_size);
            codec_prop.push_back(codec.opus.frames_per_packet);
            break;
        default :
            codec_prop.push_back(CODEC_NO_CODEC);
            TTASSERT(codec.codec != CODEC_NO_CODEC);
        }
        AppendProperty(prop, codec_prop, dest_str);
    }

    void AppendProperty(const ACE_TString& prop, const AudioConfig& audcfg, 
                        ACE_TString& dest_str)
    {
        intvec_t audcfg_prop;
        /* do not change the order since it will break compatibility 
        * with older clients */
        audcfg_prop.push_back(audcfg.enable_agc);
        audcfg_prop.push_back(audcfg.gain_level);

        AppendProperty(prop, audcfg_prop, dest_str);
    }

    void AppendProperty(const ACE_TString& prop, 
                        const ACE_INET_Addr& addr, ACE_TString& dest_str)
    {
        AppendProperty(prop, InetAddrToString(addr), dest_str);
    }

    bool GetCmdLine(const ACE_CString& input, ACE_CString& cmd, ACE_CString& remain_input)
    {
        if(input.find('\n') != ACE_TString::npos)
        {
            size_t pos = input.find('\n');
            cmd = input.substr(0, pos+1);
            size_t len = input.length();
            remain_input = input.substr(pos+1, len-pos+1);

            return true;
        }
        return false;
    }


    bool GetCmd(const ACE_CString& input, ACE_CString& cmd)
    {
        size_t nEndCommand = 0;
        while(nEndCommand < input.length() && 
            input[nEndCommand] != ' ' 
            && input[nEndCommand] != '\r' && 
            input[nEndCommand] != '\n')nEndCommand++;

        if(nEndCommand)
            cmd = input.substr(0,nEndCommand);
        return nEndCommand>0;
    }

    ACE_TString stripEOL(const ACE_TString& input)
    {
        size_t len = ACE_OS::strlen(EOL);
        if(input.length()<len)
            return input;
        if(input.substr(input.length()-len,len) == EOL)
            return input.substr(0,input.length()-len);
        else if(input.substr(input.length()-1,1) == ACE_TEXT("\n"))
            return input.substr(0,input.length()-1);
        return input;
    }

    ACE_TString InetAddrToString(const ACE_INET_Addr& addr)
    {
        ACE_TCHAR buf[MAX_STRING_LENGTH+1] = {};
        addr.addr_to_string(buf, MAX_STRING_LENGTH);
        return buf;
    }

}
