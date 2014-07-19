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
    public int nMSecPerPacket;
}
