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

import java.util.Vector;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import junit.framework.TestCase;

public abstract class TeamTalkTestCaseBase extends TestCase {

    public static boolean ENCRYPTED = false;
    public static boolean DEBUG_OUTPUT = false;
    public static final int DEF_WAIT = 15000;

    public static String ADMIN_USERNAME = "admin", ADMIN_PASSWORD = "admin", ADMIN_NICKNAME = "Admin";
    public static String IPADDR = "127.0.0.1";

    public static int TCPPORT = 0, UDPPORT = 0;

    public static String SYSTEMID = "teamtalk";
    public static String STORAGEFOLDER = System.getProperty("user.dir");

    public static int INPUTDEVICEID = -1, OUTPUTDEVICEID = -1;
    public static String VIDEODEVICEID = "None", VIDEODEVDISABLE="None"; //set to "None" to ignore video capture tests
    public static String MEDIAFILE_AUDIO = "";
    public static String MEDIAFILE_VIDEO = "";
    public static String HTTPS_MEDIAFILE = "";
    public static boolean OPUSTOOLS = true;

    public static final String CRYPTO_CERT_FILE = "ttservercert.pem", CRYPTO_KEY_FILE = "ttserverkey.pem";
    public static final String MUXEDMEDIAFILE_WAVE = "muxwavefile.wav";
    public static final String MUXEDMEDIAFILE_SPEEX = "muxwavefile_speex.ogg";
    public static final String MUXEDMEDIAFILE_SPEEX_VBR = "muxwavefile_speex_vbr.ogg";
    public static final String MUXEDMEDIAFILE_OPUS = "muxwavefile_opus.ogg";

    public Vector<TeamTalkBase> ttclients = new Vector<TeamTalkBase>();

    public abstract TeamTalkBase newClientInstance();

    protected void setUp() throws Exception {
        super.setUp();

        String prop = System.getProperty("dk.bearware.sndinputid");
        if(prop != null && !prop.isEmpty())
            this.INPUTDEVICEID = Integer.parseInt(prop);

        prop = System.getProperty("dk.bearware.sndoutputid");
        if(prop != null && !prop.isEmpty())
            this.OUTPUTDEVICEID = Integer.parseInt(prop);

        prop = System.getProperty("dk.bearware.encrypted");
        if(prop != null && !prop.isEmpty())
            this.ENCRYPTED = Integer.parseInt(prop) != 0;

        prop = System.getProperty("dk.bearware.serverip");
        if(prop != null && !prop.isEmpty())
            this.IPADDR = prop;

        prop = System.getProperty("dk.bearware.videodevid");
        if(prop != null && !prop.isEmpty())
            this.VIDEODEVICEID = prop;

        prop = System.getProperty("dk.bearware.videofile");
        if(prop != null && !prop.isEmpty())
            this.MEDIAFILE_VIDEO = prop;

        prop = System.getProperty("dk.bearware.audiofile");
        if(prop != null && !prop.isEmpty())
            this.MEDIAFILE_AUDIO = prop;

        prop = System.getProperty("dk.bearware.httpsfile");
        if(prop != null && !prop.isEmpty())
            this.HTTPS_MEDIAFILE = prop;

        prop = System.getProperty("dk.bearware.opustools");
        if(prop != null && !prop.isEmpty())
            this.OPUSTOOLS = Integer.parseInt(prop) != 0;

        if(TCPPORT == 0 && UDPPORT == 0) {
            if(this.ENCRYPTED) {
                TCPPORT = Constants.DEFAULT_TCP_PORT_ENCRYPTED;
                UDPPORT = Constants.DEFAULT_UDP_PORT_ENCRYPTED;
            }
            else {
                TCPPORT = Constants.DEFAULT_TCP_PORT;
                UDPPORT = Constants.DEFAULT_UDP_PORT;
            }
        }
    }

