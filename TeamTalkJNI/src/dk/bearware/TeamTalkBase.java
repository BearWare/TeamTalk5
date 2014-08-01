/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

package dk.bearware;

import java.util.Vector;

public abstract class TeamTalkBase
{
    private long ttInst = 0;

    public native String GetVersion();

    private native long InitTeamTalkPoll();

    public TeamTalkBase() {
        ttInst = InitTeamTalkPoll();
    }
    
    protected void finalize( ) throws Throwable {
        CloseTeamTalk(ttInst);
    }

    private native boolean CloseTeamTalk(long lpTTInstance);
    public boolean CloseTeamTalk() {
        return CloseTeamTalk(ttInst);
    }

    private native boolean GetMessage(long lpTTInstance,
                                      TTMessage pMsg,
                                      int pnWaitMs);
    public boolean GetMessage(TTMessage pMsg,
                              int pnWaitMs) {
        return GetMessage(ttInst, pMsg, pnWaitMs);
    }

    private native int GetFlags(long lpTTInstance);
    public int GetFlags() { return GetFlags(ttInst); }

    public static native boolean SetLicenseInformation(String szRegName,
                                                String szRegKey);

    public static native boolean GetDefaultSoundDevices(IntPtr lpnInputDeviceID,
                                                        IntPtr lpnOutputDeviceID);
    
    private static native boolean GetSoundDevices(SoundDevice[] lpSoundDevices,
                                                  IntPtr lpnHowMany);
    public static boolean GetSoundDevices(Vector<SoundDevice> lpSoundDevices) {
        IntPtr lpnHowMany = new IntPtr();
        if(!GetSoundDevices(null, lpnHowMany))
            return false;

        SoundDevice[] devs = new SoundDevice[lpnHowMany.value];
        if(GetSoundDevices(devs, lpnHowMany))
        {
            for(int i=0;i<lpnHowMany.value;i++)
                lpSoundDevices.add(devs[i]);
        }
        return true;
    }


    public static native boolean RestartSoundSystem();

    public static native long StartSoundLoopbackTest(int nInputDeviceID, 
                                                     int nOutputDeviceID,
                                                     int nSampleRate,
                                                     int nChannels,
                                                     boolean bDuplexMode,
                                                     AudioConfig lpAudioConfig);

    public static native boolean CloseSoundLoopbackTest(long lpTTSoundLoop);

    private native boolean InitSoundInputDevice(long lpTTInstance, int nInputDeviceID);
    public boolean InitSoundInputDevice(int nInputDeviceID) {
        return InitSoundInputDevice(ttInst, nInputDeviceID);
    }
    private native boolean InitSoundOutputDevice(long lpTTInstance, int nOutputDeviceID);
    public boolean InitSoundOutputDevice(int nOutputDeviceID) {
        return InitSoundOutputDevice(ttInst, nOutputDeviceID);
    }
    private native boolean CloseSoundInputDevice(long lpTTInstance);
    public boolean CloseSoundInputDevice() {
        return CloseSoundInputDevice(ttInst);
    }
    private native boolean CloseSoundOutputDevice(long lpTTInstance);
    public boolean CloseSoundOutputDevice() {
        return CloseSoundOutputDevice(ttInst);
    }

    private native int GetSoundInputLevel(long lpTTInstance);
    public int GetSoundInputLevel() { return GetSoundInputLevel(ttInst); }

    private native boolean SetSoundInputGainLevel(long lpTTInstance, int nLevel);
    public boolean SetSoundInputGainLevel(int nLevel)
        { return SetSoundInputGainLevel(ttInst, nLevel); }

    private native int GetSoundInputGainLevel(long lpTTInstance);
    public int GetSoundInputGainLevel() { return GetSoundInputGainLevel(ttInst); }

    private native boolean SetAudioConfig(long lpTTInstance, AudioConfig lpAudioConfig);
    public boolean SetAudioConfig(AudioConfig lpAudioConfig) {
        return SetAudioConfig(ttInst, lpAudioConfig);
    }

    private native boolean GetAudioConfig(long lpTTInstance, AudioConfig lpAudioConfig);
    public boolean GetAudioConfig(AudioConfig lpAudioConfig) {
        return GetAudioConfig(ttInst, lpAudioConfig);
    }

    private native boolean SetSoundOutputVolume(long lpTTInstance, int nVolume);
    public boolean SetSoundOutputVolume(int nVolume)
        { return SetSoundOutputVolume(ttInst, nVolume); }

