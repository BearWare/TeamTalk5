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

#if !defined(CHANNEL_H)
#define CHANNEL_H

#include <ace/SString.h>

#include <myace/MyACE.h>
#include "ttassert.h"
#include "Common.h"
#include "PacketLayout.h"

#include <map>
#include <vector>
#include <set>
#include <memory>

#define CHANNEL_SEPARATOR ACE_TEXT("/")

namespace teamtalk {

    template < typename CHANNEL, typename USER >
    class Channel
    {
    public:
        typedef std::shared_ptr< CHANNEL > channel_t;
        typedef std::shared_ptr< USER > user_t;
        typedef std::vector< channel_t > channels_t;
        typedef std::vector< user_t > users_t;

    private:
        // prevent copying
        Channel(const Channel& ch);
        const Channel& operator = (const Channel& ch);

        Channel(channel_t parent, int channelid, ChannelTypes chantype, const ACE_TString& name)
            : m_protected(false)
            , m_maxusers(MAX_USERS_IN_CHANNEL)
            , m_channelid(channelid)
            , m_userdata(0)
            , m_maxdiskusage(0)
            , m_chantype(chantype)
            , m_parent(parent)
            , m_name(name)
        {
            m_audiocodec.codec = CODEC_NO_CODEC;

            // ensure we can use std::map<>.at()
            m_transmitusers[STREAMTYPE_VOICE] = std::set<int>();
            m_transmitusers[STREAMTYPE_VIDEOCAPTURE] = std::set<int>();
            m_transmitusers[STREAMTYPE_DESKTOP] = std::set<int>();
            m_transmitusers[STREAMTYPE_MEDIAFILE] = std::set<int>();
            m_transmitusers[STREAMTYPE_CHANNELMSG] = std::set<int>();
        }

    public:
        Channel(int channelid)    //create a root
            : Channel(channel_t(), channelid, CHANNEL_DEFAULT | CHANNEL_PERMANENT, ACE_TString())
        {
            //MYTRACE("New channel: %s\n", m_name.c_str());
        }
        Channel(channel_t parent, int channelid, const ACE_TString& name)    //create a sub channel
            : Channel(parent, channelid, CHANNEL_DEFAULT, name)
        {
            //MYTRACE("New channel: %s\n", name.c_str());
        }
        virtual ~Channel()
        {
            //TTASSERT(m_mUsers.empty());
            //TTASSERT(m_vecUsers.empty());
            MYTRACE(ACE_TEXT("~Channel() - \"%s\"\n"), m_name.c_str());
        }
        channel_t GetParentChannel() const { return m_parent.lock(); }
        bool IsRootChannel() const { return !GetParentChannel(); }
        void SetName(const ACE_TString& name) { m_name = name; }
        const ACE_TString& GetName() const{return m_name;}
        void SetPassword(const ACE_TString& password){ m_password = password; }
        const ACE_TString& GetPassword() const {return m_password;}
        void SetOpPassword(const ACE_TString& oppasswd) { m_oppasswd = oppasswd; }
        const ACE_TString& GetOpPassword() const { return m_oppasswd; }
        void SetPasswordProtected(bool protect){m_protected = protect;}
        bool IsPasswordProtected() const{return m_protected || m_password.length();}
        void SetTopic(const ACE_TString& topic){m_topic = topic;}
        const ACE_TString& GetTopic() const{return m_topic;}
        void SetMaxDiskUsage(ACE_INT64 maxSize) { m_maxdiskusage = maxSize; }
        ACE_INT64 GetMaxDiskUsage() const { return m_maxdiskusage; }
        void SetMaxUsers(int maxusers) { m_maxusers = maxusers; }
        int GetMaxUsers() const { return m_maxusers; }
        int GetChannelID() const { return m_channelid; }
        void SetAudioCodec(const AudioCodec& codec) { m_audiocodec = codec; }
        const AudioCodec& GetAudioCodec() const { return m_audiocodec; }
        void SetChannelType(ChannelTypes chantype){ m_chantype = chantype; }
        void SetAudioConfig(const AudioConfig& audiocfg) { m_audiocfg = audiocfg; }
        const AudioConfig& GetAudioConfig() const { return m_audiocfg; }
        ChannelTypes GetChannelType() const { return m_chantype; }
        void SetUserData(int userdata) { m_userdata = userdata; }
        int GetUserData() const { return m_userdata; }
        ACE_TString GetChannelPath() const
        {
            ACE_TString pwc = GetName() + ACE_TString(CHANNEL_SEPARATOR);
            channel_t channel = GetParentChannel();

            while(channel)
            {
                pwc = channel->GetName() + ACE_TString(CHANNEL_SEPARATOR) + pwc;
                channel = channel->GetParentChannel();
            }

            return pwc;
        }

