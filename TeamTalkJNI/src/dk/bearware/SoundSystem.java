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

public interface SoundSystem
{
    public static final int SOUNDSYSTEM_NONE = 0;
    public static final int SOUNDSYSTEM_WINMM = 1;
    public static final int SOUNDSYSTEM_DSOUND = 2;
    public static final int SOUNDSYSTEM_ALSA = 3;
    public static final int SOUNDSYSTEM_COREAUDIO = 4;
    public static final int SOUNDSYSTEM_WASAPI = 5;
    public static final int SOUNDSYSTEM_OPENSLES = 7;
}
