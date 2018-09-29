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

public class Channel
{
    public int nParentID = 0;
    public int nChannelID = 0;
    public String szName = "";
    public String szTopic = "";
    public String szPassword = "";
    public boolean bPassword = false;
    public int uChannelType = ChannelType.CHANNEL_DEFAULT;
    public int nUserData = 0;
    public long nDiskQuota = 0;
    public String szOpPassword = "";
    public int nMaxUsers = 0;
    public AudioCodec audiocodec = new AudioCodec();
    public AudioConfig audiocfg = new AudioConfig();
    public int[][] transmitUsers = new int[Constants.TT_TRANSMITUSERS_MAX][2];
    public int[] transmitUsersQueue = new int[Constants.TT_TRANSMITQUEUE_MAX];

    public Channel() {
    }
    public Channel(boolean default_audiocodec, boolean default_audiocfg) {
        audiocodec = new AudioCodec(default_audiocodec);
        audiocfg = new AudioConfig(default_audiocfg);
    }
}
