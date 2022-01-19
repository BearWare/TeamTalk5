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
    public static final int STREAMTYPE_NONE                     = 0x0000;
    public static final int STREAMTYPE_VOICE                    = 0x0001;
    public static final int STREAMTYPE_VIDEOCAPTURE             = 0x0002;
    public static final int STREAMTYPE_MEDIAFILE_AUDIO          = 0x0004;
    public static final int STREAMTYPE_MEDIAFILE_VIDEO          = 0x0008;
    public static final int STREAMTYPE_DESKTOP                  = 0x0010;
    public static final int STREAMTYPE_DESKTOPINPUT             = 0x0020;
    public static final int STREAMTYPE_MEDIAFILE                = STREAMTYPE_MEDIAFILE_AUDIO | STREAMTYPE_MEDIAFILE_VIDEO;
    public static final int STREAMTYPE_CHANNELMSG               = 0x0040;
    public static final int STREAMTYPE_LOCALMEDIAPLAYBACK_AUDIO = 0x0080;
    public static final int STREAMTYPE_CLASSROOM_ALL            = STREAMTYPE_VOICE |
                                                                  STREAMTYPE_VIDEOCAPTURE |
                                                                  STREAMTYPE_DESKTOP |
                                                                  STREAMTYPE_MEDIAFILE |
                                                                  STREAMTYPE_CHANNELMSG;

}
