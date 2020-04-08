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

public interface SoundDeviceConstants {

    public static final int TT_SOUNDDEVICE_ID_SHARED_FLAG           = 0x00000800;
    public static final int TT_SOUNDDEVICE_ID_MASK                  = 0x000007FF;
    
    public static final int TT_SOUNDDEVICE_ID_REMOTEIO              = 0;
    public static final int TT_SOUNDDEVICE_ID_VOICEPREPROCESSINGIO  = 1;
    public static final int TT_SOUNDDEVICE_ID_OPENSLES_DEFAULT      = 0;
    public static final int TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL      = 1978;
}
