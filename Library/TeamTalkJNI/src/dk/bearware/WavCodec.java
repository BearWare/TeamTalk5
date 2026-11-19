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

public class WavCodec
{
    public int nSampleRate;
    public int nChannels;
    public int nSampleFormat;
    public int nTxIntervalMSec;

    public WavCodec() {}
    public WavCodec(boolean set_defaults) {
        if(!set_defaults)
            return;

        nSampleRate = WavConstants.DEFAULT_WAV_SAMPLERATE;
        nChannels = WavConstants.DEFAULT_WAV_CHANNELS;
        nSampleFormat = WavConstants.DEFAULT_WAV_SAMPLEFORMAT;
        nTxIntervalMSec = WavConstants.DEFAULT_WAV_TXINTERVAL;
    }
}