    protected void tearDown() throws Exception {
        super.tearDown();

        for(TeamTalkBase ttclient : ttclients) {
            ttclient.disconnect();
            if((ttclient.getFlags() & ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX) == ClientFlag.CLIENT_SNDINOUTPUT_DUPLEX) {
                ttclient.closeSoundDuplexDevices();
            }
            else {
                ttclient.closeSoundInputDevice();
                ttclient.closeSoundOutputDevice();
            }
            ttclient.closeVideoCaptureDevice();
            ttclient.stopStreamingMediaFileToChannel();
            // close TeamTalk instance since we don't know when GC
            // will do it. On Android we can only create few TT
            // instances, so we need to ensure the previous test-cases
            // have released their resources.
            ttclient.closeTeamTalk();
        }
        ttclients.clear();
    }

    protected void initSound(TeamTalkBase ttclient) {
        initSound(ttclient, false);
    }

    protected void initSound(TeamTalkBase ttclient, boolean duplex) {
        initSound(ttclient, duplex, INPUTDEVICEID, OUTPUTDEVICEID);
    }

    protected void initSound(TeamTalkBase ttclient, boolean duplex, int inputdeviceid, int outputdeviceid) {

        Vector<SoundDevice> devs = new Vector<SoundDevice>();
        assertTrue("get sound devs", ttclient.getSoundDevices(devs));

        if ("0".equals(System.getProperty("dk.bearware.verbose")) == false) {
            System.out.println("---- Sound Devices ----");
            for(int i=0;i<devs.size();i++)
                printSoundDevice(devs.get(i));
        }

        IntPtr indev = new IntPtr(), outdev = new IntPtr();
        if (inputdeviceid < 0 && outputdeviceid < 0)
           assertTrue("get default devs", ttclient.getDefaultSoundDevices(indev, outdev));

        if (inputdeviceid >= 0)
            indev.value = inputdeviceid;
        if (outputdeviceid >= 0)
            outdev.value = outputdeviceid;

        if(duplex) {
            assertTrue("init duplex devs", ttclient.initSoundDuplexDevices(indev.value, outdev.value));
        }
        else {
            assertTrue("init input dev", ttclient.initSoundInputDevice(indev.value));
            assertTrue("init output dev", ttclient.initSoundOutputDevice(outdev.value));
        }

        if ("0".equals(System.getProperty("dk.bearware.verbose")) == false) {
            System.out.println("Using sound input device #"+indev.value+" and output device #"+outdev.value);
        }
    }

    public interface ServerInterleave {
        public void interleave();
    }

    static ServerInterleave nop = new ServerInterleave() {
        public void interleave() {
        }
    };

    protected static void connect(TeamTalkBase ttclient)
    {
        connect(ttclient, SYSTEMID);
    }

    protected static void connect(TeamTalkBase ttclient, String systemID) {
        connect(ttclient, systemID, nop);
    }

    protected static void connect(TeamTalkBase ttclient, String systemID, ServerInterleave server) {
        connect(ttclient, systemID, IPADDR, TCPPORT, UDPPORT, server);
    }

