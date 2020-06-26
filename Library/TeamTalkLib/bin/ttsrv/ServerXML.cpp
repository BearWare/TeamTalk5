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

#include <ace/OS_NS_sys_socket.h>
#include "ServerXML.h"

#include <mystd/MyStd.h>
#include <TeamTalkDefs.h>
#include <stack>
#include <vector>

using namespace std;
namespace teamtalk{

    //////////////////////////////////////
    // class ServerXML
    //////////////////////////////////////

    ServerXML::ServerXML(const std::string& rootname)
        : XMLDocument(rootname, TEAMTALK_XML_VERSION)
    {
    }

    bool ServerXML::SaveFile()
    {
        SetFileVersion(TEAMTALK_XML_VERSION);
        return XMLDocument::SaveFile();
    }


    TiXmlElement* ServerXML::GetRootElement()
    {
        TiXmlElement* root = m_xmlDocument.RootElement();
        if(!root)
        {
            TiXmlElement newroot(m_rootname.c_str());
            newroot.SetAttribute("version", TEAMTALK_XML_VERSION);
            m_xmlDocument.InsertEndChild(newroot);
            root = m_xmlDocument.RootElement();
        }
        return root;
    }

    TiXmlElement* ServerXML::GetGeneralElement()
    {
        TiXmlElement* root = GetRootElement();
        if(root)
        {
            TiXmlElement* child = root->FirstChildElement("general");
            if(!child)
            {
                TiXmlElement newchild("general");
                return ReplaceElement(*root, newchild);
            }
            else
                return child;
        }
        return NULL;
    }

    TiXmlElement* ServerXML::GetFileStorageElement()
    {
        TiXmlElement* general = GetRootElement();
        if(general)
        {
            TiXmlElement* child = general->FirstChildElement("file-storage");
            if(!child)
            {
                TiXmlElement newchild("file-storage");
                return ReplaceElement(*general, newchild);
            }
            else
                return child;
        }
        return NULL;
    }

    TiXmlElement* ServerXML::GetLoggingElement()
    {
        TiXmlElement* general = GetRootElement();
        if(general)
        {
            TiXmlElement* child = general->FirstChildElement("logging");
            if(!child)
            {
                TiXmlElement newchild("logging");
                return ReplaceElement(*general, newchild);
            }
            else
                return child;
        }
        return NULL;
    }

    TiXmlElement* ServerXML::GetBandwidthLimitElement()
    {
        TiXmlElement* general = GetGeneralElement();
        if(general)
        {
            TiXmlElement* child = general->FirstChildElement("bandwidth-limits");
            if(!child)
            {
                TiXmlElement newchild("bandwidth-limits");
                return ReplaceElement(*general, newchild);
            }
            else
                return child;
        }
        return NULL;
    }

    TiXmlElement* ServerXML::GetUsersElement()
    {
        TiXmlElement* root = GetRootElement();
        if(root)
        {
            TiXmlElement* child = root->FirstChildElement("users");
            if(!child)
            {
                TiXmlElement newchild("users");
                return ReplaceElement(*root, newchild);
            }
            else
                return child;
        }
        return NULL;
    }


    TiXmlElement* ServerXML::GetServerBansElement()
    {
        TiXmlElement* root = GetRootElement();
        if(root)
        {
            TiXmlElement* child = root->FirstChildElement("serverbans");
            if(!child)
            {
                TiXmlElement newchild("serverbans");
                return ReplaceElement(*root, newchild);
            }
            else
                return child;
        }
        return NULL;
    }

    /*************************************************/
    /**************** <general> **********************/
    /*************************************************/

    std::string ServerXML::GetSystemID(const std::string& defwelcome)
    {
        return GetValue(true, "general/systemid", defwelcome);
    }
    
