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

public abstract class TeamTalkSrv /* Java 1.7 implements AutoCloseable */ {
    
    long ttsInst = 0;

    ServerCallback serverCallback;
    ServerLogger serverLogger;

    protected void finalize( ) throws Throwable {
        closeTeamTalk();
    }

    public void close() {
        closeTeamTalk();        
    }

    public static native String getVersion();

    public static native boolean setLicenseInformation(String szRegName,
                                                       String szRegKey);

    protected TeamTalkSrv() {
    }

    protected TeamTalkSrv(ServerCallback callback) {
        serverCallback = callback;
        ttsInst = initTeamTalk();
        registerServerCallback(callback);
    }

    protected TeamTalkSrv(ServerCallback callback, ServerLogger logger) {
        serverCallback = callback;
        serverLogger = logger;
        ttsInst = initTeamTalk();
        registerServerCallback(callback);
        registerServerLogger(logger);
    }

    native void registerServerCallback(long lpTTSInstance, ServerCallback callback);
    private void registerServerCallback(ServerCallback callback) {
        registerServerCallback(ttsInst, callback);
    }

    native void registerServerLogger(long lpTTSInstance, ServerLogger logger);
    private void registerServerLogger(ServerLogger logger) {
        registerServerLogger(ttsInst, logger);
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
    native int setChannelFilesRoot(long lpTTSInstance, String szFilesRoot, long nMaxDiskUsage, long nDefaultChannelQuota);
    public int setChannelFilesRoot(String szFilesRoot, long nMaxDiskUsage, long nDefaultChannelQuota) {
        return setChannelFilesRoot(ttsInst, szFilesRoot, nMaxDiskUsage, nDefaultChannelQuota);
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
    native int addFileToChannel(long lpTTSInstance, String szLocalFilePath,
                                RemoteFile lpRemoteFile);
    public int addFileToChannel(String szLocalFilePath, RemoteFile lpRemoteFile) {
        return addFileToChannel(ttsInst, szLocalFilePath, lpRemoteFile);
    }
    native int removeFileFromChannel(long lpTTSInstance, RemoteFile lpRemoteFile);
    public int removeFileFromChannel(RemoteFile lpRemoteFile) {
        return removeFileFromChannel(ttsInst, lpRemoteFile);
    }
    native int moveUser(long lpTTSInstance, int nUserID, Channel lpChannel);
    public int moveUser(int nUserID, Channel lpChannel) {
        return moveUser(ttsInst, nUserID, lpChannel);
    }
    native boolean startServer(long lpTTSInstance, String szBindIPAddr, int nTcpPort, int nUdpPort, boolean bEncrypted);
    public boolean startServer(String szBindIPAddr, int nTcpPort, int nUdpPort, boolean bEncrypted) {
        return startServer(ttsInst, szBindIPAddr, nTcpPort, nUdpPort, bEncrypted);
    }
    native boolean stopServer(long lpTTSInstance);
    public boolean stopServer() {
        return stopServer(ttsInst);
    }
}
