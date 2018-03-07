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

public interface SpeexDSPConstants
{
    public static final boolean DEFAULT_AGC_ENABLE = true;
    public static final int DEFAULT_AGC_GAINLEVEL = 8000;
    public static final int DEFAULT_AGC_INC_MAXDB = 12;
    public static final int DEFAULT_AGC_DEC_MAXDB = -40;
    public static final int DEFAULT_AGC_GAINMAXDB = 30;
    public static final boolean DEFAULT_DENOISE_ENABLE = true;
    public static final int DEFAULT_DENOISE_SUPPRESS = -30;
    public static final boolean DEFAULT_ECHO_ENABLE = true;
    public static final int DEFAULT_ECHO_SUPPRESS = -40;
    public static final int DEFAULT_ECHO_SUPPRESS_ACTIVE = -15;
}
