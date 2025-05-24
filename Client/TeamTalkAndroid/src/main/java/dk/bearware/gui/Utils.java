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

package dk.bearware.gui;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.util.Log;
import android.util.Xml;
import android.widget.Toast;

import androidx.preference.EditTextPreference;
import androidx.preference.Preference;

import com.google.gson.Gson;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xmlpull.v1.XmlSerializer;

import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Vector;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import dk.bearware.AudioCodec;
import dk.bearware.Channel;
import dk.bearware.ChannelType;
import dk.bearware.ClientError;
import dk.bearware.ClientErrorMsg;
import dk.bearware.FileTransfer;
import dk.bearware.RemoteFile;
import dk.bearware.SoundLevel;
import dk.bearware.StreamType;
import dk.bearware.Subscription;
import dk.bearware.TeamTalkBase;
import dk.bearware.User;
import dk.bearware.data.AppInfo;
import dk.bearware.data.Preferences;
import dk.bearware.data.ServerEntry;

public class Utils {

    public static final String TAG = "bearware";

    private static final Map<Integer, Integer> errorMessages = new HashMap<>();

    static {
        errorMessages.put(ClientError.CMDERR_INVALID_USERNAME, R.string.err_invalid_username);
        errorMessages.put(ClientError.CMDERR_INCORRECT_SERVER_PASSWORD, R.string.err_incorrect_server_password);
        errorMessages.put(ClientError.CMDERR_INCORRECT_CHANNEL_PASSWORD, R.string.err_incorrect_channel_password);
        errorMessages.put(ClientError.CMDERR_INVALID_ACCOUNT, R.string.err_invalid_account);
        errorMessages.put(ClientError.CMDERR_MAX_SERVER_USERS_EXCEEDED, R.string.err_max_server_users_exceeded);
        errorMessages.put(ClientError.CMDERR_MAX_CHANNEL_USERS_EXCEEDED, R.string.err_max_channel_users_exceeded);
        errorMessages.put(ClientError.CMDERR_SERVER_BANNED, R.string.err_server_banned);
        errorMessages.put(ClientError.CMDERR_NOT_AUTHORIZED, R.string.err_not_authorized);
        errorMessages.put(ClientError.CMDERR_MAX_DISKUSAGE_EXCEEDED, R.string.err_max_diskusage_exceeded);
        errorMessages.put(ClientError.CMDERR_INCORRECT_OP_PASSWORD, R.string.err_incorrect_op_password);
        errorMessages.put(ClientError.CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED, R.string.err_max_logins_per_ipaddress_exceeded);
        errorMessages.put(ClientError.CMDERR_MAX_CHANNELS_EXCEEDED, R.string.err_max_channels_exceeded);
        errorMessages.put(ClientError.CMDERR_CHANNEL_ALREADY_EXISTS, R.string.err_channel_already_exists);
        errorMessages.put(ClientError.CMDERR_USER_NOT_FOUND, R.string.err_user_not_found);
        errorMessages.put(ClientError.CMDERR_OPENFILE_FAILED, R.string.err_openfile_failed);
        errorMessages.put(ClientError.CMDERR_FILESHARING_DISABLED, R.string.err_filesharing_disabled);
        errorMessages.put(ClientError.CMDERR_CHANNEL_HAS_USERS, R.string.err_channel_has_users);
    }

    public static void notifyError(Context context, ClientErrorMsg err) {
        if (errorMessages.containsKey(err.nErrorNo)) {
            Toast.makeText(context, errorMessages.get(err.nErrorNo), Toast.LENGTH_LONG).show();
        }
        else {
            Toast.makeText(context, err.szErrorMsg, Toast.LENGTH_LONG).show();
        }
    }

    public static void setEditTextPreference(Preference preference, String text, String summary) {
        setEditTextPreference(preference, text, summary, false);
    }

