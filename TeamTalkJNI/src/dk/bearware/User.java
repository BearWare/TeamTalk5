/*
 * Copyright (c) 2005-2016, BearWare.dk
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


public class User
{
    public int nUserID;
    public String szUsername;
    public int nUserData;
    public int uUserType;
    public String szIPAddress;
    public int uVersion;
    public int nChannelID;
    public int uLocalSubscriptions;
    public int uPeerSubscriptions;
    public String szNickname;
    public int nStatusMode;
    public String szStatusMsg;
    public int uUserState;
    public String szMediaStorageDir;
    public int nVolumeVoice;
    public int nVolumeMediaFile;
    public int nStoppedDelayVoice;
    public int nStoppedDelayMediaFile;
    public boolean[] stereoPlaybackVoice;
    public boolean[] stereoPlaybackMediaFile;
    public int nBufferMSecVoice;
    public int nBufferMSecMediaFile;
    public String szClientName;
}
