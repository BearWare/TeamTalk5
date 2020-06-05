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

public interface SoundSystem
{
    public static final int SOUNDSYSTEM_NONE = 0;
    public static final int SOUNDSYSTEM_WINMM = 1;
    public static final int SOUNDSYSTEM_DSOUND = 2;
    public static final int SOUNDSYSTEM_ALSA = 3;
    public static final int SOUNDSYSTEM_COREAUDIO = 4;
    public static final int SOUNDSYSTEM_WASAPI = 5;
    public static final int SOUNDSYSTEM_OPENSLES = 7;
    public static final int SOUNDSYSTEM_OPENSLES_ANDROID = SOUNDSYSTEM_OPENSLES;
    public static final int SOUNDSYSTEM_AUDIOUNIT = 8;
    public static final int SOUNDSYSTEM_AUDIOUNIT_IOS = SOUNDSYSTEM_AUDIOUNIT;
}