    public static void setEditTextPreference(Preference preference, String text, String summary, boolean forcesummary) {
        EditTextPreference textpref = (EditTextPreference) preference;
        textpref.setText(text);
        if (!summary.isEmpty() || forcesummary)
            textpref.setSummary(summary);
    }

    public static String getEditTextPreference(Preference preference, String def_value) {
        EditTextPreference textpref = (EditTextPreference) preference;
        String s = textpref.getText(); 
        if(s == null)
            return def_value;
        return s;
    }
    
    public static Intent putServerEntry(Intent intent, ServerEntry entry) {
        return intent.putExtra(ServerEntry.class.getName(), new Gson().toJson(entry));
    }
    
    public static ServerEntry getServerEntry(Intent intent) {
        if (intent.hasExtra(ServerEntry.class.getName())) {
            return new Gson().fromJson(intent.getExtras().getString(ServerEntry.class.getName()), ServerEntry.class);
        }
        return null;
    }
    
    public static Intent putAudioCodec(Intent intent, AudioCodec entry) {
        return intent.putExtra(AudioCodec.class.getName(), new Gson().toJson(entry));
    }
    
    public static AudioCodec getAudioCodec(Intent intent) {
        if (intent.hasExtra(AudioCodec.class.getName())) {
            return new Gson().fromJson(intent.getExtras().getString(AudioCodec.class.getName()), AudioCodec.class);
        }
        return null;
    }

    public static Intent putChannel(Intent intent, Channel entry) {
        return intent.putExtra(Channel.class.getName(), new Gson().toJson(entry));
    }

    public static Channel getChannel(Intent intent) {
        if (intent.hasExtra(Channel.class.getName())) {
            return new Gson().fromJson(intent.getExtras().getString(Channel.class.getName()), Channel.class);
        }
        return null;
    }

    public static Vector<Channel> getSubChannels(int chanid, Map<Integer, Channel> channels) {
        Vector<Channel> result = new Vector<>();

        for (Entry<Integer, Channel> integerChannelEntry : channels.entrySet()) {
            Channel chan = integerChannelEntry.getValue();
            if ((chan.nParentID == chanid) && (chan.nMaxUsers > 0))
                result.add(chan);
        }
        return result;
    }

    public static Vector<Channel> getStickyChannels(int chanid, Map<Integer, Channel> channels) {
        Vector<Channel> result = new Vector<>();

        for (Entry<Integer, Channel> integerChannelEntry : channels.entrySet()) {
            Channel chan = integerChannelEntry.getValue();
            if ((chan.nParentID == chanid) && (chan.nMaxUsers <= 0))
                result.add(chan);
        }
        return result;
    }

    public static Vector<User> getUsers(int chanid, Map<Integer, User> users) {
        Vector<User> result = new Vector<>();

        for (Entry<Integer, User> integerUserEntry : users.entrySet()) {
            User user = integerUserEntry.getValue();
            if (user.nChannelID == chanid)
                result.add(user);
        }
        return result;
    }
    
    public static Vector<User> getUsers(Map<Integer, User> users) {
        Vector<User> result = new Vector<>();

        for (Entry<Integer, User> integerUserEntry : users.entrySet()) {
            result.add(integerUserEntry.getValue());
        }
        return result;
    }
    
    public static Vector<RemoteFile> getRemoteFiles(int chanid, Map<Integer, RemoteFile> remotefiles) {
        Vector<RemoteFile> result = new Vector<>();

        for (Entry<Integer, RemoteFile> integerRemoteFileEntry : remotefiles.entrySet()) {
            RemoteFile remotefile = integerRemoteFileEntry.getValue();
            if (remotefile.nChannelID == chanid)
                result.add(remotefile);
        }
        return result;
    }
    
    public static Vector<RemoteFile> getRemoteFiles(Map<Integer, RemoteFile> remotefiles) {
        Vector<RemoteFile> result = new Vector<>();

        for (Entry<Integer, RemoteFile> integerRemoteFileEntry : remotefiles.entrySet()) {
            result.add(integerRemoteFileEntry.getValue());
        }
        return result;
    }
    
