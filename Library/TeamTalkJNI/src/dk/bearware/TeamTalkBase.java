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

import java.util.Vector;

public abstract class TeamTalkBase
{
    private long ttInst = 0;

    public static native String getVersion();

    private native long initTeamTalkPoll();

    protected TeamTalkBase(boolean create_instance) {
        if(create_instance)
            ttInst = initTeamTalkPoll();
    }
    
    protected void finalize( ) throws Throwable {
        closeTeamTalk(ttInst);
    }

    private native boolean closeTeamTalk(long lpTTInstance);
    public boolean closeTeamTalk() {
        return closeTeamTalk(ttInst);
    }

    private native boolean getMessage(long lpTTInstance,
                                      TTMessage pMsg,
                                      int pnWaitMs);
    public boolean getMessage(TTMessage pMsg,
                              int pnWaitMs) {
        return getMessage(ttInst, pMsg, pnWaitMs);
    }

    private native boolean pumpMessage(long lpTTInstance,
                                       int nClientEvent,
                                       int nIdentifier);

    public boolean pumpMessage(int nClientEvent, int nIdentifier) {
        return pumpMessage(ttInst, nClientEvent, nIdentifier);
    }

    private native int getFlags(long lpTTInstance);
    public int getFlags() { return getFlags(ttInst); }

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
            for(int i=0;i<lpnHowMany.value;i++)
                lpSoundDevices.add(devs[i]);
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

    private native boolean initSoundInputDevice(long lpTTInstance, int nInputDeviceID);
    public boolean initSoundInputDevice(int nInputDeviceID) {
        return initSoundInputDevice(ttInst, nInputDeviceID);
    }
    public static native boolean initSoundInputSharedDevice(int nSampleRate, int nChannels, int nFrameSize);
    private native boolean initSoundOutputDevice(long lpTTInstance, int nOutputDeviceID);
    public boolean initSoundOutputDevice(int nOutputDeviceID) {
        return initSoundOutputDevice(ttInst, nOutputDeviceID);
    }
    public static native boolean initSoundOutputSharedDevice(int nSampleRate, int nChannels, int nFrameSize);
    private native boolean initSoundDuplexDevices(long lpTTInstance, int nInputDeviceID, int nOutputDeviceID);
    public boolean initSoundDuplexDevices(int nInputDeviceID, int nOutputDeviceID) {
        return initSoundDuplexDevices(ttInst, nInputDeviceID, nOutputDeviceID);
    }
    private native boolean closeSoundInputDevice(long lpTTInstance);
    public boolean closeSoundInputDevice() {
        return closeSoundInputDevice(ttInst);
    }
    private native boolean closeSoundOutputDevice(long lpTTInstance);
    public boolean closeSoundOutputDevice() {
        return closeSoundOutputDevice(ttInst);
    }
    private native boolean closeSoundDuplexDevices(long lpTTInstance);
    public boolean closeSoundDuplexDevices() {
        return closeSoundDuplexDevices(ttInst);
    }
    private native boolean setSoundDeviceEffects(long lpTTInstance, SoundDeviceEffects lpSoundDeviceEffects);
    public boolean setSoundDeviceEffects(SoundDeviceEffects lpSoundDeviceEffects) {
        return setSoundDeviceEffects(ttInst, lpSoundDeviceEffects);
    }
    private native boolean getSoundDeviceEffects(long lpTTInstance, SoundDeviceEffects lpSoundDeviceEffects);
    public boolean getSoundDeviceEffects(SoundDeviceEffects lpSoundDeviceEffects) {
        return getSoundDeviceEffects(ttInst, lpSoundDeviceEffects);
    }
    private native int getSoundInputLevel(long lpTTInstance);
    public int getSoundInputLevel() { return getSoundInputLevel(ttInst); }

    private native boolean setSoundInputGainLevel(long lpTTInstance, int nLevel);
    public boolean setSoundInputGainLevel(int nLevel)
        { return setSoundInputGainLevel(ttInst, nLevel); }

    private native int getSoundInputGainLevel(long lpTTInstance);
    public int getSoundInputGainLevel() { return getSoundInputGainLevel(ttInst); }

    private native boolean setSoundInputPreprocess(long lpTTInstance, SpeexDSP lpSpeexDSP);
    public boolean setSoundInputPreprocess(SpeexDSP lpSpeexDSP) {
        return setSoundInputPreprocess(ttInst, lpSpeexDSP);
    }

    private native boolean getSoundInputPreprocess(long lpTTInstance, SpeexDSP lpSpeexDSP);
    public boolean getSoundInputPreprocess(SpeexDSP lpSpeexDSP) {
        return getSoundInputPreprocess(ttInst, lpSpeexDSP);
    }

