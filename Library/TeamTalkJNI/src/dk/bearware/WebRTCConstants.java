/*
 * Copyright (c) 2005-2020, BearWare.dk
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

public interface WebRTCConstants
{
    public static final boolean DEFAULT_WEBRTC_PREAMPLIFIER_ENABLE = false;
    public static final float DEFAULT_WEBRTC_PREAMPLIFIER_GAINFACTOR = 1.0f;
    public static final boolean DEFAULT_WEBRTC_GAINCTL_ENABLE = false;
    public static final float DEFAULT_WEBRTC_GAINDB = 0;
    public static final boolean DEFAULT_WEBRTC_SAT_PROT_ENABLE = false;
    public static final float DEFAULT_WEBRTC_HEADROOM_DB = 5;
    public static final float DEFAULT_WEBRTC_MAXGAIN_DB = 50;
    public static final float DEFAULT_WEBRTC_INITIAL_GAIN_DB = 15;
    public static final float DEFAULT_WEBRTC_MAXGAIN_DBSEC = 6;
    public static final float DEFAULT_WEBRTC_MAX_OUT_NOISE = -50;
    public static final boolean DEFAULT_WEBRTC_NOISESUPPRESS_ENABLE = false;
    public static final int DEFAULT_WEBRTC_NOISESUPPRESS_LEVEL = 2;
    public static final boolean DEFAULT_WEBRTC_ECHO_CANCEL_ENABLE = false;
    public static final float WEBRTC_GAINCONTROLLER2_FIXEDGAIN_MAX = 49.9f;
}
