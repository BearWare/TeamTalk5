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

#ifndef SERVERSETTINGS_H
#define SERVERSETTINGS_H

#include <settings/Settings.h>
#include <teamtalk/Common.h>

#define TEAMTALK_XML_VERSION                    "5.1"

namespace teamtalk {

    typedef std::map< int, ChannelProp > statchannels_t;

    int GetRootChannelID(const statchannels_t& channels); //success > 0
    statchannels_t GetSubChannels(int nChannelID, const statchannels_t& channels);
    std::string GetChannelPath(int nChannelID, const statchannels_t& channels);

    std::string DateToString(time_t t);
    time_t StringToDate(std::string date);

    class ServerXML : public teamtalk::XMLDocument
    {
    public:
        ServerXML(const std::string& rootname);
        virtual bool SaveFile();

        TiXmlElement* GetRootElement();

        /***** <general> *****/
        std::string GetSystemID(const std::string& defwelcome);
        
        bool SetServerName(std::string szServerName);
        std::string GetServerName();

        bool SetMessageOfTheDay(std::string szMsg);
        std::string GetMessageOfTheDay();

        bool SetBindIPs(std::vector<std::string> ips);
        std::vector<std::string> GetBindIPs();

        bool SetHostTcpPort(int nHostTcpPort);
        int GetHostTcpPort();

        bool SetHostUdpPort(int nUdpPort);
        int GetHostUdpPort();

        bool SetMaxUsers(int nMax);
        int GetMaxUsers();

        bool SetVoiceLogging(bool enable);
        bool GetVoiceLogging();

        bool SetCertificateFile(const std::string& certfile);
        std::string GetCertificateFile();
        bool SetPrivateKeyFile(const std::string& keyfile);
        std::string GetPrivateKeyFile();

        bool SetAutoSave(bool enable);
        bool GetAutoSave();

        bool SetMaxLoginAttempts(int nMax);
        int GetMaxLoginAttempts();

        bool SetUserTimeout(int nTimeoutSec);
        int GetUserTimeout();

        bool SetMaxLoginsPerIP(int max_ip_logins);
        int GetMaxLoginsPerIP();

        bool SetLoginDelay(int delaymsec);
        int GetLoginDelay();
        
        /***** <bandwidth-limits> *****/

        bool SetVoiceTxLimit(int tx_bytes_per_sec);
        int GetVoiceTxLimit();

        bool SetVideoCaptureTxLimit(int tx_bytes_per_sec);
        int GetVideoCaptureTxLimit();

        bool SetMediaFileTxLimit(int tx_bytes_per_sec);
        int GetMediaFileTxLimit();

        bool SetDesktopTxLimit(int tx_bytes_per_sec);
        int GetDesktopTxLimit();

        bool SetTotalTxLimit(int tx_bytes_per_sec);
        int GetTotalTxLimit();
        /***** </bandwidth-limits> *****/

        bool SetDefaultDiskQuota(_INT64 diskquota);
        _INT64 GetDefaultDiskQuota();
                
        /***** </general> *****/

        /***** <logging> *****/
        bool SetServerLogMaxSize(_INT64 maxsize);
        _INT64 GetServerLogMaxSize();
        /***** </logging> *****/

        /***** <static-channels> ****/
        bool SetStaticChannels(const statchannels_t& channels);
        bool GetStaticChannels(statchannels_t& channels);
        /***** </static-channels> ****/

        /***** <file-storage> *****/
        bool SetFilesRoot(const std::string& filesroot);
        std::string GetFilesRoot();

        bool SetMaxDiskUsage(_INT64 diskquota);
        _INT64 GetMaxDiskUsage();
        /***** </file-storage> *****/

        std::vector<std::string> GetAdminIPAccessList();

        /******** <users> *******/
        void AddNewUser(const UserAccount& user);
        bool RemoveUser(const std::string& username);
        bool GetNextUser(int index, UserAccount& user);
        bool AuthenticateUser(UserAccount& user);
        bool GetUser(const std::string& username, UserAccount& user);
        /****** </users> *****/

        /********** <serverbans>  ************/
        void AddUserBan(const BannedUser& ban);
        bool RemoveUserBan(const BannedUser& ban);
        bool GetUserBan(int index, BannedUser& ban);
        int GetUserBanCount();
        bool IsUserBanned(const BannedUser& ban);
        void ClearUserBans();
        std::vector<BannedUser> GetUserBans();
        /********** </serverbans> ************/
    private:
        /**** Sections ****/
        TiXmlElement* GetGeneralElement();
        TiXmlElement* GetFileStorageElement();
        TiXmlElement* GetLoggingElement();
        TiXmlElement* GetBandwidthLimitElement();
        TiXmlElement* GetServerBansElement();
        TiXmlElement* GetUsersElement();
        TiXmlElement* GetChannelElement(const std::string& chpath);
        TiXmlElement* GetUser(const std::string& username);
        bool GetUser(const TiXmlElement& userElement, UserAccount& user) const;
        bool GetUserBan(const TiXmlElement& banElement, BannedUser& ban);
        void NewUserBan(TiXmlElement& banElement, const BannedUser& ban);
    };
}
#endif
