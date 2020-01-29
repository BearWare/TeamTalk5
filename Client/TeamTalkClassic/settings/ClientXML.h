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

#ifndef CLIENTXML_H
#define CLIENTXML_H

#include "Settings.h"

#define TEAMTALK_XML_VERSION                        "5.4"

#define TEAMTALK_XML_VERSION_DEFAULT                "5.0"

#define DEFAULT_NICKNAME                            _T("NoName")
#define DEFAULT_GENDER                              1
#define DEFAULT_PUSHTOTALK                          false
#define DEFAULT_VOICEACTIVATED                      true
#define DEFAULT_VOICEACTIVATE_LEVEL                 2

#define DEFAULT_SOUND_OUTPUT_VOLUME                 50
#define DEFAULT_SOUND_GAIN_LEVEL                    50
#define DEFAULT_MEDIA_VS_VOICE                      100

#define DEFAULT_SOUNDEVENT_USERJOINED               "Sounds\\newuser.wav"
#define DEFAULT_SOUNDEVENT_USERLEFT                 "Sounds\\removeuser.wav"
#define DEFAULT_SOUNDEVENT_PRIVTEXTMSG              "Sounds\\user_msg.wav"
#define DEFAULT_SOUNDEVENT_SERVERLOST               "Sounds\\serverlost.wav"
#define DEFAULT_SOUNDEVENT_PTTHOTKEY                "Sounds\\hotkey.wav"
#define DEFAULT_SOUNDEVENT_CHANTEXTMSG              "Sounds\\channel_msg.wav"
#define DEFAULT_SOUNDEVENT_BCASTTEXTMSG              "Sounds\\broadcast_msg.wav"
#define DEFAULT_SOUNDEVENT_FILESUPDATE              "Sounds\\fileupdate.wav"
#define DEFAULT_SOUNDEVENT_FILETXCOMPLETE           "Sounds\\filetx_complete.wav"
#define DEFAULT_SOUNDEVENT_VIDEOSESSION             "Sounds\\videosession.wav"
#define DEFAULT_SOUNDEVENT_DESKTOPSESSION           "Sounds\\desktopsession.wav"
#define DEFAULT_SOUNDEVENT_QUESTIONMODE             "Sounds\\questionmode.wav"
#define DEFAULT_SOUNDEVENT_DESKTOPACCESS            "Sounds\\desktopaccessreq.wav"
#define DEFAULT_SOUNDEVENT_CHANNELSILENT            ""
#define DEFAULT_SOUNDEVENT_VOICEACTIVATED           "Sounds\\voiceact_on.wav"
#define DEFAULT_SOUNDEVENT_VOICEDEACTIVATED         "Sounds\\voiceact_off.wav"
#define DEFAULT_SOUNDEVENT_ENABLEVOICEACT           "Sounds\\vox_enable.wav"
#define DEFAULT_SOUNDEVENT_DISABLEVOICEACT          "Sounds\\vox_disable.wav"
#define DEFAULT_SOUNDEVENT_ME_ENABLEVOICEACT        "Sounds\\vox_me_enable.wav"
#define DEFAULT_SOUNDEVENT_ME_DISABLEVOICEACT       "Sounds\\vox_me_disable.wav"
#define DEFAULT_SOUNDEVENT_TRANSMITQUEUE_READY      "Sounds\\txqueue_start.wav"
#define DEFAULT_SOUNDEVENT_TRANSMITQUEUE_STOP       "Sounds\\txqueue_stop.wav"

namespace teamtalk {

    typedef std::vector<int> HotKey;

    struct HostEntry
    {
        std::string szEntryName;
        std::string szAddress;
        int nTcpPort;
        int nUdpPort;
        bool bEncrypted;
        std::string szUsername;
        std::string szPassword;
        std::string szChannel;
        std::string szChPasswd;

        std::string szNickname;
        int nGender;
        HotKey hotkey;
        int nVoiceAct; // < 0 = disabled
        VideoFormat capformat;
        VideoCodec vidcodec;

        HostEntry()
        : nTcpPort(0)
        , nUdpPort(0)
        , bEncrypted(FALSE)
        , nGender(0)
        , nVoiceAct(-1)
        , capformat()
        , vidcodec() { }
    };