        void AddUser(int userid, user_t& user)
        {
            TTASSERT(m_mUsers.find(user->GetUserID()) == m_mUsers.end());
            m_mUsers[userid] = user;
            m_vecUsers.push_back(user);
        }
        void RemoveUser(int userid)
        {
            TTASSERT(m_mUsers.find(userid) != m_mUsers.end());
            TTASSERT(m_vecUsers.size());
            channel_t nullc;
            //remove client in this channel
            m_mUsers.erase(userid);

            for(size_t i=0;i<m_vecUsers.size();)
            {
                if(m_vecUsers[i]->GetUserID() == userid)
                    m_vecUsers.erase(m_vecUsers.begin()+i);
                else i++;
            }
        }
        int GetUsersCount() const
        {
            return (int)m_mUsers.size();
        }
        const users_t& GetUsers() const
        {
            return m_vecUsers;
        }
        int GetUsers(users_t& users, bool recursive = false) const
        {
            users.insert(users.end(), m_vecUsers.begin(), m_vecUsers.end());
            if(recursive)
            {
                for(size_t i=0;i<m_subChannels.size();i++)
                    m_subChannels[i]->GetUsers(users, recursive);
            }
            return (int)users.size();
        }
        void SetOperators(const std::set<int>& setOps)
        {
            m_setOps = setOps;
        }
        void AddOperator(int userid)
        {
            m_setOps.insert(userid);
        }
        bool IsOperator(int userid) const
        {
            return m_setOps.find(userid) != m_setOps.end();
        }
        std::set<int> RemoveOperator(int userid, bool recursive = false)
        {
            std::set<int> chanids;

            if(m_setOps.find(userid) != m_setOps.end())
            {
                m_setOps.erase(m_setOps.find(userid));
                chanids.insert(GetChannelID());
            }
            if(recursive)
            {
                std::set<int> subids;
                for(size_t i=0;i<m_subChannels.size();i++)
                {
                    subids = m_subChannels[i]->RemoveOperator(userid, recursive);
                    chanids.insert(subids.begin(), subids.end());
                }
            }
            return chanids;
        }
        const std::set<int>& GetOperators() const
        {
            return m_setOps;
        }
        std::set<int> ClearTransmitUser(int userid, bool recursive = false)
        {
            std::set<int> chanids;
            if(m_transmitusers[STREAMTYPE_VOICE].find(userid) != m_transmitusers[STREAMTYPE_VOICE].end())
                chanids.insert(GetChannelID());
            else if(m_transmitusers[STREAMTYPE_VIDEOCAPTURE].find(userid) != m_transmitusers[STREAMTYPE_VIDEOCAPTURE].end())
                chanids.insert(GetChannelID());
            else if(m_transmitusers[STREAMTYPE_DESKTOP].find(userid) != m_transmitusers[STREAMTYPE_DESKTOP].end())
                chanids.insert(GetChannelID());
            else if(m_transmitusers[STREAMTYPE_MEDIAFILE].find(userid) != m_transmitusers[STREAMTYPE_MEDIAFILE].end())
                chanids.insert(GetChannelID());

            m_transmitusers[STREAMTYPE_VOICE].erase(userid);
            m_transmitusers[STREAMTYPE_VIDEOCAPTURE].erase(userid);
            m_transmitusers[STREAMTYPE_DESKTOP].erase(userid);
            m_transmitusers[STREAMTYPE_MEDIAFILE].erase(userid);

            if(recursive)
            {
                std::set<int> subids;
                for(size_t i=0;i<m_subChannels.size();i++)
                {
                    subids = m_subChannels[i]->ClearTransmitUser(userid, recursive);
                    chanids.insert(subids.begin(), subids.end());
                }
            }
            return chanids;
        }
        user_t GetUser(int userid, bool recursive = false) const
        {
            //find user with userid
            user_t user;
            typename mapuser_t::const_iterator ite = m_mUsers.find(userid);
            if(ite == m_mUsers.end() && recursive)
            {
                //search in sub channels
                bool found = false;
                for(size_t i=0;i<m_subChannels.size() && !found;i++)
                {
                    user = m_subChannels[i]->GetUser(userid, recursive);
                    if(user)
                        found = true;
                }
            }
            else if(ite != m_mUsers.end())
                user = (*ite).second;

            return user;
        }
        bool UserExists(int userid) const
        {
            return m_mUsers.find(userid) != m_mUsers.end();
        }
        bool Compare(const channel_t& channel) const
        {
            if (!channel)
                return false;
            else
                return stringcmpnocase(channel->GetChannelPath(), GetChannelPath());
        }
        void AddSubChannel(channel_t& new_channel)
        {
            TTASSERT(new_channel);
            TTASSERT(new_channel->GetName().length());
            TTASSERT(new_channel->GetParentChannel());
            m_subChannels.push_back(new_channel);
        }
        void RemoveSubChannel(const ACE_TString& name)
        {
            for(size_t i=0;i<m_subChannels.size();i++)
            {
                if(stringcmpnocase(m_subChannels[i]->GetName(), name))
                {
                    m_subChannels.erase(m_subChannels.begin()+i);
                    break;
                }
            }
        }
        int GetSubChannelCount(bool recursive = false) const
        {
            int count = (int)m_subChannels.size();
            if(recursive)
            {
                for(size_t i=0;i<m_subChannels.size();i++)
                    count += m_subChannels[i]->GetSubChannelCount(recursive);
            }
            return count;
        }
        channel_t GetSubChannel(const ACE_TString& name) const
        {
            channel_t sub;
            for(size_t i=0;i<m_subChannels.size();i++)
                if(stringcmpnocase(m_subChannels[i]->GetName(), name))
                {
                    sub = m_subChannels[i];
                    break;
                }

                return sub;
        }
        const channels_t& GetSubChannels() const
        {
            return m_subChannels;
        }
        channel_t GetSubChannel(int nChannelID, bool recursive = false) const
        {
            channel_t channel;
            for(size_t i=0;i<m_subChannels.size();i++)
            {
                if(m_subChannels[i]->GetChannelID() == nChannelID)
                {
                    channel = m_subChannels[i];
                    break;
                }
                else if(recursive)
                {
                    channel = m_subChannels[i]->GetSubChannel(nChannelID, recursive);
                    if(channel)
                        break;
                }
            }
            return channel;
        }

