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

package dk.bearware.data;

import java.util.Calendar;
import java.util.Date;

import dk.bearware.TextMessage;

public class MyTextMessage extends TextMessage {

    public String szNickName = "";
    public Object userData;
    
    public Date time = Calendar.getInstance().getTime();
    
    public MyTextMessage(TextMessage msg, String name) {
        this.nChannelID = msg.nChannelID;
        this.nFromUserID = msg.nFromUserID;
        this.nMsgType = msg.nMsgType;
        this.nToUserID = msg.nToUserID;
        this.szFromUsername = msg.szFromUsername;
        this.szMessage = msg.szMessage;
        this.szNickName = name;
    }

    public MyTextMessage(String name) {
        this.szNickName = name;
    }
    
    public MyTextMessage() {
    }
    
    public static final int MSGTYPE_LOG_INFO    = 0x80000000;
    public static final int MSGTYPE_LOG_ERROR   = 0x40000000;
    public static final int MSGTYPE_SERVERPROP  = 0x20000000;
    
    public static MyTextMessage createLogMsg(int nMsgType, String szMessage) {
        MyTextMessage newmsg = new MyTextMessage();
        newmsg.nMsgType = nMsgType;
        newmsg.szMessage = szMessage;
        return newmsg;
    }

    public static MyTextMessage createUserDefMsg(int nMsgType, Object userData) {
        MyTextMessage newmsg = new MyTextMessage();
        newmsg.nMsgType = nMsgType;
        newmsg.userData = userData;
        return newmsg;
    }
}
