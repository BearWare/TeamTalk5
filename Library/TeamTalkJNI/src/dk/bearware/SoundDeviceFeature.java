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

public interface SoundDeviceFeature {

    public static final int SOUNDDEVICEFEATURE_NONE         = 0x0000;
    public static final int SOUNDDEVICEFEATURE_AEC          = 0x0001;
    public static final int SOUNDDEVICEFEATURE_AGC          = 0x0002;
    public static final int SOUNDDEVICEFEATURE_DENOISE      = 0x0004;
}