        ACE_INT64 GetDiskUsage() const
        {
            ACE_INT64 total = 0;
            mfiles_t::const_iterator ite=m_files.begin();
            while(ite != m_files.end())
            {
                total += ite->second.filesize;
                ite++;
            }
            return total;
        }
        void AddFile(const RemoteFile& remotefile)
        {
            TTASSERT(remotefile.channelid == m_channelid);
            m_files[remotefile.filename] = remotefile;
        }
        void RemoveFile(const ACE_TString& filename)
        {
            m_files.erase(filename);
        }
        void ClearFiles()
        {
            m_files.clear();
        }
        bool FileExists(const ACE_TString& filename) const
        {
            return m_files.find(filename) != m_files.end();
        }
        bool FileExists(int fileid, bool recursive = false) const
        {
            RemoteFile rfile;
            return GetFile(fileid, rfile, recursive);
        }
        bool GetFile(const ACE_TString& filename, RemoteFile& remotefile) const
        {
            mfiles_t::const_iterator ite = m_files.find(filename);
            if(ite != m_files.end())
            {
                remotefile = ite->second;
                return true;
            }
            return false;
        }
        bool GetFile(int fileid, RemoteFile& remotefile, bool recursive = false) const
        {
            mfiles_t::const_iterator ite=m_files.begin();
            while(ite != m_files.end())
            {
                if(ite->second.fileid == fileid)
                {
                    remotefile = ite->second;
                    return true;
                }
                ite++;
            }
            for(size_t i=0;i<m_subChannels.size() && recursive;i++)
            {
                if(m_subChannels[i]->GetFile(fileid, remotefile, recursive))
                    return true;
            }
            return false;
        }
        void GetFiles(files_t& files, bool recursive = false) const
        {
            mfiles_t::const_iterator ite=m_files.begin();
            while(ite != m_files.end())
            {
                files.push_back(ite->second);
                ite++;
            }
            if(recursive)
            {
                for(size_t i=0;i<m_subChannels.size();i++)
                    m_subChannels[i]->GetFiles(files, recursive);
            }
        }
        int GetFilesCount(bool recursive = false) const
        {
            int count = (int)m_files.size();
            for(size_t i=0;i<m_subChannels.size() && recursive;i++)
                count += m_subChannels[i]->GetFilesCount(recursive);
            return count;
        }

