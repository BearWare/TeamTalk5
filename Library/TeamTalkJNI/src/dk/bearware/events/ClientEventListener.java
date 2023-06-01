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

package dk.bearware.events;

import dk.bearware.AudioInputProgress;
import dk.bearware.BannedUser;
import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.DesktopInput;
import dk.bearware.FileTransfer;
import dk.bearware.MediaFileInfo;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.SoundDevice;
import dk.bearware.TextMessage;
import dk.bearware.User;
import dk.bearware.UserAccount;

public interface ClientEventListener {

    interface OnConnectSuccessListener {
        void onConnectSuccess();
    }
    interface OnEncryptionErrorListener {
        void onEncryptionError(int opensslErrorNo, ClientErrorMsg errmsg);
    }
    interface OnConnectFailedListener {
        void onConnectFailed();
    }
    interface OnConnectionLostListener {
        void onConnectionLost();
    }
    interface OnMaxPayloadUpdateListener {
        void onMaxPayloadUpdate(int payload_size);
    }

    interface OnCmdErrorListener {
        void onCmdError(int cmdId, ClientErrorMsg errmsg);
    }
    interface OnCmdSuccessListener {
        void onCmdSuccess(int cmdId);
    }
    interface OnCmdProcessingListener {
        void onCmdProcessing(int cmdId, boolean complete);
    }
    interface OnCmdMyselfLoggedInListener {
        void onCmdMyselfLoggedIn(int my_userid, UserAccount useraccount);
    }
    interface OnCmdMyselfLoggedOutListener {
        void onCmdMyselfLoggedOut();
    }
    interface OnCmdMyselfKickedFromChannelListener {
        void onCmdMyselfKickedFromChannel();
        void onCmdMyselfKickedFromChannel(User kicker);
    }
    interface OnCmdUserLoggedInListener {
        void onCmdUserLoggedIn(User user);
    }
    interface OnCmdUserLoggedOutListener {
        void onCmdUserLoggedOut(User user);
    }
    interface OnCmdUserUpdateListener {
        void onCmdUserUpdate(User user);
    }
    interface OnCmdUserJoinedChannelListener {
        void onCmdUserJoinedChannel(User user);
    }
    interface OnCmdUserLeftChannelListener {
        void onCmdUserLeftChannel(int channelid, User user);
    }
    interface OnCmdUserTextMessageListener {
        void onCmdUserTextMessage(TextMessage textmessage);
    }
    interface OnCmdChannelNewListener {
        void onCmdChannelNew(Channel channel);
    }
    interface OnCmdChannelUpdateListener {
        void onCmdChannelUpdate(Channel channel);
    }
    interface OnCmdChannelRemoveListener {
        void onCmdChannelRemove(Channel channel);
    }
    interface OnCmdServerUpdateListener {
        void onCmdServerUpdate(ServerProperties serverproperties);
    }
    interface OnCmdFileNewListener {
        void onCmdFileNew(RemoteFile remotefile);
    }
    interface OnCmdFileRemoveListener {
        void onCmdFileRemove(RemoteFile remotefile);
    }
    interface OnCmdUserAccountListener {
        void onCmdUserAccount(UserAccount useraccount);
    }
    interface OnCmdBannedUserListener {
        void onCmdBannedUser(BannedUser banneduser);
    }
    interface OnCmdUserAccountNewListener {
        void onCmdUserAccountNew(UserAccount useraccount);
    }
    interface OnCmdUserAccountRemoveListener {
        void onCmdUserAccountRemove(UserAccount useraccount);
    }

    interface OnUserStateChangeListener {
        void onUserStateChange(User user);
    }
    interface OnUserVideoCaptureListener {
        void onUserVideoCapture(int nUserID, int nStreamID);
    }
    interface OnUserMediaFileVideoListener {
        void onUserMediaFileVideo(int nUserID, int nStreamID);
    }
    interface OnUserDesktopWindowListener {
        void onUserDesktopWindow(int nUserID, int nStreamID);
    }
    interface OnUserDesktopCursorListener {
        void onUserDesktopCursor(int nUserID, DesktopInput desktopinput);
    }
    interface OnUserDesktopInputListener {
        void onUserDesktopInput(int nUserID, DesktopInput desktopinput);
    }
    interface OnUserRecordMediaFileListener {
        void onUserRecordMediaFile(int nUserID, MediaFileInfo mediafileinfo);
    }
    interface OnUserAudioBlockListener {
        void onUserAudioBlock(int nUserID, int nStreamType);
    }
    interface OnUserFirstVoiceStreamPacketListener {
        void onUserFirstVoiceStreamPacket(User user, int nStreamID);
    }

    interface OnInternalErrorListener {
        void onInternalError(ClientErrorMsg clienterrormsg);
    }
    interface OnVoiceActivationListener {
        void onVoiceActivation(boolean bVoiceActive);
    }
    interface OnHotKeyToggleListener {
        void onHotKeyToggle(int nHotKeyID, boolean bActive);
    }
    interface OnHotKeyTestListener {
        void onHotKeyTest(int nVkCode, boolean bActive);
    }
    interface OnFileTransferListener {
        void onFileTransfer(FileTransfer filetransfer);
    }
    interface OnDesktopWindowTransferListener {
        void onDesktopWindowTransfer(int nSessionID, int nTransferRemaining);
    }
    interface OnStreamMediaFileListener {
        void onStreamMediaFile(MediaFileInfo mediafileinfo);
    }
    interface OnLocalMediaFileListener {
        void onLocalMediaFile(MediaFileInfo mediafileinfo);
    }
    interface OnAudioInputListener {
        void onAudioInput(AudioInputProgress aip, int nStreamID);
    }
    interface OnSoundDeviceAddedListener {
        void onSoundDeviceAdded(SoundDevice sounddevice);
    }
    interface OnSoundDeviceRemovedListener {
        void onSoundDeviceRemoved(SoundDevice sounddevice);
    }
    interface OnSoundDeviceUnpluggedListener {
        void onSoundDeviceUnplugged(SoundDevice sounddevice);
    }
    interface OnSoundDeviceNewDefaultInputListener {
        void onSoundDeviceNewDefaultInput(SoundDevice sounddevice);
    }
    interface OnSoundDeviceNewDefaultOutputListener {
        void onSoundDeviceNewDefaultOutput(SoundDevice sounddevice);
    }
    interface OnSoundDeviceNewDefaultInputComDeviceListener {
        void onSoundDeviceNewDefaultInputComDevice(SoundDevice sounddevice);
    }
    interface OnSoundDeviceNewDefaultOutputComDeviceListener {
        void onSoundDeviceNewDefaultOutputComDevice(SoundDevice sounddevice);
    }
}
