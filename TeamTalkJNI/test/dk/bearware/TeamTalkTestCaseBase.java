package dk.bearware;

import java.util.Vector;

import junit.framework.TestCase;

public class TeamTalkTestCaseBase extends TestCase {

    static boolean PROEDITION = false, ENCRYPTED = false;
    static final boolean DEBUG_OUTPUT = false;
    static final int DEF_WAIT = 15000;

    static String ADMIN_USERNAME = "admin.username", ADMIN_PASSWORD = "admin.password", ADMIN_NICKNAME = "Admin";
    static String IPADDR = "my.server.ip.address";

    static int TCPPORT = 10333, UDPPORT = 10333;

    static final String UPLOADFILE = "filename.txt";
    static final String DOWNLOADFILE = "filename.txt";
    static final String MUXEDMEDIAFILE = "muxwavefile.wav";
    static final String MEDIAFILE_AUDIO = "music.wav";
    static final String MEDIAFILE = "video.avi";
    Vector<TeamTalkBase> ttclients = new Vector<TeamTalkBase>();
    
    protected void setUp() throws Exception {
        super.setUp();
    }

    protected void tearDown() throws Exception {
        super.tearDown();
        
        for(TeamTalkBase ttclient : ttclients) {
            ttclient.disconnect();
            ttclient.closeSoundInputDevice();
            ttclient.closeSoundOutputDevice();
            ttclient.closeVideoCaptureDevice();
            ttclient.stopStreamingMediaFileToChannel();
        }
        ttclients.clear();
    }
     
    protected void initSound(TeamTalkBase ttclient)
    {
        IntPtr howmany = new IntPtr(0);

        Vector<SoundDevice> devs = new Vector<SoundDevice>();
        assertTrue(ttclient.getSoundDevices(devs));
        System.out.println("---- Sound Devices ----");
        for(int i=0;i<devs.size();i++)
            printSoundDevice(devs.get(i));

        IntPtr indev = new IntPtr(), outdev = new IntPtr();
        assertTrue(ttclient.getDefaultSoundDevices(indev, outdev));
        
        assertTrue("init input dev", ttclient.initSoundInputDevice(indev.value));
        assertTrue("init output dev", ttclient.initSoundOutputDevice(outdev.value));

        
        SpeexDSP spxdsp = new SpeexDSP(true), spxdsp2 = new SpeexDSP();
        assertTrue(ttclient.setSoundInputPreprocess(spxdsp));

        assertTrue(ttclient.getSoundInputPreprocess(spxdsp2));
        assertEquals("agc1", spxdsp.bEnableAGC, spxdsp2.bEnableAGC);
        assertEquals("agc2", spxdsp.nGainLevel, spxdsp2.nGainLevel);
        assertEquals("agc3", spxdsp.nMaxIncDBSec, spxdsp2.nMaxIncDBSec);
        assertEquals("agc4", spxdsp.nMaxDecDBSec, spxdsp2.nMaxDecDBSec);
        assertEquals("agc5", spxdsp.nMaxGainDB, spxdsp2.nMaxGainDB);
        assertEquals("agc6", spxdsp.bEnableDenoise, spxdsp2.bEnableDenoise);
        assertEquals("agc7", spxdsp.nMaxNoiseSuppressDB, spxdsp2.nMaxNoiseSuppressDB);
        assertEquals("agc8", spxdsp.nEchoSuppress, spxdsp2.nEchoSuppress);
        assertEquals("agc9", spxdsp.nEchoSuppressActive, spxdsp2.nEchoSuppressActive);
    }

