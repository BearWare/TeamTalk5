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

package dk.bearware.backend;

public class TeamTalkConstants {

    public static final int STATUSMODE_AVAILABLE        = 0x00000000;
    public static final int STATUSMODE_AWAY             = 0x00000001; 
    public static final int STATUSMODE_QUESTION         = 0x00000002;
    public static final int STATUSMODE_MODE             = 0x000000FF;

    public static final int STATUSMODE_FLAGS            = 0xFFFFFF00;
    public static final int STATUSMODE_FEMALE           = 0x00000100;
    public static final int STATUSMODE_VIDEOTX          = 0x00000200;
    public static final int STATUSMODE_DESKTOP          = 0x00000400;
    public static final int STATUSMODE_STREAM_MEDIAFILE = 0x00000800;

    public static final int OPUS_MIN_TXINTERVALMSEC = 20;
    public static final int OPUS_MAX_TXINTERVALMSEC = 500;
    public static final int OPUS_DEFAULT_FRAMESIZEMSEC = 0; //framesize = txinterval

    public static final int SPEEX_MIN_TXINTERVALMSEC = 20;
    public static final int SPEEX_MAX_TXINTERVALMSEC = 500;
}
