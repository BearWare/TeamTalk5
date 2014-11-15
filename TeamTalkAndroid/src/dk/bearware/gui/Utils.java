/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
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

package dk.bearware.gui;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.net.HttpURLConnection;
import java.net.URL;
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
import com.google.gson.Gson;

import dk.bearware.AudioCodec;
import dk.bearware.BitmapFormat;
import dk.bearware.Channel;
import dk.bearware.FileTransfer;
import dk.bearware.RemoteFile;
import dk.bearware.User;
import dk.bearware.data.ServerEntry;
import android.content.Intent;
import android.graphics.Bitmap;
import android.preference.EditTextPreference;
import android.preference.Preference;

public class Utils {

    public static void setEditTextPreference(Preference preference, String text, String summary) {
        EditTextPreference textpref = (EditTextPreference) preference;
        textpref.setText(text);
        if (summary.length() > 0)
            textpref.setSummary(summary);
    }

    public static String getEditTextPreference(Preference preference) {
        EditTextPreference textpref = (EditTextPreference) preference;
        return textpref.getText();
    }
    
    public static Intent putServerEntry(Intent intent, ServerEntry entry) {
        return intent.putExtra(ServerEntry.class.getName(), new Gson().toJson(entry));
    }
    
    public static ServerEntry getServerEntry(Intent intent) {
        if (intent.hasExtra(ServerEntry.class.getName())) {
            return new Gson().fromJson(intent.getExtras().getString(ServerEntry.class.getName()),  ServerEntry.class);
        }
        return null;
    }
    
    public static Intent putAudioCodec(Intent intent, AudioCodec entry) {
        return intent.putExtra(AudioCodec.class.getName(), new Gson().toJson(entry));
    }
    
    public static AudioCodec getAudioCodec(Intent intent) {
        if (intent.hasExtra(AudioCodec.class.getName())) {
            return new Gson().fromJson(intent.getExtras().getString(AudioCodec.class.getName()),  AudioCodec.class);
        }
        return null;
    }
    
    public static Vector<Channel> getSubChannels(int chanid, Map<Integer, Channel> channels) {
        Vector<Channel> result = new Vector<Channel>();
        
        Iterator<Entry<Integer, Channel>> it = channels.entrySet().iterator();

        while (it.hasNext()) {
            Channel chan = it.next().getValue();
            if (chan.nParentID == chanid)
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
}
