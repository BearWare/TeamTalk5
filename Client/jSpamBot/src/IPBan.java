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

import dk.bearware.BannedUser;
import dk.bearware.BanType;
import dk.bearware.TeamTalkBase;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.Vector;

public class IPBan {
    // bans already on server (doListBans() result)
    Vector<BannedUser> serverbans = new Vector<>();
    // IPs/networks to ban (loaded from file)
    Vector<String> networks;
    int networkindex = -1;

    public IPBan(Vector<String> networks) {
        this.networks = networks;
    }

    public static Vector<String> loadFile(String filename) {
        Vector<String> networks = new Vector<>();
        File file = new File(filename);
        if (file.exists()) {
            try (BufferedReader br = new BufferedReader(new FileReader(file))) {
                String line;
                while ((line = br.readLine()) != null) {
                    networks.add(line);
                }
            }
            catch(IOException e) {
                System.err.println("Failed to load "+filename+": " + e);
            }
        }
        return networks;
    }

    public void addBan(BannedUser ban) {
        serverbans.add(ban);
    }

    public int syncBans(TeamTalkBase ttinst) {
        Vector<String> remote = new Vector<>();
        for (BannedUser b : serverbans) {
            if ((b.uBanTypes & BanType.BANTYPE_IPADDR) == BanType.BANTYPE_IPADDR)
                remote.add(b.szIPAddress);
        }

        for (++this.networkindex; this.networkindex < networks.size();++this.networkindex) {
            String ipaddr = networks.elementAt(this.networkindex);
            if (!remote.contains(ipaddr)) {
                BannedUser b = new BannedUser();
                b.szIPAddress = ipaddr;
                b.uBanTypes = BanType.BANTYPE_IPADDR;
                return ttinst.doBan(b);
            }
        }
        return 0;
    }
}