    private native boolean setSoundInputPreprocessEx(long lpTTInstance, AudioPreprocessor lpAudioPreprocessor);
    public boolean setSoundInputPreprocess(AudioPreprocessor lpAudioPreprocessor) {
        return setSoundInputPreprocessEx(ttInst, lpAudioPreprocessor);
    }

    private native boolean getSoundInputPreprocessEx(long lpTTInstance, AudioPreprocessor lpAudioPreprocessor);
    public boolean getSoundInputPreprocess(AudioPreprocessor lpAudioPreprocessor) {
        return getSoundInputPreprocessEx(ttInst, lpAudioPreprocessor);
    }
    
    private native boolean setSoundOutputVolume(long lpTTInstance, int nVolume);
    public boolean setSoundOutputVolume(int nVolume)
        { return setSoundOutputVolume(ttInst, nVolume); }

    private native int getSoundOutputVolume(long lpTTInstance);
    public int getSoundOutputVolume()
        { return getSoundOutputVolume(ttInst); }

    private native boolean setSoundOutputMute(long lpTTInstance, boolean bMuteAll);
    public boolean setSoundOutputMute(boolean bMuteAll)
        { return setSoundOutputMute(ttInst, bMuteAll); }

    private native boolean enable3DSoundPositioning(long lpTTInstance,
                                                    boolean bEnable);
    public boolean enable3DSoundPositioning(boolean bEnable) {
        return enable3DSoundPositioning(ttInst, bEnable);
    }
    
    private native boolean autoPositionUsers(long lpTTInstance);
    public boolean autoPositionUsers() { return autoPositionUsers(ttInst); }

    private native boolean enableAudioBlockEvent(long lpTTInstance, int nUserID,
                                                 int nStreamType, boolean bEnable);
    public boolean enableAudioBlockEvent(int nUserID, int nStreamType, boolean bEnable)
        { return enableAudioBlockEvent(ttInst, nUserID, nStreamType, bEnable); }

    private native boolean enableAudioBlockEventEx(long lpTTInstance, int nUserID,
                                                   int nStreamType, AudioFormat lpAudioFormat, boolean bEnable);
    public boolean enableAudioBlockEvent(int nUserID, int nStreamType, AudioFormat lpAudioFormat, boolean bEnable)
        { return enableAudioBlockEventEx(ttInst, nUserID, nStreamType, lpAudioFormat, bEnable); }
    
    private native boolean insertAudioBlock(long lpTTInstance, AudioBlock lpAudioBlock);
    public boolean insertAudioBlock(AudioBlock lpAudioBlock) {
        return insertAudioBlock(ttInst, lpAudioBlock);
    }
    
    private native boolean enableVoiceTransmission(long lpTTInstance, 
                                                   boolean bEnable);
    public boolean enableVoiceTransmission(boolean bEnable) {
        return enableVoiceTransmission(ttInst, bEnable);
    }

    private native boolean enableVoiceActivation(long lpTTInstance,
                                                 boolean bEnable);
    public boolean enableVoiceActivation(boolean bEnable)
        { return enableVoiceActivation(ttInst, bEnable); }
    
    private native boolean setVoiceActivationLevel(long lpTTInstance,
                                                   int nLevel);
    public boolean setVoiceActivationLevel(int nLevel)
        { return setVoiceActivationLevel(ttInst, nLevel); }

    private native int getVoiceActivationLevel(long lpTTInstance);
    public int getVoiceActivationLevel()
        { return getVoiceActivationLevel(ttInst); }

    private native boolean setVoiceActivationStopDelay(long lpTTInstance,
                                                       int nDelayMSec);
    public boolean setVoiceActivationStopDelay(int nDelayMSec)
        { return setVoiceActivationStopDelay(nDelayMSec); }

    private native int getVoiceActivationStopDelay(long lpTTInstance);
    public int getVoiceActivationStopDelay()
        { return getVoiceActivationStopDelay(ttInst); }

    private native boolean startRecordingMuxedAudioFile(long lpTTInstance,
                                                        AudioCodec lpAudioCodec,
                                                        String szAudioFileName,
                                                        int uAFF);
    public boolean startRecordingMuxedAudioFile(AudioCodec lpAudioCodec,
                                                String szAudioFileName,
                                                int uAFF) {
        return startRecordingMuxedAudioFile(ttInst, lpAudioCodec, szAudioFileName, uAFF);
    }
    private native boolean startRecordingMuxedAudioFileEx(long lpTTInstance,
                                                          int nChannelID,
                                                          String szAudioFileName,
                                                          int uAFF);
    public boolean startRecordingMuxedAudioFile(int nChannelID,
                                                String szAudioFileName,
                                                int uAFF) {
        return startRecordingMuxedAudioFileEx(ttInst, nChannelID, szAudioFileName, uAFF);
    }