        void SetVoiceUsers(const std::set<int>& userids) { m_transmitusers[STREAMTYPE_VOICE] = userids; }
        const std::set<int>& GetVoiceUsers() const { return m_transmitusers.at(STREAMTYPE_VOICE); }

        void SetVideoUsers(const std::set<int>& userids) { m_transmitusers[STREAMTYPE_VIDEOCAPTURE] = userids; }
        const std::set<int>& GetVideoUsers() const { return m_transmitusers.at(STREAMTYPE_VIDEOCAPTURE); }

        void SetDesktopUsers(const std::set<int>& userids) { m_transmitusers[STREAMTYPE_DESKTOP] = userids; }
        const std::set<int>& GetDesktopUsers() const { return m_transmitusers.at(STREAMTYPE_DESKTOP); }

        void SetMediaFileUsers(const std::set<int>& userids) { m_transmitusers[STREAMTYPE_MEDIAFILE] = userids; }
        const std::set<int>& GetMediaFileUsers() const { return m_transmitusers.at(STREAMTYPE_MEDIAFILE); }

        void SetChannelTextMsgUsers(const std::set<int>& userids) { m_transmitusers[STREAMTYPE_CHANNELMSG] = userids; }
        const std::set<int>& GetChannelTextMsgUsers() const { return m_transmitusers.at(STREAMTYPE_CHANNELMSG); }

        bool CanTransmit(int userid, StreamType txtype)
        {
            const std::set<int>& txusers = m_transmitusers[txtype];

            if ((m_chantype & CHANNEL_CLASSROOM) &&
                txusers.find(userid) == txusers.end() &&
                txusers.find(TRANSMITUSERS_FREEFORALL) == txusers.end())
                return false;

            if ((m_chantype & CHANNEL_CLASSROOM) == CHANNEL_DEFAULT &&
                txusers.find(userid) != txusers.end())
                return false;

            return true;
        }

        void SetTransmitQueue(const std::vector<int>& users)
        {
            m_transmitqueue = users;
        }

        void SetTransmitSwitchDelay(const ACE_Time_Value& tm)
        {
            m_transmitswitch_delay = tm;
        }

        const ACE_Time_Value& GetTransmitSwitchDelay() const
        {
            return m_transmitswitch_delay;
        }

        const std::vector<int>& GetTransmitQueue() const
        {
            return m_transmitqueue;
        }

        void AddUserBan(const BannedUser& ban) { RemoveUserBan(ban); m_bans.push_back(ban); }

        bool IsBanned(const BannedUser& testban) const
        {
            auto i = std::find_if(m_bans.begin(), m_bans.end(),
                                  [testban](BannedUser ban)
                                  {
                                      return ban.Match(testban);
                                  });
            return i != m_bans.end();
        }

        void RemoveUserBan(const BannedUser& ban)
        {
            auto i = std::find_if(m_bans.begin(), m_bans.end(),
                                  [ban](BannedUser testban)
                                  {
                                      return ban.Same(testban);
                                  });
            if(i != m_bans.end())
                m_bans.erase(i);
        }

        const std::vector<BannedUser>& GetBans() const { return m_bans; }

