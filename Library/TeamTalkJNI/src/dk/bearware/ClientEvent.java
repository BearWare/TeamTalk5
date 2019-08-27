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

public interface ClientEvent
{
    public static final int CLIENTEVENT_NONE = 0;

    public static final int CLIENTEVENT_CON_SUCCESS  = CLIENTEVENT_NONE + 10;
    public static final int CLIENTEVENT_CON_FAILED = CLIENTEVENT_NONE + 20;
    public static final int CLIENTEVENT_CON_LOST = CLIENTEVENT_NONE + 30;
    public static final int CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED = CLIENTEVENT_NONE + 40;

    public static final int CLIENTEVENT_CMD_PROCESSING = CLIENTEVENT_NONE + 200;
    public static final int CLIENTEVENT_CMD_ERROR = CLIENTEVENT_NONE + 210;
    public static final int CLIENTEVENT_CMD_SUCCESS = CLIENTEVENT_NONE + 220;

    public static final int CLIENTEVENT_CMD_MYSELF_LOGGEDIN = CLIENTEVENT_NONE + 230;
    public static final int CLIENTEVENT_CMD_MYSELF_LOGGEDOUT = CLIENTEVENT_NONE + 240;
    public static final int CLIENTEVENT_CMD_MYSELF_KICKED = CLIENTEVENT_NONE + 250;

    public static final int CLIENTEVENT_CMD_USER_LOGGEDIN = CLIENTEVENT_NONE + 260;
    public static final int CLIENTEVENT_CMD_USER_LOGGEDOUT = CLIENTEVENT_NONE + 270;
    public static final int CLIENTEVENT_CMD_USER_UPDATE = CLIENTEVENT_NONE + 280;
    public static final int CLIENTEVENT_CMD_USER_JOINED = CLIENTEVENT_NONE + 290;
    public static final int CLIENTEVENT_CMD_USER_LEFT = CLIENTEVENT_NONE + 300;
    public static final int CLIENTEVENT_CMD_USER_TEXTMSG = CLIENTEVENT_NONE + 310;

    public static final int CLIENTEVENT_CMD_CHANNEL_NEW = CLIENTEVENT_NONE + 320;
    public static final int CLIENTEVENT_CMD_CHANNEL_UPDATE = CLIENTEVENT_NONE + 330;
    public static final int CLIENTEVENT_CMD_CHANNEL_REMOVE = CLIENTEVENT_NONE + 340;

    public static final int CLIENTEVENT_CMD_SERVER_UPDATE = CLIENTEVENT_NONE + 350;
    public static final int CLIENTEVENT_CMD_SERVERSTATISTICS = CLIENTEVENT_NONE + 360;

    public static final int CLIENTEVENT_CMD_FILE_NEW = CLIENTEVENT_NONE + 370;
    public static final int CLIENTEVENT_CMD_FILE_REMOVE = CLIENTEVENT_NONE + 380;

    public static final int CLIENTEVENT_CMD_USERACCOUNT = CLIENTEVENT_NONE + 390;
    public static final int CLIENTEVENT_CMD_BANNEDUSER = CLIENTEVENT_NONE + 400;

    public static final int CLIENTEVENT_USER_STATECHANGE = CLIENTEVENT_NONE + 500;
    public static final int CLIENTEVENT_USER_VIDEOCAPTURE = CLIENTEVENT_NONE + 510;
    public static final int CLIENTEVENT_USER_MEDIAFILE_VIDEO = CLIENTEVENT_NONE + 520;
    public static final int CLIENTEVENT_USER_DESKTOPWINDOW = CLIENTEVENT_NONE + 530;
    public static final int CLIENTEVENT_USER_DESKTOPCURSOR = CLIENTEVENT_NONE + 540;
    public static final int CLIENTEVENT_USER_DESKTOPINPUT = CLIENTEVENT_NONE + 550;
    public static final int CLIENTEVENT_USER_RECORD_MEDIAFILE = CLIENTEVENT_NONE + 560;
    public static final int CLIENTEVENT_USER_AUDIOBLOCK = CLIENTEVENT_NONE + 570;
    
    public static final int CLIENTEVENT_INTERNAL_ERROR = CLIENTEVENT_NONE + 1000;
    public static final int CLIENTEVENT_VOICE_ACTIVATION = CLIENTEVENT_NONE + 1010;

    public static final int CLIENTEVENT_HOTKEY = CLIENTEVENT_NONE + 1020;
    public static final int CLIENTEVENT_HOTKEY_TEST = CLIENTEVENT_NONE + 1030;

    public static final int CLIENTEVENT_FILETRANSFER = CLIENTEVENT_NONE + 1040;

    public static final int CLIENTEVENT_DESKTOPWINDOW_TRANSFER = CLIENTEVENT_NONE + 1050;

    public static final int CLIENTEVENT_STREAM_MEDIAFILE = CLIENTEVENT_NONE + 1060;

    public static final int CLIENTEVENT_LOCAL_MEDIAFILE = CLIENTEVENT_NONE + 1070;
}
