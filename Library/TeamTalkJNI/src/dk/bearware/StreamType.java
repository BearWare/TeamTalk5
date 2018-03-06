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

public interface StreamType {
    public static int STREAMTYPE_NONE                     = 0x0000;
    public static int STREAMTYPE_VOICE                    = 0x0001;
    public static int STREAMTYPE_VIDEOCAPTURE             = 0x0002;
    public static int STREAMTYPE_MEDIAFILE_AUDIO          = 0x0004;
    public static int STREAMTYPE_MEDIAFILE_VIDEO          = 0x0008;
    public static int STREAMTYPE_DESKTOP                  = 0x0010;
    public static int STREAMTYPE_DESKTOPINPUT             = 0x0020;
}