    private native boolean stopRecordingMuxedAudioFile(long lpTTInstance);
    public boolean stopRecordingMuxedAudioFile() {
        return stopRecordingMuxedAudioFile(ttInst);
    }

    private native boolean stopRecordingMuxedAudioFileEx(long lpTTInstance, int nChannelID);
    public boolean stopRecordingMuxedAudioFile(int nChannelID) {
        return stopRecordingMuxedAudioFileEx(ttInst, nChannelID);
    }
    
    private native boolean startVideoCaptureTransmission(long lpTTInstance,
                                                         VideoCodec lpVideoCodec);
    public boolean startVideoCaptureTransmission(VideoCodec lpVideoCodec) {
        return startVideoCaptureTransmission(ttInst, lpVideoCodec);
    }

    private native boolean stopVideoCaptureTransmission(long lpTTInstance);
    public boolean stopVideoCaptureTransmission() {
        return stopVideoCaptureTransmission(ttInst);
    }

    private static native boolean getVideoCaptureDevices(VideoCaptureDevice[] lpVideoDevices,
                                                         IntPtr lpnHowMany);
    public static boolean getVideoCaptureDevices(Vector<VideoCaptureDevice> lpVideoDevices) {
        IntPtr lpnHowMany = new IntPtr();
        if(!getVideoCaptureDevices(null, lpnHowMany))
            return false;

        VideoCaptureDevice[] devs = new VideoCaptureDevice[lpnHowMany.value];
        if(getVideoCaptureDevices(devs, lpnHowMany))
        {
            for(int i=0;i<lpnHowMany.value;i++)
                lpVideoDevices.add(devs[i]);
        }
        return true;
    }

    private native boolean initVideoCaptureDevice(long lpTTInstance, String szDeviceID,
                                                  VideoFormat lpVideoFormat);

    public boolean initVideoCaptureDevice(String szDeviceID,
                                          VideoFormat lpVideoFormat) {
        return initVideoCaptureDevice(ttInst, szDeviceID, lpVideoFormat);
    }

    private native boolean closeVideoCaptureDevice(long lpTTInstance);

    public boolean closeVideoCaptureDevice() {
        return closeVideoCaptureDevice(ttInst);
    }

    private native VideoFrame acquireUserVideoCaptureFrame(long lpTTInstance,
                                                           int nUserID);
    public VideoFrame acquireUserVideoCaptureFrame(int nUserID) {
        return acquireUserVideoCaptureFrame(ttInst, nUserID);
    }

/*
    private native boolean releaseVideoCaptureFrame(long lpTTInstance,
                                                      int nUserID);
    public boolean releaseVideoCaptureFrame(int nUserID) {
        return releaseVideoCaptureFrame(ttInst, nUserID);
    }
*/

    private native boolean startStreamingMediaFileToChannel(long lpTTInstance,
                                                            String szMediaFilePath,
                                                            VideoCodec lpVideoCodec);
    public boolean startStreamingMediaFileToChannel(String szMediaFilePath,
                                                    VideoCodec lpVideoCodec) {
        return startStreamingMediaFileToChannel(ttInst, szMediaFilePath,
                                                lpVideoCodec);
    }
    private native boolean startStreamingMediaFileToChannelEx(long lpTTInstance,
                                                              String szMediaFilePath,
                                                              MediaFilePlayback lpMediaFilePlayback,
                                                              VideoCodec lpVideoCodec);
    public boolean startStreamingMediaFileToChannel(String szMediaFilePath,
                                                    MediaFilePlayback lpMediaFilePlayback,
                                                    VideoCodec lpVideoCodec) {
        return startStreamingMediaFileToChannelEx(ttInst, szMediaFilePath,
                                                  lpMediaFilePlayback,
                                                  lpVideoCodec);
    }

    private native boolean updateStreamingMediaFileToChannel(long lpTTInstance,
                                                             MediaFilePlayback lpMediaFilePlayback,
                                                             VideoCodec lpVideoCodec);
    public boolean updateStreamingMediaFileToChannel(MediaFilePlayback lpMediaFilePlayback,
                                                     VideoCodec lpVideoCodec) {
        return updateStreamingMediaFileToChannel(ttInst, lpMediaFilePlayback, lpVideoCodec);
    }
    
    private native boolean stopStreamingMediaFileToChannel(long lpTTInstance);
    public boolean stopStreamingMediaFileToChannel() {
        return stopStreamingMediaFileToChannel(ttInst);
    }