    private native int GetSoundOutputVolume(long lpTTInstance);
    public int GetSoundOutputVolume()
        { return GetSoundOutputVolume(ttInst); }

    private native boolean SetSoundOutputMute(long lpTTInstance, boolean bMuteAll);
    public boolean SetSoundOutputMute(boolean bMuteAll)
        { return SetSoundOutputMute(ttInst, bMuteAll); }

    private native boolean Enable3DSoundPositioning(long lpTTInstance,
                                                    boolean bEnable);
    public boolean Enable3DSoundPositioning(boolean bEnable) {
        return Enable3DSoundPositioning(ttInst, bEnable);
    }
    
    private native boolean AutoPositionUsers(long lpTTInstance);
    public boolean AutoPositionUsers() { return AutoPositionUsers(ttInst); }

    private native boolean EnableAudioBlockEvent(long lpTTInstance,
                                                 boolean bEnable);
    public boolean EnableAudioBlockEvent(boolean bEnable)
        { return EnableAudioBlockEvent(ttInst, bEnable); }

    private native boolean EnableVoiceTransmission(long lpTTInstance, 
                                                   boolean bEnable);
    public boolean EnableVoiceTransmission(boolean bEnable) {
        return EnableVoiceTransmission(ttInst, bEnable);
    }

    private native boolean EnableVoiceActivation(long lpTTInstance,
                                                 boolean bEnable);
    public boolean EnableVoiceActivation(boolean bEnable)
        { return EnableVoiceActivation(ttInst, bEnable); }
    
    private native boolean SetVoiceActivationLevel(long lpTTInstance,
                                                   int nLevel);
    public boolean SetVoiceActivationLevel(int nLevel)
        { return SetVoiceActivationLevel(nLevel); }

    private native int GetVoiceActivationLevel(long lpTTInstance);
    public int GetVoiceActivationLevel()
        { return GetVoiceActivationLevel(ttInst); }

    private native boolean SetVoiceActivationStopDelay(long lpTTInstance,
                                                       int nDelayMSec);
    public boolean SetVoiceActivationStopDelay(int nDelayMSec)
        { return SetVoiceActivationStopDelay(nDelayMSec); }

    private native int GetVoiceActivationStopDelay(long lpTTInstance);
    public int GetVoiceActivationStopDelay()
        { return GetVoiceActivationStopDelay(ttInst); }

    /** TODO: not implemented
    private native boolean StartVideoCaptureTransmission(long lpTTInstance,
                                                         VideoCodec lpVideoCodec);
    public boolean StartVideoCaptureTransmission(VideoCodec lpVideoCodec) {
        return StartVideoCaptureTransmission(ttInst, lpVideoCodec);
    }

    private native boolean StopVideoCaptureTransmission(long lpTTInstance);
    public boolean StopVideoCaptureTransmission() {
        return StopVideoCaptureTransmission(ttInst);
    }

    private native boolean StartStreamingMediaFileToChannel(long lpTTInstance,
                                                            String szMediaFilePath,
                                                            VideoCodec lpVideoCodec);
    public boolean StartStreamingMediaFileToChannel(String szMediaFilePath,
                                                    VideoCodec lpVideoCodec) {
        return StartStreamingMediaFileToChannel(ttInst, szMediaFilePath,
                                                lpVideoCodec);
    }

    private native boolean StopStreamingMediaFileToChannel(long lpTTInstance);
    public boolean StopStreamingMediaFileToChannel() {
        return StopStreamingMediaFileToChannel(ttInst);
    }

    public static native boolean GetMediaFileInfo(String szMediaFilePath,
                                                  MediaFileInfo lpMediaFileInfo);

    private native VideoFrame AcquireUserMediaVideoFrame(long lpTTInstance,
                                                         int nUserID);
    public VideoFrame AcquireUserMediaVideoFrame(int nUserID) {
        return AcquireUserMediaVideoFrame(ttInst, nUserID);
    }

    private native boolean ReleaseUserMediaVideoFrame(long lpTTInstance,
                                                      int nUserID);
    public boolean ReleaseUserMediaVideoFrame(int nUserID) {
        return ReleaseUserMediaVideoFrame(ttInst, nUserID);
    }

    private native boolean SendDesktopCursorPosition(long lpTTInstance,
                                                     int nPosX, int nPosY);
    public boolean SendDesktopCursorPosition(int nPosX, int nPosY) {
        return SendDesktopCursorPosition(ttInst, nPosX, nPosY);
    }

    private native boolean SendDesktopInput(long lpTTInstance,
                                            int nUserID,
                                            DesktopInput[] lpDesktopInputs);
    public boolean SendDesktopInput(int nUserID,
                                    DesktopInput[] lpDesktopInputs) {
        return SendDesktopInput(ttInst, nUserID, lpDesktopInputs);
    }
    */