    public static Vector<FileTransfer> getFileTransfers(int chanid, Map<Integer, FileTransfer> filetransfers) {
        Vector<FileTransfer> result = new Vector<>();

        for (Entry<Integer, FileTransfer> integerFileTransferEntry : filetransfers.entrySet()) {
            FileTransfer transfer = integerFileTransferEntry.getValue();
            if (transfer.nChannelID == chanid)
                result.add(transfer);
        }
        return result;
    }
    
    public static Vector<FileTransfer> getFileTransfers(Map<Integer, FileTransfer> filetransfers) {
        Vector<FileTransfer> result = new Vector<>();

        for (Entry<Integer, FileTransfer> integerFileTransferEntry : filetransfers.entrySet()) {
            result.add(integerFileTransferEntry.getValue());
        }
        return result;
    }

    public static int toggleSubscription(TeamTalkBase ttclient, User user, int streamtype, boolean on) {
        if (on) {
            return ttclient.doSubscribe(user.nUserID, streamtype);
        } else {
            return ttclient.doUnsubscribe(user.nUserID, streamtype);
        }
    }

    public static boolean isTransmitAllowed(User user, Channel chan, int streamtype) {
        for (int i = 0; i < chan.transmitUsers.length; i++) {
            if (chan.transmitUsers[i][0] == user.nUserID && (chan.transmitUsers[i][1] & streamtype) == streamtype) {
                return (chan.uChannelType & ChannelType.CHANNEL_CLASSROOM) == ChannelType.CHANNEL_CLASSROOM;
            }
        }
        return (chan.uChannelType & ChannelType.CHANNEL_CLASSROOM) == ChannelType.CHANNEL_DEFAULT;
    }

    public static void toggleTransmitUsers(User user, Channel chan, int streamtype, boolean allow) {

        if ((chan.uChannelType & ChannelType.CHANNEL_CLASSROOM) == ChannelType.CHANNEL_DEFAULT)
            allow = !allow;

        if (allow) {
            for (int i = 0; i < chan.transmitUsers.length; i++) {
                if (chan.transmitUsers[i][0] == user.nUserID || chan.transmitUsers[i][0] == 0) {
                    chan.transmitUsers[i][0] = user.nUserID;
                    chan.transmitUsers[i][1] &= ~streamtype;
                    // remove user if no stream type is active
                    if (chan.transmitUsers[i][1] == StreamType.STREAMTYPE_NONE) {
                        chan.transmitUsers[i][0] = 0;
                        for (int j=i;j<chan.transmitUsers.length-1;++j) {
                            chan.transmitUsers[j][0] = chan.transmitUsers[j+1][0];
                            chan.transmitUsers[j][1] = chan.transmitUsers[j+1][1];
                        }
                    }
                    break;
                }
            }
        } else {
            for (int i = 0; i < chan.transmitUsers.length; i++) {
                if (chan.transmitUsers[i][0] == user.nUserID || chan.transmitUsers[i][0] == 0) {
                    chan.transmitUsers[i][0] = user.nUserID;
                    chan.transmitUsers[i][1] |= streamtype;
                    break;
                }
            }
        }
    }
    
    public static String getURL(String urlToRead) {
        URL url;
        HttpURLConnection conn;
        BufferedReader rd;
        StringBuilder result = new StringBuilder();
        try {
            url = new URL(urlToRead);
            conn = (HttpURLConnection) url.openConnection();
            conn.setRequestMethod("GET");
            rd = new BufferedReader(new InputStreamReader(conn.getInputStream()));
            char[] buff = new char[1024];
            int len;
            while((len = rd.read(buff)) > 0) {
                result.append(buff, 0, len);
            }
            rd.close();
        }
        catch(IOException e) {
            Log.d(TAG, "Failed to receive URL: "+urlToRead+". " + e);
        }
        
        return result.toString();
    }
    
