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

public abstract class TeamTalkSrv /* Java 1.7 implements AutoCloseable */ {

    long ttsInst = 0;

    ServerCallback serverCallback;
    ServerLogger serverLogger;

    @SuppressWarnings({"deprecation","removal"})
    @Override
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

    public native void registerServerCallback(ServerCallback callback);

    public native void registerServerLogger(ServerLogger logger);

    native long initTeamTalk();
    native void closeTeamTalk();

    public native boolean setEncryptionContext(String szCertificateFile, String szPrivateKeyFile);

    native boolean setEncryptionContextEx(EncryptionContext lpEncryptionContext);
    public boolean setEncryptionContext(EncryptionContext lpEncryptionContext) {
        return setEncryptionContextEx(lpEncryptionContext);
    }
    public native boolean runEventLoop(int pnWaitMs);

    public native int setChannelFilesRoot(String szFilesRoot, long nMaxDiskUsage, long nDefaultChannelQuota);

    public native int updateServer(ServerProperties lpServerProperties);

    public native int makeChannel(Channel lpChannel);

    public native int updateChannel(Channel lpChannel);

    public native int removeChannel(int nChannelID);

    public native int addFileToChannel(String szLocalFilePath, RemoteFile lpRemoteFile);

    public native int removeFileFromChannel(RemoteFile lpRemoteFile);

    public native int moveUser(int nUserID, Channel lpChannel);

    public native int sendTextMessage(TextMessage lpTextMessage);

    public native boolean startServer(String szBindIPAddr, int nTcpPort, int nUdpPort, boolean bEncrypted);

    public native boolean startServerSysID(String szBindIPAddr, int nTcpPort, int nUdpPort, boolean bEncrypted, String szSystemID);

    public native boolean stopServer();
}