    private native boolean Connect(long lpTTInstance,
                                   String szHostAddress,
                                   int nTcpPort, 
                                   int nUdpPort, 
                                   int nLocalTcpPort, 
                                   int nLocalUdpPort,
                                   boolean bEncrypted);
    public boolean Connect(String szHostAddress,
                           int nTcpPort, 
                           int nUdpPort, 
                           int nLocalTcpPort, 
                           int nLocalUdpPort,
                           boolean bEncrypted) {
        return Connect(ttInst, szHostAddress, nTcpPort, nUdpPort, 
                       nLocalTcpPort, nLocalUdpPort, bEncrypted);
    }
    
    private native boolean ConnectEx(long lpTTInstance,
                                     String szHostAddress,
                                     int nTcpPort, 
                                     int nUdpPort, 
                                     String szBindIPAddr,
                                     int nLocalTcpPort, 
                                     int nLocalUdpPort,
                                     boolean bEncrypted);
    public boolean ConnectEx(String szHostAddress,
                             int nTcpPort, 
                             int nUdpPort, 
                             String szBindIPAddr,
                             int nLocalTcpPort, 
                             int nLocalUdpPort,
                             boolean bEncrypted) {
        return ConnectEx(ttInst, szHostAddress, nTcpPort, nUdpPort, 
                         szBindIPAddr, nLocalTcpPort, nLocalUdpPort,
                         bEncrypted);
    }
    private native boolean Disconnect(long lpTTInstance);
    public boolean Disconnect() {
        return Disconnect(ttInst);
    }
    private native boolean QueryMaxPayload(long lpTTInstance, int nUserID);
    public boolean QueryMaxPayload(int nUserID) {
        return QueryMaxPayload(ttInst, nUserID);
    }

    private native boolean GetClientStatistics(long lpTTInstance, ClientStatistics lpClientStatistics);
    public boolean GetClientStatistics(ClientStatistics lpClientStatistics) {
        return GetClientStatistics(ttInst, lpClientStatistics);
    }

    private native int DoPing(long lpTTInstance);
    public int DoPing() {
        return DoPing(ttInst);
    }

    private native int DoLogin(long lpTTInstance,
                              String szNickname, 
                              String szUsername,
                              String szPassword);
    public int DoLogin(String szNickname, 
                       String szUsername,
                       String szPassword) {
        return DoLogin(ttInst, szNickname,
                       szUsername, szPassword);
    }
    private native int DoLogout(long lpTTInstance);
    public int DoLogout() { return DoLogout(ttInst); }

