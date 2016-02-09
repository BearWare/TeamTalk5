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
        registerServerCallback(callback);
    }

    native void registerServerCallback(long lpTTSInstance, ServerCallback callback);
    private void registerServerCallback(ServerCallback callback) {
        registerServerCallback(ttsInst, callback);
    }

    private native long initTeamTalk();
    native void closeTeamTalk(long lpTTSInstance);
    private void closeTeamTalk() {
        closeTeamTalk(ttsInst);
    }
    native boolean setEncryptionContext(long lpTTSInstance, String szCertificateFile, String szPrivateKeyFile);
    public boolean setEncryptionContext(String szCertificateFile, String szPrivateKeyFile) {
        return setEncryptionContext(ttsInst, szCertificateFile, szPrivateKeyFile);
    }
    native boolean runEventLoop(long lpTTSInstance, int pnWaitMs);
    public boolean runEventLoop(int pnWaitMs) {
        return runEventLoop(ttsInst, pnWaitMs);
    }
    native int updateServer(long lpTTSInstance, ServerProperties lpServerProperties);
    public int updateServer(ServerProperties lpServerProperties) {
        return updateServer(ttsInst, lpServerProperties);
    }
    native int makeChannel(long lpTTSInstance, Channel lpChannel);
    public int makeChannel(Channel lpChannel) {
        return makeChannel(ttsInst, lpChannel);
    }
    native int updateChannel(long lpTTSInstance, Channel lpChannel);
    public int updateChannel(Channel lpChannel) {
        return updateChannel(ttsInst, lpChannel);
    }
    native int removeChannel(long lpTTSInstance, int nChannelID);
    public int removeChannel(int nChannelID) {
        return removeChannel(ttsInst, nChannelID);
    }
    native int moveUser(long lpTTSInstance, int nUserID, Channel lpChannel);
    public int moveUser(int nUserID, Channel lpChannel) {
        return moveUser(ttsInst, nUserID, lpChannel);
    }
    native boolean startServer(long lpTTSInstance, String szBindIPAddr, int nTcpPort, int nUdpPort, boolean bEncrypted);
    public boolean startServer(String szBindIPAddr, int nTcpPort, int nUdpPort, boolean bEncrypted) {
        return startServer(ttsInst, szBindIPAddr, nTcpPort, nUdpPort, bEncrypted);
    }
}