    private native int initLocalPlayback(long lpTTInstance,
                                         String szMediaFilePath,
                                         MediaFilePlayback lpMediaFilePlayback);
    public int initLocalPlayback(String szMediaFilePath,
                                 MediaFilePlayback lpMediaFilePlayback) {
        return initLocalPlayback(ttInst, szMediaFilePath, lpMediaFilePlayback);
    }

    private native boolean updateLocalPlayback(long lpTTInstance,
                                               int nPlaybackSessionID,
                                               MediaFilePlayback lpMediaFilePlayback);
    public boolean updateLocalPlayback(int nPlaybackSessionID,
                                       MediaFilePlayback lpMediaFilePlayback) {
        return updateLocalPlayback(ttInst, nPlaybackSessionID, lpMediaFilePlayback);
    }

    private native boolean stopLocalPlayback(long lpTTInstance,
                                             int nPlaybackSessionID);
    public boolean stopLocalPlayback(int nPlaybackSessionID) {
        return stopLocalPlayback(ttInst, nPlaybackSessionID);
    }

    public static native boolean getMediaFileInfo(String szMediaFilePath,
                                                  MediaFileInfo lpMediaFileInfo);

    private native VideoFrame acquireUserMediaVideoFrame(long lpTTInstance,
                                                         int nUserID);
    public VideoFrame acquireUserMediaVideoFrame(int nUserID) {
        return acquireUserMediaVideoFrame(ttInst, nUserID);
    }

/*
    private native boolean releaseUserMediaVideoFrame(long lpTTInstance,
                                                      int nUserID);
    public boolean releaseUserMediaVideoFrame(int nUserID) {
        return releaseUserMediaVideoFrame(ttInst, nUserID);
    }
*/
    private native int sendDesktopWindow(long lpTTInstance,
                                         DesktopWindow lpDesktopWindow,
                                         int nConvertBitmap);
    public int sendDesktopWindow(DesktopWindow lpDesktopWindow,
                                 int nConvertBitmap) {
        return sendDesktopWindow(ttInst, lpDesktopWindow, nConvertBitmap);
    }

    private native boolean closeDesktopWindow(long lpTTInstance);
    public boolean closeDesktopWindow() {
        return closeDesktopWindow(ttInst);
    }

    private native boolean sendDesktopCursorPosition(long lpTTInstance,
                                                     int nPosX, int nPosY);
    public boolean sendDesktopCursorPosition(int nPosX, int nPosY) {
        return sendDesktopCursorPosition(ttInst, nPosX, nPosY);
    }

    private native boolean sendDesktopInput(long lpTTInstance,
                                            int nUserID,
                                            DesktopInput[] lpDesktopInputs);
    public boolean sendDesktopInput(int nUserID,
                                    DesktopInput[] lpDesktopInputs) {
        return sendDesktopInput(ttInst, nUserID, lpDesktopInputs);
    }

    private native DesktopWindow acquireUserDesktopWindow(long lpTTInstance,
                                                          int nUserID);
    public DesktopWindow acquireUserDesktopWindow(int nUserID) {
        return acquireUserDesktopWindow(ttInst, nUserID);
    }

    private native DesktopWindow acquireUserDesktopWindowEx(long lpTTInstance,
                                                            int nUserID,
                                                            int nBitmapFormat);
    public DesktopWindow acquireUserDesktopWindowEx(int nUserID,
                                                    int /*BitmapFormat*/nBitmapFormat) {
        return acquireUserDesktopWindowEx(ttInst, nUserID, nBitmapFormat);
    }

    private native boolean connect(long lpTTInstance,
                                   String szHostAddress,
                                   int nTcpPort, 
                                   int nUdpPort, 
                                   int nLocalTcpPort, 
                                   int nLocalUdpPort,
                                   boolean bEncrypted);
    public boolean connect(String szHostAddress,
                           int nTcpPort, 
                           int nUdpPort, 
                           int nLocalTcpPort, 
                           int nLocalUdpPort,
                           boolean bEncrypted) {
        return (szHostAddress != null) ?
            connect(ttInst, szHostAddress, nTcpPort, nUdpPort, 
                    nLocalTcpPort, nLocalUdpPort, bEncrypted) :
            false;
    }

    private native boolean connectSysID(long lpTTInstance,
                                   String szHostAddress,
                                   int nTcpPort, 
                                   int nUdpPort, 
                                   int nLocalTcpPort, 
                                   int nLocalUdpPort,
                                   boolean bEncrypted,
                                   String szSystemID);
    public boolean connectSysID(String szHostAddress,
                                int nTcpPort, 
                                int nUdpPort, 
                                int nLocalTcpPort, 
                                int nLocalUdpPort,
                                boolean bEncrypted,
                                String szSystemID) {
        return (szHostAddress != null && szSystemID != null) ?
            connectSysID(ttInst, szHostAddress, nTcpPort, nUdpPort, 
                         nLocalTcpPort, nLocalUdpPort, bEncrypted, szSystemID) :
            false;
    }
    
