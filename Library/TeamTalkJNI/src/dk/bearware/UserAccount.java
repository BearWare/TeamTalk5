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

public class UserAccount
{
    public String szUsername = "";
    public String szPassword = "";
    public int uUserType = UserType.USERTYPE_NONE;
    public int uUserRights = UserRight.USERRIGHT_NONE;
    public int nUserData = 0;
    public String szNote = "";
    public String szInitChannel = "";
    public int[] autoOperatorChannels = new int[Constants.TT_CHANNELS_OPERATOR_MAX];
    public int nAudioCodecBpsLimit;
    public AbusePrevention abusePrevent = new AbusePrevention();

    public void copy(UserAccount u)
    {
        szUsername = u.szUsername;
        szPassword = u.szPassword;
        uUserType = u.uUserType;
        uUserRights = u.uUserRights;
        nUserData = u.nUserData;
        szNote = u.szNote;
        szInitChannel = u.szInitChannel;
        autoOperatorChannels = u.autoOperatorChannels.clone();
        nAudioCodecBpsLimit = u.nAudioCodecBpsLimit;
        abusePrevent = u.abusePrevent;
    }
}