    //used for join channel
    struct JoinChannel
    {
        std::string szChannelPath;
        std::string szPassword;
        std::string szTopic;
        std::string szOpPasswd;
    };


    class ClientXML : public XMLDocument
    {
    public:
        ClientXML(const std::string& rootname) : XMLDocument(rootname, TEAMTALK_XML_VERSION){}
        virtual bool SaveFile();

        /******** <main> ********/
        bool SetWindowPlacement(int x, int y, int cx, int cy);
        bool GetWindowPlacement(int& x, int& y, int& cx, int& cy);

        bool SetWindowExtended(bool bExtended);
        bool GetWindowExtended();

        /******** </main> ********/

        /******** <general> *******/
        bool SetNickname(const std::string& szNickname);
        std::string GetNickname(std::string def_nickname = std::string());

        void SetBearWareLogin(const std::string& szUsername, const std::string& szToken);
        bool GetBearWareLogin(std::string& szUsername, std::string& szToken);

        bool SetProfileName(const std::string& szProfilename);
        std::string GetProfileName();

        bool SetGender(int nGender);
        int GetGender(int nDefGender = DEFAULT_GENDER);

        bool SetPushToTalk(bool bEnable);
        bool GetPushToTalk(bool bDefEnable = DEFAULT_PUSHTOTALK);

        bool SetPushToTalkKey(const HotKey& keykey);
        bool GetPushToTalkKey(HotKey& hotkey);

        bool SetVoiceActivated(bool bEnable);
        bool GetVoiceActivated(bool bDefVoiceAct = DEFAULT_VOICEACTIVATED);

        bool SetVoiceActivationLevel(int nLevel);
        int GetVoiceActivationLevel(int nDefVoxLevel = DEFAULT_VOICEACTIVATE_LEVEL);

        int SetInactivityDelay(int nDelay);
        int GetInactivityDelay();

        bool SetDisableVadOnIdle(bool bEnable);
        bool GetDisableVadOnIdle();

        bool SetDesktopShareMode(int nMode);
        int GetDesktopShareMode();

        bool SetDesktopShareRgbMode(int nMode);
        int GetDesktopShareRgbMode();

        bool SetDesktopShareUpdateInterval(int nInterval);
        int GetDesktopShareUpdateInterval();

        bool SetFirewallInstall(bool bAsked);
        bool GetFirewallInstall(bool bDefValue);
        /******** </general> *******/

        /******** <window> ********/
        bool SetFont(const std::string& szFontName, int nSize, bool bBold, bool bUnderline, bool bItalic);
        bool GetFont(std::string& szFontName, int& nSize, bool& bBold, bool& bUnderline, bool& bItalic);

        bool SetStartMinimized(bool bEnable);
        bool GetStartMinimized();

        bool SetMinimizeToTray(bool bEnable);
        bool GetMinimizeToTray();

        bool SetPopupOnMessage(bool bEnable);
        bool GetPopupOnMessage();

        bool SetAlwaysOnTop(bool bEnable);
        bool GetAlwaysOnTop();

        bool SetShowUserCount(bool bEnable);
        bool GetShowUserCount();

        bool SetJoinDoubleClick(bool bEnable);
        bool GetJoinDoubleClick();

        bool SetQuitClearChannels(bool bEnable);
        bool GetQuitClearChannels();

        bool SetMessageTimeStamp(bool bEnable);
        bool GetMessageTimeStamp();

        bool SetLanguageFile(const std::string& szLanguageFile);
        std::string GetLanguageFile();

        bool SetCloseTransferDialog(bool bEnable);
        bool GetCloseTransferDialog();

        bool SetVuMeterUpdate(bool bEnable);
        bool GetVuMeterUpdate();

        bool SetCheckApplicationUpdates(bool bEnable);
        bool GetCheckApplicationUpdates();

        bool SetShowUsernames(bool bEnable);
        bool GetShowUsernames();

        bool SetMaxTextLength(int nLength);
        int GetMaxTextLength(int nDefault);

        bool SetShowPublicServers(bool bEnable);
        bool GetShowPublicServers();

        // must match SortOrder in channel's tree
        bool SetSortOrder(int nSorting);
        int GetSortOrder();

        /******** </window> *******/


        /******** <client> *********/
        bool SetClientTcpPort(int nPort);
        int GetClientTcpPort(int nDefPort);

