/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

package dk.bearware;

public class OpusCodec
{
    public int nSampleRate;
    public int nChannels;
    public int nApplication;
    public int nComplexity;
    public boolean bFEC;
    public boolean bDTX;
    public int nBitRate;
    public boolean bVBR;
    public boolean bVBRConstraint;
    public int nTxIntervalMSec;
    
    public OpusCodec() {}
    public OpusCodec(boolean set_defaults) {
        if(!set_defaults)
            return;

        nSampleRate = OpusConstants.DEFAULT_OPUS_SAMPLERATE;
        nChannels = OpusConstants.DEFAULT_OPUS_CHANNELS;
        nApplication = OpusConstants.DEFAULT_OPUS_APPLICATION;
        nComplexity = OpusConstants.DEFAULT_OPUS_COMPLEXITY;
        bFEC = OpusConstants.DEFAULT_OPUS_FEC;
        bDTX = OpusConstants.DEFAULT_OPUS_DTX;
        nBitRate = OpusConstants.DEFAULT_OPUS_BITRATE;
        bVBR = OpusConstants.DEFAULT_OPUS_VBR;
        bVBRConstraint = OpusConstants.DEFAULT_OPUS_VBRCONSTRAINT;
        nTxIntervalMSec = OpusConstants.DEFAULT_OPUS_DELAY;
    }
}