    protected static void connect(TeamTalkBase ttclient, String systemID,
                                  String hostaddr, int tcpport, int udpport, ServerInterleave server)
    {
        assertTrue("connect call", ttclient.connectSysID(hostaddr, tcpport, udpport, 0, 0, ENCRYPTED, systemID));

        assertTrue("wait connect", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CON_SUCCESS, DEF_WAIT, server));
    }

    protected static void login(TeamTalkBase ttclient, String nick, String username, String passwd) {
        login(ttclient, nick, username, passwd, "");
    }

    protected static void login(TeamTalkBase ttclient, String nick, String username, String passwd, String clientname) {
        login(ttclient, nick, username, passwd, clientname, nop);
    }

    protected static void login(TeamTalkBase ttclient, String nick, String username,
                                String passwd, String clientname, ServerInterleave server)
    {
        int cmdid = ttclient.doLoginEx(nick, username, passwd, clientname);
        assertTrue("do login", cmdid > 0);

        TTMessage msg = new TTMessage();
        assertTrue("wait login", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN, DEF_WAIT, msg, server));

        UserAccount account = msg.useraccount;
        assertEquals("username set", username, account.szUsername);
        //Assert.AreEqual(passwd, account.szPassword, "password set");
        assertTrue("Wait login complete", waitCmdComplete(ttclient, cmdid, DEF_WAIT));
        assertTrue("Authorized", hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_AUTHORIZED));
    }

    protected void resetServerProperties() {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, ADMIN_NICKNAME + "resetServerProperties()", ADMIN_USERNAME, ADMIN_PASSWORD);

        // reset server properties
        ServerProperties prop = new ServerProperties();
        prop.szServerName = "TeamTalk 5 Server";
        prop.szMOTD = "";
        prop.szMOTDRaw = "";
        prop.nMaxUsers = 1000;
        prop.nMaxLoginAttempts = 0;
        prop.nMaxLoginsPerIPAddress = 0;
        prop.nLoginDelayMSec = 0;
        prop.nMaxVoiceTxPerSecond = 0;
        prop.nMaxVideoCaptureTxPerSecond = 0;
        prop.nMaxMediaFileTxPerSecond = 0;
        prop.nMaxDesktopTxPerSecond = 0;
        prop.nMaxTotalTxPerSecond = 0;
        prop.bAutoSave = false;
        prop.nTcpPort = TCPPORT;
        prop.nUdpPort = UDPPORT;
        prop.nUserTimeout = 60;
        prop.szServerVersion = "";
        prop.szServerProtocolVersion = "";
        prop.szAccessToken = "";
        assertTrue("reset server properties", waitCmdSuccess(ttclient, ttclient.doUpdateServer(prop), DEF_WAIT));

        // reset bans
        int cmdid = ttclient.doListBans(0, 0, 10000);
        assertTrue("do list bans", cmdid > 0);
        removeBans(ttclient, cmdid);

        cmdid = ttclient.doListBans(ttclient.getRootChannelID(), 0, 10000);
        assertTrue("do list root bans", cmdid > 0);
        removeBans(ttclient, cmdid);

        // erase user accounts
        Vector<UserAccount> accounts = new Vector<UserAccount>();
        cmdid = ttclient.doListUserAccounts(0, 10000);
        TTMessage msg = new TTMessage();
        while (cmdid > 0 && ttclient.getMessage(msg, DEF_WAIT)) {
            switch (msg.nClientEvent) {
            case ClientEvent.CLIENTEVENT_CMD_USERACCOUNT :
                accounts.add(msg.useraccount);
                break;
            case ClientEvent.CLIENTEVENT_CMD_PROCESSING :
                if (msg.nSource == cmdid && msg.bActive == false) {
                    cmdid = 0;
                }
                break;
            }
        }
        
        for (UserAccount account : accounts) {
            if (account.szUsername.equals(ADMIN_USERNAME))
                continue;
            
            assertTrue("del account", waitCmdSuccess(ttclient, ttclient.doDeleteUserAccount(account.szUsername), DEF_WAIT));
        }

        // reset root channel
        Channel root = new Channel(true, false);
        root.nChannelID = ttclient.getRootChannelID();
        root.uChannelType = ChannelType.CHANNEL_PERMANENT;
        root.nMaxUsers = prop.nMaxUsers;
        assertTrue("reset root channel", waitCmdSuccess(ttclient, ttclient.doUpdateChannel(root), DEF_WAIT));

        assertTrue("Disconnect", ttclient.disconnect());
    }

    protected void removeBans(TeamTalkBase ttclient, int cmdid) {
        Vector<BannedUser> bans = new Vector<BannedUser>();
        TTMessage msg = new TTMessage();
        while (cmdid > 0 && ttclient.getMessage(msg, DEF_WAIT)) {
            switch (msg.nClientEvent) {
            case ClientEvent.CLIENTEVENT_CMD_BANNEDUSER :
                bans.add(msg.banneduser);
                break;
            case ClientEvent.CLIENTEVENT_CMD_PROCESSING :
                if (msg.nSource == cmdid && msg.bActive == false) {
                    cmdid = 0;
                }
                break;
            }
        }

        for (BannedUser ban : bans) {
            assertTrue("del ban", waitCmdSuccess(ttclient, ttclient.doUnBanUserEx(ban), DEF_WAIT));
        }
    }
    
    protected void makeUserAccount(String nickname, String username, String password, int userrights)
    {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, ADMIN_NICKNAME + "makeUserAccount()", ADMIN_USERNAME, ADMIN_PASSWORD);
        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = username;
        useraccount.szPassword = password;
        useraccount.uUserRights = userrights;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        assertTrue("New user account ok", waitCmdSuccess(ttclient, ttclient.doNewUserAccount(useraccount), DEF_WAIT));
        assertTrue("Disconnect", ttclient.disconnect());
    }

    protected static void joinRoot(TeamTalkBase ttclient) {
        joinRoot(ttclient, nop);
    }

    protected static void joinRoot(TeamTalkBase ttclient, ServerInterleave server)
    {
        assertTrue("Auth ok", hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_AUTHORIZED));

        assertTrue("root exists", ttclient.getRootChannelID() > 0);

        int cmdid = ttclient.doJoinChannelByID(ttclient.getRootChannelID(), "");

        assertTrue("do join root", cmdid > 0);

        assertTrue("Wait join complete", waitCmdComplete(ttclient, cmdid, DEF_WAIT, server));

        assertEquals("In root channel", ttclient.getMyChannelID(), ttclient.getRootChannelID());
    }

    protected static boolean waitForEvent(TeamTalkBase ttclient, int nClientEvent,
                                          int waittimeout, TTMessage msg) {
        return waitForEvent(ttclient, nClientEvent, waittimeout, msg, nop);
    }

    protected static boolean waitForEvent(TeamTalkBase ttclient, int nClientEvent,
                                          int waittimeout, ServerInterleave interleave) {
        return waitForEvent(ttclient, nClientEvent, waittimeout, new TTMessage(), interleave);
    }
