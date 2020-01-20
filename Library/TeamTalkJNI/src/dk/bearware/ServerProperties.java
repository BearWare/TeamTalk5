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

public class ServerProperties
{
    public String szServerName;
    public String szMOTD;
    public String szMOTDRaw;
    public int nMaxUsers;
    public int nMaxLoginAttempts;
    public int nMaxLoginsPerIPAddress;
    public int nLoginDelayMSec;
    public int nMaxVoiceTxPerSecond;
    public int nMaxVideoCaptureTxPerSecond;
    public int nMaxMediaFileTxPerSecond;
    public int nMaxDesktopTxPerSecond;
    public int nMaxTotalTxPerSecond;
    public boolean bAutoSave;
    public int nTcpPort;
    public int nUdpPort;
    public int nUserTimeout;
    public String szServerVersion;
    public String szServerProtocolVersion;
    public String szAccessToken;
}
