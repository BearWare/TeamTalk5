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

package dk.bearware;

public class ServerProperties
{
    public String szServerName;
    public String szMOTD;
    public String szMOTDRaw;
    public int nMaxUsers;
    public int nMaxLoginAttempts;
    public int nMaxLoginsPerIPAddress;
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
}