    private native boolean connectEx(long lpTTInstance,
                                     String szHostAddress,
                                     int nTcpPort, 
                                     int nUdpPort, 
                                     String szBindIPAddr,
                                     int nLocalTcpPort, 
                                     int nLocalUdpPort,
                                     boolean bEncrypted);
    public boolean connectEx(String szHostAddress,
                             int nTcpPort, 
                             int nUdpPort, 
                             String szBindIPAddr,
                             int nLocalTcpPort, 
                             int nLocalUdpPort,
                             boolean bEncrypted) {
        return connectEx(ttInst, szHostAddress, nTcpPort, nUdpPort, 
                         szBindIPAddr, nLocalTcpPort, nLocalUdpPort,
                         bEncrypted);
    }
    private native boolean disconnect(long lpTTInstance);
    public boolean disconnect() {
        return disconnect(ttInst);
    }
    private native boolean queryMaxPayload(long lpTTInstance, int nUserID);
    public boolean queryMaxPayload(int nUserID) {
        return queryMaxPayload(ttInst, nUserID);
    }

    private native boolean getClientStatistics(long lpTTInstance, ClientStatistics lpClientStatistics);
    public boolean getClientStatistics(ClientStatistics lpClientStatistics) {
        return getClientStatistics(ttInst, lpClientStatistics);
    }

    private native boolean setClientKeepAlive(long lpTTInstance, ClientKeepAlive lpClientKeepAlive);
    public boolean setClientKeepAlive(ClientKeepAlive lpClientKeepAlive) {
        return setClientKeepAlive(ttInst, lpClientKeepAlive);
    }

    private native boolean getClientKeepAlive(long lpTTInstance, ClientKeepAlive lpClientKeepAlive);
    public boolean getClientKeepAlive(ClientKeepAlive lpClientKeepAlive) {
        return getClientKeepAlive(ttInst, lpClientKeepAlive);
    }

    private native int doPing(long lpTTInstance);
    public int doPing() {
        return doPing(ttInst);
    }

    private native int doLogin(long lpTTInstance,
                              String szNickname, 
                              String szUsername,
                              String szPassword);
    public int doLogin(String szNickname, 
                       String szUsername,
                       String szPassword) {
        return (szNickname != null && szUsername != null && szPassword != null) ?
            doLogin(ttInst, szNickname,
                    szUsername, szPassword) :
            -1;
    }

    private native int doLoginEx(long lpTTInstance,
                                 String szNickname, 
                                 String szUsername,
                                 String szPassword,
                                 String szClientName);
    public int doLoginEx(String szNickname, 
                         String szUsername,
                         String szPassword,
                         String szClientName) {
        return (szNickname != null && szUsername != null && szPassword != null && szClientName != null) ?
            doLoginEx(ttInst, szNickname, szUsername, szPassword, szClientName) : -1;
    }

    private native int doLogout(long lpTTInstance);
    public int doLogout() { return doLogout(ttInst); }

