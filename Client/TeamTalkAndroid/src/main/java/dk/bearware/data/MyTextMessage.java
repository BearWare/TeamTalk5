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

import java.nio.charset.StandardCharsets;
import java.util.Calendar;
import java.util.Date;
import java.util.Vector;

import dk.bearware.Constants;
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

    public MyTextMessage(MyTextMessage msg) {
        super(msg);
        szNickName = msg.szNickName;
        userData = msg.userData;
        time = msg.time;
    }

    public Vector<MyTextMessage> split() {
        MyTextMessage newmsg = new MyTextMessage(this);
        Vector<MyTextMessage> result = new Vector<>();
        if (szMessage.getBytes(StandardCharsets.UTF_8).length <= Constants.TT_STRLEN - 1) {
            newmsg.bMore = false;
            result.add(newmsg);
            return result;
        }

        newmsg = new MyTextMessage(newmsg);
        newmsg.bMore = true;

        String remain = szMessage;
        int curlen = remain.length();
        while (remain.substring(0, curlen).getBytes(StandardCharsets.UTF_8).length > Constants.TT_STRLEN - 1)
            curlen /= 2;

        int half = Constants.TT_STRLEN / 2;
        while (half > 0)
        {
            int utf8strlen = remain.substring(0, Math.min(curlen + half, remain.length())).getBytes(StandardCharsets.UTF_8).length;
            if (utf8strlen <= Constants.TT_STRLEN - 1)
                curlen += half;
            if (utf8strlen == Constants.TT_STRLEN - 1)
                break;
            half /= 2;
        }

        newmsg.szMessage = remain.substring(0, curlen);
        result.add(newmsg);

        newmsg = new MyTextMessage(newmsg);
        newmsg.szMessage = remain.substring(curlen);
        result.addAll(newmsg.split());
        return result;
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
