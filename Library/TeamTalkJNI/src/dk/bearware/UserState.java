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

public interface UserState {
    public static final int USERSTATE_NONE                  = 0x00000000;
    public static final int USERSTATE_VOICE                 = 0x00000001;
    public static final int USERSTATE_MUTE_VOICE            = 0x00000002;
    public static final int USERSTATE_MUTE_MEDIAFILE        = 0x00000004;
    public static final int USERSTATE_DESKTOP               = 0x00000008;
    public static final int USERSTATE_VIDEOCAPTURE          = 0x00000010;
    public static final int USERSTATE_MEDIAFILE_AUDIO       = 0x00000020;
    public static final int USERSTATE_MEDIAFILE_VIDEO       = 0x00000040;
    public static final int USERSTATE_MEDIAFILE             = USERSTATE_MEDIAFILE_AUDIO |
                                                              USERSTATE_MEDIAFILE_VIDEO;

}
