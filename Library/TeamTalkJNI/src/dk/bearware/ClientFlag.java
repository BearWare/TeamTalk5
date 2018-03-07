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

public interface ClientFlag
{
    public static final int CLIENT_CLOSED                    = 0x00000000;

    public static final int CLIENT_SNDINPUT_READY            = 0x00000001;
    public static final int CLIENT_SNDOUTPUT_READY           = 0x00000002;
    public static final int CLIENT_SNDINOUTPUT_DUPLEX        = 0x00000004;
    public static final int CLIENT_SNDINPUT_VOICEACTIVATED   = 0x00000008;
    public static final int CLIENT_SNDINPUT_VOICEACTIVE      = 0x00000010;
    public static final int CLIENT_SNDOUTPUT_MUTE            = 0x00000020;
    public static final int CLIENT_SNDOUTPUT_AUTO3DPOSITION  = 0x00000040;

    public static final int CLIENT_VIDEOCAPTURE_READY        = 0x00000080;

    public static final int CLIENT_TX_VOICE                  = 0x00000100;
    public static final int CLIENT_TX_VIDEOCAPTURE           = 0x00000200;
    public static final int CLIENT_TX_DESKTOP                = 0x00000400;

    public static final int CLIENT_DESKTOP_ACTIVE            = 0x00000800;
    public static final int CLIENT_MUX_AUDIOFILE             = 0x00001000;

    public static final int CLIENT_CONNECTING                = 0x00002000;
    public static final int CLIENT_CONNECTED                 = 0x00004000;
    public static final int CLIENT_CONNECTION                = CLIENT_CONNECTING | CLIENT_CONNECTED;

    public static final int CLIENT_AUTHORIZED                = 0x00008000;

    public static final int CLIENT_STREAM_AUDIO              = 0x00010000;
    public static final int CLIENT_STREAM_VIDEO              = 0x00020000;
}