    protected static void connect(TeamTalkBase ttclient)
    {
        assertTrue("connect call", ttclient.connect(IPADDR, TCPPORT, UDPPORT, 0, 0, ENCRYPTED));

        assertTrue("wait connect", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CON_SUCCESS, 1000));
    }

    protected static void login(TeamTalkBase ttclient, String nick, String username, String passwd)
    {
        int cmdid = ttclient.doLogin(nick, username, passwd);
        assertTrue("do login", cmdid > 0);

        TTMessage msg = new TTMessage();
        assertTrue("wait login", waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_MYSELF_LOGGEDIN, DEF_WAIT, msg));

        UserAccount account = msg.useraccount;
        assertEquals("username set", username, account.szUsername);
        //Assert.AreEqual(passwd, account.szPassword, "password set");
        assertTrue("Wait login complete", waitCmdComplete(ttclient, cmdid, 1000));
        assertTrue(hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_AUTHORIZED));
    }

    protected void makeUserAccount(String nickname, String username, String password, int userrights)
    {
        TeamTalkBase ttclient = newClientInstance();
        connect(ttclient);
        login(ttclient, nickname, ADMIN_USERNAME, ADMIN_PASSWORD);
        UserAccount useraccount = new UserAccount();
        useraccount.szUsername = username;
        useraccount.szPassword = password;
        useraccount.uUserRights = userrights;
        useraccount.uUserType = UserType.USERTYPE_DEFAULT;
        assertTrue(waitCmdSuccess(ttclient, ttclient.doNewUserAccount(useraccount), DEF_WAIT));
        assertTrue(ttclient.disconnect());
    }

    protected static void joinRoot(TeamTalkBase ttclient)
    {
        assertTrue("Auth ok", hasFlag(ttclient.getFlags(), ClientFlag.CLIENT_AUTHORIZED));

        assertTrue("root exists", ttclient.getRootChannelID() > 0);

        int cmdid = ttclient.doJoinChannelByID(ttclient.getRootChannelID(), "");
        
        assertTrue("do join root", cmdid > 0);

        assertTrue("Wait join complete", waitCmdComplete(ttclient, cmdid, 1000));
        
        assertEquals(ttclient.getMyChannelID(), ttclient.getRootChannelID());
    }

    
    protected static boolean waitForEvent(TeamTalkBase ttclient, int nClientEvent, int waittimeout, TTMessage msg)
    {
        long start = System.currentTimeMillis();
        TTMessage tmp = new TTMessage();
        while (ttclient.getMessage(tmp, waittimeout) && tmp.nClientEvent != nClientEvent)
        {
            if(DEBUG_OUTPUT) {
                System.out.println(System.currentTimeMillis() + " #" + ttclient.getMyUserID() + ": " + tmp.nClientEvent);
                if(tmp.nClientEvent == ClientEvent.CLIENTEVENT_CMD_ERROR) {
                    System.out.println("Command error: " + tmp.clienterrormsg.szErrorMsg);
                }
            }
            if(System.currentTimeMillis() - start >= waittimeout)
                break;
        }

        if (tmp.nClientEvent == nClientEvent)
        {
            if (DEBUG_OUTPUT)
                System.out.println(System.currentTimeMillis() + " #" + ttclient.getMyUserID() + ": " + tmp.nClientEvent);

            msg.nClientEvent = tmp.nClientEvent;
            msg.ttType = tmp.ttType;
            msg.nSource = tmp.nSource;

            msg.bActive = tmp.bActive;
            msg.channel = tmp.channel;
            msg.clienterrormsg = tmp.clienterrormsg;
            msg.desktopinput = tmp.desktopinput;
            msg.filetransfer = tmp.filetransfer;
            msg.mediafileinfo = tmp.mediafileinfo;
            msg.nBytesRemain = tmp.nBytesRemain;
            msg.nPayloadSize = tmp.nPayloadSize;
            msg.nStreamID = tmp.nStreamID;
            msg.remotefile = tmp.remotefile;
            msg.serverproperties = tmp.serverproperties;
            msg.serverstatistics = tmp.serverstatistics;
            msg.textmessage = tmp.textmessage;
            msg.user = tmp.user;
            msg.useraccount = tmp.useraccount;
            msg.banneduser = tmp.banneduser;
            msg.nStreamType = tmp.nStreamType;
            //if assert fails it's because the TTType isn't handled here
            assertTrue(tmp.ttType <= TTType.__STREAMTYPE);
        }
        return tmp.nClientEvent == nClientEvent;
    }
    

    protected static boolean waitForEvent(TeamTalkBase ttclient, int nClientEvent, int waittimeout)
    {
        TTMessage msg = new TTMessage();
        return waitForEvent(ttclient, nClientEvent, waittimeout, msg);
    }

    protected static boolean waitCmdComplete(TeamTalkBase ttclient, int cmdid, int waittimeout)
    {
        TTMessage msg = new TTMessage();

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_PROCESSING, waittimeout, msg))
        {
            if (msg.nSource == cmdid && msg.bActive == false)
                return true;
        }
        return false;
    }
    
    protected static boolean waitCmdSuccess(TeamTalkBase ttclient, int cmdid, int waittimeout)
    {
        TTMessage msg = new TTMessage();

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_SUCCESS, waittimeout, msg))
        {
            if (msg.nSource == cmdid)
                return true;
        }

        return false;
    }

    protected static boolean waitCmdError(TeamTalkBase ttclient, int cmdid, int waittimeout)
    {
        TTMessage msg = new TTMessage();

        while (waitForEvent(ttclient, ClientEvent.CLIENTEVENT_CMD_ERROR, waittimeout, msg))
        {
            if (msg.nSource == cmdid)
                return true;
        }

        return false;
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
                chan.audiocodec.opus.nApplication = OpusConstants.OPUS_APPLICATION_AUDIO;
                chan.audiocodec.opus.nChannels = 1;
                chan.audiocodec.opus.nBitRate = 64000;
                chan.audiocodec.opus.nComplexity = 5;
                chan.audiocodec.opus.nTxIntervalMSec = 20;
                chan.audiocodec.opus.nSampleRate = 48000;
                chan.audiocodec.opus.bDTX = true;
                chan.audiocodec.opus.bFEC = true;
                chan.audiocodec.opus.bVBR = true;
                chan.audiocodec.opus.bVBRConstraint = false;
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

    TeamTalkBase newClientInstance()
    {
        TeamTalkBase ttclient;
        if(PROEDITION)
            ttclient = new TeamTalk5Pro();
        else
            ttclient = new TeamTalk5();

        ttclients.add(ttclient);
        return ttclient;
    }
    
    public static String getCurrentMethod()
    {
        return Thread.currentThread().getStackTrace()[2].getMethodName();
    }

    static boolean hasFlag(int flags, int flag) {
        return (flags & flag) == flag;
    }
    
    static void printSoundDevice(SoundDevice dev) {
        System.out.println("Sound dev " + Integer.toString(dev.nDeviceID) + ":");
        System.out.println("\tName:" + dev.szDeviceName);
        System.out.println("\tInput channels: " + Integer.toString(dev.nMaxInputChannels));
        System.out.println("\tOutput channels: " + Integer.toString(dev.nMaxOutputChannels));
        System.out.print("\tInput sample rates: ");
        for(int j=0;j<dev.inputSampleRates.length;j++)
            System.out.print(Integer.toString(dev.inputSampleRates[j]) + ", ");
        System.out.print("\tOutput sample rates: ");
        for(int j=0;j<dev.outputSampleRates.length;j++)
            System.out.print(Integer.toString(dev.outputSampleRates[j]) + ", ");
        System.out.println("");
        System.out.println("\tDefault sample rate: " + Integer.toString(dev.nDefaultSampleRate));
    }    
}
