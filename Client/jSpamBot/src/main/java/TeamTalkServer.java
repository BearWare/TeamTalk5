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

import java.util.Objects;

public class TeamTalkServer {
    public String ipaddr;
    public int tcpport, udpport;
    public boolean encrypted;

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        TeamTalkServer that = (TeamTalkServer) o;
        return tcpport == that.tcpport && udpport == that.udpport && encrypted == that.encrypted && Objects.equals(ipaddr, that.ipaddr);
    }

    @Override
    public int hashCode() {
        return Objects.hash(ipaddr, tcpport, udpport, encrypted);
    }

    public TeamTalkServer(String ipaddr, int tcpport, int udpport, boolean encrypted) {
        this.ipaddr = ipaddr;
        this.tcpport = tcpport;
        this.udpport = udpport;
        this.encrypted = encrypted;
    }
}