        bool SetClientUdpPort(int nPort);
        int GetClientUdpPort(int nDefPort);

        bool SetAutoConnectToLastest(bool bEnable);
        bool GetAutoConnectToLastest();

        bool SetReconnectOnDropped(bool bEnable);
        bool GetReconnectOnDropped();

        bool SetAutoJoinRootChannel(bool bEnable);
        bool GetAutoJoinRootChannel();

        bool SetAudioLogStorageFormat(int aff);
        int GetAudioLogStorageFormat();

        bool SetAudioLogStorageMode(int mode);
        int GetAudioLogStorageMode();

        bool SetAudioLogStorage(const std::string& audiofolder);
        std::string GetAudioLogStorage();

        bool SetChanTextLogStorage(const std::string& audiofolder);
        std::string GetChanTextLogStorage();

        bool SetUserTextLogStorage(const std::string& audiofolder);
        std::string GetUserTextLogStorage();

        bool SetDefaultSubscriptions(int subs);
        int GetDefaultSubscriptions();
        /********* </client> ********/

        /********* <soundsystem> ********/
        bool SetSoundInputDevice(int nDevice);
        int GetSoundInputDevice(int nDefDeviceId);

        bool SetSoundInputDevice(const std::string& devid);
        std::string GetSoundInputDevice();

        bool SetSoundOutputDevice(int nDevice);
        int GetSoundOutputDevice(int nDefDeviceId);

        bool SetSoundOutputDevice(const std::string& devid);
        std::string GetSoundOutputDevice();

        bool SetSoundOutputVolume(int nVolume);
        int GetSoundOutputVolume(int def_vol);

        bool SetMediaStreamVsVoice(int nPercent);
        int GetMediaStreamVsVoice(int nDefPercent);

        bool SetSoundMixerDevice(int nDevice);
        int GetSoundMixerDevice();

        bool SetAutoPositioning(bool bEnable);
        bool GetAutoPositioning();

        bool SetAGC(bool bEnable);
        bool GetAGC(bool bDefValue);

        bool SetDenoise(bool bEnable);
        bool GetDenoise(bool bDefValue);

        bool SetVoiceGain(bool bEnable);
        bool GetVoiceGain();

        bool SetVoiceGainLevel(int nGain);
        int GetVoiceGainLevel(int nDefGain);

        bool SetDuplexMode(bool bEnable);
        bool GetDuplexMode(bool bDefValue);

        bool SetEchoCancel(bool bEnable);
        bool GetEchoCancel(bool bDefValue);
        /********* </soundsystem> ********/

        /********* <events> *************/
        bool SetEventSoundsEnabled(unsigned uSoundEvents);
        unsigned GetEventSoundsEnabled(unsigned uDefEvents = 0);

        bool SetEventNewUser(const std::string& szPath);
        std::string GetEventNewUser(std::string szDefPath = DEFAULT_SOUNDEVENT_USERJOINED);

        bool SetEventNewMessage(const std::string& szPath);
        std::string GetEventNewMessage(std::string szDefPath = DEFAULT_SOUNDEVENT_PRIVTEXTMSG);

        bool SetEventRemovedUser(const std::string& szPath);
        std::string GetEventRemovedUser(std::string szDefPath = DEFAULT_SOUNDEVENT_USERLEFT);

        bool SetEventServerLost(const std::string& szPath);
        std::string GetEventServerLost(std::string szDefPath = DEFAULT_SOUNDEVENT_SERVERLOST);

        bool SetEventChannelSilent(const std::string& szPath);
        std::string GetEventChannelSilent(std::string szDefPath = DEFAULT_SOUNDEVENT_CHANNELSILENT);

        bool SetEventHotKey(const std::string& szPath);
        std::string GetEventHotKey(std::string szDefPath = DEFAULT_SOUNDEVENT_PTTHOTKEY);

        bool SetEventChannelMsg(const std::string& szPath);
        std::string GetEventChannelMsg(std::string szDefPath = DEFAULT_SOUNDEVENT_CHANTEXTMSG);

        bool SetEventBroadcastMsg(const std::string& szPath);
        std::string GetEventBroadcastMsg(std::string szDefPath = DEFAULT_SOUNDEVENT_BCASTTEXTMSG);

