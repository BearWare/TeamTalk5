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

public interface ChannelType
{
    public static final int CHANNEL_DEFAULT             = 0x0000;
    public static final int CHANNEL_PERMANENT           = 0x0001;
    public static final int CHANNEL_SOLO_TRANSMIT       = 0x0002;
    public static final int CHANNEL_CLASSROOM           = 0x0004;
    public static final int CHANNEL_OPERATOR_RECVONLY   = 0x0008;
    public static final int CHANNEL_NO_VOICEACTIVATION  = 0x0010;
    public static final int CHANNEL_NO_RECORDING        = 0x0020;
}
