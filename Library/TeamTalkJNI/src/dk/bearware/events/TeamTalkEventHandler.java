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

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;

import dk.bearware.AudioInputProgress;
import dk.bearware.BannedUser;
import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.ClientEvent;
import dk.bearware.DesktopInput;
import dk.bearware.FileTransfer;
import dk.bearware.MediaFileInfo;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.TTMessage;
import dk.bearware.TTType;
import dk.bearware.TeamTalkBase;
import dk.bearware.TextMessage;
import dk.bearware.User;
import dk.bearware.UserAccount;

public class TeamTalkEventHandler {

    abstract class ProcessTTMessage {
        public Object o;
        ProcessTTMessage(Object o) { this.o = o; }
        abstract void processTTMessage(TTMessage pMsg);
    }

    Map<Integer, Vector<ProcessTTMessage>> listeners = new HashMap<>();

    private Vector<ProcessTTMessage> get(int event) {
        Vector<ProcessTTMessage> v = listeners.get(event);
        if (v == null) {
            v = new Vector<>();
            listeners.put(event, v);
        }
        return v;
    }

    private Vector<ProcessTTMessage> remove(int event, Object l) {
        Vector<ProcessTTMessage> v = get(event);
        for (int i=0;i<v.size();) {
            if (v.get(i).o == l)
                v.remove(i);
            else ++i;
        }
        return v;
    }

    private void register(int event, Object l, boolean enable, ProcessTTMessage p) {
        Vector<ProcessTTMessage> v = remove(event, l);
        if (enable) {
            v.add(p);
        }
    }

    public void unregisterListener(Object l) {
        for (int key : listeners.keySet())
            remove(key, l);
    }

    /** @deprecated use registerOn* methods instead */
    @Deprecated
    public void addConnectionListener(ConnectionListener l) {
        registerOnConnectSuccessListener(l, true);
        registerOnEncryptionErrorListener(l, true);
        registerOnConnectFailedListener(l, true);
        registerOnConnectionLostListener(l, true);
        registerOnMaxPayloadUpdateListener(l, true);
    }
    /** @deprecated use registerOn* methods instead */
    @Deprecated
    public void removeConnectionListener(ConnectionListener l) {
        registerOnConnectSuccessListener(l, false);
        registerOnEncryptionErrorListener(l, false);
        registerOnConnectFailedListener(l, false);
        registerOnConnectionLostListener(l, false);
        registerOnMaxPayloadUpdateListener(l, false);
    }

