/*
 * Copyright (c) 2005-2016, BearWare.dk
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

package dk.bearware;

public class TeamTalkSrv {
    
    static {
        System.loadLibrary("TeamTalk5Pro-jni");
    }

    long ttsInst = 0;

    ServerCallback serverCallback;

    protected void finalize( ) throws Throwable {
        closeTeamTalk();
    }

    public TeamTalkSrv(ServerCallback callback) {
        serverCallback = callback;
        ttsInst = initTeamTalk();
    }

    private native long initTeamTalk();
    private native void closeTeamTalk();
    public native boolean setEncryptionContext(String szCertificateFile, String szPrivateKeyFile);
    public native boolean runEventLoop(int pnWaitMs);
    public native int updateServer(ServerProperties lpServerProperties);
    public native int makeChannel(Channel lpChannel);
    public native int updateChannel(Channel lpChannel);
    public native int removeChannel(int nChannelID);
    public native int moveUser(int nUserID, Channel lpChannel);
    public native boolean startServer(String szBindIPAddr, int nTcpPort, int nUdpPort, boolean bEncrypted);
}
