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
