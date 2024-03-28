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

package dk.bearware;

import java.util.Arrays;
import java.util.Vector;

public abstract class TeamTalkBase implements AutoCloseable
{
    private long ttInst = 0;

    @SuppressWarnings({"deprecation","removal"})
    @Override
    protected void finalize( ) throws Throwable {
        closeTeamTalk();
        ttInst = 0;
        super.finalize();
    }

    @Override
    public void close() throws Exception {
        closeTeamTalk();
        ttInst = 0;
    }

    public static native String getVersion();

    private native long initTeamTalkPoll();

    protected TeamTalkBase(boolean create_instance) {
        if(create_instance)
            ttInst = initTeamTalkPoll();
    }

    public native boolean closeTeamTalk();

    public native boolean getMessage(TTMessage pMsg,
                                     int pnWaitMs);

    public native boolean pumpMessage(int nClientEvent,
                                      int nIdentifier);

    public native int getFlags();

    public static native boolean setLicenseInformation(String szRegName,
                                                       String szRegKey);

    public static native boolean getDefaultSoundDevices(IntPtr lpnInputDeviceID,
                                                        IntPtr lpnOutputDeviceID);

    private static native boolean getSoundDevices(SoundDevice[] lpSoundDevices,
                                                  IntPtr lpnHowMany);
    public static boolean getSoundDevices(Vector<SoundDevice> lpSoundDevices) {
        IntPtr lpnHowMany = new IntPtr();
        if(!getSoundDevices(null, lpnHowMany))
            return false;

        SoundDevice[] devs = new SoundDevice[lpnHowMany.value];
        if(getSoundDevices(devs, lpnHowMany))
        {
            lpSoundDevices.addAll(Arrays.asList(devs).subList(0, lpnHowMany.value));
        }
        return true;
    }


    public static native boolean restartSoundSystem();

    public static native long startSoundLoopbackTest(int nInputDeviceID,
                                                     int nOutputDeviceID,
                                                     int nSampleRate,
                                                     int nChannels,
                                                     boolean bDuplexMode,
                                                     SpeexDSP lpSpeexDSP);

    private static native long startSoundLoopbackTestEx(int nInputDeviceID,
                                                       int nOutputDeviceID,
                                                       int nSampleRate,
                                                       int nChannels,
                                                       boolean bDuplexMode,
                                                       AudioPreprocessor lpAudioPreprocessor,
                                                       SoundDeviceEffects lpSoundDeviceEffects);

    public static long startSoundLoopbackTest(int nInputDeviceID,
                                              int nOutputDeviceID,
                                              int nSampleRate,
                                              int nChannels,
                                              boolean bDuplexMode,
                                              AudioPreprocessor lpAudioPreprocessor,
                                              SoundDeviceEffects lpSoundDeviceEffects) {
        return startSoundLoopbackTestEx(nInputDeviceID, nOutputDeviceID, nSampleRate,
                                        nChannels, bDuplexMode, lpAudioPreprocessor,
                                        lpSoundDeviceEffects);
    }


    public static native boolean closeSoundLoopbackTest(long lpTTSoundLoop);

    public native boolean initSoundInputDevice(int nInputDeviceID);

    public static native boolean initSoundInputSharedDevice(int nSampleRate, int nChannels, int nFrameSize);
    
    public native boolean initSoundOutputDevice(int nOutputDeviceID);

    public static native boolean initSoundOutputSharedDevice(int nSampleRate, int nChannels, int nFrameSize);

    public native boolean initSoundDuplexDevices(int nInputDeviceID, int nOutputDeviceID);

    public native boolean closeSoundInputDevice();

    public native boolean closeSoundOutputDevice();

    public native boolean closeSoundDuplexDevices();

    public native boolean setSoundDeviceEffects(SoundDeviceEffects lpSoundDeviceEffects);

    public native boolean getSoundDeviceEffects(SoundDeviceEffects lpSoundDeviceEffects);

    public native int getSoundInputLevel();

    public native boolean setSoundInputGainLevel(int nLevel);

    public native int getSoundInputGainLevel();

    public native boolean setSoundInputPreprocess(SpeexDSP lpSpeexDSP);

