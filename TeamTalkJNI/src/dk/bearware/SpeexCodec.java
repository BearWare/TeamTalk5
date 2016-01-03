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

public class SpeexCodec
{
    public int nBandmode;
    public int nQuality;
    public int nTxIntervalMSec;
    public boolean bStereoPlayback;
    
    public SpeexCodec() {}
    public SpeexCodec(boolean set_defaults) {
        if(!set_defaults)
            return;

        nBandmode = SpeexConstants.DEFAULT_SPEEX_BANDMODE;
        nQuality = SpeexConstants.DEFAULT_SPEEX_QUALITY;
        nTxIntervalMSec = SpeexConstants.DEFAULT_SPEEX_DELAY;
        bStereoPlayback = SpeexConstants.DEFAULT_SPEEX_SIMSTEREO;
    }
}
