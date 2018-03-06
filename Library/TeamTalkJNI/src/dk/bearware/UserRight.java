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

public interface UserRight
{
    public static final int USERRIGHT_NONE                        = 0x00000000; 
    public static final int USERRIGHT_MULTI_LOGIN                 = 0x00000001; 
    public static final int USERRIGHT_VIEW_ALL_USERS              = 0x00000002;
    public static final int USERRIGHT_CREATE_TEMPORARY_CHANNEL    = 0x00000004;
    public static final int USERRIGHT_MODIFY_CHANNELS             = 0x00000008;
    public static final int USERRIGHT_TEXTMESSAGE_BROADCAST       = 0x00000010; 
    public static final int USERRIGHT_KICK_USERS                  = 0x00000020; 
    public static final int USERRIGHT_BAN_USERS                   = 0x00000040; 
    public static final int USERRIGHT_MOVE_USERS                  = 0x00000080; 
    public static final int USERRIGHT_OPERATOR_ENABLE             = 0x00000100;
    public static final int USERRIGHT_UPLOAD_FILES                = 0x00000200;
    public static final int USERRIGHT_DOWNLOAD_FILES              = 0x00000400;
    public static final int USERRIGHT_UPDATE_SERVERPROPERTIES     = 0x00000800;
    public static final int USERRIGHT_TRANSMIT_VOICE              = 0x00001000;
    public static final int USERRIGHT_TRANSMIT_VIDEOCAPTURE       = 0x00002000;
    public static final int USERRIGHT_TRANSMIT_DESKTOP            = 0x00004000;
    public static final int USERRIGHT_TRANSMIT_DESKTOPINPUT       = 0x00008000;
    public static final int USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO    = 0x00010000;
    public static final int USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO    = 0x00020000;
    public static final int USERRIGHT_LOCKED_NICKNAME             = 0x00040000;
    public static final int USERRIGHT_LOCKED_STATUS               = 0x00080000;

    public static final int USERRIGHT_ALL                         = 0xFFFFFFFF;
    
}