/*
    protected static boolean waitForEvent(TeamTalkBase ttclient, int nClientEvent,
                                          int waittimeout, TeamTalkEventHandler eventhandler,
                                          ServerInterleave interleave) {

        long start = System.currentTimeMillis();
        TTMessage tmp = new TTMessage();
        boolean gotmsg;
        do {
            gotmsg = eventhandler.processEvent(ttclient, waittimeout);

            interleave.interleave();

            if(System.currentTimeMillis() - start >= waittimeout)
                break;
        }
        while (!gotmsg || tmp.nClientEvent != nClientEvent);
    }
*/
    protected static boolean waitForEvent(TeamTalkBase ttclient, int nClientEvent,
                                          int waittimeout, TTMessage msg, ServerInterleave interleave) {
        long start = System.currentTimeMillis();
        TTMessage tmp = new TTMessage();
        boolean gotmsg;
        do {
            // caller might pass 'nClientEvent =
            // ClientEvent.CLIENTEVENT_NONE' which is default in
            // TTMessage. So set to something unsupported.
            tmp.nClientEvent = -1;

            gotmsg = ttclient.getMessage(tmp, 0);

            interleave.interleave();

            if(DEBUG_OUTPUT && gotmsg) {
                System.out.println(System.currentTimeMillis() + " #" + ttclient.getMyUserID() + ": " + tmp.nClientEvent);
                if(tmp.nClientEvent == ClientEvent.CLIENTEVENT_CMD_ERROR) {
                    System.out.println("Command error: " + tmp.clienterrormsg.szErrorMsg);
                }
            }
        }
        while (tmp.nClientEvent != nClientEvent && (System.currentTimeMillis() - start <= waittimeout || gotmsg));

        if (tmp.nClientEvent == nClientEvent)
        {
            if (DEBUG_OUTPUT)
                System.out.println("Success. Event: " + nClientEvent);

            msg.nClientEvent = tmp.nClientEvent;
            msg.ttType = tmp.ttType;
            msg.nSource = tmp.nSource;

            msg.channel = tmp.channel;
            msg.clienterrormsg = tmp.clienterrormsg;
            msg.desktopinput = tmp.desktopinput;
            msg.filetransfer = tmp.filetransfer;
            msg.mediafileinfo = tmp.mediafileinfo;
            msg.remotefile = tmp.remotefile;
            msg.serverproperties = tmp.serverproperties;
            msg.serverstatistics = tmp.serverstatistics;
            msg.textmessage = tmp.textmessage;
            msg.user = tmp.user;
            msg.useraccount = tmp.useraccount;
            msg.banneduser = tmp.banneduser;
            msg.bActive = tmp.bActive;
            msg.nBytesRemain = tmp.nBytesRemain;
            msg.nStreamID = tmp.nStreamID;
            msg.nPayloadSize = tmp.nPayloadSize;
            msg.nStreamType = tmp.nStreamType;
            msg.audioinputprogress = tmp.audioinputprogress;
            //if assert fails it's because the TTType isn't handled here
            assertTrue("TTType unhandled: " + tmp.ttType, tmp.ttType <= TTType.__AUDIOINPUTPROGRESS);
        }
        else
        {
            if (DEBUG_OUTPUT)
                System.out.println("Failed. Event: " + nClientEvent);
        }
        return tmp.nClientEvent == nClientEvent;
    }


    protected static boolean waitForEvent(TeamTalkBase ttclient, int nClientEvent, int waittimeout)  {
        TTMessage msg = new TTMessage();
        return waitForEvent(ttclient, nClientEvent, waittimeout, msg);
    }

    protected static boolean waitCmdComplete(TeamTalkBase ttclient, int cmdid, int waittimeout) {
        return waitCmdComplete(ttclient, cmdid, waittimeout, nop);
    }

    protected static boolean waitCmdComplete(TeamTalkBase ttclient, int cmdid, int waittimeout, ServerInterleave interleave) {
        TTMessage msg = new TTMessage();

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_PROCESSING, waittimeout, msg, interleave))
        {
            if (msg.nSource == cmdid && msg.bActive == false)
                return true;
        }
        return false;
    }

    protected static boolean waitCmdSuccess(TeamTalkBase ttclient, int cmdid, int waittimeout) {
        return waitCmdSuccess(ttclient, cmdid, waittimeout, nop);
    }

    protected static boolean waitCmdSuccess(TeamTalkBase ttclient, int cmdid,
                                            int waittimeout, ServerInterleave interleave) {
        TTMessage msg = new TTMessage();
        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, waittimeout, msg, interleave))
        {
            if (msg.nSource == cmdid) {
                waitCmdComplete(ttclient, cmdid, waittimeout, interleave);
                return true;
            }
        }

        return false;
    }

    protected static boolean waitCmdError(TeamTalkBase ttclient, int cmdid, int waittimeout, TTMessage msg) {
        return waitCmdError(ttclient, cmdid, waittimeout, msg, nop);
    }

    protected static boolean waitCmdError(TeamTalkBase ttclient, int cmdid, int waittimeout, ServerInterleave interleave) {
        return waitCmdError(ttclient, cmdid, waittimeout, new TTMessage(), interleave);
    }
    protected static boolean waitCmdError(TeamTalkBase ttclient, int cmdid, int waittimeout, TTMessage msg, ServerInterleave interleave) {

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, waittimeout, msg, interleave))
        {
            if (msg.nSource == cmdid) {
                waitCmdComplete(ttclient, cmdid, waittimeout, interleave);
                return true;
            }
        }

        return false;
    }

    protected static boolean waitCmdError(TeamTalkBase ttclient, int cmdid, int waittimeout)
    {
        TTMessage msg = new TTMessage();
        return waitCmdError(ttclient, cmdid, waittimeout, msg);
    }

    protected static Channel buildDefaultChannel(TeamTalkBase ttclient, String name) {
        return buildDefaultChannel(ttclient, name, Codec.OPUS_CODEC);
    }

    protected static Channel buildDefaultChannel(TeamTalkBase ttclient, String name, int codec) {
        Channel chan = new Channel();
        chan.nParentID = ttclient.getRootChannelID();
        chan.szName = name;
        chan.szTopic = "a topic";
        chan.nMaxUsers = 128;
        chan.audiocodec.nCodec = codec;
        switch(codec) {
            case Codec.OPUS_CODEC :
                chan.audiocodec.opus = new OpusCodec(true);
                break;
            case Codec.SPEEX_CODEC :
                chan.audiocodec.speex = new SpeexCodec(true);
                break;
            case Codec.SPEEX_VBR_CODEC :
                chan.audiocodec.speex_vbr = new SpeexVBRCodec(true);
                break;
        }
        return chan;
    }

    public static String getTestMethodName()
    {
        for (StackTraceElement ste : Thread.currentThread().getStackTrace()) {
            if (ste.getMethodName().startsWith("test"))
                return ste.getMethodName();
        }
        assertTrue("no test method found", false);
        return "";
    }

    static boolean hasFlag(int flags, int flag) {
        return (flags & flag) == flag;
    }

    static void printSoundDevice(SoundDevice dev) {
        System.out.println("Sound dev " + Integer.toString(dev.nDeviceID) + ":");
        System.out.println("\tName:" + dev.szDeviceName);
        switch (dev.nSoundSystem) {
        case SoundSystem.SOUNDSYSTEM_NONE :
            System.out.println("\tSound System: None");
            break;
        case SoundSystem.SOUNDSYSTEM_WINMM :
            System.out.println("\tSound System: Windows MM");
            break;
        case SoundSystem.SOUNDSYSTEM_DSOUND :
            System.out.println("\tSound System: DirectSound");
            break;
        case SoundSystem.SOUNDSYSTEM_ALSA :
            System.out.println("\tSound System: ALSA");
            break;
        case SoundSystem.SOUNDSYSTEM_COREAUDIO :
            System.out.println("\tSound System: CoreAudio");
            break;
        case SoundSystem.SOUNDSYSTEM_WASAPI :
            System.out.println("\tSound System: WASAPI");
            break;
        case SoundSystem.SOUNDSYSTEM_OPENSLES_ANDROID :
            System.out.println("\tSound System: OpenSL ES for Android");
            break;
        case SoundSystem.SOUNDSYSTEM_AUDIOUNIT_IOS :
            System.out.println("\tSound System: AudioUnit for iOS");
            break;
        }
        System.out.println("\tInput channels: " + Integer.toString(dev.nMaxInputChannels));
        System.out.println("\tOutput channels: " + Integer.toString(dev.nMaxOutputChannels));
        System.out.print("\tInput sample rates: ");
        for(int j=0;j<dev.inputSampleRates.length && dev.inputSampleRates[j] != 0;j++)
            System.out.print(Integer.toString(dev.inputSampleRates[j]) + ", ");
        System.out.println();
        System.out.print("\tOutput sample rates: ");
        for(int j=0;j<dev.outputSampleRates.length && dev.outputSampleRates[j] != 0;j++)
            System.out.print(Integer.toString(dev.outputSampleRates[j]) + ", ");
        System.out.println();
        System.out.println("\tDefault sample rate: " + Integer.toString(dev.nDefaultSampleRate));
    }

    public static SoundDevice getSoundDevice(TeamTalkBase ttclient, int deviceid) {
        Vector<SoundDevice> devs = new Vector<SoundDevice>();
        ttclient.getSoundDevices(devs);
        for(SoundDevice d : devs) {
            if (d.nDeviceID == deviceid)
                return d;
        }
        return null;
    }

    public static boolean supportsInputSampleRate(SoundDevice indev, int samplerate) {

        boolean inputsr = false;
        for (int sr : indev.inputSampleRates)
            inputsr |= sr == samplerate;

        return inputsr;
    }

    public static boolean supportsOutputSampleRate(SoundDevice outdev, int samplerate) {

        boolean outputsr = false;
        for (int sr : outdev.outputSampleRates)
            outputsr |= sr == samplerate;

        return outputsr;
    }

    static boolean supportsDuplexMode(TeamTalkBase ttclient, int inputdeviceid, int outputdeviceid, int samplerate) {

        if (inputdeviceid == -1 || outputdeviceid == -1) {
            IntPtr indev = new IntPtr(), outdev = new IntPtr();
            ttclient.getDefaultSoundDevices(indev, outdev);
            inputdeviceid = inputdeviceid == -1? indev.value : inputdeviceid;
            outputdeviceid = outputdeviceid == -1? outdev.value : outputdeviceid;
        }

        SoundDevice indev = getSoundDevice(ttclient, inputdeviceid),
            outdev = getSoundDevice(ttclient, outputdeviceid);

        assertTrue("indev set", indev != null);
        assertTrue("outdev set", outdev != null);

        boolean inputsr = supportsInputSampleRate(indev, samplerate),
            outputsr = supportsOutputSampleRate(outdev, samplerate);

        return inputsr && outputsr;
    }

    static FileOutputStream newWaveFile(String filename, int samplerate, int channels, int bytesize) throws IOException {
        FileOutputStream fs = new FileOutputStream(filename);

        fs.write(new String("RIFF").getBytes());
        int v = bytesize + 36 - 8;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF), (byte)((v>>16) & 0xFF), (byte)((v>>24) & 0xFF)}); //WRITE_BYTES - 36 - 8
        fs.write(new String("WAVEfmt ").getBytes());
        fs.write(new byte[] {0x10, 0x0, 0x0, 0x0}); //hdr size
        fs.write(new byte[] {0x1, 0x0}); //type
        fs.write(new byte[] {(byte)channels, 0x0}); //channels
        v = samplerate;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF), (byte)((v>>16) & 0xFF), (byte)((v>>24) & 0xFF)}); //sample rate
        v = (samplerate * 16 * channels) / 8;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF), (byte)((v>>16) & 0xFF), (byte)((v>>24) & 0xFF)}); //bytes/sec
        v = (16 * channels) / 8;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF)}); //block align
        fs.write(new byte[] {0x10, 0x0}); //bit depth
        fs.write(new String("data").getBytes());
        v = bytesize - 44;
        fs.write(new byte[] {(byte)(v & 0xFF), (byte)((v>>8) & 0xFF), (byte)((v>>16) & 0xFF), (byte)((v>>24) & 0xFF)}); //WRITE_BYTES - 44
        return fs;
    }

    static byte[] audioToByteArray(short[] audio) {
        ByteBuffer buf = ByteBuffer.allocate(audio.length * 2);
        for(int i = 0; i<audio.length; ++i) {
            buf.putShort(audio[i]);
        }
        return buf.array();
    }

    static short[] audioToShortArray(byte[] audio) {
        short[] converted = new short[audio.length / 2];
        ByteBuffer.wrap(audio).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().get(converted);
        return converted;
    }

    static short[] generateTone(int freq, int samplerate, int channels, int durationMSec) {
        double volume = 8000;
        double duration = durationMSec;
        duration /= 1000;
        int samples = (int)(duration * samplerate);
        short[] buffer = new short[samples*channels];

        for (int i=0; i < samples; i++) {
            double t = (double)i / samplerate;
            double v = volume * Math.sin((double)freq * t * 2. * Math.PI);

            if (v > 32767)
                v = 32767;
            else if(v < -32768)
                v = -32768;

            if (channels == 1)
                buffer[i] = (short)v;
            else {
                buffer[2 * i] = (short)v;
                buffer[2 * i + 1] = (short)v;
            }
        }
        return buffer;
    }

    static byte[] generateToneAsByte(int freq, int samplerate, int channels, int durationMSec) {
        short[] tone = generateTone(freq, samplerate, channels, durationMSec);
        return audioToByteArray(tone);
    }

}
