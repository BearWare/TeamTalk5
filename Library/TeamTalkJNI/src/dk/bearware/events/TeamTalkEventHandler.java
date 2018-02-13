/*
 * Copyright (c) 2005-2017, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

package dk.bearware.events;

import java.util.List;
import java.util.Vector;

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
    
    List<ConnectionListener> conListener = new Vector<ConnectionListener>();
    List<CommandListener> cmdListener = new Vector<CommandListener>();
    List<UserListener> userListener = new Vector<UserListener>();
    List<ClientListener> clientListener = new Vector<ClientListener>();

    public void addConnectionListener(ConnectionListener l) {
        removeConnectionListener(l); // ensure no duplicates
        conListener.add(l);
    }

    public void addCommandListener(CommandListener l) {
        removeCommandListener(l);
        cmdListener.add(l);
    }

    public void addUserListener(UserListener l) {
        removeUserListener(l);
        userListener.add(l);
    }

    public void addClientListener(ClientListener l) {
        removeClientListener(l);
        clientListener.add(l);
    }

    public void removeConnectionListener(ConnectionListener l) {
        conListener.remove(l);
    }

    public void removeCommandListener(CommandListener l) {
        cmdListener.remove(l);
    }

    public void removeUserListener(UserListener l) {
        userListener.remove(l);
    }

    public void removeClientListener(ClientListener l) {
        clientListener.remove(l);
    }
    
    public boolean processEvent(TeamTalkBase ttclient, int timeoutMsecs) {

        TTMessage pMsg = new TTMessage();
        if(!ttclient.getMessage(pMsg, timeoutMsecs))
            return false;
        
        switch(pMsg.nClientEvent) {
            case ClientEvent.CLIENTEVENT_CON_SUCCESS : {
                for(ConnectionListener l : conListener)
                    l.onConnectSuccess();
                break;
            }
            case ClientEvent.CLIENTEVENT_CON_FAILED : {
                for(ConnectionListener l : conListener)
                    l.onConnectFailed();
                break;
            }
            case ClientEvent.CLIENTEVENT_CON_LOST : {
                for(ConnectionListener l : conListener)
                    l.onConnectionLost();
                break;
            }
            case ClientEvent.CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED : {
                for(ConnectionListener l : conListener)
                    l.onMaxPayloadUpdate(pMsg.nPayloadSize);
                break;
            }
            case ClientEvent.CLIENTEVENT_CMD_PROCESSING : {
                for(CommandListener l : cmdListener)
                    l.onCmdProcessing(pMsg.nSource, !pMsg.bActive);
                break;
            }
            case ClientEvent.CLIENTEVENT_CMD_ERROR : {
                assert (pMsg.ttType == TTType.__CLIENTERRORMSG);
                for(CommandListener l : cmdListener)
                    l.onCmdError(pMsg.nSource, pMsg.clienterrormsg);
                break;
            }
            case ClientEvent.CLIENTEVENT_CMD_SUCCESS : {
                for(CommandListener l : cmdListener)
                    l.onCmdSuccess(pMsg.nSource);
                break;
            }
            case ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN : {
                assert (pMsg.ttType == TTType.__USERACCOUNT);
                for(CommandListener l : cmdListener)
                    l.onCmdMyselfLoggedIn(pMsg.nSource, pMsg.useraccount);
                break;
            }
            case ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDOUT : {
                for(CommandListener l : cmdListener)
                    l.onCmdMyselfLoggedOut();
                break;
            }
            case ClientEvent.CLIENTEVENT_CMD_MYSELF_KICKED : {
                for(CommandListener l : cmdListener)
                    if(pMsg.ttType == TTType.__NONE)
                        l.onCmdMyselfKickedFromChannel();
                    else if(pMsg.ttType == TTType.__USER)
                        l.onCmdMyselfKickedFromChannel(pMsg.user);
                break;
            }
            case ClientEvent.CLIENTEVENT_CMD_USER_LOGGEDIN : {
                assert (pMsg.ttType == TTType.__USER);
                User user = pMsg.user;

                for(CommandListener l : cmdListener)
                    l.onCmdUserLoggedIn(user);

                break;
            }
            case ClientEvent.CLIENTEVENT_CMD_USER_LOGGEDOUT : {
                assert (pMsg.ttType == TTType.__USER);
                User user = pMsg.user;

                for(CommandListener l : cmdListener)
                    l.onCmdUserLoggedOut(user);
            }
            break;
            case ClientEvent.CLIENTEVENT_CMD_USER_UPDATE : {
                assert (pMsg.ttType == TTType.__USER);
                User user = pMsg.user;

                for(CommandListener l : cmdListener)
                    l.onCmdUserUpdate(user);
            }
            break;
            case ClientEvent.CLIENTEVENT_CMD_USER_JOINED : {
                assert (pMsg.ttType == TTType.__USER);
                User user = pMsg.user;

                for(CommandListener l : cmdListener)
                    l.onCmdUserJoinedChannel(user);
            }
            break;
            case ClientEvent.CLIENTEVENT_CMD_USER_LEFT : {
                assert (pMsg.ttType == TTType.__USER);
                User user = pMsg.user;

                for(CommandListener l : cmdListener)
                    l.onCmdUserLeftChannel(pMsg.nSource, user);
            }
            break;
            case ClientEvent.CLIENTEVENT_CMD_USER_TEXTMSG : {
                assert (pMsg.ttType == TTType.__TEXTMESSAGE);
                TextMessage msg = pMsg.textmessage;

                for(CommandListener l : cmdListener)
                    l.onCmdUserTextMessage(msg);
            }
            break;
            case ClientEvent.CLIENTEVENT_CMD_CHANNEL_NEW : {
                assert (pMsg.ttType == TTType.__CHANNEL);
                Channel newchan = pMsg.channel;

                for(CommandListener l : cmdListener)
                    l.onCmdChannelNew(newchan);
            }
            break;
            case ClientEvent.CLIENTEVENT_CMD_CHANNEL_UPDATE : {
                assert (pMsg.ttType == TTType.__CHANNEL);
                Channel chan = pMsg.channel;

                for(CommandListener l : cmdListener)
                    l.onCmdChannelUpdate(chan);
            }
            break;
            case ClientEvent.CLIENTEVENT_CMD_CHANNEL_REMOVE : {
                assert (pMsg.ttType == TTType.__CHANNEL);
                Channel chan = pMsg.channel;

                for(CommandListener l : cmdListener)
                    l.onCmdChannelRemove(chan);
            }
            break;
            case ClientEvent.CLIENTEVENT_CMD_SERVER_UPDATE : {
                assert (pMsg.ttType == TTType.__SERVERPROPERTIES);
                ServerProperties srvprop = pMsg.serverproperties;

                for(CommandListener l : cmdListener)
                    l.onCmdServerUpdate(srvprop);
            }
            break;
            case ClientEvent.CLIENTEVENT_CMD_FILE_NEW : {
                assert (pMsg.ttType == TTType.__REMOTEFILE);

                RemoteFile file = pMsg.remotefile;

                for(CommandListener l : cmdListener)
                    l.onCmdFileNew(file);
            }
            break;
            case ClientEvent.CLIENTEVENT_CMD_FILE_REMOVE : {
                assert (pMsg.ttType == TTType.__REMOTEFILE);

                RemoteFile file = pMsg.remotefile;

                for(CommandListener l : cmdListener)
                    l.onCmdFileRemove(file);
            }
            break;
            case ClientEvent.CLIENTEVENT_CMD_USERACCOUNT : {
                assert (pMsg.ttType == TTType.__USERACCOUNT);

                UserAccount useraccount = pMsg.useraccount;

                for(CommandListener l : cmdListener)
                    l.onCmdUserAccount(useraccount);
            }
            break;
            case ClientEvent.CLIENTEVENT_CMD_BANNEDUSER : {
                assert (pMsg.ttType == TTType.__BANNEDUSER);

                BannedUser banneduser  = pMsg.banneduser;

                for(CommandListener l : cmdListener)
                    l.onCmdBannedUser(banneduser);
            }
            break;
            case ClientEvent.CLIENTEVENT_USER_STATECHANGE : {
                assert (pMsg.ttType == TTType.__USER);
                User user = pMsg.user;

                for(UserListener l : userListener)
                    l.onUserStateChange(user);
            }
            break;
            case ClientEvent.CLIENTEVENT_USER_VIDEOCAPTURE : {
                assert (pMsg.ttType == TTType.__INT32);

                int nUserID = pMsg.nSource;
                int nStreamID = pMsg.nStreamID;
                for(UserListener l : userListener)
                    l.onUserVideoCapture(nUserID, nStreamID);
            }
            break;
            case ClientEvent.CLIENTEVENT_USER_MEDIAFILE_VIDEO : {
                assert (pMsg.ttType == TTType.__INT32);

                int nUserID = pMsg.nSource;
                int nStreamID = pMsg.nStreamID;
                for(UserListener l : userListener)
                    l.onUserMediaFileVideo(nUserID, nStreamID);
            }
            break;
            case ClientEvent.CLIENTEVENT_USER_DESKTOPWINDOW : {
                assert (pMsg.ttType == TTType.__INT32);

                int nUserID = pMsg.nSource;
                int nStreamID = pMsg.nStreamID;
                for(UserListener l : userListener)
                    l.onUserDesktopWindow(nUserID, nStreamID);
            }
            break;
            case ClientEvent.CLIENTEVENT_USER_DESKTOPCURSOR : {
                assert (pMsg.ttType == TTType.__DESKTOPINPUT);

                int nUserID = pMsg.nSource;
                DesktopInput desktopinput = pMsg.desktopinput;

                for(UserListener l : userListener)
                    l.onUserDesktopCursor(nUserID, desktopinput);
            }
            break;
            case ClientEvent.CLIENTEVENT_USER_RECORD_MEDIAFILE : {
                assert (pMsg.ttType == TTType.__NONE);

                int nUserID = pMsg.nSource;
                MediaFileInfo mediafileinfo = pMsg.mediafileinfo;
                for(UserListener l : userListener)
                    l.onUserRecordMediaFile(nUserID, mediafileinfo);
            }
            break;
            case ClientEvent.CLIENTEVENT_USER_AUDIOBLOCK : {
                assert (pMsg.ttType == TTType.__STREAMTYPE);
                
                int nUserID = pMsg.nSource;
                for(UserListener l : userListener)
                    l.onUserAudioBlock(nUserID, pMsg.nStreamType);
            }
            break;
            case ClientEvent.CLIENTEVENT_INTERNAL_ERROR : {
                assert (pMsg.ttType == TTType.__CLIENTERRORMSG);

                ClientErrorMsg clienterrormsg = pMsg.clienterrormsg;

                for(ClientListener l : clientListener)
                    l.onInternalError(clienterrormsg);
            }
            break;
            case ClientEvent.CLIENTEVENT_VOICE_ACTIVATION : {
                assert (pMsg.ttType == TTType.__TTBOOL);

                boolean bActive = pMsg.bActive;

                for(ClientListener l : clientListener)
                    l.onVoiceActivation(bActive);
            }
            break;
            case ClientEvent.CLIENTEVENT_HOTKEY : {
                assert (pMsg.ttType == TTType.__TTBOOL);

                int nHotKeyID = pMsg.nSource;
                boolean bActive = pMsg.bActive;

                for(ClientListener l : clientListener)
                    l.onHotKeyToggle(nHotKeyID, bActive);
            }
            break;
            case ClientEvent.CLIENTEVENT_HOTKEY_TEST : {
                assert (pMsg.ttType == TTType.__TTBOOL);

                int nVkCode = pMsg.nSource;
                boolean bActive = pMsg.bActive;

                for(ClientListener l : clientListener)
                    l.onHotKeyTest(nVkCode, bActive);
            }
            break;
            case ClientEvent.CLIENTEVENT_FILETRANSFER : {
                assert (pMsg.ttType == TTType.__FILETRANSFER);

                FileTransfer filetransfer = pMsg.filetransfer;
                for(ClientListener l : clientListener)
                    l.onFileTransfer(filetransfer);
            }
            break;
            case ClientEvent.CLIENTEVENT_DESKTOPWINDOW_TRANSFER : {
                assert (pMsg.ttType == TTType.__INT32);

                int nSessionID = pMsg.nSource;
                int nBytesRemain = pMsg.nBytesRemain;

                for(ClientListener l : clientListener)
                    l.onDesktopWindowTransfer(nSessionID, nBytesRemain);
            }
            break;
            case ClientEvent.CLIENTEVENT_STREAM_MEDIAFILE : {
                assert(pMsg.ttType == TTType.__MEDIAFILEINFO);
                
                MediaFileInfo mediafileinfo = pMsg.mediafileinfo;
                for(ClientListener l : clientListener)
                    l.onStreamMediaFile(mediafileinfo);
            }
            break;
        }
        
        return true;
    }
}
