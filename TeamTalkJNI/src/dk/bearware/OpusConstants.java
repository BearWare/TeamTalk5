/*
 * Copyright (c) 2005-2014, BearWare.dk
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

public class OpusConstants
{
    public static final int OPUS_APPLICATION_VOIP = 2048;
    public static final int OPUS_APPLICATION_AUDIO = 2049;
    public static final int OPUS_MIN_BITRATE = 6000;
    public static final int OPUS_MAX_BITRATE = 510000;
    
    public static final int DEFAULT_OPUS_APPLICATION = OPUS_APPLICATION_VOIP;
    public static final int DEFAULT_OPUS_SAMPLERATE = 48000;
    public static final int DEFAULT_OPUS_CHANNELS = 1;
    public static final int DEFAULT_OPUS_COMPLEXITY = 10;
    public static final boolean DEFAULT_OPUS_FEC = true;
    public static final boolean DEFAULT_OPUS_DTX = false;
    public static final boolean DEFAULT_OPUS_VBR = true;
    public static final boolean DEFAULT_OPUS_VBRCONSTRAINT = false;
    public static final int DEFAULT_OPUS_BITRATE = 32000;
    public static final int DEFAULT_OPUS_DELAY = 20;

}
