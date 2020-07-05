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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Vector;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;

import com.facebook.CallbackManager;
import com.facebook.FacebookCallback;
import com.facebook.FacebookException;
import com.facebook.FacebookSdk;
import com.facebook.login.LoginManager;
import com.facebook.login.LoginResult;
import com.google.gson.Gson;

import dk.bearware.AudioCodec;
import dk.bearware.Channel;
import dk.bearware.ClientError;
import dk.bearware.ClientErrorMsg;
import dk.bearware.FileTransfer;
import dk.bearware.RemoteFile;
import dk.bearware.SoundLevel;
import dk.bearware.User;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.data.AppInfo;
import dk.bearware.data.Preferences;
import dk.bearware.data.ServerEntry;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.CountDownTimer;
import android.preference.PreferenceManager;
import androidx.annotation.NonNull;
import androidx.preference.EditTextPreference;
import androidx.preference.Preference;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

public class Utils {

    public static final String TAG = "bearware";

    private static Map<Integer, Integer> errorMessages = new HashMap<Integer, Integer>();

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
        if (summary.length() > 0 || forcesummary)
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
        Vector<Channel> result = new Vector<Channel>();

        Iterator<Entry<Integer, Channel>> it = channels.entrySet().iterator();

        while (it.hasNext()) {
            Channel chan = it.next().getValue();
            if ((chan.nParentID == chanid) && (chan.nMaxUsers > 0))
                result.add(chan);
        }
        return result;
    }

    public static Vector<Channel> getStickyChannels(int chanid, Map<Integer, Channel> channels) {
        Vector<Channel> result = new Vector<Channel>();

        Iterator<Entry<Integer, Channel>> it = channels.entrySet().iterator();

        while (it.hasNext()) {
            Channel chan = it.next().getValue();
            if ((chan.nParentID == chanid) && (chan.nMaxUsers <= 0))
                result.add(chan);
        }
        return result;
    }

    public static Vector<User> getUsers(int chanid, Map<Integer, User> users) {
        Vector<User> result = new Vector<User>();
        Iterator<Entry<Integer, User>> it = users.entrySet().iterator();

        while (it.hasNext()) {
            User user = it.next().getValue();
            if (user.nChannelID == chanid)
                result.add(user);
        }
        return result;
    }
    
    public static Vector<User> getUsers(Map<Integer, User> users) {
        Vector<User> result = new Vector<User>();
        Iterator<Entry<Integer, User>> it = users.entrySet().iterator();

        while (it.hasNext()) {
            result.add(it.next().getValue());
        }
        return result;
    }
    
    public static Vector<RemoteFile> getRemoteFiles(int chanid, Map<Integer, RemoteFile> remotefiles) {
        Vector<RemoteFile> result = new Vector<RemoteFile>();
        Iterator<Entry<Integer, RemoteFile>> it = remotefiles.entrySet().iterator();

        while (it.hasNext()) {
            RemoteFile remotefile = it.next().getValue();
            if (remotefile.nChannelID == chanid)
                result.add(remotefile);
        }
        return result;
    }
    
    public static Vector<RemoteFile> getRemoteFiles(Map<Integer, RemoteFile> remotefiles) {
        Vector<RemoteFile> result = new Vector<RemoteFile>();
        Iterator<Entry<Integer, RemoteFile>> it = remotefiles.entrySet().iterator();

        while (it.hasNext()) {
            result.add(it.next().getValue());
        }
        return result;
    }
    
    public static Vector<FileTransfer> getFileTransfers(int chanid, Map<Integer, FileTransfer> filetransfers) {
        Vector<FileTransfer> result = new Vector<FileTransfer>();
        Iterator<Entry<Integer, FileTransfer>> it = filetransfers.entrySet().iterator();

        while (it.hasNext()) {
            FileTransfer transfer = it.next().getValue();
            if (transfer.nChannelID == chanid)
                result.add(transfer);
        }
        return result;
    }
    
    public static Vector<FileTransfer> getFileTransfers(Map<Integer, FileTransfer> filetransfers) {
        Vector<FileTransfer> result = new Vector<FileTransfer>();
        Iterator<Entry<Integer, FileTransfer>> it = filetransfers.entrySet().iterator();

        while (it.hasNext()) {
            result.add(it.next().getValue());
        }
        return result;
    }
    
    public static String getURL(String urlToRead) {
        URL url;
        HttpURLConnection conn;
        BufferedReader rd;
        String line;
        String result = "";
        try {
            url = new URL(urlToRead);
            conn = (HttpURLConnection) url.openConnection();
            conn.setRequestMethod("GET");
            rd = new BufferedReader(new InputStreamReader(conn.getInputStream()));
            while((line = rd.readLine()) != null) {
                result += line;
            }
            rd.close();
        }
        catch(IOException e) {
            Log.d(TAG, "Failed to receive URL: "+urlToRead+". " + e.toString());
        }
        
        return result;
    }
    
    public static Vector<ServerEntry> getXmlServerEntries(String xml) {
        Vector<ServerEntry> servers = new Vector<ServerEntry>();
        DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
        DocumentBuilder dBuilder;
        Document doc;
        try {
            dBuilder = dbFactory.newDocumentBuilder();
            doc = dBuilder.parse(new InputSource(new StringReader(xml)));
        }
        catch(Exception e) {
            e.printStackTrace();
            System.out.println("BearWare Exception: " + e);
            return servers;
        }
        
        doc.getDocumentElement().normalize();
        
        NodeList nList = doc.getElementsByTagName("host");
        for (int i = 0; i < nList.getLength(); i++) {
            Node nNode = nList.item(i);
            if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                Element eElement = (Element) nNode;
                ServerEntry entry = new ServerEntry();
                entry.public_server = true;
                entry.rememberLastChannel = false;
                NodeList nHost = eElement.getElementsByTagName("name");
                if(nHost.getLength()>0)
                    entry.servername = nHost.item(0).getTextContent();
                nHost = eElement.getElementsByTagName("address");
                if(nHost.getLength()>0)
                    entry.ipaddr = nHost.item(0).getTextContent();
                nHost = eElement.getElementsByTagName("tcpport");
                try {
                    if(nHost.getLength() > 0)
                        entry.tcpport = Integer.parseInt(nHost.item(0).getTextContent());
                    nHost = eElement.getElementsByTagName("udpport");
                    if(nHost.getLength() > 0)
                        entry.udpport = Integer.parseInt(nHost.item(0).getTextContent());
                }
                catch(NumberFormatException e) {
                    continue;
                }
                nHost = eElement.getElementsByTagName("encrypted");
                if(nHost.getLength()>0)
                    entry.encrypted = nHost.item(0).getTextContent().equalsIgnoreCase("true");
                //process <auth>
                NodeList nListAuth = eElement.getElementsByTagName("auth");
                for(int j = 0;j<nListAuth.getLength();j++) {
                    nNode = nListAuth.item(j);
                    if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                        Element eElement1 = (Element) nNode;
                        NodeList nAuth = eElement1.getElementsByTagName("username");
                        if(nAuth.getLength()>0)
                            entry.username = nAuth.item(0).getTextContent();
                        nAuth = eElement1.getElementsByTagName("password");
                        if(nAuth.getLength()>0)
                            entry.password = nAuth.item(0).getTextContent();
                    }
                }
                //process <join>
                NodeList nListJoin = eElement.getElementsByTagName("join");
                for(int k=0;k<nListJoin.getLength();k++) {
                    nNode = nListJoin.item(k);
                    if(nNode.getNodeType() == Node.ELEMENT_NODE) {
                        Element eElement1 = (Element) nNode;
                        NodeList nJoin = eElement1.getElementsByTagName("channel");
                        if(nJoin.getLength()>0)
                            entry.channel = nJoin.item(0).getTextContent();
                        nJoin = eElement1.getElementsByTagName("password");
                        if(nJoin.getLength()>0)
                            entry.chanpasswd = nJoin.item(0).getTextContent();
                    }
                }
                servers.add(entry);
            }
        }
        
        return servers;
    }

    public static void facebookLogin(Activity activity) {
        Vector<String> permission = new Vector<>();
        permission.add("public_profile");
        LoginManager.getInstance().logInWithReadPermissions(activity, permission);
    }

    public static FacebookCallback<LoginResult> createFacebookLogin(final Activity activity,
                                                                    final TeamTalkService ttservice,
                                                                    final ServerEntry serverentry) {
        FacebookCallback<LoginResult> fbcallback = new FacebookCallback<LoginResult>() {
            @Override
            public void onSuccess(LoginResult loginResult) {
                String token = loginResult.getAccessToken().getToken();

                serverentry.password = AppInfo.WEBLOGIN_FACEBOOK_PASSWDPREFIX + token;
                ttservice.setServerEntry(serverentry);

                // no idea why a timer is necessary to kick-start the ttservice's connect method...
                new CountDownTimer(1, 1) {
                    @Override
                    public void onFinish() {
                        if (!ttservice.reconnect())
                            Toast.makeText(activity,
                                    R.string.err_connection, Toast.LENGTH_LONG).show();
                    }

                    @Override
                    public void onTick(long millisUntilFinished) {
                    }
                }.start();
            }

            @Override
            public void onCancel() {
                Log.d(AppInfo.TAG, String.format("Facebook login was cancelled."));
                Toast.makeText(activity,
                        R.string.err_facebooklogin, Toast.LENGTH_LONG).show();
            }

            @Override
            public void onError(FacebookException error) {
                Toast.makeText(activity,
                        R.string.err_facebooklogin, Toast.LENGTH_LONG).show();
                Log.d(AppInfo.TAG, String.format("Facebook login failed. Exception: %s", error.toString()));
            }
        };
        return fbcallback;
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

}
