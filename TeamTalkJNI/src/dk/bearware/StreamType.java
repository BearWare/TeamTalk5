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

public interface StreamType {
    public static int STREAMTYPE_NONE                     = 0x0000;
    public static int STREAMTYPE_VOICE                    = 0x0001;
    public static int STREAMTYPE_VIDEOCAPTURE             = 0x0002;
    public static int STREAMTYPE_MEDIAFILE_AUDIO          = 0x0004;
    public static int STREAMTYPE_MEDIAFILE_VIDEO          = 0x0008;
    public static int STREAMTYPE_DESKTOP                  = 0x0010;
    public static int STREAMTYPE_DESKTOPINPUT             = 0x0020;
}