    public native boolean getSoundInputPreprocess(SpeexDSP lpSpeexDSP);

    private native boolean setSoundInputPreprocessEx(AudioPreprocessor lpAudioPreprocessor);
    public boolean setSoundInputPreprocess(AudioPreprocessor lpAudioPreprocessor) {
        return setSoundInputPreprocessEx(lpAudioPreprocessor);
    }

    private native boolean getSoundInputPreprocessEx(AudioPreprocessor lpAudioPreprocessor);
    public boolean getSoundInputPreprocess(AudioPreprocessor lpAudioPreprocessor) {
        return getSoundInputPreprocessEx(lpAudioPreprocessor);
    }

    public native boolean setSoundOutputVolume(int nVolume);

    public native int getSoundOutputVolume();

    public native boolean setSoundOutputMute(boolean bMuteAll);

    public native boolean enable3DSoundPositioning(boolean bEnable);

    public native boolean autoPositionUsers();

    public native boolean enableAudioBlockEvent(int nUserID, int uStreamTypes, boolean bEnable);

    private native boolean enableAudioBlockEventEx(int nUserID,
                                                   int uStreamTypes, AudioFormat lpAudioFormat, boolean bEnable);
    public boolean enableAudioBlockEvent(int nUserID, int uStreamTypes, AudioFormat lpAudioFormat, boolean bEnable) {
        return enableAudioBlockEventEx(nUserID, uStreamTypes, lpAudioFormat, bEnable);
    }

    public native boolean insertAudioBlock(AudioBlock lpAudioBlock);

    public native boolean enableVoiceTransmission(boolean bEnable);

    public native boolean enableVoiceActivation(boolean bEnable);

    public native boolean setVoiceActivationLevel(int nLevel);

    public native int getVoiceActivationLevel();

    public native boolean setVoiceActivationStopDelay(int nDelayMSec);

    public native int getVoiceActivationStopDelay();

    public native boolean startRecordingMuxedAudioFile(AudioCodec lpAudioCodec,
                                                       String szAudioFileName,
                                                       int uAFF);

    private native boolean startRecordingMuxedAudioFileEx(int nChannelID,
                                                         String szAudioFileName,
                                                         int uAFF);

    public boolean startRecordingMuxedAudioFile(int nChannelID,
                                                String szAudioFileName,
                                                int uAFF) {
        return startRecordingMuxedAudioFileEx(nChannelID, szAudioFileName, uAFF);
    }

    public native boolean startRecordingMuxedStreams(int uStreamTypes,
                                                     AudioCodec lpAudioCodec,
                                                     String szAudioFileName,
                                                     int uAFF);

    public native boolean stopRecordingMuxedAudioFile();

    private native boolean stopRecordingMuxedAudioFileEx(int nChannelID);
    public boolean stopRecordingMuxedAudioFile(int nChannelID) {
        return stopRecordingMuxedAudioFileEx(nChannelID);
    }

    public native boolean startVideoCaptureTransmission(VideoCodec lpVideoCodec);

    public native boolean stopVideoCaptureTransmission();

    private static native boolean getVideoCaptureDevices(VideoCaptureDevice[] lpVideoDevices,
                                                         IntPtr lpnHowMany);
    public static boolean getVideoCaptureDevices(Vector<VideoCaptureDevice> lpVideoDevices) {
        IntPtr lpnHowMany = new IntPtr();
        if(!getVideoCaptureDevices(null, lpnHowMany))
            return false;

        VideoCaptureDevice[] devs = new VideoCaptureDevice[lpnHowMany.value];
        if(getVideoCaptureDevices(devs, lpnHowMany))
        {
            lpVideoDevices.addAll(Arrays.asList(devs).subList(0, lpnHowMany.value));
        }
        return true;
    }

    public native boolean initVideoCaptureDevice(String szDeviceID, VideoFormat lpVideoFormat);

    public native boolean closeVideoCaptureDevice();

    public native VideoFrame acquireUserVideoCaptureFrame(int nUserID);

/*
    private native boolean releaseVideoCaptureFrame(int nUserID);
*/