    public static Vector<ServerEntry> getXmlServerEntries(String xml) {
        Vector<ServerEntry> servers = new Vector<>();
        DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
        DocumentBuilder dBuilder;
        Document doc;
        try {
            dBuilder = dbFactory.newDocumentBuilder();
            doc = dBuilder.parse(new InputSource(new StringReader(xml)));
        }
        catch(Exception e) {
            Log.e(TAG, "Failed to parse server entries");
            return servers;
        }
        
        doc.getDocumentElement().normalize();
        
        NodeList nList = doc.getElementsByTagName("host");
        for (int i = 0; i < nList.getLength(); i++) {
            Node hostnode = nList.item(i);
            if (hostnode.getNodeType() == Node.ELEMENT_NODE) {
                Element hostelement = (Element) hostnode;
                ServerEntry entry = new ServerEntry();
                NodeList namenode = hostelement.getElementsByTagName("name");
                if (namenode.getLength() > 0)
                    entry.servername = namenode.item(0).getTextContent();
                NodeList ipaddrnode = hostelement.getElementsByTagName("address");
                if (ipaddrnode.getLength() > 0)
                    entry.ipaddr = ipaddrnode.item(0).getTextContent();
                NodeList tcpportnode = hostelement.getElementsByTagName("tcpport");
                try {
                    if (tcpportnode.getLength() > 0)
                        entry.tcpport = Integer.parseInt(tcpportnode.item(0).getTextContent());
                    NodeList udpportnode = hostelement.getElementsByTagName("udpport");
                    if (udpportnode.getLength() > 0)
                        entry.udpport = Integer.parseInt(udpportnode.item(0).getTextContent());
                }
                catch(NumberFormatException e) {
                    continue;
                }
                NodeList listingnode = hostelement.getElementsByTagName("listing");
                if (listingnode.getLength() > 0) {
                    switch (listingnode.item(0).getTextContent()) {
                        case "official" :
                            entry.servertype = ServerEntry.ServerType.OFFICIAL;
                            break;
                        case "public" :
                            entry.servertype = ServerEntry.ServerType.PUBLIC;
                            break;
                        case "private" :
                            entry.servertype = ServerEntry.ServerType.UNOFFICIAL;
                            break;
                    }
                }
                NodeList encryptednode = hostelement.getElementsByTagName("encrypted");
                if (encryptednode.getLength() > 0)
                    entry.encrypted = encryptednode.item(0).getTextContent().equalsIgnoreCase("true");
                // process <trusted-certificate>
                NodeList certificatenode = hostelement.getElementsByTagName("trusted-certificate");
                if (certificatenode.getLength() > 0) {
                    Node trustednode = certificatenode.item(0);
                    if (trustednode.getNodeType() == Node.ELEMENT_NODE) {
                        Element trustedelement = (Element)trustednode;
                        NodeList cacertnode = trustedelement.getElementsByTagName("certificate-authority-pem");
                        if (cacertnode.getLength() > 0)
                            entry.cacert = cacertnode.item(0).getTextContent();
                        NodeList clientcertnode = trustedelement.getElementsByTagName("client-certificate-pem");
                        if (clientcertnode.getLength() > 0)
                            entry.clientcert = clientcertnode.item(0).getTextContent();
                        NodeList clientkeynode = trustedelement.getElementsByTagName("client-private-key-pem");
                        if (clientkeynode.getLength() > 0)
                            entry.clientcertkey = clientkeynode.item(0).getTextContent();
                        NodeList verifynode = trustedelement.getElementsByTagName("verify-peer");
                        if (verifynode.getLength() > 0)
                            entry.verifypeer = verifynode.item(0).getTextContent().equalsIgnoreCase("true");
                    }
                }
                //process <auth>
                NodeList authlist = hostelement.getElementsByTagName("auth");
                if (authlist.getLength() > 0) {
                    Node authnode = authlist.item(0);
                    if (authnode.getNodeType() == Node.ELEMENT_NODE) {
                        Element authelement = (Element) authnode;
                        NodeList usernamenode = authelement.getElementsByTagName("username");
                        if (usernamenode.getLength() > 0)
                            entry.username = usernamenode.item(0).getTextContent();
                        NodeList passwordnode = authelement.getElementsByTagName("password");
                        if (passwordnode.getLength() > 0)
                            entry.password = passwordnode.item(0).getTextContent();
                        NodeList nicknamenode = authelement.getElementsByTagName("nickname");
                        if (nicknamenode.getLength() > 0)
                            entry.nickname = nicknamenode.item(0).getTextContent();
                    }
                }
                //process <join>
                NodeList joinlist = hostelement.getElementsByTagName("join");
                if (joinlist.getLength() > 0) {
                    Node joinnode = joinlist.item(0);
                    if (joinnode.getNodeType() == Node.ELEMENT_NODE) {
                        Element joinelement = (Element) joinnode;
                        NodeList joinlastchannelnode = hostelement.getElementsByTagName("join-last-channel");
                        if (joinlastchannelnode.getLength() > 0)
                            entry.rememberLastChannel = joinlastchannelnode.item(0).getTextContent().equalsIgnoreCase("true");
                        NodeList channelnode = joinelement.getElementsByTagName("channel");
                        if (channelnode.getLength() > 0)
                            entry.channel = channelnode.item(0).getTextContent();
                        NodeList passwordnode = joinelement.getElementsByTagName("password");
                        if (passwordnode.getLength() > 0)
                            entry.chanpasswd = passwordnode.item(0).getTextContent();
                    }
                }
                //process <stats>
                NodeList statslist = hostelement.getElementsByTagName("stats");
                if (statslist.getLength() > 0) {
                    Node statsnode = statslist.item(0);
                    if (statsnode.getNodeType() == Node.ELEMENT_NODE) {
                        Element statselement = (Element) statsnode;
                        NodeList mothnode = statselement.getElementsByTagName("motd");
                        if (mothnode.getLength() > 0) {
                            entry.stats_motd = mothnode.item(0).getTextContent();
                        }
                        NodeList countrynode = statselement.getElementsByTagName("country");
                        if (countrynode.getLength() > 0) {
                            entry.stats_country = countrynode.item(0).getTextContent();
                        }
                        NodeList usercountnode = statselement.getElementsByTagName("user-count");
                        if (usercountnode.getLength() > 0) {
                            try {
                                entry.stats_usercount = Integer.parseInt(usercountnode.item(0).getTextContent());
                            }
                            catch (NumberFormatException ignored) {
                            }
                         }
                    }
                }
                servers.add(entry);
            }
        }
        
        return servers;
    }