    private native int DoJoinChannel(long lpTTInstance,
                                     Channel lpChannel);
    public int DoJoinChannel(Channel lpChannel) {
        return DoJoinChannel(ttInst, lpChannel);
    }
    private native int DoJoinChannelByID(long lpTTInstance,
                                         int nChannelID, 
                                         String szPassword);
    public int DoJoinChannelByID(int nChannelID, 
                                 String szPassword) {
        return DoJoinChannelByID(ttInst, nChannelID, szPassword);
    }
    private native int DoLeaveChannel(long lpTTInstance);
    public int DoLeaveChannel() { 
        return DoLeaveChannel(ttInst);
    }
    private native int DoChangeNickname(long lpTTInstance,
                                        String szNewNick);
    public int DoChangeNickname(String szNewNick) { 
        return DoChangeNickname(ttInst, szNewNick);
    }
    private native int DoChangeStatus(long lpTTInstance, int nStatusMode, String szStatusMessage);
    public int DoChangeStatus(int nStatusMode, String szStatusMessage) {
        return DoChangeStatus(ttInst, nStatusMode, szStatusMessage);
    }
    private native int DoTextMessage(long lpTTInstance, TextMessage lpTextMessage);
    public int DoTextMessage(TextMessage lpTextMessage) {
        return DoTextMessage(ttInst, lpTextMessage);
    }
    private native int DoChannelOp(long lpTTInstance, int nUserID, int nChannelID, boolean bMakeOperator);
    public int DoChannelOp(int nUserID, int nChannelID, boolean bMakeOperator) {
        return DoChannelOp(ttInst, nUserID, nChannelID, bMakeOperator);
    }
    private native int DoChannelOpEx(long lpTTInstance, int nUserID, int nChannelID, 
                                     String szOpPassword,boolean bMakeOperator);
    public int DoChannelOpEx(int nUserID, int nChannelID, 
                             String szOpPassword, boolean bMakeOperator) {
        return DoChannelOpEx(ttInst, nUserID, nChannelID, szOpPassword, bMakeOperator);
    }
    private native int DoKickUser(long lpTTInstance, int nUserID, int nChannelID);
    public int DoKickUser(int nUserID, int nChannelID) {
        return DoKickUser(ttInst, nUserID, nChannelID);
    }
    private native int DoSendFile(long lpTTInstance, int nChannelID, String szLocalFilePath);
    public int DoSendFile(int nChannelID, String szLocalFilePath) {
        return DoSendFile(ttInst, nChannelID, szLocalFilePath);
    }
    private native int DoRecvFile(long lpTTInstance, int nChannelID, int nFileID,
                                  String szLocalFilePath);
    public int DoRecvFile(int nChannelID, int nFileID,
                          String szLocalFilePath) {
        return DoRecvFile(ttInst, nChannelID, nFileID, szLocalFilePath);
    }
    private native int DoDeleteFile(long lpTTInstance, int nChannelID, int nFileID);
    public int DoDeleteFile(int nChannelID, int nFileID) {
        return DoDeleteFile(ttInst, nChannelID, nFileID);
    }
    private native int DoSubscribe(long lpTTInstance, int uSubscriptions);
    public int DoSubscribe(int uSubscriptions) {
        return DoSubscribe(ttInst, uSubscriptions);
    }
    private native int DoUnsubscribe(long lpTTInstance, int uSubscriptions);
    public int DoUnsubscribe(int uSubscriptions) {
        return DoUnsubscribe(ttInst, uSubscriptions);
    }
    private native int DoMakeChannel(long lpTTInstance, Channel lpChannel);
    public int DoMakeChannel(Channel lpChannel) {
        return DoMakeChannel(ttInst, lpChannel);
    }
    private native int DoUpdateChannel(long lpTTInstance, Channel lpChannel);
    public int DoUpdateChannel(Channel lpChannel) {
        return DoUpdateChannel(ttInst, lpChannel);
    }
    private native int DoRemoveChannel(long lpTTInstance, int nChannelID);
    public int DoRemoveChannel(int nChannelID) {
        return DoRemoveChannel(ttInst, nChannelID);
    }
    private native int DoUpdateServer(long lpTTInstance, ServerProperties lpServerProperties);
    public int DoUpdateServer(ServerProperties lpServerProperties) {
        return DoUpdateServer(ttInst, lpServerProperties);
    }
    private native int DoListUserAccounts(long lpTTInstance, int nIndex, int nCount);
    public int DoListUserAccounts(int nIndex, int nCount) {
        return DoListUserAccounts(ttInst, nIndex, nCount);
    }
    private native int DoNewUserAccount(long lpTTInstance, UserAccount lpUserAccount);
    public int DoNewUserAccount(UserAccount lpUserAccount) {
        return DoNewUserAccount(ttInst, lpUserAccount);
    }
    private native int DoDeleteUserAccount(long lpTTInstance, String szUsername);
    public int DoDeleteUserAccount(String szUsername) {
        return DoDeleteUserAccount(ttInst, szUsername);
    }
    private native int DoBanUser(long lpTTInstance, int nUserID);
    public int DoBanUser(int nUserID) {
        return DoBanUser(ttInst, nUserID);
    }
    private native int DoBanIPAddress(long lpTTInstance, String szIPAddress);
    public int DoBanIPAddress(String szIPAddress) {
        return DoBanIPAddress(ttInst, szIPAddress);
    }
    private native int DoUnBanUser(long lpTTInstance, String szIPAddress);
    public int DoUnBanUser(String szIPAddress) {
        return DoUnBanUser(ttInst, szIPAddress);
    }
    private native int DoListBans(long lpTTInstance, int nIndex, int nCount);
    public int DoListBans(int nIndex, int nCount) {
        return DoListBans(ttInst, nIndex, nCount);
    }
    private native int DoSaveConfig(long lpTTInstance);
    public int DoSaveConfig() {
        return DoSaveConfig(ttInst);
    }
    private native int DoQueryServerStats(long lpTTInstance);
    public int DoQueryServerStats() {
        return DoQueryServerStats(ttInst);
    }
    private native int DoQuit(long lpTTInstance);
    public int DoQuit() {
        return DoQuit(ttInst);
    }
    private native boolean GetServerProperties(long lpTTInstance,
                                               ServerProperties lpServerProperties);
    public boolean GetServerProperties(ServerProperties lpServerProperties)
        { return GetServerProperties(ttInst, lpServerProperties); }
    private native boolean GetServerUsers(long lpTTInstance,
                                          User[] lpUserIDs, IntPtr lpnHowMany);
    public boolean GetServerUsers(User[] lpUsers, IntPtr lpnHowMany) {
        return GetServerUsers(ttInst, lpUsers, lpnHowMany);
    }
    private native int GetRootChannelID(long lpTTInstance);
    public int GetRootChannelID() {
        return GetRootChannelID(ttInst);
    }
    private native int GetMyChannelID(long lpTTInstance);
    public int GetMyChannelID() {
        return GetMyChannelID(ttInst);
    }
    private native boolean GetChannel(long lpTTInstance,
                                     int nChannelID,
                                     Channel lpChannel);
    public boolean GetChannel(int nChannelID,
                              Channel lpChannel) {
        return GetChannel(ttInst, nChannelID, lpChannel);
    }
    private native String GetChannelPath(long lpTTInstance,
                                         int nChannelID);
    public String GetChannelPath(int nChannelID) {
        return GetChannelPath(ttInst, nChannelID);
    }
    private native int GetChannelIDFromPath(long lpTTInstance,
                                            String szChannelPath);
    public int GetChannelIDFromPath(String szChannelPath) {
        return GetChannelIDFromPath(ttInst, szChannelPath);
    }
    private native boolean GetChannelUsers(long lpTTInstance,
                                           int nChannelID,
                                           User[] lpUsers,
                                           IntPtr lpnHowMany);
    public boolean GetChannelUsers(int nChannelID,
                                   User[] lpUsers,
                                   IntPtr lpnHowMany) {
        return GetChannelUsers(ttInst, nChannelID, lpUsers, lpnHowMany);
    }

