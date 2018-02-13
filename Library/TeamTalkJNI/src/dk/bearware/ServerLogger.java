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

package dk.bearware;

public class ServerLogger {
    
    public void userConnected(User lpUser) {
    }
    
    public void userLoggedIn(User lpUser) {
    }

    public void userLoggedOut(User lpUser) {
    }

    public void userDisconnected(User lpUser) {
    }

    public void userTimedout(User lpUser) {
    }

    public void userKicked(User lpKicker, User lpKickee, Channel lpChannel) {
    }

    public void userBanned(User lpBanner, User lpBanee, Channel lpChannel) {
    }
    
    public void userUnbanned(User lpUnbanner, String szIPAddress) {
    }

    public void userUpdated(User lpUser) {
    }

    public void userJoinedChannel(User lpUser, Channel lpChannel) {
    }
    
    public void userLeftChannel(User lpUser, Channel lpChannel) {
    }

    public void userMoved(User lpMover, User lpMovee) {
    }

    public void userTextMessage(User lpUser, TextMessage lpTextMessage) {
    }

    public void channelCreated(Channel lpChannel, User lpUser) {
    }

    public void channelUpdated(Channel lpChannel, User lpUser) {
    }
    
    public void channelRemoved(Channel lpChannel, User lpUser) {
    }

    public void fileUploaded(RemoteFile lpRemoteFile, User lpUser) {
    }

    public void fileDownloaded(RemoteFile lpRemoteFile, User lpUser) {
    }

    public void fileDeleted(RemoteFile lpRemoteFile, User lpUser) {
    }

    public void serverUpdated(ServerProperties lpServerProperties, User lpUser) {
    }

    public void serverSavedConfig(User lpUser) {
    }
}