    public static boolean saveServers(Vector<ServerEntry> servers, String path) {
        try {
            FileOutputStream fos = new FileOutputStream(path);
            XmlSerializer serializer = Xml.newSerializer();
            serializer.setOutput(fos, "UTF-8");
            serializer.startDocument(null, Boolean.TRUE);
            serializer.setFeature("http://xmlpull.org/v1/doc/features.html#indent-output", true);
            serializer.startTag(null, "teamtalk").attribute(null, "version", "5.0");
            for (ServerEntry server : servers) {
                serializer.startTag(null, "host");
                serializer.startTag(null, "name").text(server.servername).endTag(null, "name");
                serializer.startTag(null, "address").text(server.ipaddr).endTag(null, "address");
                serializer.startTag(null, "tcpport").text(String.valueOf(server.tcpport)).endTag(null, "tcpport");
                serializer.startTag(null, "udpport").text(String.valueOf(server.udpport)).endTag(null, "udpport");
                serializer.startTag(null, "encrypted").text(String.valueOf(server.encrypted)).endTag(null, "encrypted");
                serializer.startTag(null, "auth");
                serializer.startTag(null, "username").text(server.username).endTag(null, "username");
                serializer.startTag(null, "password").text(server.password).endTag(null, "password");
                serializer.startTag(null, "nickname").text(server.nickname).endTag(null, "nickname");
                serializer.endTag(null, "auth");
                serializer.startTag(null, "join");
                serializer.startTag(null, "join-last-channel").text(String.valueOf(server.rememberLastChannel)).endTag(null, "join-last-channel");
                serializer.startTag(null, "channel").text(server.channel).endTag(null, "channel");
                serializer.startTag(null, "password").text(server.chanpasswd).endTag(null, "password");
                serializer.endTag(null, "join");
                serializer.endTag(null, "host");
            }
            serializer.endTag(null, "teamtalk");
            serializer.endDocument();
            serializer.flush();
            fos.close();
        }
        catch(Exception e) {
            Log.d(TAG, "Unable to save file " + path,  e);
            return false;
        }
        return true;
    }

