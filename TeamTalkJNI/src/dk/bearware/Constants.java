/*
 * Copyright (c) 2005-2014, BearWare.dk
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

public interface Constants
{
    public static final int TT_STRLEN = 512;
    public static final int TT_USERID_MAX = 0xFFF;
    public static final int TT_CHANNELID_MAX = 0xFFF;
    public static final int TT_VIDEOFORMATS_MAX = 128;
    public static final int TT_VOICEUSERS_MAX = 16;
    public static final int TT_VIDEOUSERS_MAX = 16;
    public static final int TT_DESKTOPUSERS_MAX = 16;
    public static final int TT_MEDIAFILEUSERS_MAX = 16;
    public static final int TT_CLASSROOM_FREEFORALL = 0xFFFF;
    public static final int TT_CHANNELS_OPERATOR_MAX = 16;
    public static final int TT_SAMPLERATES_MAX = 16;
    public static final int TT_DESKTOPINPUT_MAX = 16;
    public static final int TT_DESKTOPINPUT_KEYCODE_IGNORE = 0xFFFFFFFF;
    public static final int TT_DESKTOPINPUT_MOUSEPOS_IGNORE = 0xFFFF;
    public static final int TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN = 0x1000;
    public static final int TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN = 0x1001;
    public static final int TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN = 0x1002;

    public static final int DEFAULT_TCP_PORT = 10333;
    public static final int DEFAULT_UDP_PORT = 10333;
    public static final int DEFAULT_TCP_PORT_ENCRYPTED = 10443;
    public static final int DEFAULT_UDP_PORT_ENCRYPTED = 10443;
}
