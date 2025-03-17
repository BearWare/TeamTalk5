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

import java.util.Vector;

import dk.bearware.ServerProperties;
import dk.bearware.TextMsgType;
import dk.bearware.gui.AccessibilityAssistant;
import dk.bearware.gui.R;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

public class TextMessageAdapter extends BaseAdapter {

    private Vector<MyTextMessage> messages, // reference to TeamTalkService's messages (modified by other thread)
            messagesUpdateView; // copy of 'this.message' after update
    
    private final LayoutInflater inflater;
    private final AccessibilityAssistant accessibilityAssistant;
    
    private int myuserid;
    
    private boolean show_logs = true;
    
    int def_bg_color, def_text_color;

    // color for text messages from other users
    int user_bg_color = 0xff4c9fff, user_text_color = Color.WHITE;
    // color for text message from oneself
    int self_bg_color = 0xff659f5d, self_text_color = Color.WHITE;
    // color for log-message type MSGTYPE_LOG_INFO
    int loginfo_bg_color, loginfo_text_color; //uses default color scheme
    // color for log-message type MSGTYPE_LOG_ERROR
    int logerr_bg_color = 0xffcd0028, logerr_text_color = Color.WHITE;
    // color for log-message type MSGTYPE_SERVERPROP
    int srvinfo_bg_color = Color.DKGRAY, srvinfo_text_color = Color.WHITE;
    
    public TextMessageAdapter(Context context, AccessibilityAssistant accessibilityAssistant,
                              Vector<MyTextMessage> msgs, int myuserid) {
        this(context, accessibilityAssistant);
        setMyUserID(myuserid);
        
        setTextMessages(msgs);
    }

    @SuppressWarnings("ResourceType")
    public TextMessageAdapter(Context context, AccessibilityAssistant accessibilityAssistant) {
        inflater = LayoutInflater.from(context);
        this.accessibilityAssistant = accessibilityAssistant;
        setTextMessages(new Vector<>());
        
        TypedArray array = context.getTheme().obtainStyledAttributes(new int[] {
            android.R.attr.colorBackground, 
            android.R.attr.textColorPrimary, 
        });
        def_bg_color = array.getColor(0, 0xFF00FF);
        def_text_color = array.getColor(1, 0xFF00FF);

        array.recycle();

        loginfo_bg_color = def_bg_color;
        loginfo_text_color = def_text_color;
    }
    
    public void setTextMessages(Vector<MyTextMessage> msgs) {
        this.messages = msgs;
        copyToMessagesView();
    }

    private void copyToMessagesView() {
        this.messagesUpdateView = (Vector<MyTextMessage>)this.messages.clone();
    }
    
    Vector<MyTextMessage> getMessages() {
        if(show_logs)
            return this.messagesUpdateView;
        
        Vector<MyTextMessage> result = new Vector<>();
        for(MyTextMessage m : this.messagesUpdateView) {
            switch(m.nMsgType) {
                case MyTextMessage.MSGTYPE_LOG_ERROR :
                case MyTextMessage.MSGTYPE_LOG_INFO :
                    break;
                default :
                    result.add(m);
                    break;
            }
        }
        return result;
    }
    
    public void setMyUserID(int userid) {
        myuserid = userid;
    }
    
    public void showLogMessages(boolean enable) {
        show_logs = enable;
    }
    
    @Override
    public int getCount() {
        return getMessages().size();
    }

    @Override
    public Object getItem(int position) {
        return getMessages().get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }
    
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        MyTextMessage txtmsg = getMessages().get(position);
        
        int bg_color = Color.BLACK, text_color = Color.WHITE;
        
        switch(txtmsg.nMsgType) {
            case TextMsgType.MSGTYPE_CHANNEL :
            case TextMsgType.MSGTYPE_BROADCAST :
            case TextMsgType.MSGTYPE_USER : {
                if(convertView == null ||
                   convertView.findViewById(R.id.item_textmsg) == null)
                    convertView = inflater.inflate(R.layout.item_textmsg, parent, false);
                
                if(txtmsg.nFromUserID == myuserid) {
                    bg_color = self_bg_color;
                    text_color = self_text_color;
                }
                else {
                    bg_color = user_bg_color;
                    text_color = user_text_color;
                }
                
                TextView name = (TextView) convertView.findViewById(R.id.name_text);
                TextView msgtext = (TextView) convertView.findViewById(R.id.msg_text);
                TextView msgdate = (TextView) convertView.findViewById(R.id.time_text);

                name.setText(txtmsg.szNickName);
                msgdate.setText(txtmsg.time.toString());
                msgtext.setText(txtmsg.szMessage);
                
                name.setTextColor(text_color);
                msgdate.setTextColor(text_color);
                msgtext.setTextColor(text_color);
                break;
            }
            case MyTextMessage.MSGTYPE_SERVERPROP : {
                if(convertView == null ||
                   convertView.findViewById(R.id.item_textmsg_srvinfo) == null) {
                    convertView = inflater.inflate(R.layout.item_textmsg_srvinfo, parent, false);
                }

                bg_color = srvinfo_bg_color;
                text_color = srvinfo_text_color;

                TextView logmsg = (TextView) convertView.findViewById(R.id.srvname_text);
                TextView logmotd = (TextView) convertView.findViewById(R.id.srvmotd_text);
                TextView logtm = (TextView) convertView.findViewById(R.id.logtime_text);
                
                ServerProperties p = (ServerProperties)txtmsg.userData;
                logmsg.setText(p.szServerName);
                logmotd.setText(p.szMOTD);
                logtm.setText(txtmsg.time.toString());
                
                logmsg.setTextColor(text_color);
                logtm.setTextColor(text_color);
                break;
            }
            case MyTextMessage.MSGTYPE_LOG_ERROR :
            case MyTextMessage.MSGTYPE_LOG_INFO :
            default : {
                if(convertView == null ||
                   convertView.findViewById(R.id.item_textmsg_logmsg) == null) {
                    convertView = inflater.inflate(R.layout.item_textmsg_logmsg, parent, false);
                }

                switch(txtmsg.nMsgType) {
                    case MyTextMessage.MSGTYPE_LOG_ERROR :
                        bg_color = logerr_bg_color;
                        text_color = logerr_text_color;
                        break;
                    case MyTextMessage.MSGTYPE_LOG_INFO : 
                        bg_color = loginfo_bg_color;
                        text_color = loginfo_text_color;
                        break;
                }

                TextView logmsg = (TextView) convertView.findViewById(R.id.logmsg_text);
                TextView logtm = (TextView) convertView.findViewById(R.id.logtime_text);
                
                logmsg.setText(txtmsg.szMessage);
                logtm.setText(txtmsg.time.toString());
                
                logmsg.setTextColor(text_color);
                logtm.setTextColor(text_color);
                break;
            }
        }
        
        convertView.setBackgroundColor(bg_color);
        convertView.setAccessibilityDelegate(accessibilityAssistant);
        
        return convertView;
    }

    @Override
    public void notifyDataSetChanged() {
        copyToMessagesView();
        super.notifyDataSetChanged();
    }
}
