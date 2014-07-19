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

public class SpeexCodec
{
    public static final int SPEEX_NB_MIN_BITRATE = 2150;
    public static final int SPEEX_NB_MAX_BITRATE = 24600;
    public static final int SPEEX_WB_MIN_BITRATE = 3950;
    public static final int SPEEX_WB_MAX_BITRATE = 42200;
    public static final int SPEEX_UWB_MIN_BITRATE = 4150;
    public static final int SPEEX_UWB_MAX_BITRATE = 44000;
    
    public int nBandmode;
    public int nQuality;
    public int nMSecPerPacket;
    public boolean bStereoPlayback;
}