    public void registerOnConnectSuccessListener(ClientEventListener.OnConnectSuccessListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CON_SUCCESS, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    l.onConnectSuccess();
                }
            });
    }
    public void registerOnEncryptionErrorListener(ClientEventListener.OnEncryptionErrorListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CON_CRYPT_ERROR, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    l.onEncryptionError(pMsg.nSource, pMsg.clienterrormsg);
                }
            });
    }
    public void registerOnConnectFailedListener(ClientEventListener.OnConnectFailedListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CON_FAILED, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    l.onConnectFailed();
                }
            });
    }
    public void registerOnConnectionLostListener(ClientEventListener.OnConnectionLostListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CON_LOST, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    l.onConnectionLost();
                }
            });
    }
    public void registerOnMaxPayloadUpdateListener(ClientEventListener.OnMaxPayloadUpdateListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    l.onMaxPayloadUpdate(pMsg.nPayloadSize);
                }
            });
    }

    /** @deprecated use registerOnCmd* methods instead */
    @Deprecated
    public void addCommandListener(CommandListener l) {
        registerOnCmdProcessing(l, true);
        registerOnCmdError(l, true);
        registerOnCmdSuccess(l, true);
        registerOnCmdMyselfLoggedIn(l, true);
        registerOnCmdMyselfLoggedOut(l, true);
        registerOnCmdMyselfKickedFromChannel(l, true);
        registerOnCmdUserLoggedIn(l, true);
        registerOnCmdUserLoggedOut(l, true);
        registerOnCmdUserUpdate(l, true);
        registerOnCmdUserJoinedChannel(l, true);
        registerOnCmdUserLeftChannel(l, true);
        registerOnCmdUserTextMessage(l, true);
        registerOnCmdChannelNew(l, true);
        registerOnCmdChannelUpdate(l, true);
        registerOnCmdChannelRemove(l, true);
        registerOnCmdServerUpdate(l, true);
        registerOnCmdFileNew(l, true);
        registerOnCmdFileRemove(l, true);
        registerOnCmdUserAccount(l, true);
        registerOnCmdUserAccountNew(l, true);
        registerOnCmdUserAccountRemove(l, true);
        registerOnCmdBannedUser(l, true);
    }
    /** @deprecated use registerOnCmd* methods instead */
    @Deprecated
    public void removeCommandListener(CommandListener l) {
        registerOnCmdProcessing(l, false);
        registerOnCmdError(l, false);
        registerOnCmdSuccess(l, false);
        registerOnCmdMyselfLoggedIn(l, false);
        registerOnCmdMyselfLoggedOut(l, false);
        registerOnCmdMyselfKickedFromChannel(l, false);
        registerOnCmdUserLoggedIn(l, false);
        registerOnCmdUserLoggedOut(l, false);
        registerOnCmdUserUpdate(l, false);
        registerOnCmdUserJoinedChannel(l, false);
        registerOnCmdUserLeftChannel(l, false);
        registerOnCmdUserTextMessage(l, false);
        registerOnCmdChannelNew(l, false);
        registerOnCmdChannelUpdate(l, false);
        registerOnCmdChannelRemove(l, false);
        registerOnCmdServerUpdate(l, false);
        registerOnCmdFileNew(l, false);
        registerOnCmdFileRemove(l, false);
        registerOnCmdUserAccount(l, false);
        registerOnCmdUserAccountNew(l, false);
        registerOnCmdUserAccountRemove(l, false);
        registerOnCmdBannedUser(l, false);
    }
    public void registerOnCmdProcessing(ClientEventListener.OnCmdProcessingListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_PROCESSING, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__TTBOOL);
                    l.onCmdProcessing(pMsg.nSource, !pMsg.bActive);
                }
            });
    }
    public void registerOnCmdError(ClientEventListener.OnCmdErrorListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_ERROR, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__CLIENTERRORMSG);
                    l.onCmdError(pMsg.nSource, pMsg.clienterrormsg);
                }
            });
    }
    public void registerOnCmdSuccess(ClientEventListener.OnCmdSuccessListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_SUCCESS, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    l.onCmdSuccess(pMsg.nSource);
                }
            });
    }
    public void registerOnCmdMyselfLoggedIn(ClientEventListener.OnCmdMyselfLoggedInListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__USERACCOUNT);
                    l.onCmdMyselfLoggedIn(pMsg.nSource, pMsg.useraccount);
                }
            });
    }
    public void registerOnCmdMyselfLoggedOut(ClientEventListener.OnCmdMyselfLoggedOutListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDOUT, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    l.onCmdMyselfLoggedOut();
                }
            });
    }
    public void registerOnCmdMyselfKickedFromChannel(ClientEventListener.OnCmdMyselfKickedFromChannelListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_MYSELF_KICKED, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    if(pMsg.ttType == TTType.__NONE)
                        l.onCmdMyselfKickedFromChannel();
                    else if(pMsg.ttType == TTType.__USER)
                        l.onCmdMyselfKickedFromChannel(pMsg.user);
                }
            });
    }
    public void registerOnCmdUserLoggedIn(ClientEventListener.OnCmdUserLoggedInListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_USER_LOGGEDIN, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__USER);
                    User user = pMsg.user;
                    l.onCmdUserLoggedIn(user);
                }
            });
    }
    public void registerOnCmdUserLoggedOut(ClientEventListener.OnCmdUserLoggedOutListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_USER_LOGGEDOUT, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__USER);
                    User user = pMsg.user;
                    l.onCmdUserLoggedOut(user);
                }
            });
    }
    public void registerOnCmdUserUpdate(ClientEventListener.OnCmdUserUpdateListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_USER_UPDATE, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__USER);
                    User user = pMsg.user;
                    l.onCmdUserUpdate(user);
                }
            });
    }
    public void registerOnCmdUserJoinedChannel(ClientEventListener.OnCmdUserJoinedChannelListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_USER_JOINED, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__USER);
                    User user = pMsg.user;
                    l.onCmdUserJoinedChannel(user);
                }
            });
    }
    public void registerOnCmdUserLeftChannel(ClientEventListener.OnCmdUserLeftChannelListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_USER_LEFT, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__USER);
                    User user = pMsg.user;
                    l.onCmdUserLeftChannel(pMsg.nSource, user);
                }
            });
    }
    public void registerOnCmdUserTextMessage(ClientEventListener.OnCmdUserTextMessageListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_USER_TEXTMSG, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__TEXTMESSAGE);
                    TextMessage msg = pMsg.textmessage;
                    l.onCmdUserTextMessage(msg);
                }
            });
    }
    public void registerOnCmdChannelNew(ClientEventListener.OnCmdChannelNewListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__CHANNEL);
                    l.onCmdChannelNew(pMsg.channel);
                }
            });
    }
    public void registerOnCmdChannelUpdate(ClientEventListener.OnCmdChannelUpdateListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__CHANNEL);
                    l.onCmdChannelUpdate(pMsg.channel);
                }
            });
    }
    public void registerOnCmdChannelRemove(ClientEventListener.OnCmdChannelRemoveListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_CHANNEL_REMOVE, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__CHANNEL);
                    l.onCmdChannelRemove(pMsg.channel);
                }
            });
    }
    public void registerOnCmdServerUpdate(ClientEventListener.OnCmdServerUpdateListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_SERVER_UPDATE, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__SERVERPROPERTIES);
                    l.onCmdServerUpdate(pMsg.serverproperties);
                }
            });
    }
    public void registerOnCmdFileNew(ClientEventListener.OnCmdFileNewListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_FILE_NEW, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__REMOTEFILE);
                    l.onCmdFileNew(pMsg.remotefile);
                }
            });
    }
    public void registerOnCmdFileRemove(ClientEventListener.OnCmdFileRemoveListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_FILE_REMOVE, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__REMOTEFILE);
                    l.onCmdFileRemove(pMsg.remotefile);
                }
            });
    }
    public void registerOnCmdUserAccount(ClientEventListener.OnCmdUserAccountListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_USERACCOUNT, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__USERACCOUNT);
                    l.onCmdUserAccount(pMsg.useraccount);
                }
            });
    }
    public void registerOnCmdUserAccountNew(ClientEventListener.OnCmdUserAccountNewListener l, boolean enable) {
        register( ClientEvent.CLIENTEVENT_CMD_USERACCOUNT_NEW, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__USERACCOUNT);
                    l.onCmdUserAccountNew(pMsg.useraccount);
                }
            });
    }
    public void registerOnCmdUserAccountRemove(ClientEventListener.OnCmdUserAccountRemoveListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_USERACCOUNT_REMOVE, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__USERACCOUNT);
                    l.onCmdUserAccountRemove(pMsg.useraccount);
                }
            });
    }
    public void registerOnCmdBannedUser(ClientEventListener.OnCmdBannedUserListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_CMD_BANNEDUSER, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__BANNEDUSER);
                    l.onCmdBannedUser(pMsg.banneduser);
                }
            });
    }

    /** @deprecated use registerOnUser* methods instead */
    @Deprecated
    public void addUserListener(UserListener l) {
        registerOnUserStateChange(l, true);
        registerOnUserVideoCapture(l, true);
        registerOnUserMediaFileVideo(l, true);
        registerOnUserDesktopWindow(l, true);
        registerOnUserDesktopCursor(l, true);
        registerOnUserDesktopInput(l, true);
        registerOnUserRecordMediaFile(l, true);
        registerOnUserAudioBlock(l, true);
        registerOnUserFirstVoiceStreamPacket(l, true);
    }
    /** @deprecated use registerOnUser* methods instead */
    @Deprecated
    public void removeUserListener(UserListener l) {
        registerOnUserStateChange(l, false);
        registerOnUserVideoCapture(l, false);
        registerOnUserMediaFileVideo(l, false);
        registerOnUserDesktopWindow(l, false);
        registerOnUserDesktopCursor(l, false);
        registerOnUserDesktopInput(l, false);
        registerOnUserRecordMediaFile(l, false);
        registerOnUserAudioBlock(l, false);
        registerOnUserFirstVoiceStreamPacket(l, false);
    }
    public void registerOnUserStateChange(ClientEventListener.OnUserStateChangeListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_USER_STATECHANGE, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__USER);
                    l.onUserStateChange(pMsg.user);
                }
            });
    }
    public void registerOnUserVideoCapture(ClientEventListener.OnUserVideoCaptureListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_USER_VIDEOCAPTURE, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__INT32);
                    int nUserID = pMsg.nSource;
                    int nStreamID = pMsg.nStreamID;
                    l.onUserVideoCapture(nUserID, nStreamID);
                }
            });
    }
    public void registerOnUserMediaFileVideo(ClientEventListener.OnUserMediaFileVideoListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_USER_MEDIAFILE_VIDEO, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__INT32);
                    int nUserID = pMsg.nSource;
                    int nStreamID = pMsg.nStreamID;
                    l.onUserMediaFileVideo(nUserID, nStreamID);
                }
            });
    }
    public void registerOnUserDesktopWindow(ClientEventListener.OnUserDesktopWindowListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__INT32);
                    int nUserID = pMsg.nSource;
                    int nStreamID = pMsg.nStreamID;
                    l.onUserDesktopWindow(nUserID, nStreamID);
                }
            });
    }
    public void registerOnUserDesktopCursor(ClientEventListener.OnUserDesktopCursorListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_USER_DESKTOPCURSOR, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__DESKTOPINPUT);
                    int nUserID = pMsg.nSource;
                    l.onUserDesktopCursor(nUserID, pMsg.desktopinput);
                }
            });
    }
    public void registerOnUserDesktopInput(ClientEventListener.OnUserDesktopInputListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_USER_DESKTOPINPUT, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__DESKTOPINPUT);
                    int nUserID = pMsg.nSource;
                    l.onUserDesktopInput(nUserID, pMsg.desktopinput);
                }
            });
    }
    public void registerOnUserRecordMediaFile(ClientEventListener.OnUserRecordMediaFileListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__NONE);
                    int nUserID = pMsg.nSource;
                    l.onUserRecordMediaFile(nUserID, pMsg.mediafileinfo);
                }
            });
    }
    public void registerOnUserAudioBlock(ClientEventListener.OnUserAudioBlockListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__STREAMTYPE);
                    int nUserID = pMsg.nSource;
                    l.onUserAudioBlock(nUserID, pMsg.nStreamType);
                }
            });
    }
    public void registerOnUserFirstVoiceStreamPacket(ClientEventListener.OnUserFirstVoiceStreamPacketListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_USER_FIRSTVOICESTREAMPACKET, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert(pMsg.ttType == TTType.__USER);
                    User user = pMsg.user;
                    l.onUserFirstVoiceStreamPacket(user, pMsg.nSource);
                }
            });
    }

    /** @deprecated use registerOn* methods instead */
    @Deprecated
    public void addClientListener(ClientListener l) {
        registerOnInternalError(l, true);
        registerOnVoiceActivation(l, true);
        registerOnHotKeyToggle(l, true);
        registerOnHotKeyTest(l, true);
        registerOnFileTransfer(l, true);
        registerOnDesktopWindowTransfer(l, true);
        registerOnStreamMediaFile(l, true);
        registerOnLocalMediaFile(l, true);
        registerOnAudioInput(l, true);
        registerOnSoundDeviceAdded(l, true);
        registerOnSoundDeviceRemoved(l, true);
        registerOnSoundDeviceUnplugged(l, true);
        registerOnSoundDeviceNewDefaultInput(l, true);
        registerOnSoundDeviceNewDefaultOutput(l, true);
        registerOnSoundDeviceNewDefaultInputComDevice(l, true);
        registerOnSoundDeviceNewDefaultOutputComDevice(l, true);
    }
    /** @deprecated use registerOn* methods instead */
    @Deprecated
    public void removeClientListener(ClientListener l) {
        registerOnInternalError(l, false);
        registerOnVoiceActivation(l, false);
        registerOnHotKeyToggle(l, false);
        registerOnHotKeyTest(l, false);
        registerOnFileTransfer(l, false);
        registerOnDesktopWindowTransfer(l, false);
        registerOnStreamMediaFile(l, false);
        registerOnLocalMediaFile(l, false);
        registerOnAudioInput(l, false);
        registerOnSoundDeviceAdded(l, false);
        registerOnSoundDeviceRemoved(l, false);
        registerOnSoundDeviceUnplugged(l, false);
        registerOnSoundDeviceNewDefaultInput(l, false);
        registerOnSoundDeviceNewDefaultOutput(l, false);
        registerOnSoundDeviceNewDefaultInputComDevice(l, false);
        registerOnSoundDeviceNewDefaultOutputComDevice(l, false);
    }
    public void registerOnInternalError(ClientEventListener.OnInternalErrorListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_INTERNAL_ERROR, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__CLIENTERRORMSG);
                    l.onInternalError(pMsg.clienterrormsg);
                }
            });
    }
    public void registerOnVoiceActivation(ClientEventListener.OnVoiceActivationListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_VOICE_ACTIVATION, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__TTBOOL);
                    l.onVoiceActivation(pMsg.bActive);
                }
            });
    }
    public void registerOnHotKeyToggle(ClientEventListener.OnHotKeyToggleListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_HOTKEY, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__TTBOOL);
                    int nHotKeyID = pMsg.nSource;
                    boolean bActive = pMsg.bActive;
                    l.onHotKeyToggle(nHotKeyID, bActive);
                }
            });
    }
    public void registerOnHotKeyTest(ClientEventListener.OnHotKeyTestListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_HOTKEY_TEST, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__TTBOOL);
                    int nVkCode = pMsg.nSource;
                    boolean bActive = pMsg.bActive;
                    l.onHotKeyTest(nVkCode, bActive);
                }
            });
    }
    public void registerOnFileTransfer(ClientEventListener.OnFileTransferListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_FILETRANSFER, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__FILETRANSFER);
                    l.onFileTransfer(pMsg.filetransfer);
                }
            });
    }
    public void registerOnDesktopWindowTransfer(ClientEventListener.OnDesktopWindowTransferListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert (pMsg.ttType == TTType.__INT32);
                    int nSessionID = pMsg.nSource;
                    int nBytesRemain = pMsg.nBytesRemain;
                    l.onDesktopWindowTransfer(nSessionID, nBytesRemain);
                }
            });
    }
    public void registerOnStreamMediaFile(ClientEventListener.OnStreamMediaFileListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert(pMsg.ttType == TTType.__MEDIAFILEINFO);
                    l.onStreamMediaFile(pMsg.mediafileinfo);
                }
            });
    }
    public void registerOnLocalMediaFile(ClientEventListener.OnLocalMediaFileListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_LOCAL_MEDIAFILE, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert(pMsg.ttType == TTType.__MEDIAFILEINFO);
                    l.onLocalMediaFile(pMsg.mediafileinfo);
                }
            });
    }
    public void registerOnAudioInput(ClientEventListener.OnAudioInputListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_AUDIOINPUT, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert(pMsg.ttType == TTType.__AUDIOINPUTPROGRESS);

                    l.onAudioInput(pMsg.audioinputprogress, pMsg.nSource);
                }
            });
    }
    public void registerOnSoundDeviceAdded(ClientEventListener.OnSoundDeviceAddedListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_SOUNDDEVICE_ADDED, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert(pMsg.ttType == TTType.__SOUNDDEVICE);
                    l.onSoundDeviceAdded(pMsg.sounddevice);
                }
            });
    }
    public void registerOnSoundDeviceRemoved(ClientEventListener.OnSoundDeviceRemovedListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_SOUNDDEVICE_REMOVED, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert(pMsg.ttType == TTType.__SOUNDDEVICE);
                    l.onSoundDeviceRemoved(pMsg.sounddevice);
                }
            });
    }
    public void registerOnSoundDeviceUnplugged(ClientEventListener.OnSoundDeviceUnpluggedListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_SOUNDDEVICE_UNPLUGGED, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert(pMsg.ttType == TTType.__SOUNDDEVICE);
                    l.onSoundDeviceUnplugged(pMsg.sounddevice);
                }
            });
    }
    public void registerOnSoundDeviceNewDefaultInput(ClientEventListener.OnSoundDeviceNewDefaultInputListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_INPUT, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert(pMsg.ttType == TTType.__SOUNDDEVICE);
                    l.onSoundDeviceNewDefaultInput(pMsg.sounddevice);
                }
            });
    }
    public void registerOnSoundDeviceNewDefaultOutput(ClientEventListener.OnSoundDeviceNewDefaultOutputListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_OUTPUT, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert(pMsg.ttType == TTType.__SOUNDDEVICE);
                    l.onSoundDeviceNewDefaultOutput(pMsg.sounddevice);
                }
            });
    }
    public void registerOnSoundDeviceNewDefaultInputComDevice(ClientEventListener.OnSoundDeviceNewDefaultInputComDeviceListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_INPUT_COMDEVICE, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert(pMsg.ttType == TTType.__SOUNDDEVICE);
                    l.onSoundDeviceNewDefaultInputComDevice(pMsg.sounddevice);
                }
            });
    }
    public void registerOnSoundDeviceNewDefaultOutputComDevice(ClientEventListener.OnSoundDeviceNewDefaultOutputComDeviceListener l, boolean enable) {
        register(ClientEvent.CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_OUTPUT_COMDEVICE, l, enable, new ProcessTTMessage(l) {
                @Override
                void processTTMessage(TTMessage pMsg) {
                    assert(pMsg.ttType == TTType.__SOUNDDEVICE);
                    l.onSoundDeviceNewDefaultOutputComDevice(pMsg.sounddevice);
                }
            });
    }

    public boolean processEvent(TeamTalkBase ttclient, int timeoutMsecs) {

        TTMessage pMsg = new TTMessage();
        if(!ttclient.getMessage(pMsg, timeoutMsecs))
            return false;

        for (ProcessTTMessage l : get(pMsg.nClientEvent))
            l.processTTMessage(pMsg);

        return true;
    }
}