        ChannelProp GetChannelProp() const
        {
            ChannelProp prop;
            prop.name = m_name;
            prop.passwd = m_password;
            prop.topic = m_topic;
            prop.oppasswd = m_oppasswd;
            prop.diskquota = m_maxdiskusage;
            prop.maxusers = m_maxusers;
            prop.bProtected = IsPasswordProtected();
            prop.setops = m_setOps;
            prop.channelid = m_channelid;
            prop.chantype = m_chantype;
            if(GetParentChannel())
                prop.parentid = GetParentChannel()->GetChannelID();
            else
                prop.parentid = 0;
            prop.userdata = m_userdata;
            prop.audiocodec = m_audiocodec;
            prop.audiocfg = m_audiocfg;
            GetFiles(prop.files, false);
            prop.transmitusers = m_transmitusers;
            prop.transmitqueue = m_transmitqueue;
            prop.transmitswitchdelay = int(GetTransmitSwitchDelay().msec());
            prop.bans = m_bans;
            return prop;
        }

#if defined(ENABLE_ENCRYPTION)
        void SetEncryptKey(const uint8_t* cryptkey)
        {
            for(int i=0;i<CRYPTKEY_SIZE;i++)
                m_cryptkey[i] = cryptkey[i];
        }
        const uint8_t* GetEncryptKey() const { return m_cryptkey; }
    protected:
        uint8_t m_cryptkey[CRYPTKEY_SIZE];
#endif

    protected:
        //pair element for m_mUsers
        typedef std::map<int, user_t > mapuser_t;
        //users in channel
        mapuser_t m_mUsers;
        users_t m_vecUsers; //cached users for fast access (same as in 'm_mUsers')
        //child channels
        channels_t m_subChannels;
        ACE_TString m_name;
        ACE_TString m_password;
        ACE_TString m_oppasswd;
        ACE_TString m_topic;
        std::set< int > m_setOps;
        std::weak_ptr< CHANNEL > m_parent;
        bool m_protected;
        ACE_INT64 m_maxdiskusage;
        typedef std::map<ACE_TString, RemoteFile> mfiles_t;
        mfiles_t m_files;
        int m_maxusers;
        int m_channelid;
        int m_userdata;
        AudioCodec m_audiocodec;
        AudioConfig m_audiocfg;
        ChannelTypes m_chantype;
        //classroom transmission
        transmitusers_t m_transmitusers;
        //solo transmission
        std::vector<int> m_transmitqueue;
        ACE_Time_Value m_transmitswitch_delay;
        bannedusers_t m_bans;
    };

    /**** Global helper functions ****/

    strings_t TokenizeChannelPath(const ACE_TString& str);

    bool ChannelsEquals(const ACE_TString& chanpath1, const ACE_TString& chanpath2);

    template < class CHANNEL >
    std::shared_ptr< CHANNEL > ChangeChannel(std::shared_ptr< CHANNEL > channel, const strings_t& tokNames)
    {
        for(size_t i=0;i<tokNames.size() && channel;i++)
            channel = channel->GetSubChannel(tokNames[i]);

        return channel;
    }

    template < class CHANNEL >
    std::shared_ptr< CHANNEL > ChangeChannel(std::shared_ptr< CHANNEL > channel, const ACE_TString& chanpath)
    {
        std::shared_ptr< CHANNEL > c;
        strings_t tokNames = TokenizeChannelPath(chanpath);
        c = ChangeChannel< CHANNEL >(channel, tokNames);
        return c;
    }

    template < class CHANNEL >
    std::set<int> GetChannelIDs(std::shared_ptr< CHANNEL > channel,
                                const strings_t& channelpaths)
    {
        std::set<int> result;
        std::shared_ptr< CHANNEL > tmp;
        for(size_t i=0;i<channelpaths.size();i++)
        {
            tmp = ChangeChannel(channel, channelpaths[i]);
            if (tmp)
                result.insert(tmp->GetChannelID());
        }
        return result;
    }

    template < class CHANNEL >
    strings_t GetChannelPaths(std::shared_ptr< CHANNEL > channel,
                              const std::set<int>& channelids)
    {
        strings_t result;

        std::shared_ptr< CHANNEL > tmp;
        std::set<int>::const_iterator ii = channelids.begin();
        for(;ii!=channelids.end();ii++)
        {
            if(channel->GetChannelID() == *ii)
                tmp = channel;
            else
                tmp = channel->GetSubChannel(*ii, true);

            if (tmp)
                result.push_back(tmp->GetChannelPath());
        }
        return result;
    }
}
#endif
