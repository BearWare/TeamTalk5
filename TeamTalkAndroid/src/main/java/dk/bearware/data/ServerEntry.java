/*
 * Copyright (c) 2005-2017, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

package dk.bearware.data;

public class ServerEntry {

    //key names from 'serverentry_preferences'
    public static final String KEY_SERVERNAME = "servername",
                               KEY_IPADDR = "ipaddr",
                               KEY_TCPPORT = "tcpport",
                               KEY_UDPPORT = "udpport",
                               KEY_USERNAME = "username",
                               KEY_PASSWORD = "password",
                               KEY_NICKNAME = "nickname",
                               KEY_CHANNEL = "channel",
                               KEY_CHANPASSWD = "chanpasswd",
                               KEY_REMEMBER_LAST_CHANNEL = "remember_last_channel",
                               KEY_ENCRYPTED = "encrypted";
    
    public String servername = "";
    public String ipaddr = "";
    public int tcpport = 0, udpport = 0;
    public String username = "", password = "";
    public String nickname = "";
    public String channel = "", chanpasswd = "";
    public boolean rememberLastChannel = true;
    public boolean encrypted = false;
    public boolean public_server = false;
//    
//    public void putBundle(Bundle bundle) {
//        bundle.putString(KEY_SERVERNAME, servername);
//        bundle.putString(KEY_IPADDR, ipaddr);
//        bundle.putInt(KEY_TCPPORT, tcpport);
//        bundle.putInt(KEY_UDPPORT, udpport);
//        bundle.putString(KEY_USERNAME, username);
//        bundle.putString(KEY_PASSWORD, password);
//        bundle.putString(KEY_CHANNEL, channel);
//        bundle.putString(KEY_CHANPASSWD, chanpasswd);
//    }
//    
//    public static ServerEntry fromBundle(Bundle bundle) {
//        if(bundle.getString(KEY_IPADDR, "").isEmpty())
//            return null;
//        
//        ServerEntry entry = new ServerEntry();
//        entry.servername = bundle.getString(KEY_SERVERNAME, "");
//        entry.ipaddr = bundle.getString(KEY_IPADDR, "");
//        entry.tcpport = bundle.getInt(KEY_TCPPORT, 10333);
//        entry.udpport = bundle.getInt(KEY_UDPPORT, 10333);
//        entry.username = bundle.getString(KEY_USERNAME, ""); 
//        entry.password = bundle.getString(KEY_PASSWORD, "");
//        entry.channel = bundle.getString(KEY_CHANNEL, "/");
//        entry.chanpasswd = bundle.getString(KEY_CHANPASSWD, "");
//        return entry;
//    }
}