        bool SetEventFilesUpd(const std::string& szPath);
        std::string GetEventFilesUpd(std::string szDefPath = DEFAULT_SOUNDEVENT_FILESUPDATE);

        bool SetEventTransferEnd(const std::string& szPath);
        std::string GetEventTransferEnd(std::string szDefPath = DEFAULT_SOUNDEVENT_FILETXCOMPLETE);

        bool SetEventVideoSession(const std::string& szPath);
        std::string GetEventVideoSession(std::string szDefPath = DEFAULT_SOUNDEVENT_VIDEOSESSION);

        bool SetEventDesktopSession(const std::string& szPath);
        std::string GetEventDesktopSession(std::string szDefPath = DEFAULT_SOUNDEVENT_DESKTOPSESSION);

        bool SetEventQuestionMode(const std::string& szPath);
        std::string GetEventQuestionMode(std::string szDefPath = DEFAULT_SOUNDEVENT_QUESTIONMODE);

        bool SetEventDesktopAccessReq(const std::string& szPath);
        std::string GetEventDesktopAccessReq(std::string szDefPath = DEFAULT_SOUNDEVENT_DESKTOPACCESS);

        bool SetEventVoiceActivated(const std::string& szPath);
        std::string GetEventVoiceActivated(std::string szDefPath = DEFAULT_SOUNDEVENT_VOICEACTIVATED);

        bool SetEventVoiceDeactivated(const std::string& szPath);
        std::string GetEventVoiceDeactivated(std::string szDefPath = DEFAULT_SOUNDEVENT_VOICEDEACTIVATED);

        bool SetEventEnableVoiceActivation(const std::string& szPath);
        std::string GetEventEnableVoiceActivation(std::string szDefPath = DEFAULT_SOUNDEVENT_ENABLEVOICEACT);

        bool SetEventDisableVoiceActivation(const std::string& szPath);
        std::string GetEventDisableVoiceActivation(std::string szDefPath = DEFAULT_SOUNDEVENT_DISABLEVOICEACT);

        bool SetEventMeEnableVoiceActivation(const std::string& szPath);
        std::string GetEventMeEnableVoiceActivation(std::string szDefPath = DEFAULT_SOUNDEVENT_ME_ENABLEVOICEACT);

        bool SetEventMeDisableVoiceActivation(const std::string& szPath);
        std::string GetEventMeDisableVoiceActivation(std::string szDefPath = DEFAULT_SOUNDEVENT_ME_DISABLEVOICEACT);

        bool SetEventTransmitQueueHead(std::string szPath);
        std::string GetEventTransmitQueueHead(std::string szDefPath = DEFAULT_SOUNDEVENT_TRANSMITQUEUE_READY);

        bool SetEventTransmitQueueStop(std::string szPath);
        std::string GetEventTransmitQueueStop(std::string szDefPath = DEFAULT_SOUNDEVENT_TRANSMITQUEUE_STOP);

        bool SetEventVibrateOnMsg(int nIndex);
        int GetEventVibrateOnMsg();

        bool SetEventFlashOnMsg(int nIndex);
        int GetEventFlashOnMsg();

        bool SetEventTTSEvents(TTSEvents uEvents);
        TTSEvents GetEventTTSEvents();
        /********* </events> ************/

        /********* <advanced> ***********/
        bool SetMixerAutoSelection(bool bEnable);
        bool GetMixerAutoSelection();

        bool SetMixerAutoSelectInput(int nIndex);
        int GetMixerAutoSelectInput();

        bool SetMixerBoostBugCompensation(bool bEnable);
        bool GetMixerBoostBugCompensation();

        bool SetLowLevelMouseHook(bool bEnable);
        bool GetLowLevelMouseHook();
        /********* </advanced> **********/

        /********* <shortcuts> **********/
        bool SetHotKeyVoiceAct(const HotKey& hotkey);
        bool GetHotKeyVoiceAct(HotKey& hotkey);

        bool SetHotKeyVolumePlus(const HotKey& hotkey);
        bool GetHotKeyVolumePlus(HotKey& hotkey);

        bool SetHotKeyVolumeMinus(const HotKey& hotkey);
        bool GetHotKeyVolumeMinus(HotKey& hotkey);

