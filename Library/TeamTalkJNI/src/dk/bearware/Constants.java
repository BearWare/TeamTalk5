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

public interface Constants
{
    public static final int TT_STRLEN = 512;
    public static final int TT_USERID_MAX = 0xFFF;
    public static final int TT_CHANNELID_MAX = 0xFFF;
    public static final int TT_VIDEOFORMATS_MAX = 1024;
    public static final int TT_TRANSMITUSERS_MAX = 128;
    public static final int TT_CLASSROOM_FREEFORALL = 0xFFFF;
    public static final int TT_CHANNELS_OPERATOR_MAX = 16;
    public static final int TT_TRANSMITQUEUE_MAX = 16;
    public static final int TT_SAMPLERATES_MAX = 16;
    public static final int TT_DESKTOPINPUT_MAX = 16;
    public static final int TT_DESKTOPINPUT_KEYCODE_IGNORE = 0xFFFFFFFF;
    public static final int TT_DESKTOPINPUT_MOUSEPOS_IGNORE = 0xFFFF;
    public static final int TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN = 0x1000;
    public static final int TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN = 0x1001;
    public static final int TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN = 0x1002;
    public static final int TT_TRANSMITUSERS_USERID_INDEX = 0;
    public static final int TT_TRANSMITUSERS_STREAMTYPE_INDEX = 1;
    public static final int TT_LOCAL_USERID = 0;
    public static final int TT_LOCAL_TX_USERID = 0x1002;
    public static final int TT_MUXED_USERID = 0x1001;

    public static final int DEFAULT_TCP_PORT = 10333;
    public static final int DEFAULT_UDP_PORT = 10333;
    public static final int DEFAULT_TCP_PORT_ENCRYPTED = 10443;
    public static final int DEFAULT_UDP_PORT_ENCRYPTED = 10443;
}
