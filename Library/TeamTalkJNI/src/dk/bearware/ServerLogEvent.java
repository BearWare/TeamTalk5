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

public interface ServerLogEvent {

    public static final int SERVERLOGEVENT_NONE                        = 0x00000000;
    public static final int SERVERLOGEVENT_USER_CONNECTED              = 0x00000001;
    public static final int SERVERLOGEVENT_USER_DISCONNECTED           = 0x00000002;
    public static final int SERVERLOGEVENT_USER_LOGGEDIN               = 0x00000004;
    public static final int SERVERLOGEVENT_USER_LOGGEDOUT              = 0x00000008;
    public static final int SERVERLOGEVENT_USER_LOGINFAILED            = 0x00000010;
    public static final int SERVERLOGEVENT_USER_TIMEDOUT               = 0x00000020;
    public static final int SERVERLOGEVENT_USER_KICKED                 = 0x00000040;
    public static final int SERVERLOGEVENT_USER_BANNED                 = 0x00000080;
    public static final int SERVERLOGEVENT_USER_UNBANNED               = 0x00000100;
    public static final int SERVERLOGEVENT_USER_UPDATED                = 0x00000200;
    public static final int SERVERLOGEVENT_USER_JOINEDCHANNEL          = 0x00000400;
    public static final int SERVERLOGEVENT_USER_LEFTCHANNEL            = 0x00000800;
    public static final int SERVERLOGEVENT_USER_MOVED                  = 0x00001000;
    public static final int SERVERLOGEVENT_USER_TEXTMESSAGE_PRIVATE    = 0x00002000;
    public static final int SERVERLOGEVENT_USER_TEXTMESSAGE_CUSTOM     = 0x00004000;
    public static final int SERVERLOGEVENT_USER_TEXTMESSAGE_CHANNEL    = 0x00008000;
    public static final int SERVERLOGEVENT_USER_TEXTMESSAGE_BROADCAST  = 0x00010000;
    public static final int SERVERLOGEVENT_CHANNEL_CREATED             = 0x00020000;
    public static final int SERVERLOGEVENT_CHANNEL_UPDATED             = 0x00040000;
    public static final int SERVERLOGEVENT_CHANNEL_REMOVED             = 0x00080000;
    public static final int SERVERLOGEVENT_FILE_UPLOADED               = 0x00100000;
    public static final int SERVERLOGEVENT_FILE_DOWNLOADED             = 0x00200000;
    public static final int SERVERLOGEVENT_FILE_DELETED                = 0x00400000;
    public static final int SERVERLOGEVENT_SERVER_UPDATED              = 0x00800000;
    public static final int SERVERLOGEVENT_SERVER_SAVECONFIG           = 0x01000000;

    public static final int SERVERLOGEVENT_DEFAULT                     = SERVERLOGEVENT_USER_CONNECTED |
                                                                         SERVERLOGEVENT_USER_DISCONNECTED |
                                                                         SERVERLOGEVENT_USER_LOGGEDIN |
                                                                         SERVERLOGEVENT_USER_LOGGEDOUT |
                                                                         SERVERLOGEVENT_USER_LOGINFAILED |
                                                                         SERVERLOGEVENT_USER_TIMEDOUT |
                                                                         SERVERLOGEVENT_USER_KICKED |
                                                                         SERVERLOGEVENT_USER_BANNED |
                                                                         SERVERLOGEVENT_USER_UNBANNED |
                                                                         SERVERLOGEVENT_USER_UPDATED |
                                                                         SERVERLOGEVENT_USER_JOINEDCHANNEL |
                                                                         SERVERLOGEVENT_USER_LEFTCHANNEL |
                                                                         SERVERLOGEVENT_USER_MOVED |
                                                                         SERVERLOGEVENT_USER_TEXTMESSAGE_PRIVATE |
                                                                         SERVERLOGEVENT_USER_TEXTMESSAGE_CUSTOM |
                                                                         SERVERLOGEVENT_USER_TEXTMESSAGE_CHANNEL |
                                                                         SERVERLOGEVENT_USER_TEXTMESSAGE_BROADCAST |
                                                                         SERVERLOGEVENT_CHANNEL_CREATED |
                                                                         SERVERLOGEVENT_CHANNEL_UPDATED |
                                                                         SERVERLOGEVENT_CHANNEL_REMOVED |
                                                                         SERVERLOGEVENT_FILE_UPLOADED |
                                                                         SERVERLOGEVENT_FILE_DOWNLOADED |
                                                                         SERVERLOGEVENT_FILE_DELETED |
                                                                         SERVERLOGEVENT_SERVER_UPDATED |
                                                                         SERVERLOGEVENT_SERVER_SAVECONFIG;
}
