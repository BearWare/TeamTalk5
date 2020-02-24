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

public class OpusConstants {
    public static final int OPUS_APPLICATION_VOIP = 2048;
    public static final int OPUS_APPLICATION_AUDIO = 2049;
    public static final int OPUS_MIN_BITRATE = 6000;
    public static final int OPUS_MAX_BITRATE = 510000;
    public static final int OPUS_MIN_FRAMESIZE = 2;
    public static final int OPUS_MAX_FRAMESIZE = 60;
    public static final int OPUS_REALMAX_FRAMESIZE = 120;
    
    public static final int DEFAULT_OPUS_APPLICATION = OPUS_APPLICATION_VOIP;
    public static final int DEFAULT_OPUS_SAMPLERATE = 48000;
    public static final int DEFAULT_OPUS_CHANNELS = 1;
    public static final int DEFAULT_OPUS_COMPLEXITY = 10;
    public static final boolean DEFAULT_OPUS_FEC = true;
    public static final boolean DEFAULT_OPUS_DTX = true;
    public static final boolean DEFAULT_OPUS_VBR = true;
    public static final boolean DEFAULT_OPUS_VBRCONSTRAINT = false;
    public static final int DEFAULT_OPUS_BITRATE = 32000;
    public static final int DEFAULT_OPUS_DELAY = 40;
}