        bool SetHotKeyMuteAll(const HotKey& hotkey);
        bool GetHotKeyMuteAll(HotKey& hotkey);

        bool SetHotKeyVoiceGainPlus(const HotKey& hotkey);
        bool GetHotKeyVoiceGainPlus(HotKey& hotkey);

        bool SetHotKeyVoiceGainMinus(const HotKey& hotkey);
        bool GetHotKeyVoiceGainMinus(HotKey& hotkey);

        bool SetHotKeyMinRestore(const HotKey& hotkey);
        bool GetHotKeyMinRestore(HotKey& hotkey);
        /********* </shortcuts> *********/

        /********* <videocapture> *********/
        bool SetVideoCaptureDevice(const std::string& viddev);
        std::string GetVideoCaptureDevice();

        bool SetVideoCaptureFormat(int index);
        int GetVideoCaptureFormat(int nDefIndex);

        bool SetVideoCaptureFormat(const VideoFormat& capformat);
        bool GetVideoCaptureFormat(VideoFormat& capformat);

        bool SetVideoCodecBitrate(int bitrate);
        int GetVideoCodecBitrate(int nDefBitrate);

        bool SetVideoCaptureEnabled(bool enabled);
        bool GetVideoCaptureEnabled(bool def_value);
        /********* </videocapture> *********/

        /********* <latesthosts> ********/
        bool AddLatestHostEntry(const HostEntry& entry);
        bool RemoveLatestHostEntry(const HostEntry& entry);
        int GetLatestHostEntryCount();
        bool GetLatestHostEntry(int index, HostEntry& entry);
        /********* </latesthosts> *******/

        /********** <hostmanager> **********/
        bool AddHostManagerEntry(const HostEntry& entry);
        bool RemoveHostManagerEntry(const std::string entryname);
        int GetHostManagerEntryCount();
        bool GetHostManagerEntry(int index, HostEntry& entry);
        bool GetHostManagerEntry(const std::string& entryname, HostEntry& entry);
        /********** </hostmanager> **********/

        /********** <mediafiles> *********/
        bool SetLastMediaFiles(const std::vector<std::string>& filenames);
        std::vector<std::string> GetLastMediaFiles();

        void SetAudioPreprocessor(AudioPreprocessorType preproc);
        AudioPreprocessorType GetAudioPreprocessor(AudioPreprocessorType defaultvalue);

        void SetTTAudioPreprocessor(const TTAudioPreprocessor& ttaud);
        TTAudioPreprocessor GetTTAudioPreprocessor();

        void SetSpeexDSPAudioPreprocessor(const SpeexDSP& spxdsp);
        SpeexDSP GetSpeexDSPAudioPreprocessor();

        void SetMediaFilePlayback(const MediaFilePlayback& mfp);
        MediaFilePlayback GetMediaFilePlayback();

        void SetVideoCodec(const VideoCodec& codec);
        VideoCodec GetVideoCodec();

        void SetMediaFileRepeat(bool repeat);
        bool GetMediaFileRepeat(bool defaultvalue);
        /********** </mediafiles> *********/
    protected:
        TiXmlElement* GetRootElement();
        TiXmlElement* GetMainElement();
        TiXmlElement* GetGeneralElement();
        TiXmlElement* GetWindowElement();
        TiXmlElement* GetClientElement();
        TiXmlElement* GetSoundSystemElement();
        TiXmlElement* GetEventsElement();
        TiXmlElement* GetVideoElement();
        TiXmlElement* GetAdvancedElement();
        TiXmlElement* GetShortCutsElement();
        TiXmlElement* GetHostManagerElement();
        TiXmlElement* GetLatestHostsElement();
        TiXmlElement* GetMediaFilesElement();

        void PutHotKey(TiXmlElement& parent, const HotKey& hotkey);
        bool GetHotKey( const TiXmlElement& parent, HotKey& hotkey);
    };

    class TTFile : public XMLDocument
    {
    public:
       TTFile(const std::string& rootname) : 
          XMLDocument(rootname, TEAMTALK_XML_VERSION) { }

       void SetHostEntry(const HostEntry& entry);
       bool GetHostEntry(HostEntry& entry, int i);
       TiXmlElement* GetRootElement();
       bool HasClientSetup();
    };

}
#endif
