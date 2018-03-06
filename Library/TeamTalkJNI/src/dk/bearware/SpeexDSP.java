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

public class SpeexDSP
{
    public boolean bEnableAGC;
    public int nGainLevel;
    public int nMaxIncDBSec;
    public int nMaxDecDBSec;
    public int nMaxGainDB;
    public boolean bEnableDenoise;
    public int nMaxNoiseSuppressDB;
    public boolean bEnableEchoCancellation;
    public int nEchoSuppress;
    public int nEchoSuppressActive;

    public SpeexDSP() {
    }

    public SpeexDSP(boolean set_defaults) {
        if(!set_defaults)
            return;

        bEnableAGC = SpeexDSPConstants.DEFAULT_AGC_ENABLE;
        nGainLevel = SpeexDSPConstants.DEFAULT_AGC_GAINLEVEL;
        nMaxIncDBSec = SpeexDSPConstants.DEFAULT_AGC_INC_MAXDB;
        nMaxDecDBSec = SpeexDSPConstants.DEFAULT_AGC_DEC_MAXDB;
        nMaxGainDB = SpeexDSPConstants.DEFAULT_AGC_GAINMAXDB;
        bEnableDenoise = SpeexDSPConstants.DEFAULT_DENOISE_ENABLE;
        nMaxNoiseSuppressDB = SpeexDSPConstants.DEFAULT_DENOISE_SUPPRESS;
        bEnableEchoCancellation = SpeexDSPConstants.DEFAULT_ECHO_ENABLE;
        nEchoSuppress = SpeexDSPConstants.DEFAULT_ECHO_SUPPRESS;
        nEchoSuppressActive = SpeexDSPConstants.DEFAULT_ECHO_SUPPRESS_ACTIVE;
    }
}
