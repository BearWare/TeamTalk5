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

public interface AudioFileFormat
{
    public static final int AFF_NONE                 = 0;
    public static final int AFF_CHANNELCODEC_FORMAT  = 1;
    public static final int AFF_WAVE_FORMAT          = 2;
    public static final int AFF_MP3_16KBIT_FORMAT    = 3;
    public static final int AFF_MP3_32KBIT_FORMAT    = 4;
    public static final int AFF_MP3_64KBIT_FORMAT    = 5;
    public static final int AFF_MP3_128KBIT_FORMAT   = 6;
    public static final int AFF_MP3_256KBIT_FORMAT   = 7;
}
