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

public class ClientError {

    public static final int CMDERR_SUCCESS                          = 0;

    public static final int CMDERR_SYNTAX_ERROR                     = 1000;
    public static final int CMDERR_UNKNOWN_COMMAND                  = 1001;
    public static final int CMDERR_MISSING_PARAMETER                = 1002;
    public static final int CMDERR_INCOMPATIBLE_PROTOCOLS           = 1003;
    public static final int CMDERR_UNKNOWN_AUDIOCODEC               = 1004;
    public static final int CMDERR_INVALID_USERNAME                 = 1005;

    public static final int CMDERR_INCORRECT_SERVER_PASSWORD        = 2000;
    public static final int CMDERR_INCORRECT_CHANNEL_PASSWORD       = 2001;
    public static final int CMDERR_INVALID_ACCOUNT                  = 2002;
    public static final int CMDERR_MAX_SERVER_USERS_EXCEEDED        = 2003;
    public static final int CMDERR_MAX_CHANNEL_USERS_EXCEEDED       = 2004;
    public static final int CMDERR_SERVER_BANNED                    = 2005;
    public static final int CMDERR_NOT_AUTHORIZED                   = 2006;
    public static final int CMDERR_MAX_DISKUSAGE_EXCEEDED           = 2008;

    public static final int CMDERR_INCORRECT_OP_PASSWORD            = 2010;
    public static final int CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED = 2011;
    public static final int CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED = 2012;
    public static final int CMDERR_MAX_CHANNELS_EXCEEDED            = 2013;
    public static final int CMDERR_COMMAND_FLOOD                    = 2014;
    public static final int CMDERR_CHANNEL_BANNED                   = 2015;

    public static final int CMDERR_NOT_LOGGEDIN                     = 3000;
    public static final int CMDERR_ALREADY_LOGGEDIN                 = 3001;
    public static final int CMDERR_NOT_IN_CHANNEL                   = 3002;
    public static final int CMDERR_ALREADY_IN_CHANNEL               = 3003;
    public static final int CMDERR_CHANNEL_ALREADY_EXISTS           = 3004;
    public static final int CMDERR_CHANNEL_NOT_FOUND                = 3005;
    public static final int CMDERR_USER_NOT_FOUND                   = 3006;
    public static final int CMDERR_BAN_NOT_FOUND                    = 3007;
    public static final int CMDERR_FILETRANSFER_NOT_FOUND           = 3008;
    public static final int CMDERR_OPENFILE_FAILED                  = 3009;
    public static final int CMDERR_ACCOUNT_NOT_FOUND                = 3010;
    public static final int CMDERR_FILE_NOT_FOUND                   = 3011;
    public static final int CMDERR_FILE_ALREADY_EXISTS              = 3012;
    public static final int CMDERR_FILESHARING_DISABLED             = 3013;
    public static final int CMDERR_CHANNEL_HAS_USERS                = 3015;
    public static final int CMDERR_LOGINSERVICE_UNAVAILABLE         = 3016;

    public static final int INTERR_SNDINPUT_FAILURE                 = 10000;
    public static final int INTERR_SNDOUTPUT_FAILURE                = 10001;
    public static final int INTERR_AUDIOCODEC_INIT_FAILED           = 10002;
    public static final int INTERR_SPEEXDSP_INIT_FAILED             = 10003;
    public static final int INTERR_AUDIOPREPROCESSOR_INIT_FAILED    = 10003;
    public static final int INTERR_TTMESSAGE_QUEUE_OVERFLOW         = 10004;
    public static final int INTERR_SNDEFFECT_FAILURE                = 10005;
}
