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

public interface UserState {
    public static final int USERSTATE_NONE                  = 0x00000000;
    public static final int USERSTATE_VOICE                 = 0x00000001;
    public static final int USERSTATE_MUTE_VOICE            = 0x00000002;
    public static final int USERSTATE_MUTE_MEDIAFILE        = 0x00000004;
    public static final int USERSTATE_DESKTOP               = 0x00000008;
    public static final int USERSTATE_VIDEOCAPTURE          = 0x00000010;
    public static final int USERSTATE_MEDIAFILE_AUDIO       = 0x00000020;
    public static final int USERSTATE_MEDIAFILE_VIDEO       = 0x00000040;
    public static final int USERSTATE_MEDIAFILE             = USERSTATE_MEDIAFILE_AUDIO |
        USERSTATE_MEDIAFILE_AUDIO;

}