    private native boolean GetChannelFiles(long lpTTInstance,
                                           int nChannelID,
                                           RemoteFile[] lpRemoteFiles,
                                           IntPtr lpnHowMany);
    public boolean GetChannelFiles(int nChannelID,
                                   RemoteFile[] lpRemoteFiles,
                                   IntPtr lpnHowMany) {
        return GetChannelFiles(ttInst, nChannelID, lpRemoteFiles, lpnHowMany);
    }

    private native boolean GetChannelFile(long lpTTInstance,
                                          int nChannelID,
                                          int nFileID, 
                                          RemoteFile lpRemoteFile);
    public boolean GetChannelFile(int nChannelID,
                                  int nFileID, 
                                  RemoteFile lpRemoteFile) {
        return GetChannelFile(ttInst, nChannelID, nFileID, lpRemoteFile);
    }
    private native boolean IsChannelOperator(long lpTTInstance,
                                      int nUserID,
                                      int nChannelID);
    public boolean IsChannelOperator(int nUserID,
                                     int nChannelID) {
        return IsChannelOperator(ttInst, nUserID, nChannelID);
    }

    private native boolean GetServerChannels(long lpTTInstance,
                                             Channel[] lpChannels,
                                             IntPtr lpnHowMany);
    //TODO: return array instead of query
    public boolean GetServerChannels(Channel[] lpChannels,
                                     IntPtr lpnHowMany) {
        return GetServerChannels(ttInst, lpChannels, lpnHowMany);
    }

    private native int GetMyUserID(long lpTTInstance);
    public int GetMyUserID() {
        return GetMyUserID(ttInst);
    }
    private native boolean GetMyUserAccount(long lpTTInstance,
                                            UserAccount lpUserAccount);
    public boolean GetMyUserAccount(UserAccount lpUserAccount) {
        return GetMyUserAccount(ttInst, lpUserAccount);
    }
    private native boolean GetUser(long lpTTInstance,
                                  int nUserID,
                                  User lpUser);
    public boolean GetUser(int nUserID,
                           User lpUser) {
        return GetUser(ttInst, nUserID, lpUser);
    }
    private native boolean GetUserStatistics(long lpTTInstance,
                                             int nUserID,
                                             UserStatistics lpUserStatistics);
    public boolean GetUserStatistics(int nUserID,
                           UserStatistics lpUserStatistics) {
        return GetUserStatistics(ttInst, nUserID, lpUserStatistics);
    }

