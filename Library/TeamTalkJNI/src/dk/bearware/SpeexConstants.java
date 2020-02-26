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

public interface SpeexConstants {

    public static final int SPEEX_BANDMODE_NARROW = 0;
    public static final int SPEEX_BANDMODE_WIDE = 1;
    public static final int SPEEX_BANDMODE_UWIDE = 2;

    public static final int SPEEX_QUALITY_MIN = 0;
    public static final int SPEEX_QUALITY_MAX = 10;

    public static final int SPEEX_NB_MIN_BITRATE = 2150;
    public static final int SPEEX_NB_MAX_BITRATE = 24600;
    public static final int SPEEX_WB_MIN_BITRATE = 3950;
    public static final int SPEEX_WB_MAX_BITRATE = 42200;
    public static final int SPEEX_UWB_MIN_BITRATE = 4150;
    public static final int SPEEX_UWB_MAX_BITRATE = 44000;

    public static final int DEFAULT_SPEEX_BANDMODE = 1;
    public static final int DEFAULT_SPEEX_QUALITY = 4;
    public static final int DEFAULT_SPEEX_DELAY = 40;
    public static final boolean DEFAULT_SPEEX_SIMSTEREO = false;
    public static final int DEFAULT_SPEEX_BITRATE = 0;
    public static final int DEFAULT_SPEEX_MAXBITRATE = 0;
    public static final boolean DEFAULT_SPEEX_DTX = true;

}
