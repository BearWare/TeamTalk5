/*
 * Copyright (c) 2005-2016, BearWare.dk
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
