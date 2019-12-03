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

public class TTMessage
{
    public int nClientEvent;
    public int nSource;
    public int ttType;

    public Channel channel;
    public ClientErrorMsg clienterrormsg;
    public DesktopInput desktopinput;
    public FileTransfer filetransfer;
    public MediaFileInfo mediafileinfo;
    public RemoteFile remotefile;
    public ServerProperties serverproperties;
    public ServerStatistics serverstatistics;
    public TextMessage textmessage;
    public User user;
    public UserAccount useraccount;
    public BannedUser banneduser;
    public boolean bActive;
    public int nBytesRemain;
    public int nStreamID;
    public int nPayloadSize;
    public int nStreamType;
    public AudioInputProgress audioinputprogress;

    public TTMessage() { }
}