    private native int doJoinChannel(long lpTTInstance,
                                     Channel lpChannel);
    public int doJoinChannel(Channel lpChannel) {
        return doJoinChannel(ttInst, lpChannel);
    }
    private native int doJoinChannelByID(long lpTTInstance,
                                         int nChannelID, 
                                         String szPassword);
    public int doJoinChannelByID(int nChannelID, 
                                 String szPassword) {
        return doJoinChannelByID(ttInst, nChannelID, szPassword);
    }
    private native int doLeaveChannel(long lpTTInstance);
    public int doLeaveChannel() { 
        return doLeaveChannel(ttInst);
    }
    private native int doChangeNickname(long lpTTInstance,
                                        String szNewNick);
    public int doChangeNickname(String szNewNick) { 
        return doChangeNickname(ttInst, szNewNick);
    }
    private native int doChangeStatus(long lpTTInstance, int nStatusMode, String szStatusMessage);
    public int doChangeStatus(int nStatusMode, String szStatusMessage) {
        return doChangeStatus(ttInst, nStatusMode, szStatusMessage);
    }
    private native int doTextMessage(long lpTTInstance, TextMessage lpTextMessage);
    public int doTextMessage(TextMessage lpTextMessage) {
        return doTextMessage(ttInst, lpTextMessage);
    }
    private native int doChannelOp(long lpTTInstance, int nUserID, int nChannelID, boolean bMakeOperator);
    public int doChannelOp(int nUserID, int nChannelID, boolean bMakeOperator) {
        return doChannelOp(ttInst, nUserID, nChannelID, bMakeOperator);
    }
    private native int doChannelOpEx(long lpTTInstance, int nUserID, int nChannelID, 
                                     String szOpPassword,boolean bMakeOperator);
    public int doChannelOpEx(int nUserID, int nChannelID, 
                             String szOpPassword, boolean bMakeOperator) {
        return doChannelOpEx(ttInst, nUserID, nChannelID, szOpPassword, bMakeOperator);
    }
    private native int doKickUser(long lpTTInstance, int nUserID, int nChannelID);
    public int doKickUser(int nUserID, int nChannelID) {
        return doKickUser(ttInst, nUserID, nChannelID);
    }
    private native int doSendFile(long lpTTInstance, int nChannelID, String szLocalFilePath);
    public int doSendFile(int nChannelID, String szLocalFilePath) {
        return doSendFile(ttInst, nChannelID, szLocalFilePath);
    }
    private native int doRecvFile(long lpTTInstance, int nChannelID, int nFileID,
                                  String szLocalFilePath);
    public int doRecvFile(int nChannelID, int nFileID,
                          String szLocalFilePath) {
        return doRecvFile(ttInst, nChannelID, nFileID, szLocalFilePath);
    }
    private native int doDeleteFile(long lpTTInstance, int nChannelID, int nFileID);
    public int doDeleteFile(int nChannelID, int nFileID) {
        return doDeleteFile(ttInst, nChannelID, nFileID);
    }
    private native int doSubscribe(long lpTTInstance, int nUserID, int uSubscriptions);
    public int doSubscribe(int nUserID, int uSubscriptions) {
        return doSubscribe(ttInst, nUserID, uSubscriptions);
    }
    private native int doUnsubscribe(long lpTTInstance, int nUserID, int uSubscriptions);
    public int doUnsubscribe(int nUserID, int uSubscriptions) {
        return doUnsubscribe(ttInst, nUserID, uSubscriptions);
    }
    private native int doMakeChannel(long lpTTInstance, Channel lpChannel);
    public int doMakeChannel(Channel lpChannel) {
        return doMakeChannel(ttInst, lpChannel);
    }
    private native int doUpdateChannel(long lpTTInstance, Channel lpChannel);
    public int doUpdateChannel(Channel lpChannel) {
        return doUpdateChannel(ttInst, lpChannel);
    }
    private native int doRemoveChannel(long lpTTInstance, int nChannelID);
    public int doRemoveChannel(int nChannelID) {
        return doRemoveChannel(ttInst, nChannelID);
    }
    private native int doMoveUser(long lpTTInstance, int nUserID, int nChannelID);
    public int doMoveUser(int nUserID, int nChannelID) {
        return doMoveUser(ttInst, nUserID, nChannelID);
    }
    private native int doUpdateServer(long lpTTInstance, ServerProperties lpServerProperties);
    public int doUpdateServer(ServerProperties lpServerProperties) {
        return doUpdateServer(ttInst, lpServerProperties);
    }
    private native int doListUserAccounts(long lpTTInstance, int nIndex, int nCount);
    public int doListUserAccounts(int nIndex, int nCount) {
        return doListUserAccounts(ttInst, nIndex, nCount);
    }
    private native int doNewUserAccount(long lpTTInstance, UserAccount lpUserAccount);
    public int doNewUserAccount(UserAccount lpUserAccount) {
        return doNewUserAccount(ttInst, lpUserAccount);
    }
    private native int doDeleteUserAccount(long lpTTInstance, String szUsername);
    public int doDeleteUserAccount(String szUsername) {
        return doDeleteUserAccount(ttInst, szUsername);
    }
    private native int doBanUser(long lpTTInstance, int nUserID, int nChannelID);
    public int doBanUser(int nUserID, int nChannelID) {
        return doBanUser(ttInst, nUserID, nChannelID);
    }
    private native int doBanUserEx(long lpTTInstance, int nUserID, int uBanTypes);
    public int doBanUserEx(int nUserID, int uBanTypes) {
        return doBanUserEx(ttInst, nUserID, uBanTypes);
    }
    private native int doBan(long lpTTInstance, BannedUser lpBannedUser);
    public int doBan(BannedUser lpBannedUser) {
        return doBan(ttInst, lpBannedUser);
    }
    private native int doBanIPAddress(long lpTTInstance, String szIPAddress, int nChannelID);
    public int doBanIPAddress(String szIPAddress, int nChannelID) {
        return doBanIPAddress(ttInst, szIPAddress, nChannelID);
    }
    private native int doUnBanUser(long lpTTInstance, String szIPAddress, int nChannelID);
    public int doUnBanUser(String szIPAddress, int nChannelID) {
        return doUnBanUser(ttInst, szIPAddress, nChannelID);
    }
    private native int doUnBanUserEx(long lpTTInstance, BannedUser lpBannedUser);
    public int doUnBanUserEx(BannedUser lpBannedUser) {
        return doUnBanUserEx(ttInst, lpBannedUser);
    }
    private native int doListBans(long lpTTInstance, int nChannelID, int nIndex, int nCount);
    public int doListBans(int nChannelID, int nIndex, int nCount) {
        return doListBans(ttInst, nChannelID, nIndex, nCount);
    }
    private native int doSaveConfig(long lpTTInstance);
    public int doSaveConfig() {
        return doSaveConfig(ttInst);
    }
    private native int doQueryServerStats(long lpTTInstance);
    public int doQueryServerStats() {
        return doQueryServerStats(ttInst);
    }
    private native int doQuit(long lpTTInstance);
    public int doQuit() {
        return doQuit(ttInst);
    }
    private native boolean getServerProperties(long lpTTInstance,
                                               ServerProperties lpServerProperties);
    public boolean getServerProperties(ServerProperties lpServerProperties)
        { return getServerProperties(ttInst, lpServerProperties); }
    private native boolean getServerUsers(long lpTTInstance,
                                          User[] lpUserIDs, IntPtr lpnHowMany);
    public boolean getServerUsers(User[] lpUsers, IntPtr lpnHowMany) {
        return getServerUsers(ttInst, lpUsers, lpnHowMany);
    }
    private native int getRootChannelID(long lpTTInstance);
    public int getRootChannelID() {
        return getRootChannelID(ttInst);
    }
    private native int getMyChannelID(long lpTTInstance);
    public int getMyChannelID() {
        return getMyChannelID(ttInst);
    }
    private native boolean getChannel(long lpTTInstance,
                                     int nChannelID,
                                     Channel lpChannel);
    public boolean getChannel(int nChannelID,
                              Channel lpChannel) {
        return getChannel(ttInst, nChannelID, lpChannel);
    }
    private native String getChannelPath(long lpTTInstance,
                                         int nChannelID);
    public String getChannelPath(int nChannelID) {
        return getChannelPath(ttInst, nChannelID);
    }
    private native int getChannelIDFromPath(long lpTTInstance,
                                            String szChannelPath);
    public int getChannelIDFromPath(String szChannelPath) {
        return getChannelIDFromPath(ttInst, szChannelPath);
    }
    private native boolean getChannelUsers(long lpTTInstance,
                                           int nChannelID,
                                           User[] lpUsers,
                                           IntPtr lpnHowMany);
    public boolean getChannelUsers(int nChannelID,
                                   User[] lpUsers,
                                   IntPtr lpnHowMany) {
        return getChannelUsers(ttInst, nChannelID, lpUsers, lpnHowMany);
    }

