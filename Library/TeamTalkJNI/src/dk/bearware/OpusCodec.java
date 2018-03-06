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