    public native boolean startStreamingMediaFileToChannel(String szMediaFilePath,
                                                           VideoCodec lpVideoCodec);

    private native boolean startStreamingMediaFileToChannelEx(String szMediaFilePath,
                                                              MediaFilePlayback lpMediaFilePlayback,
                                                              VideoCodec lpVideoCodec);
    public boolean startStreamingMediaFileToChannel(String szMediaFilePath,
                                                    MediaFilePlayback lpMediaFilePlayback,
                                                    VideoCodec lpVideoCodec) {
        return startStreamingMediaFileToChannelEx(szMediaFilePath,
                                                  lpMediaFilePlayback,
                                                  lpVideoCodec);
    }

    public native boolean updateStreamingMediaFileToChannel(MediaFilePlayback lpMediaFilePlayback,
                                                             VideoCodec lpVideoCodec);

    public native boolean stopStreamingMediaFileToChannel();

    public native int initLocalPlayback(String szMediaFilePath,
                                         MediaFilePlayback lpMediaFilePlayback);

    public native boolean updateLocalPlayback(int nPlaybackSessionID,
                                              MediaFilePlayback lpMediaFilePlayback);

    public native boolean stopLocalPlayback(int nPlaybackSessionID);

    public static native boolean getMediaFileInfo(String szMediaFilePath,
                                                  MediaFileInfo lpMediaFileInfo);

    public native VideoFrame acquireUserMediaVideoFrame(int nUserID);

/*
    private native boolean releaseUserMediaVideoFrame(int nUserID);
*/
    public native int sendDesktopWindow(DesktopWindow lpDesktopWindow,
                                        int nConvertBitmap);

    public native boolean closeDesktopWindow();

    public native boolean sendDesktopCursorPosition(int nPosX, int nPosY);

    public native boolean sendDesktopInput(int nUserID,
                                           DesktopInput[] lpDesktopInputs);

    public native DesktopWindow acquireUserDesktopWindow(int nUserID);

    public native DesktopWindow acquireUserDesktopWindowEx(int nUserID,
                                                           int nBitmapFormat);

    public native boolean setEncryptionContext(EncryptionContext lpEncryptionContext);

    public native boolean connect(String szHostAddress,
                                  int nTcpPort,
                                  int nUdpPort,
                                  int nLocalTcpPort,
                                  int nLocalUdpPort,
                                  boolean bEncrypted);

    public native boolean connectSysID(String szHostAddress,
                                       int nTcpPort,
                                       int nUdpPort,
                                       int nLocalTcpPort,
                                       int nLocalUdpPort,
                                       boolean bEncrypted,
                                       String szSystemID);

    public native boolean connectEx(String szHostAddress,
                                    int nTcpPort,
                                    int nUdpPort,
                                    String szBindIPAddr,
                                    int nLocalTcpPort,
                                    int nLocalUdpPort,
                                    boolean bEncrypted);

    public native boolean disconnect();
    
    public native boolean queryMaxPayload(int nUserID);

    public native boolean getClientStatistics(ClientStatistics lpClientStatistics);

    public native boolean setClientKeepAlive(ClientKeepAlive lpClientKeepAlive);

    public native boolean getClientKeepAlive(ClientKeepAlive lpClientKeepAlive);

    public native int doPing();

    public native int doLogin(String szNickname,
                              String szUsername,
                              String szPassword);

    public native int doLoginEx(String szNickname,
                                String szUsername,
                                String szPassword,
                                String szClientName);

    public native int doLogout();

    public native int doJoinChannel(Channel lpChannel);

    public native int doJoinChannelByID(int nChannelID,
                                        String szPassword);

    public native int doLeaveChannel();

    public native int doChangeNickname(String szNewNick);

    public native int doChangeStatus(int nStatusMode, String szStatusMessage);

    public native int doTextMessage(TextMessage lpTextMessage);

    public native int doChannelOp(int nUserID, int nChannelID, boolean bMakeOperator);

    public native int doChannelOpEx(int nUserID, int nChannelID,
                                    String szOpPassword,boolean bMakeOperator);

    public native int doKickUser(int nUserID, int nChannelID);