    private native boolean SetUserVolume(long lpTTInstance,
                                         int nUserID,
                                         int nStreamType,
                                         int nVolume);
    public boolean SetUserVolume(int nUserID,
                                 int nStreamType,
                                 int nVolume) {
        return SetUserVolume(ttInst, nUserID, nStreamType, nVolume);
    }
    private native boolean SetUserGainLevel(long lpTTInstance,
                                            int nUserID,
                                            int nStreamType,
                                            int nGainLevel);
    public boolean SetUserGainLevel(int nUserID,
                                    int nStreamType,
                                    int nGainLevel) {
        return SetUserGainLevel(ttInst, nUserID, nStreamType, nGainLevel);
    }
    private native boolean SetUserMute(long lpTTInstance,
                                       int nUserID,
                                       int nStreamType,
                                       boolean bMute);
    public boolean SetUserMute(int nUserID,
                               int nStreamType,
                               boolean bMute) {
        return SetUserMute(ttInst, nUserID, nStreamType, bMute);
    }
    private native boolean SetUserStoppedPlaybackDelay(long lpTTInstance,
                                                       int nUserID,
                                                       int nStreamType,
                                                       int nDelayMSec);
    public boolean SetUserStoppedPlaybackDelay(int nUserID,
                                              int nStreamType,
                                              int nDelayMSec) {
        return SetUserStoppedPlaybackDelay(ttInst, nUserID, nStreamType, nDelayMSec);
    }
    private native boolean SetUserPosition(long lpTTInstance,
                                           int nUserID,
                                           int nStreamType,
                                           float x, float y, float z);
    public boolean SetUserPosition(int nUserID,
                                   int nStreamType,
                                   float x, float y, float z) {
        return SetUserPosition(ttInst, nUserID, nStreamType, x, y, z);
    }
    private native boolean SetUserStereo(long lpTTInstance,
                                         int nUserID,
                                         int nStreamType,
                                         boolean bLeftSpeaker,
                                         boolean bRightSpeaker);
    public boolean SetUserStereo(int nUserID,
                                 int nStreamType,
                                 boolean bLeftSpeaker,
                                 boolean bRightSpeaker) {
        return SetUserStereo(ttInst, nUserID, nStreamType, bLeftSpeaker, bRightSpeaker);
    }
    private native boolean SetUserAudioFolder(long lpTTInstance,
                                              int nUserID,
                                              String szFolderPath,
                                              String szFileNameVars,
                                              int uAFF);
    public boolean SetUserAudioFolder(int nUserID,
                                      String szFolderPath,
                                      String szFileNameVars, int uAFF) {
        return SetUserAudioFolder(ttInst, nUserID, szFolderPath, szFileNameVars, uAFF);
    }
    private native boolean SetUserAudioStreamBufferSize(long lpTTInstance,
                                                        int nUserID, int uStreamType, int nMSec);
    public boolean SetUserAudioStreamBufferSize(int nUserID, int uStreamType, int nMSec) {
        return SetUserAudioStreamBufferSize(ttInst, nUserID, uStreamType, nMSec);
    }
    private native boolean GetFileTransferInfo(long lpTTInstance,
                                               int nTransferID, FileTransfer lpFileTransfer);
    public boolean GetFileTransferInfo(int nTransferID, FileTransfer lpFileTransfer) {
        return GetFileTransferInfo(ttInst, nTransferID, lpFileTransfer);
    }
    private native boolean CancelFileTransfer(long lpTTInstance,
                                              int nTransferID);
    public boolean CancelFileTransfer(int nTransferID) {
        return CancelFileTransfer(ttInst, nTransferID);
    }
    private native boolean GetBannedUsers(long lpTTInstance,
                                          BannedUser[] lpBannedUsers,
                                          IntPtr lpnHowMany);
    public boolean GetBannedUsers(BannedUser[] lpBannedUsers,
                                  IntPtr lpnHowMany) {
        return GetBannedUsers(ttInst, lpBannedUsers, lpnHowMany);
    }
    private native boolean GetUserAccounts(long lpTTInstance,
                                           UserAccount[] lpUserAccounts,
                                           IntPtr lpnHowMany);
    public boolean GetUserAccounts(UserAccount[] lpUserAccounts,
                                   IntPtr lpnHowMany) {
        return GetUserAccounts(ttInst, lpUserAccounts, lpnHowMany);
    }
    public native String GetErrorMessage(int nError);
}