    private native boolean getChannelFiles(long lpTTInstance,
                                           int nChannelID,
                                           RemoteFile[] lpRemoteFiles,
                                           IntPtr lpnHowMany);
    public boolean getChannelFiles(int nChannelID,
                                   RemoteFile[] lpRemoteFiles,
                                   IntPtr lpnHowMany) {
        return getChannelFiles(ttInst, nChannelID, lpRemoteFiles, lpnHowMany);
    }

    private native boolean getChannelFile(long lpTTInstance,
                                          int nChannelID,
                                          int nFileID, 
                                          RemoteFile lpRemoteFile);
    public boolean getChannelFile(int nChannelID,
                                  int nFileID, 
                                  RemoteFile lpRemoteFile) {
        return getChannelFile(ttInst, nChannelID, nFileID, lpRemoteFile);
    }
    private native boolean isChannelOperator(long lpTTInstance,
                                      int nUserID,
                                      int nChannelID);
    public boolean isChannelOperator(int nUserID,
                                     int nChannelID) {
        return isChannelOperator(ttInst, nUserID, nChannelID);
    }

    private native boolean getServerChannels(long lpTTInstance,
                                             Channel[] lpChannels,
                                             IntPtr lpnHowMany);
    //TODO: return array instead of query
    public boolean getServerChannels(Channel[] lpChannels,
                                     IntPtr lpnHowMany) {
        return getServerChannels(ttInst, lpChannels, lpnHowMany);
    }

