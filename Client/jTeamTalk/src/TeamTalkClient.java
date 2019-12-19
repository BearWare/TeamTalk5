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

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Vector;
import java.util.Arrays;

import java.io.File;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.IOException;

import dk.bearware.BannedUser;
import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.ClientFlag;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.SoundDevice;
import dk.bearware.SoundSystem;
import dk.bearware.TeamTalk5;
import dk.bearware.TeamTalkBase;
import dk.bearware.TextMessage;
import dk.bearware.User;
import dk.bearware.UserAccount;
import dk.bearware.UserRight;
import dk.bearware.events.CommandListener;
import dk.bearware.events.ConnectionListener;
import dk.bearware.events.TeamTalkEventHandler;;

public class TeamTalkClient
implements ConnectionListener, CommandListener {

    TeamTalkBase ttclient = new TeamTalk5();
    TeamTalkEventHandler handler = new TeamTalkEventHandler();
    
    Map<Integer, Channel> channels = new HashMap<Integer, Channel>();
    Map<Integer, User> users = new HashMap<Integer, User>();
    
    int cmdid_completed = 0, cmdid_success = 0;

    static Vector<String> badwords = new Vector<String>();

    public static void main(String[] args) throws IOException {

        System.out.println("TeamTalk 5 client example for Java");

        File file = new File("badwords.txt");
        if (file.exists()) {
            BufferedReader br = new BufferedReader(new FileReader(file));
            String line;
            while ((line = br.readLine()) != null) {
                badwords.addAll(Arrays.asList(line.split(",")));
            }

            while (badwords.remove(""));
        }
        
        String ipaddr = "";
        int tcpport, udpport;
        boolean encrypted = false;

        ipaddr = System.getProperty("dk.bearware.ipaddr");
        if (ipaddr == null) {
            ipaddr = getInput("Type IP-address of server to connect to", 
                              "tt5eu.bearware.dk");
        }
        
        String tcpp = System.getProperty("dk.bearware.tcpport");
        if (tcpp == null)
            tcpport = Integer.parseInt(getInput("Type TCP port of server to connect to",
                                                "10335"));
        else
            tcpport = Integer.parseInt(tcpp);

        String udpp = System.getProperty("dk.bearware.udpport");
        if (udpp == null)
            udpport = Integer.parseInt(getInput("Type UDP port of server to connect to",
                                                "10335"));
        else
            udpport = Integer.parseInt(udpp);
        String enc = System.getProperty("dk.bearware.encrypted");
        if (enc == null)
            encrypted = getInput("Is server using encryption (y/n)", "n").contains("y");
        else
            encrypted = enc.equals("1") || enc.equals("true");

        String username = "", passwd = "";
        username = System.getProperty("dk.bearware.username");
        if (username == null)
            username = getInput("Type username", "guest");
        passwd = System.getProperty("dk.bearware.password");
        if (passwd == null)
            passwd = getInput("Type password", "");
        
        while (true) {
            TeamTalkClient inst = new TeamTalkClient();
            inst.configureSoundDevices();
            inst.run(ipaddr, tcpport, udpport, encrypted, username, passwd, true);
        }
    }
    
    static String getInput(String def) {
        if(def.length()>0)
            System.out.print(" (" + def + "): ");
        else
            System.out.print(": ");
        String input = System.console().readLine();
        if(input.length() == 0)
            return def;
        return input;
    }

    static String getInput(String prompt, String def) {
        System.out.print(prompt);
        return getInput(def);
    }

    TeamTalkClient() {
        handler.addConnectionListener(this);
        handler.addCommandListener(this);
    }

    void run(String ipaddr, int tcpport, int udpport, boolean encrypted, String username, String passwd, boolean joinroot) {
        
        if (!ttclient.connect(ipaddr, tcpport, udpport, 0, 0, encrypted)) {
            System.err.print("Failed to connect to server");
            return;
        }
        
        // wait at most 20 seconds for connection to complete
        handler.processEvent(ttclient, 20000);
        
        if((ttclient.getFlags() & ClientFlag.CLIENT_CONNECTED) == 0) {
            System.err.println("Failed to connect to server");
            return;
        }
        
        System.out.println("Logging in...");
        
        // perform login
        int cmdid = ttclient.doLogin("jTeamTalk", username, passwd);
        System.out.println("Issued login cmd #" + cmdid);
        
        // wait for login to complete
        while(cmdid_completed != cmdid)
            handler.processEvent(ttclient, 5000);
        
        // check that login succeeded
        if(cmdid_success != cmdid) {
            System.err.println("Failed to login");
        }
        
        // list channels and users
        System.out.println("Channels and users on server:");
        Iterator ic = channels.entrySet().iterator();
        while (ic.hasNext()) {
            Map.Entry<Integer, Channel> c_pair = (Map.Entry)ic.next();
            Channel chan = c_pair.getValue();
            System.out.print("\t #" + chan.nChannelID + " " + ttclient.getChannelPath(chan.nChannelID));
            System.out.println();
            // list users
            Iterator iu = users.entrySet().iterator();
            while (iu.hasNext()) {
                Map.Entry<Integer, User> u_pair = (Map.Entry)iu.next();
                User user = u_pair.getValue();
                if(user.nChannelID == chan.nChannelID) {
                    System.out.print("\t\t" + "#"+ user.nUserID + " " + user.szNickname);
                    System.out.println();
                }
            }
        }

        if (!joinroot) {
            int chanid = Integer.parseInt(getInput("Type ID of channel to join", "1"));
            String chpasswd = "";
            if(channels.get(chanid).bPassword)
                chpasswd = getInput("Type channel password", "");
            cmdid = ttclient.doJoinChannelByID(chanid, chpasswd);
        }
        else {
            cmdid = ttclient.doJoinChannelByID(ttclient.getRootChannelID(), "");
        }
        
        // wait for login to complete
        while(cmdid_completed != cmdid)
            handler.processEvent(ttclient, 5000);
        
        // check that login succeeded
        if(cmdid_success != cmdid) {
            System.err.println("Failed to join channel");
        }
        
        // run forever
        while((ttclient.getFlags() & ClientFlag.CLIENT_AUTHORIZED) != 0) {
            handler.processEvent(ttclient, -1);
            System.out.println("Processed event");
        }
    }

    void configureSoundDevices() {
        Vector<SoundDevice> snddevs = new Vector<SoundDevice>();
        Map<String, SoundDevice> map = new HashMap<String, SoundDevice>();
        if (TeamTalkBase.getSoundDevices(snddevs)) {
            System.out.println("Listing sound devices:");
            for (SoundDevice dev : snddevs) {
                printSoundDevice(dev);
                map.put(String.valueOf(dev.nDeviceID), dev);
            }
        }
        SoundDevice dev;
        int indev = -1, outdev = -1;
        String prop = System.getProperty("dk.bearware.sndinput");
        if (prop == null)
            dev = map.get(getInput("Type ID of sound device to use for recording", "1978"));
        else
            dev = map.get(prop);

        if(dev != null)
            indev = dev.nDeviceID;

        prop = System.getProperty("dk.bearware.sndoutput");
        if (prop == null)
            dev = map.get(getInput("Type ID of sound device to use for playback: ", "1978"));
        else
            dev = map.get(prop);
        
        if(dev != null)
            outdev = dev.nDeviceID; 
        
        if(indev >= 0) {
            if(!ttclient.initSoundInputDevice(indev))
                System.err.println("Failed to configure sound recording device");
        }
        if(outdev >= 0) {
            if(!ttclient.initSoundOutputDevice(outdev))
                System.err.println("Failed to configure sound playback device");            
        }
    }
    
    void printSoundDevice(SoundDevice dev) {
        System.out.println();
        System.out.println("Device ID #" + dev.nDeviceID + ":");
        System.out.print("\tSound System: ");
        switch (dev.nSoundSystem) {
        case SoundSystem.SOUNDSYSTEM_ALSA:
            System.out.println("ALSA");
            break;
        case SoundSystem.SOUNDSYSTEM_COREAUDIO:
            System.out.println("CoreAudio");
            break;
        case SoundSystem.SOUNDSYSTEM_DSOUND:
            System.out.println("DirectSound");
            break;
        case SoundSystem.SOUNDSYSTEM_OPENSLES:
            System.out.println("OpenSL ES");
            break;
        case SoundSystem.SOUNDSYSTEM_WASAPI:
            System.out.println("Windows Audio Session");
            break;
        case SoundSystem.SOUNDSYSTEM_WINMM:
            System.out.println("Windows Multimedia");
            break;
        default :
            System.out.println("Unknown");
            break;
        }
        System.out.println("\tName: " + dev.szDeviceName);
        System.out.println("\tInput channels: " + dev.nMaxInputChannels);
        System.out.print("\tInput samplerates: ");
        for (int i=0;i<dev.inputSampleRates.length && dev.inputSampleRates[i]>0;i++)
            System.out.print(dev.inputSampleRates[i] + ",");
        System.out.println();
        System.out.println("\tOutput channels: " + dev.nMaxOutputChannels);
        System.out.print("\tOutput samplerates: ");
        for (int i=0;i<dev.outputSampleRates.length && dev.outputSampleRates[i] != 0;i++)
            System.out.print(dev.outputSampleRates[i] + ",");
        System.out.println();
    }

    public boolean containsBadWord(String value) {
        value = value.toLowerCase();

        String[] words = value.split("\\W");
        
        for (String word : words) {
            if (word.isEmpty())
                continue;
            if (badwords.contains(word))
                return true;
        }
        return false;
    }

    public boolean cleanUser(User user) {
        if (containsBadWord(user.szNickname))
            return false;
        if (containsBadWord(user.szStatusMsg))
            return false;
        return true;
    }

    public boolean cleanChannel(Channel chan) {
        if (containsBadWord(chan.szName))
            return false;
        if (containsBadWord(chan.szTopic))
            return false;
        return true;
    }

    public boolean cleanTextMessage(TextMessage msg) {
        if (containsBadWord(msg.szMessage))
            return false;
        return true;
    }
    
    public void onConnectFailed() {
        System.err.println("Failed to connect to server...");
        System.exit(1);
    }

    public void onConnectSuccess() {
        System.out.println("Connected to server...");
    }

    public void onConnectionLost() {
        System.err.println("Lost connection to server...");
        System.exit(1);
    }

    public void onMaxPayloadUpdate(int arg0) {
        // TODO Auto-generated method stub
        
    }

    public void onCmdBannedUser(BannedUser arg0) {
        // TODO Auto-generated method stub
        
    }

    public void onCmdChannelNew(Channel chan) {
        channels.put(chan.nChannelID, chan);

        if (!cleanChannel(chan))
            ttclient.doRemoveChannel(chan.nChannelID);
    }

    public void onCmdChannelRemove(Channel chan) {
        channels.remove(chan.nChannelID);
    }

    public void onCmdChannelUpdate(Channel chan) {
        channels.put(chan.nChannelID, chan);

        if (!cleanChannel(chan) && chan.nParentID != 0)
            ttclient.doRemoveChannel(chan.nChannelID);
    }

    public void onCmdError(int cmdid, ClientErrorMsg err) {
        System.err.println("Command #" + cmdid + " failed.");
        System.err.println("Error code: " + err.nErrorNo);
        System.err.println("Error message: " + err.szErrorMsg);
    }

    public void onCmdFileNew(RemoteFile arg0) {
        // TODO Auto-generated method stub
        
    }

    public void onCmdFileRemove(RemoteFile arg0) {
    }

    public void onCmdMyselfKickedFromChannel() {
        // TODO Auto-generated method stub
        
    }

    public void onCmdMyselfKickedFromChannel(User user) {
        // TODO Auto-generated method stub
        
    }

    public void onCmdMyselfLoggedIn(int userid, UserAccount useraccount) {
        System.out.println("Got userID #" + userid);
        System.out.println("User rights assigned:");
        if((useraccount.uUserRights & UserRight.USERRIGHT_VIEW_ALL_USERS) != 0)
            System.out.println("\tSee all users");
        if((useraccount.uUserRights & UserRight.USERRIGHT_CREATE_TEMPORARY_CHANNEL) != 0)
            System.out.println("\tCreate temporary channel");
        if((useraccount.uUserRights & UserRight.USERRIGHT_MODIFY_CHANNELS) != 0)
                System.out.println("\tModify all channels");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_VOICE) != 0)
                System.out.println("\tTransmit voice");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_VIDEOCAPTURE) != 0)
                System.out.println("\tTransmit video from webcam");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_DESKTOP) != 0)
                System.out.println("\tTransmit desktop");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_DESKTOPINPUT) != 0)
                System.out.println("\tControl desktops");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO) != 0)
                System.out.println("\tTransmit media files containing video");
        if((useraccount.uUserRights & UserRight.USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO) != 0)
                System.out.println("\tTransmit media files containing audio");
        if((useraccount.uUserRights & UserRight.USERRIGHT_DOWNLOAD_FILES) != 0)
                System.out.println("\tDownload files");
        if((useraccount.uUserRights & UserRight.USERRIGHT_UPLOAD_FILES) != 0)
                System.out.println("\tUpload files");
    }

    public void onCmdMyselfLoggedOut() {
        // TODO Auto-generated method stub
        
    }

    public void onCmdProcessing(int cmdid, boolean complete) {
        if(complete)
            cmdid_completed = cmdid;
    }

    public void onCmdServerUpdate(ServerProperties srvprop) {
        System.out.println("Server name: " + srvprop.szServerName);
        System.out.println("Server MOTD: " + srvprop.szMOTD);
    }

    public void onCmdSuccess(int cmdid) {
        System.out.println("Command #" + cmdid + " succeeded");
        cmdid_success = cmdid;
    }

    public void onCmdUserAccount(UserAccount arg0) {
        // TODO Auto-generated method stub
        
    }

    public void onCmdUserJoinedChannel(User user) {
        users.put(user.nUserID, user);
        
        System.out.println("User #" + user.nUserID + " " +
                           user.szNickname +
                           " joined channel \"" + 
                           channels.get(user.nChannelID).szName + "\"");

        if (!cleanUser(user))
            ttclient.doKickUser(user.nUserID, 0);
    }

    public void onCmdUserLeftChannel(int chanid, User user) {
        users.put(user.nUserID, user);
        
        System.out.println("User #" + user.nUserID + " " +
                           user.szNickname +
                           " left channel \"" + 
                           channels.get(chanid).szName + "\"");
    }

    public void onCmdUserLoggedIn(User user) {
        users.put(user.nUserID, user);
        
        System.out.println("User #" + user.nUserID + " " +
                           user.szNickname + " logged in");
        if (!cleanUser(user))
            ttclient.doKickUser(user.nUserID, 0);
    }

    public void onCmdUserLoggedOut(User user) {
        users.remove(user.nUserID);

        System.out.println("User #" + user.nUserID + " " +
                           user.szNickname + " logged out");
    }

    public void onCmdUserTextMessage(TextMessage textmsg) {
        if (!cleanTextMessage(textmsg))
            ttclient.doKickUser(textmsg.nFromUserID, 0);
    }

    public void onCmdUserUpdate(User user) {
        users.put(user.nUserID, user);
        if (!cleanUser(user))
            ttclient.doKickUser(user.nUserID, 0);
    }
}