    public static int refVolume(double percent)
    {
        // 50 % = SoundLevel.SOUND_VOLUME_DEFAULT
        
        //82.832*EXP(0.0508*x) - 50
        percent = Math.max(0, percent);
        percent = Math.min(100, percent);
        
        double d = 82.832 * Math.exp(0.0508 * percent) - 50;
        return (int)d;
    }

    public static int refVolumeToPercent(int volume)
    {
        volume = Math.max(volume, SoundLevel.SOUND_VOLUME_MIN);
        volume = Math.min(volume, SoundLevel.SOUND_VOLUME_MAX);

        double d = (volume + 50) / 82.832;
        d = Math.log(d) / 0.0508;
        return (int)(d + .5);
    }

    public static int refGain(double percent)
    {
        // 50 % = SoundLevel.SOUND_GAIN_DEFAULT
        
        percent = Math.max(0, percent);
        percent = Math.min(100, percent);

        return (int)(82.832 * Math.exp(0.0508 * percent) - 50);
    }

    public static int refGainToPercent(int gain)
    {
        gain = Math.max(gain, SoundLevel.SOUND_GAIN_MIN);
        gain = Math.min(gain, SoundLevel.SOUND_GAIN_MAX);

        double d = (gain + 50) / 82.832;
        d = Math.log(d) / 0.0508;
        return (int)(d + .5);
    }
    
    public static Bitmap drawTextToBitmap(Context gContext, int width, int height, String gText) {
        Resources resources = gContext.getResources();
        float scale = resources.getDisplayMetrics().density;

        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);

        Canvas canvas = new Canvas(bitmap);
        // new antialised Paint
        Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        // text color - #3D3D3D
        paint.setColor(Color.rgb(61, 61, 61));
        // text size in pixels
        paint.setTextSize((int) (14 * scale));
        // text shadow
        paint.setShadowLayer(1f, 0f, 1f, Color.WHITE);

        // draw text to the Canvas center
        Rect bounds = new Rect();
        paint.getTextBounds(gText, 0, gText.length(), bounds);
        int x = (bitmap.getWidth() - bounds.width())/2;
        int y = (bitmap.getHeight() + bounds.height())/2;

        canvas.drawText(gText, x, y, paint);

        return bitmap;
    }
    
    public static String getDisplayName(Context context, User user) {
        SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(context); 
        if(pref.getBoolean(Preferences.PREF_GENERAL_SHOWUSERNAMES, false)) {
            return user.szUsername;
        }
        if (TextUtils.isEmpty(user.szNickname)) {
            return context.getString(R.string.pref_default_nickname) + " - #" + user.nUserID;
        }
        return user.szNickname;
    }

    public static boolean isWebLogin(String username) {
        return username.equals(AppInfo.WEBLOGIN_BEARWARE_USERNAME) ||
                username.endsWith(AppInfo.WEBLOGIN_BEARWARE_USERNAMEPOSTFIX);
    }

}
