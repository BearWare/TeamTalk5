import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Vector;

import dk.bearware.BannedUser;
import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.ClientFlag;
import dk.bearware.RemoteFile;
import dk.bearware.ServerProperties;
import dk.bearware.SoundDevice;
import dk.bearware.SoundSystem;
import dk.bearware.TeamTalk5Pro;
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

    TeamTalkBase ttclient = new TeamTalk5Pro();
    TeamTalkEventHandler handler = new TeamTalkEventHandler();
    
    Map<Integer, Channel> channels = new HashMap<Integer, Channel>();
    Map<Integer, User> users = new HashMap<Integer, User>();
    
    int cmdid_completed = 0, cmdid_success = 0;

    public static void main(String[] args) {

        System.out.println("TeamTalk 5 client example for Java");
        TeamTalkClient inst = new TeamTalkClient();
        inst.run();
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

    void run() {
        configureSoundDevices();
        
        if(!connectToServer()) {
            System.err.print("Failed to connect to server");
        }
        
        // wait at most 20 seconds for connection to complete
        handler.processEvent(ttclient, 20000);
        
        if((ttclient.getFlags() & ClientFlag.CLIENT_CONNECTED) == 0) {
            System.err.println("Failed to connect to server");
            System.exit(1);
        }
        
        System.out.println("Logging in...");
        String username, passwd;
        username = getInput("Type username", "guest");
        passwd = getInput("Type password", "guest");
        
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
        
        int chanid = Integer.parseInt(getInput("Type ID of channel to join", "1"));
        String chpasswd = "";
        if(channels.get(chanid).bPassword)
            chpasswd = getInput("Type channel password", "");
        cmdid = ttclient.doJoinChannelByID(chanid, chpasswd);
        
        // wait for login to complete
        while(cmdid_completed != cmdid)
            handler.processEvent(ttclient, 5000);
        
        // check that login succeeded
        if(cmdid_success != cmdid) {
            System.err.println("Failed to join channel");
        }
        
        // run forever
        while(true) {
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
        dev = map.get(getInput("Type ID of sound device to use for recording", "")); 
        if(dev != null)
            indev = dev.nDeviceID; 
        dev = map.get(getInput("Type ID of sound device to use for playback: ", ""));
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
    
    boolean connectToServer() {
        String ipaddr = "";
        int tcpport, udpport;
        boolean encrypted = false;
        
        ipaddr = getInput("Type IP-address of server to connect to", 
                          "tt5eu.bearware.dk");
        tcpport = Integer.parseInt(getInput("Type TCP port of server to connect to",
                                            "10335"));
        udpport = Integer.parseInt(getInput("Type UDP port of server to connect to",
                                            "10335"));
        encrypted = getInput("Is server using encryption", "y/n").contains("y");
        return ttclient.connect(ipaddr, tcpport, udpport, 0, 0, encrypted);
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
    }

    public void onCmdChannelRemove(Channel chan) {
        channels.remove(chan.nChannelID);
    }

    public void onCmdChannelUpdate(Channel chan) {
        channels.put(chan.nChannelID, chan);
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
    }

    public void onCmdUserLoggedOut(User user) {
        users.remove(user.nUserID);

        System.out.println("User #" + user.nUserID + " " +
                           user.szNickname + " logged out");
    }

    public void onCmdUserTextMessage(TextMessage arg0) {
        // TODO Auto-generated method stub
        
    }

    public void onCmdUserUpdate(User user) {
        users.put(user.nUserID, user);
    }
}