    private native int getMyUserID(long lpTTInstance);
    public int getMyUserID() {
        return getMyUserID(ttInst);
    }
    private native boolean getMyUserAccount(long lpTTInstance,
                                            UserAccount lpUserAccount);
    public boolean getMyUserAccount(UserAccount lpUserAccount) {
        return getMyUserAccount(ttInst, lpUserAccount);
    }
    private native boolean getUser(long lpTTInstance,
                                  int nUserID,
                                  User lpUser);
    public boolean getUser(int nUserID,
                           User lpUser) {
        return getUser(ttInst, nUserID, lpUser);
    }
    private native boolean getUserStatistics(long lpTTInstance,
                                             int nUserID,
                                             UserStatistics lpUserStatistics);
    public boolean getUserStatistics(int nUserID,
                           UserStatistics lpUserStatistics) {
        return getUserStatistics(ttInst, nUserID, lpUserStatistics);
    }

    private native boolean setUserVolume(long lpTTInstance,
                                         int nUserID,
                                         int nStreamType,
                                         int nVolume);
    public boolean setUserVolume(int nUserID,
                                 int nStreamType,
                                 int nVolume) {
        return setUserVolume(ttInst, nUserID, nStreamType, nVolume);
    }
    private native boolean setUserMute(long lpTTInstance,
                                       int nUserID,
                                       int nStreamType,
                                       boolean bMute);
    public boolean setUserMute(int nUserID,
                               int nStreamType,
                               boolean bMute) {
        return setUserMute(ttInst, nUserID, nStreamType, bMute);
    }
    private native boolean setUserStoppedPlaybackDelay(long lpTTInstance,
                                                       int nUserID,
                                                       int nStreamType,
                                                       int nDelayMSec);
    public boolean setUserStoppedPlaybackDelay(int nUserID,
                                              int nStreamType,
                                              int nDelayMSec) {
        return setUserStoppedPlaybackDelay(ttInst, nUserID, nStreamType, nDelayMSec);
    }
    private native boolean setUserPosition(long lpTTInstance,
                                           int nUserID,
                                           int nStreamType,
                                           float x, float y, float z);
    public boolean setUserPosition(int nUserID,
                                   int nStreamType,
                                   float x, float y, float z) {
        return setUserPosition(ttInst, nUserID, nStreamType, x, y, z);
    }
    private native boolean setUserStereo(long lpTTInstance,
                                         int nUserID,
                                         int nStreamType,
                                         boolean bLeftSpeaker,
                                         boolean bRightSpeaker);
    public boolean setUserStereo(int nUserID,
                                 int nStreamType,
                                 boolean bLeftSpeaker,
                                 boolean bRightSpeaker) {
        return setUserStereo(ttInst, nUserID, nStreamType, bLeftSpeaker, bRightSpeaker);
    }
    private native boolean setUserMediaStorageDir(long lpTTInstance,
                                              int nUserID,
                                              String szFolderPath,
                                              String szFileNameVars,
                                              int uAFF);
    public boolean setUserMediaStorageDir(int nUserID,
                                      String szFolderPath,
                                      String szFileNameVars, int uAFF) {
        return setUserMediaStorageDir(ttInst, nUserID, szFolderPath, szFileNameVars, uAFF);
    }
    private native boolean setUserAudioStreamBufferSize(long lpTTInstance,
                                                        int nUserID, int uStreamType, int nMSec);
    public boolean setUserAudioStreamBufferSize(int nUserID, int uStreamType, int nMSec) {
        return setUserAudioStreamBufferSize(ttInst, nUserID, uStreamType, nMSec);
    }
    private native AudioBlock acquireUserAudioBlock(long lpTTInstance,
                                                    int nStreamType, int nUserID);
    public AudioBlock acquireUserAudioBlock(int nStreamType, int nUserID) {
        return acquireUserAudioBlock(ttInst, nStreamType, nUserID);
    }
    private native boolean getFileTransferInfo(long lpTTInstance,
                                               int nTransferID, FileTransfer lpFileTransfer);
    public boolean getFileTransferInfo(int nTransferID, FileTransfer lpFileTransfer) {
        return getFileTransferInfo(ttInst, nTransferID, lpFileTransfer);
    }
    private native boolean cancelFileTransfer(long lpTTInstance,
                                              int nTransferID);
    public boolean cancelFileTransfer(int nTransferID) {
        return cancelFileTransfer(ttInst, nTransferID);
    }
    public native String getErrorMessage(int nError);
    private native boolean DBG_SetSoundInputTone(long lpTTInstance,
                                                 int uStreamTypes,
                                                 int nFrequency);
    public boolean DBG_SetSoundInputTone(int uStreamTypes,
                                         int nFrequency) {
        return DBG_SetSoundInputTone(ttInst, uStreamTypes, nFrequency);
    }
    public static native boolean DBG_WriteAudioFileTone(MediaFileInfo lpMediaFileInfo,
                                                        int nFrequency);
}