    bool ServerXML::SetServerName(string szServerName)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            PutString(*parent, "server-name", szServerName);
            return true;
        }
        else
            return false;
    }

    string ServerXML::GetServerName()
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            string s;
            GetString(*parent, "server-name", s);
            return s;
        }
        return "";
    }

    bool ServerXML::SetMessageOfTheDay(string szMsg)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            PutString(*parent, "motd", szMsg);
            return true;
        }
        else
            return false;
    }

    string ServerXML::GetMessageOfTheDay()
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            string s;
            GetString(*parent, "motd", s);
            return s;
        }
        return "";
    }

    bool ServerXML::SetBindIPs(std::vector<std::string> ips)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            for (auto child = parent->FirstChildElement("bind-ip");
                 child; child = parent->FirstChildElement("bind-ip"))
            {
                parent->RemoveChild(child);
            }
            
            for (auto ip : ips)
            {
                TiXmlElement bind("bind-ip");
                PutElementText(bind, ip);
                AppendElement(*parent, bind);
            }
            return true;
        }
        else
            return false;
    }

    std::vector<std::string> ServerXML::GetBindIPs()
    {
        string ip;
        std::vector<std::string> ips;
        
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            for (auto child = parent->FirstChildElement("bind-ip");
                 child; child = child->NextSiblingElement("bind-ip"))
            {
                GetElementText(*child, ip);
                if (std::find(ips.begin(), ips.end(), ip) == ips.end())
                    ips.push_back(ip);
            }
        }
        return ips;
    }

    bool ServerXML::SetHostTcpPort(int nHostTcpPort)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            PutInteger(*parent, "tcpport", nHostTcpPort);
            return true;
        }
        else
            return false;
    }

    int ServerXML::GetHostTcpPort()
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            int nValue = UNDEFINED;
            GetInteger(*parent, "tcpport", nValue);
            return nValue;
        }
        return UNDEFINED;
    }

    bool ServerXML::SetHostUdpPort(int nUdpPort)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            PutInteger(*parent, "udpport", nUdpPort);
            return true;
        }
        else
            return false;
    }

    int ServerXML::GetHostUdpPort()
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            int nValue = UNDEFINED;
            GetInteger(*parent, "udpport", nValue);
            return nValue;
        }
        return UNDEFINED;
    }

    bool ServerXML::SetMaxUsers(int nMax)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            PutInteger(*parent, "max-users", nMax);
            return true;
        }
        else
            return false;
    }

    int ServerXML::GetMaxUsers()
    {
        int nValue = UNDEFINED;
        TiXmlElement* hosting = GetGeneralElement();
        if(hosting)
            GetInteger(*hosting, "max-users", nValue);
        return nValue;
    }

    bool ServerXML::SetVoiceLogging(bool enable)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            PutBoolean(*parent, "voice-logging", enable);
            return true;
        }
        else
            return false;
    }

    bool ServerXML::GetVoiceLogging()
    {
        bool enabled = false;
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
            GetBoolean(*parent, "voice-logging", enabled);
        return enabled;
    }

    bool ServerXML::SetCertificateFile(const std::string& certfile)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            PutString(*parent, "certificate-file", certfile);
            return true;
        }
        else
            return false;
    }

    std::string ServerXML::GetCertificateFile()
    {
        string s;
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
            GetString(*parent, "certificate-file", s);
        return s;
    }

    bool ServerXML::SetPrivateKeyFile(const std::string& keyfile)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            PutString(*parent, "privatekey-file", keyfile);
            return true;
        }
        else
            return false;
    }

    std::string ServerXML::GetPrivateKeyFile()
    {
        string s;
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
            GetString(*parent, "privatekey-file", s);
        return s;
    }

    bool ServerXML::SetAutoSave(bool enable)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            PutBoolean(*parent, "auto-save", enable);
            return true;
        }
        else
            return false;
    }

    bool ServerXML::GetAutoSave()
    {
        bool enabled = true;
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
            GetBoolean(*parent, "auto-save", enabled);
        return enabled;
    }

    bool ServerXML::SetMaxLoginAttempts(int nMax)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            PutInteger(*parent, "login-attempts", nMax);
            return true;
        }
        else
            return false;
    }

    int ServerXML::GetMaxLoginAttempts()
    {
        int nValue = 0;
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
            GetInteger(*parent, "login-attempts", nValue);
        return nValue;
    }

    bool ServerXML::SetMaxLoginsPerIP(int max_ip_logins)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            PutInteger(*parent, "max-logins-per-ipaddr", max_ip_logins);
            return true;
        }
        else
            return false;
    }

    int ServerXML::GetMaxLoginsPerIP()
    {
        int nValue = 0;
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
            GetInteger(*parent, "max-logins-per-ipaddr", nValue);
        return nValue;
    }

    bool ServerXML::SetLoginDelay(int delaymsec)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            PutInteger(*parent, "login-delay-msec", delaymsec);
            return true;
        }
        else
            return false;
    }

    int ServerXML::GetLoginDelay()
    {
        int nValue = 0;
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
            GetInteger(*parent, "login-delay-msec", nValue);
        return nValue;
    }

    bool ServerXML::SetUserTimeout(int nTimeoutSec)
    {
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
        {
            PutInteger(*parent, "user-timeout", nTimeoutSec);
            return true;
        }
        else
            return false;
    }

    int ServerXML::GetUserTimeout()
    {
        int val = USER_TIMEOUT;
        TiXmlElement* parent = GetGeneralElement();
        if(parent)
            GetInteger(*parent, "user-timeout", val);
        return val;
    }

    bool ServerXML::SetVoiceTxLimit(int tx_bytes_per_sec)
    {
        TiXmlElement* parent = GetBandwidthLimitElement();
        if(parent)
        {
            PutInteger(*parent, "voicetx-limit", tx_bytes_per_sec);
            return true;
        }
        else
            return false;
    }

    int ServerXML::GetVoiceTxLimit()
    {
        int val = 0;
        TiXmlElement* parent = GetBandwidthLimitElement();
        if(parent)
            GetInteger(*parent, "voicetx-limit", val);
        return val;
    }

    bool ServerXML::SetVideoCaptureTxLimit(int tx_bytes_per_sec)
    {
        TiXmlElement* parent = GetBandwidthLimitElement();
        if(parent)
        {
            PutInteger(*parent, "vidcaptx-limit", tx_bytes_per_sec);
            return true;
        }
        else
            return false;
    }

    int ServerXML::GetVideoCaptureTxLimit()
    {
        int val = 0;
        TiXmlElement* parent = GetBandwidthLimitElement();
        if(parent)
            GetInteger(*parent, "vidcaptx-limit", val);
        return val;
    }

    bool ServerXML::SetMediaFileTxLimit(int tx_bytes_per_sec)
    {
        TiXmlElement* parent = GetBandwidthLimitElement();
        if(parent)
        {
            PutInteger(*parent, "mediafiletx-limit", tx_bytes_per_sec);
            return true;
        }
        else
            return false;
    }

    int ServerXML::GetMediaFileTxLimit()
    {
        int val = 0;
        TiXmlElement* parent = GetBandwidthLimitElement();
        if(parent)
            GetInteger(*parent, "mediafiletx-limit", val);
        return val;
    }

    bool ServerXML::SetDesktopTxLimit(int tx_bytes_per_sec)
    {
        TiXmlElement* parent = GetBandwidthLimitElement();
        if(parent)
        {
            PutInteger(*parent, "desktoptx-limit", tx_bytes_per_sec);
            return true;
        }
        else
            return false;
    }

    int ServerXML::GetDesktopTxLimit()
    {
        int val = 0;
        TiXmlElement* parent = GetBandwidthLimitElement();
        if(parent)
            GetInteger(*parent, "desktoptx-limit", val);
        return val;
    }

    bool ServerXML::SetTotalTxLimit(int tx_bytes_per_sec)
    {
        TiXmlElement* parent = GetBandwidthLimitElement();
        if(parent)
        {
            PutInteger(*parent, "totaltx-limit", tx_bytes_per_sec);
            return true;
        }
        else
            return false;
    }

    int ServerXML::GetTotalTxLimit()
    {
        int val = 0;
        TiXmlElement* parent = GetBandwidthLimitElement();
        if(parent)
            GetInteger(*parent, "totaltx-limit", val);
        return val;
    }
    /***** </bandwidth-limits> </general> *****/

    
    /***** </general> *****/

    bool ServerXML::SetServerLogMaxSize(_INT64 maxsize)
    {
        TiXmlElement* pParent = GetLoggingElement();
        if(pParent)
        {
            PutInteger(*pParent, "server-log-maxsize", maxsize);
            return true;
        }
        else
            return false;
    }

    _INT64 ServerXML::GetServerLogMaxSize()
    {
        _INT64 nValue = 0;
        TiXmlElement* hosting = GetLoggingElement();
        if(hosting)
            GetInteger(*hosting, "server-log-maxsize", nValue);
        return nValue;
    }

    /***** <file-storage> *****/
    bool ServerXML::SetMaxDiskUsage(_INT64 diskquota)
    {
        TiXmlElement* child = GetFileStorageElement();
        if(child)
        {
            PutInteger(*child, "max-diskusage", diskquota);
            return true;
        }
        return false;
    }

    _INT64 ServerXML::GetMaxDiskUsage()
    {
        _INT64 ret = 0;
        TiXmlElement* child = GetFileStorageElement();
        if(child)
            GetInteger(*child, "max-diskusage", ret);
        return ret;
    }

    bool ServerXML::SetDefaultDiskQuota(_INT64 diskquota)
    {
        TiXmlElement* child = GetFileStorageElement();
        if(child)
        {
            PutInteger(*child, "channel-diskquota", diskquota);
            return true;
        }
        return false;
    }

    _INT64 ServerXML::GetDefaultDiskQuota()
    {
        _INT64 ret = 0;
        TiXmlElement* child = GetFileStorageElement();
        if(child)
            GetInteger(*child, "channel-diskquota", ret);
        return ret;
    }
    
    bool ServerXML::SetFilesRoot(const std::string& filesroot)
    {
        TiXmlElement* child = GetFileStorageElement();
        if(child)
        {
            PutString(*child, "files-root", filesroot);
            return true;
        }
        return false;
    }

    std::string ServerXML::GetFilesRoot()
    {
        string ret;
        TiXmlElement* child = GetFileStorageElement();
        if(child)
            GetString(*child, "files-root", ret);
        return ret;
    }
    /***** </file-storage> *****/

    /***** <permanent-channels> ****/

    bool ServerXML::SetStaticChannels(const statchannels_t& channels)
    {
        int nChannelID = GetRootChannelID(channels);
        if(nChannelID > 0)
        {
            TiXmlElement set("permanent-channels");
            std::stack< ChannelProp > chanStack;
            std::stack< TiXmlElement* > xmlStack;

            statchannels_t::const_iterator ite = channels.find(nChannelID);
            assert(ite != channels.end());
            ChannelProp chan = ite->second;
            chanStack.push(chan);
            xmlStack.push(&set);

            while(chanStack.size()>0)
            {
                chan = chanStack.top();
                chanStack.pop();
                TiXmlElement* xmlElem = xmlStack.top();
                xmlStack.pop();

                TiXmlElement xmlChan("channel");

                //put comment with channel path
                TiXmlComment comment;
                string chpath = " Channel path: " + GetChannelPath(chan.channelid, channels) + " ";
                comment.SetValue(chpath.c_str());
                xmlChan.InsertEndChild(comment);

                //put channel properties
                PutInteger(xmlChan, "channel-id", chan.channelid);
                if(chan.parentid != 0)
                    PutString(xmlChan, "name", UnicodeToUtf8(chan.name).c_str());
                PutString(xmlChan, "password", UnicodeToUtf8(chan.passwd).c_str());
                PutString(xmlChan, "topic", UnicodeToUtf8(chan.topic).c_str());
                PutInteger(xmlChan, "diskquota", (_INT64)chan.diskquota);
                PutString(xmlChan, "op-password", UnicodeToUtf8(chan.oppasswd).c_str());
                PutInteger(xmlChan, "max-users", chan.maxusers);
                PutInteger(xmlChan, "channel-type", (int)chan.chantype);
                PutInteger(xmlChan, "userdata", chan.userdata);

                //save channel codec
                TiXmlElement codecElement("audio-codec");
                PutInteger(codecElement, "codec-type", chan.audiocodec.codec);
                switch(chan.audiocodec.codec)
                {
                default :
                case CODEC_NO_CODEC :
                    break;
                case CODEC_SPEEX :
                    PutInteger(codecElement, "bandmode", chan.audiocodec.speex.bandmode);
                    PutInteger(codecElement, "quality", chan.audiocodec.speex.quality);
                    PutInteger(codecElement, "fpp", chan.audiocodec.speex.frames_per_packet);
                    PutBoolean(codecElement, "sim-stereo", chan.audiocodec.speex.sim_stereo);
                    break;
                case CODEC_SPEEX_VBR :
                    PutInteger(codecElement, "bandmode", chan.audiocodec.speex_vbr.bandmode);
                    PutInteger(codecElement, "vbr-quality", chan.audiocodec.speex_vbr.vbr_quality);
                    PutInteger(codecElement, "bitrate", chan.audiocodec.speex_vbr.bitrate);
                    PutInteger(codecElement, "maxbitrate", chan.audiocodec.speex_vbr.max_bitrate);
                    PutBoolean(codecElement, "dtx", chan.audiocodec.speex_vbr.dtx);
                    PutInteger(codecElement, "fpp", chan.audiocodec.speex_vbr.frames_per_packet);
                    PutBoolean(codecElement, "sim-stereo", chan.audiocodec.speex_vbr.sim_stereo);
                    break;
                case CODEC_OPUS :
                    PutInteger(codecElement, "samplerate", chan.audiocodec.opus.samplerate);
                    PutInteger(codecElement, "channels", chan.audiocodec.opus.channels);
                    PutInteger(codecElement, "application", chan.audiocodec.opus.application);
                    PutInteger(codecElement, "complexity", chan.audiocodec.opus.complexity);
                    PutBoolean(codecElement, "fec", chan.audiocodec.opus.fec);
                    PutBoolean(codecElement, "dtx", chan.audiocodec.opus.dtx);
                    PutInteger(codecElement, "bitrate", chan.audiocodec.opus.bitrate);
                    PutBoolean(codecElement, "vbr", chan.audiocodec.opus.vbr);
                    PutBoolean(codecElement, "vbr-constraint", chan.audiocodec.opus.vbr_constraint);
                    PutInteger(codecElement, "framesize", chan.audiocodec.opus.frame_size);
                    PutInteger(codecElement, "fpp", chan.audiocodec.opus.frames_per_packet);
                    break;
                }
                ReplaceElement(xmlChan, codecElement);

                TiXmlElement audcfgElement("audio-config");
                PutBoolean(audcfgElement, "enable-agc", chan.audiocfg.enable_agc);
                PutInteger(audcfgElement, "gain-level", chan.audiocfg.gain_level);
                ReplaceElement(xmlChan, audcfgElement);

                TiXmlElement txusersElement("transmit-users");
                PutBoolean(txusersElement, "voice-tx-all",
                           chan.transmitusers[STREAMTYPE_VOICE].find(TRANSMITUSERS_FREEFORALL) != chan.transmitusers[STREAMTYPE_VOICE].end());
                PutBoolean(txusersElement, "videocapture-tx-all",
                           chan.transmitusers[STREAMTYPE_VIDEOCAPTURE].find(TRANSMITUSERS_FREEFORALL) != chan.transmitusers[STREAMTYPE_VIDEOCAPTURE].end());
                PutBoolean(txusersElement, "mediafile-tx-all",
                           chan.transmitusers[STREAMTYPE_MEDIAFILE].find(TRANSMITUSERS_FREEFORALL) != chan.transmitusers[STREAMTYPE_MEDIAFILE].end());
                PutBoolean(txusersElement, "desktopshare-tx-all",
                           chan.transmitusers[STREAMTYPE_DESKTOP].find(TRANSMITUSERS_FREEFORALL) != chan.transmitusers[STREAMTYPE_DESKTOP].end());
                ReplaceElement(xmlChan, txusersElement);

                //save channel files
                TiXmlElement filesElement("files");
                if(chan.files.size())
                {
                    for(size_t i=0;i<chan.files.size();i++)
                    {
                        TiXmlElement fileElement("file");
                        fileElement.SetAttribute("name", UnicodeToUtf8(chan.files[i].filename).c_str());
                        PutString(fileElement, "internalname", UnicodeToUtf8(chan.files[i].internalname).c_str());
                        PutInteger(fileElement, "filesize", (_INT64)chan.files[i].filesize);
                        PutString(fileElement, "username", UnicodeToUtf8(chan.files[i].username).c_str());
                        ReplaceElement(filesElement, fileElement);
                    }
                }
                ReplaceElement(xmlChan, filesElement);

                TiXmlElement bansElement("channelbans");
                if(chan.bans.size())
                {
                    for(size_t i=0;i<chan.bans.size();i++)
                    {
                        TiXmlElement banElement("channelban");
                        NewUserBan(banElement, chan.bans[i]);
                        AppendElement(bansElement, banElement);
                    }
                }
                ReplaceElement(xmlChan, bansElement);

                TiXmlElement* newChanElement = AppendElement(*xmlElem, xmlChan);
                assert(newChanElement);

                statchannels_t subs = GetSubChannels(chan.channelid, channels);
                for(ite=subs.begin();ite!=subs.end();ite++)
                {
                    xmlStack.push(newChanElement);
                    chanStack.push(ite->second);
                }
            }

            TiXmlElement* child = GetRootElement();
            ReplaceElement(*child, set);
        }

        return true;
    }

    bool ServerXML::GetStaticChannels(statchannels_t& channels)
    {
        TiXmlElement* root = GetRootElement();
        if(!root)
            return false;

        //create void channel
        ChannelProp parent;
        parent.parentid = 0;
        parent.channelid = 0;

        TiXmlElement* pStatic = root->FirstChildElement("permanent-channels");
        if(!pStatic)
            return false;

        std::stack<TiXmlElement*> xmlStack;
        xmlStack.push(pStatic);
        std::stack< ChannelProp > chanStack;
        chanStack.push(parent);

        while(xmlStack.size()>0)
        {
            TiXmlElement* item = xmlStack.top();
            xmlStack.pop();
            parent = chanStack.top();
            chanStack.pop();

            TiXmlElement* child = NULL;
            for(child = item->FirstChildElement("channel");
                child;
                child = child->NextSiblingElement("channel"))
            {
                string value;
                //create new channel
                ChannelProp newchan;
                GetInteger(*child, "channel-id", newchan.channelid);
                //if channel id is already in use we need to generate a new one
                while(channels.find(newchan.channelid) != channels.end())newchan.channelid++;
                if(child->Parent() != pStatic)
                    GetString(*child, "name", value); //make sure we don't give the root a name
                newchan.name = Utf8ToUnicode(value.c_str());
                newchan.parentid = parent.channelid;
                if(GetString(*child, "password", value))
                    newchan.passwd = Utf8ToUnicode(value.c_str());
                if(GetString(*child, "topic", value))
                    newchan.topic = Utf8ToUnicode(value.c_str());
                GetInteger(*child, "diskquota", (_INT64&)newchan.diskquota);
                if(GetString(*child, "op-password", value))
                    newchan.oppasswd = Utf8ToUnicode(value.c_str());
                newchan.maxusers = MAX_USERS_IN_CHANNEL;
                GetInteger(*child, "max-users", newchan.maxusers);
                GetInteger(*child, "channel-type", (int&)newchan.chantype);
                GetInteger(*child, "userdata", newchan.userdata);

                //get codec

                TiXmlElement* codecElement = child->FirstChildElement("audio-codec");
                if(codecElement)
                {
                    int codec = CODEC_NO_CODEC;
                    GetInteger(*codecElement, "codec-type", codec);
                    bool b = true;
                    switch(codec)
                    {
                    default :
                        codec = CODEC_NO_CODEC;
                    case CODEC_NO_CODEC :
                        break;
                    case CODEC_SPEEX :
                        b &= GetInteger(*codecElement, "bandmode", newchan.audiocodec.speex.bandmode);
                        b &= GetInteger(*codecElement, "quality", newchan.audiocodec.speex.quality);
                        b &= GetInteger(*codecElement, "fpp", newchan.audiocodec.speex.frames_per_packet);
                        newchan.audiocodec.speex.sim_stereo = false; //optional
                        GetBoolean(*codecElement, "sim-stereo", newchan.audiocodec.speex.sim_stereo);
                        break;
                    case CODEC_SPEEX_VBR :
                        b &= GetInteger(*codecElement, "bandmode", newchan.audiocodec.speex_vbr.bandmode);
                        GetInteger(*codecElement, "vbr-quality", newchan.audiocodec.speex_vbr.vbr_quality);
                        GetInteger(*codecElement, "bitrate", newchan.audiocodec.speex_vbr.bitrate);
                        GetInteger(*codecElement, "maxbitrate", newchan.audiocodec.speex_vbr.max_bitrate);
                        GetBoolean(*codecElement, "dtx", newchan.audiocodec.speex_vbr.dtx);
                        b &= GetInteger(*codecElement, "fpp", newchan.audiocodec.speex_vbr.frames_per_packet);
                        GetBoolean(*codecElement, "sim-stereo", newchan.audiocodec.speex_vbr.sim_stereo);
                        break;
                    case CODEC_OPUS :
                        b &= GetInteger(*codecElement, "samplerate", newchan.audiocodec.opus.samplerate);
                        b &= GetInteger(*codecElement, "channels", newchan.audiocodec.opus.channels);
                        b &= GetInteger(*codecElement, "application", newchan.audiocodec.opus.application);
                        b &= GetInteger(*codecElement, "complexity", newchan.audiocodec.opus.complexity);
                        b &= GetBoolean(*codecElement, "fec", newchan.audiocodec.opus.fec);
                        b &= GetBoolean(*codecElement, "dtx", newchan.audiocodec.opus.dtx);
                        b &= GetInteger(*codecElement, "bitrate", newchan.audiocodec.opus.bitrate);
                        b &= GetBoolean(*codecElement, "vbr", newchan.audiocodec.opus.vbr);
                        b &= GetBoolean(*codecElement, "vbr-constraint", newchan.audiocodec.opus.vbr_constraint);
                        b &= GetInteger(*codecElement, "framesize", newchan.audiocodec.opus.frame_size);
                        newchan.audiocodec.opus.frames_per_packet = 1;
                        GetInteger(*codecElement, "fpp", newchan.audiocodec.opus.frames_per_packet);
                        break;
                    }
                    if(b)
                        newchan.audiocodec.codec = (teamtalk::Codec)codec;
                }

                TiXmlElement* audcfgElement = child->FirstChildElement("audio-config");
                if(audcfgElement)
                {
                    GetBoolean(*audcfgElement, "enable-agc", newchan.audiocfg.enable_agc);
                    GetInteger(*audcfgElement, "gain-level", newchan.audiocfg.gain_level);
                }

                //get class-room free-for-all
                bool b = false;
                TiXmlElement* txusersElement = child->FirstChildElement("transmit-users");
                if(txusersElement)
                {
                    if(GetBoolean(*txusersElement, "voice-tx-all", b) && b)
                        newchan.transmitusers[STREAMTYPE_VOICE].insert(TRANSMITUSERS_FREEFORALL);
                    if(GetBoolean(*txusersElement, "videocapture-tx-all", b) && b)
                        newchan.transmitusers[STREAMTYPE_VIDEOCAPTURE].insert(TRANSMITUSERS_FREEFORALL);
                    if(GetBoolean(*txusersElement, "mediafile-tx-all", b) && b)
                        newchan.transmitusers[STREAMTYPE_MEDIAFILE].insert(TRANSMITUSERS_FREEFORALL);
                    if(GetBoolean(*txusersElement, "desktopshare-tx-all", b) && b)
                        newchan.transmitusers[STREAMTYPE_DESKTOP].insert(TRANSMITUSERS_FREEFORALL);
                }

                //get files
                TiXmlElement* files = child->FirstChildElement("files");
                if(files)
                {
                    TiXmlElement* nextfile = NULL;
                    for(nextfile=files->FirstChildElement("file");
                        nextfile && files;
                        nextfile=nextfile->NextSiblingElement("file"))
                    {
                        RemoteFile entry;
                        string tmp;
                        entry.filename = Utf8ToUnicode(nextfile->Attribute("name"));
                        if(GetString(*nextfile, "internalname", tmp))
                            entry.internalname = Utf8ToUnicode(tmp.c_str());
                        GetInteger(*nextfile, "filesize", (_INT64&)entry.filesize);
                        if(GetString(*nextfile, "username", tmp))
                            entry.username = Utf8ToUnicode(tmp.c_str());
                        newchan.files.push_back(entry);
                    }
                }

                TiXmlElement* bans = child->FirstChildElement("channelbans");
                if(bans)
                {
                    TiXmlElement* nextban = NULL;
                    for(nextban=bans->FirstChildElement("channelban");
                        nextban;
                        nextban=nextban->NextSiblingElement("channelban"))
                    {
                        BannedUser ban;
                        if(GetUserBan(*nextban, ban))
                            newchan.bans.push_back(ban);
                    }
                }

                //insert channel into set of static channels
                channels[newchan.channelid] = newchan;

                chanStack.push(newchan);
                xmlStack.push(child);
            }
        }

        return true;
    }
    /***** </permanent-channels> ****/

    std::vector<std::string> ServerXML::GetAdminIPAccessList()
    {
        vector<std::string> ips;
        TiXmlElement* parent = GetRootElement();
        if(parent)
        {
            TiXmlElement* admins = parent->FirstChildElement("admin-access");
            if(admins)
            {
                for(TiXmlElement* ip=admins->FirstChildElement("ip");
                    ip; ip = ip->NextSiblingElement("ip"))
                {
                    string ipStr;
                    GetElementText(*ip, ipStr);
                    ips.push_back(ipStr);
                }
            }
        }
        return ips;
    }


    /********** <serverbans> ************/
    void ServerXML::AddUserBan(const BannedUser& ban)
    {
        // prevent duplicates
        while(RemoveUserBan(ban));

        TiXmlElement* parent = GetServerBansElement();

        TiXmlElement element("serverban");
        NewUserBan(element, ban);
        if(parent)
            AppendElement(*parent, element);
    }

    bool ServerXML::RemoveUserBan(const BannedUser& ban)
    {
        int i = 0, c = GetUserBanCount();
        BannedUser tmp;
        while(GetUserBan(i, tmp) && !tmp.Same(ban)) i++;

        TiXmlElement* item = GetServerBansElement();
        if(i < c && item)
        {
            int e = 0;
            for(TiXmlElement* child = item->FirstChildElement("serverban");
                child; child = child->NextSiblingElement("serverban"))
            {
                if (i == e++)
                {
                    item->RemoveChild(child);
                    break;
                }
            }
        }
        return i != c;
    }

    bool ServerXML::GetUserBan(int index, BannedUser& ban)
    {
        TiXmlElement* item = GetServerBansElement();
        if(item)
        {
            int i = 0;
            for(TiXmlElement* child = item->FirstChildElement("serverban");
                child;
                child = child->NextSiblingElement("serverban"))
            {
                if(i == index)
                {
                    return GetUserBan(*child, ban);
                }
                i++;
            }
        }
        return false;
    }

    bool ServerXML::GetUserBan(const TiXmlElement& banElement, BannedUser& ban)
    {
        string tmp;
        if(banElement.Attribute("type"))
            ban.bantype = BanType(str2i(banElement.Attribute("type")));
        else
            ban.bantype = BANTYPE_DEFAULT;

        GetString(banElement, "bantime", tmp);
        ban.bantime = StringToDate(tmp);
        tmp.clear();
        GetString(banElement, "nickname", tmp);
        ban.nickname = Utf8ToUnicode(tmp.c_str());
        tmp.clear();
        GetString(banElement, "username", tmp);
        ban.username = Utf8ToUnicode(tmp.c_str());
        tmp.clear();
        GetString(banElement, "channel-path", tmp);
        ban.chanpath = Utf8ToUnicode(tmp.c_str());
        tmp.clear();
        if(!GetString(banElement, "ip-address", tmp) &&
           banElement.Attribute("address"))
            tmp = banElement.Attribute("address"); // pre XML v5.1
        ban.ipaddr = Utf8ToUnicode(tmp.c_str());
        return true;
    }
    
    void ServerXML::NewUserBan(TiXmlElement& banElement, const BannedUser& ban)
    {
        banElement.SetAttribute("type", i2str(ban.bantype).c_str());

        PutString(banElement, "bantime", DateToString(ban.bantime.sec()).c_str());
        PutString(banElement, "ip-address", UnicodeToUtf8(ban.ipaddr).c_str());
        PutString(banElement, "nickname", UnicodeToUtf8(ban.nickname).c_str());
        PutString(banElement, "username", UnicodeToUtf8(ban.username).c_str());
        PutString(banElement, "channel-path", UnicodeToUtf8(ban.chanpath).c_str());
    }

    int ServerXML::GetUserBanCount()
    {
        int c = 0;
        TiXmlElement* item = GetServerBansElement();
        if(item)
        {
            for(TiXmlElement* child = item->FirstChildElement("serverban");
                child;
                child = child->NextSiblingElement("serverban"))
            {
                c++;
            }
        }
        return c;
    }

    bool ServerXML::IsUserBanned(const BannedUser& ban)
    {
        int c = GetUserBanCount();
        for(int i=0;i<c;i++)
        {
            BannedUser tmp;
            if(GetUserBan(i, tmp) && tmp.Match(ban))
            {
                return true;
            }
        }
        return false;
    }

    void ServerXML::ClearUserBans()
    {
        TiXmlElement* item = GetServerBansElement();
        if(item)
            item->Clear();
    }

    std::vector<BannedUser> ServerXML::GetUserBans()
    {
        std::vector<BannedUser> result;
        for(int i=0;i<GetUserBanCount();i++)
        {
            BannedUser ban;
            GetUserBan(i, ban);
            result.push_back(ban);
        }
        return result;
    }
    /********** </serverbans> ************/

    /******* <users> ******/
    void ServerXML::AddNewUser(const UserAccount& user)
    {
        TiXmlElement* users = GetUsersElement();
        if(!users)
            return;

        TiXmlElement userElement("user");
        PutString(userElement, "username", UnicodeToUtf8(user.username).c_str());
        PutString(userElement, "password", UnicodeToUtf8(user.passwd).c_str());
        PutInteger(userElement, "user-type", (int)user.usertype);
        PutInteger(userElement, "user-rights", (int)user.userrights);
        PutString(userElement, "note", UnicodeToUtf8(user.note).c_str());
        PutInteger(userElement, "userdata", user.userdata);
        PutString(userElement, "init-channel", UnicodeToUtf8(user.init_channel).c_str());
        TiXmlElement opchanElement("channel-operator");
        for(intset_t::const_iterator i=user.auto_op_channels.begin();
            i!=user.auto_op_channels.end();i++)
        {
            TiXmlElement chanElement("channel");
            PutElementText(chanElement, i2str(*i));
            AppendElement(opchanElement, chanElement);
        }
        PutInteger(userElement, "audiocodec-bps-limit", user.audiobpslimit);

        TiXmlElement abuseElement("abuse-prevention");
        PutInteger(abuseElement, "commands-limit", user.abuse.n_cmds);
        PutInteger(abuseElement, "commands-interval-msec", user.abuse.cmd_msec);
        ReplaceElement(userElement, abuseElement);

        ReplaceElement(userElement, opchanElement);

        AppendElement(*users, userElement);
    }

    bool ServerXML::RemoveUser(const std::string& username)
    {
        TiXmlElement* users = GetUsersElement();
        if(!users)
            return false;
        TiXmlElement* userElement = users->FirstChildElement("user");
        while(userElement)
        {
            string tmp;
            GetString(*userElement, "username", tmp);
            if(username == tmp)
            {
                users->RemoveChild(userElement);
                return true;
            }
            userElement = userElement->NextSiblingElement("user");
        }
        return false;
    }

    bool ServerXML::GetNextUser(int index, UserAccount& user)
    {
        TiXmlElement* users = GetUsersElement();
        if(!users)
            return false;
        TiXmlElement* userElement = users->FirstChildElement("user");
        while(userElement && index-- > 0)
            userElement = userElement->NextSiblingElement("user");
        if(userElement)
            return GetUser(*userElement, user);
        return false;
    }

    bool ServerXML::GetUser(const TiXmlElement& userElement, UserAccount& user) const
    {
        bool b = true;
        string s1,s2, s3, s4;
        int user_type = 0, userdata = 0, userrights = 0, bpslimit = 0;
        b &= GetString(userElement, "username", s1);
        b &= GetString(userElement, "password", s2);
        b &= GetInteger(userElement, "user-type", user_type);
        b &= GetInteger(userElement, "user-rights", userrights);
        GetString(userElement, "note", s3);
        GetInteger(userElement, "userdata", userdata);
        GetString(userElement, "init-channel", s4);
        GetInteger(userElement, "audiocodec-bps-limit", bpslimit);

        if(b)
        {
            user.username = Utf8ToUnicode(s1.c_str());
            user.passwd = Utf8ToUnicode(s2.c_str());
            user.usertype = user_type;
            user.userrights = userrights;
            user.note = Utf8ToUnicode(s3.c_str());
            user.userdata = userdata;
            user.init_channel = Utf8ToUnicode(s4.c_str());
            user.audiobpslimit = bpslimit;
        }

        //enumerate auto-op channels
        const TiXmlElement* opchanElement = userElement.FirstChildElement("channel-operator");
        if(opchanElement)
        {
            opchanElement = opchanElement->FirstChildElement("channel");
            while(opchanElement)
            {
                string channel;
                GetElementText(*opchanElement, channel);
                user.auto_op_channels.insert(int(str2i(channel)));
                opchanElement = opchanElement->NextSiblingElement("channel");
            }
        }

        const TiXmlElement* abuseElement = userElement.FirstChildElement("abuse-prevention");
        if(abuseElement)
        {
            GetInteger(*abuseElement, "commands-limit", user.abuse.n_cmds);
            GetInteger(*abuseElement, "commands-interval-msec", user.abuse.cmd_msec);
        }

        return b;
    }


    bool ServerXML::AuthenticateUser(UserAccount& user)
    {
        int i = 0;
        UserAccount int_user;
        TiXmlElement* userElement = GetUser(UnicodeToUtf8(user.username).c_str());
        if(!userElement)
            return false;
        if(!GetUser(*userElement, int_user))
            return false;

        if(int_user.username == user.username &&
           int_user.passwd == user.passwd)
        {
            user = int_user;
            if(user.usertype == USERTYPE_ADMIN)
                user.userrights = USERRIGHT_ALL;
            return true;
        }
        return false;
    }
    
    TiXmlElement* ServerXML::GetUser(const std::string& username)
    {
        TiXmlElement* users = GetUsersElement();
        if(!users)
            return NULL;

        TiXmlElement* userElement = users->FirstChildElement("user");
        while(userElement)
        {
            string tmp;
            GetString(*userElement, "username", tmp);
            if(username == tmp)
                return userElement;
            userElement = userElement->NextSiblingElement("user");
        }
        return NULL;
    }

    bool ServerXML::GetUser(const std::string& username, UserAccount& user)
    {
        int i = 0;
        UserAccount int_user;
        while(GetNextUser(i++, int_user))
        {
            if(Utf8ToUnicode(username.c_str()) == int_user.username)
            {
                user = int_user;
                return true;
            }
            int_user = UserAccount(); //reset
        }
        return false;
    }

    /******* </users> ******/

    /********** files in static channels **************/
    TiXmlElement* ServerXML::GetChannelElement(const std::string& chpath)
    {
        TiXmlElement* pElement = GetRootElement();
        if(!pElement)
            return NULL;
        pElement = pElement->FirstChildElement("permanent-channels");
        if(!pElement)
            return NULL;
        stdstrings_t tokens = stdtokenize(chpath, "/");
        while(pElement && tokens.size())
        {
            pElement = pElement->FirstChildElement("channel");
            string val;
            if(GetString(*pElement, "name", val) && val == tokens[0])
                tokens.erase(tokens.begin());
            else
                pElement = pElement->NextSiblingElement("channel");
        }
        return pElement;
    }


    int GetRootChannelID(const statchannels_t& channels)
    {
        statchannels_t::const_iterator ite;
        for(ite=channels.begin();ite!=channels.end();ite++)
        {
            if(ite->second.parentid == 0)
                return ite->second.channelid;
        }
        return 0;
    }

    statchannels_t GetSubChannels(int nChannelID, const statchannels_t& channels)
    {
        statchannels_t subs;
        statchannels_t::const_iterator ite;
        for(ite=channels.begin();ite!=channels.end();ite++)
        {
            if(ite->second.parentid == nChannelID)
                subs.insert(*ite);
        }
        return subs;
    }

    std::string GetChannelPath(int nChannelID, const statchannels_t& channels)
    {
        string szPath;
        int nCurChanID = nChannelID;

        std::vector<string> names;
        while(nCurChanID != 0)
        {
            statchannels_t::const_iterator ite = channels.find(nCurChanID);
            if(ite == channels.end())
            {
                assert(0);
                break;
            }

            names.insert(names.begin(), UnicodeToUtf8(ite->second.name).c_str());
            nCurChanID = ite->second.parentid;
        }

        for(size_t i=0;i<names.size();i++)
            szPath += names[i] + "/";

        return szPath;
    }

    std::string DateToString(time_t t)
    {
        tm* tt = localtime(&t);
        char buff[100];
        sprintf(buff, "%.4d/%.2d/%.2d %.2d:%.2d", tt->tm_year+1900, tt->tm_mon+1, tt->tm_mday, tt->tm_hour, tt->tm_min);
        return buff;
    }

    time_t StringToDate(std::string date)
    {
        tm t = {};
        stdstrings_t tokens = stdtokenize(date, "/ :");
        if(tokens.size() == 5)
        {
            t.tm_isdst = 1;
            t.tm_year = int(str2i(tokens[0])-1900);
            t.tm_mon = int(str2i(tokens[1])-1);
            t.tm_mday = int(str2i(tokens[2]));
            t.tm_hour = int(str2i(tokens[3]));
            t.tm_min = int(str2i(tokens[4]));
            return mktime(&t);
        }
        return 0;
    }

}
