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

package dk.bearware.backend;

public class TeamTalkConstants {

    public static final int STATUSMODE_AVAILABLE        = 0x00000000;
    public static final int STATUSMODE_AWAY             = 0x00000001; 
    public static final int STATUSMODE_QUESTION         = 0x00000002;
    public static final int STATUSMODE_MODE             = 0x000000FF;

    public static final int STATUSMODE_FLAGS            = 0xFFFFFF00;
    public static final int STATUSMODE_FEMALE           = 0x00000100;
    public static final int STATUSMODE_VIDEOTX          = 0x00000200;
    public static final int STATUSMODE_DESKTOP          = 0x00000400;
    public static final int STATUSMODE_STREAM_MEDIAFILE = 0x00000800;

}