    public native int doSendFile(int nChannelID, String szLocalFilePath);

    public native int doRecvFile(int nChannelID, int nFileID,
                                 String szLocalFilePath);

    public native int doDeleteFile(int nChannelID, int nFileID);

    public native int doSubscribe(int nUserID, int uSubscriptions);

    public native int doUnsubscribe(int nUserID, int uSubscriptions);

    public native int doMakeChannel(Channel lpChannel);

    public native int doUpdateChannel(Channel lpChannel);

    public native int doRemoveChannel(int nChannelID);

    public native int doMoveUser(int nUserID, int nChannelID);

    public native int doUpdateServer(ServerProperties lpServerProperties);

    public native int doListUserAccounts(int nIndex, int nCount);

    public native int doNewUserAccount(UserAccount lpUserAccount);

    public native int doDeleteUserAccount(String szUsername);

    public native int doBanUser(int nUserID, int nChannelID);

    public native int doBanUserEx(int nUserID, int uBanTypes);

    public native int doBan(BannedUser lpBannedUser);

    public native int doBanIPAddress(String szIPAddress, int nChannelID);

    public native int doUnBanUser(String szIPAddress, int nChannelID);

    public native int doUnBanUserEx(BannedUser lpBannedUser);

    public native int doListBans(int nChannelID, int nIndex, int nCount);

    public native int doSaveConfig();

    public native int doQueryServerStats();

    public native int doQuit();

    public native boolean getServerProperties(ServerProperties lpServerProperties);

    public native boolean getServerUsers(User[] lpUserIDs, IntPtr lpnHowMany);

    public native int getRootChannelID();

    public native int getMyChannelID();

    public native boolean getChannel(int nChannelID,
                                     Channel lpChannel);

    public native String getChannelPath(int nChannelID);

    public native int getChannelIDFromPath(String szChannelPath);

    public native boolean getChannelUsers(int nChannelID,
                                          User[] lpUsers,
                                          IntPtr lpnHowMany);

    public native boolean getChannelFiles(int nChannelID,
                                          RemoteFile[] lpRemoteFiles,
                                          IntPtr lpnHowMany);

    public native boolean getChannelFile(int nChannelID,
                                         int nFileID,
                                         RemoteFile lpRemoteFile);

    public native boolean isChannelOperator(int nUserID, int nChannelID);

    public native boolean getServerChannels(Channel[] lpChannels,
                                            IntPtr lpnHowMany);

    public native int getMyUserID();

    public native boolean getMyUserAccount(UserAccount lpUserAccount);

    public native boolean getUser(int nUserID, User lpUser);

    public native boolean getUserStatistics(int nUserID,
                                            UserStatistics lpUserStatistics);

    public native boolean setUserVolume(int nUserID,
                                        int nStreamType,
                                        int nVolume);

    public native boolean setUserMute(int nUserID,
                                      int nStreamType,
                                      boolean bMute);

    public native boolean setUserStoppedPlaybackDelay(int nUserID,
                                                      int nStreamType,
                                                      int nDelayMSec);

    public native boolean setUserPosition(int nUserID,
                                          int nStreamType,
                                          float x, float y, float z);

    public native boolean setUserStereo(int nUserID,
                                         int nStreamType,
                                         boolean bLeftSpeaker,
                                         boolean bRightSpeaker);

    public native boolean setUserMediaStorageDir(int nUserID,
                                                 String szFolderPath,
                                                 String szFileNameVars,
                                                 int uAFF);
    public native boolean setUserAudioStreamBufferSize(int nUserID, int uStreamType, int nMSec);

    public native AudioBlock acquireUserAudioBlock(int uStreamTypes, int nUserID);

    public native boolean getFileTransferInfo(int nTransferID, FileTransfer lpFileTransfer);

    public native boolean cancelFileTransfer(int nTransferID);

    public static native String getErrorMessage(int nError);
    public native boolean DBG_SetSoundInputTone(int uStreamTypes,
                                                int nFrequency);
    public static native boolean DBG_WriteAudioFileTone(MediaFileInfo lpMediaFileInfo,
                                                        int nFrequency);
}
