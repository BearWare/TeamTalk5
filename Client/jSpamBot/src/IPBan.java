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

import dk.bearware.BanType;
import dk.bearware.BannedUser;
import dk.bearware.TeamTalkBase;
import dk.bearware.UserAccount;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.Vector;
import java.util.logging.Logger;

public class IPBan {
    // bans already on server (doListBans() result)
    Vector<BannedUser> serverbans = new Vector<>();
    // IPs/networks to ban (loaded from file)
    Vector<String> networks;
    int networkindex = -1;

    Logger logger;

    public IPBan(Vector<String> networks, Logger log) {
        this.networks = networks;
        this.logger = log;
    }

    public static Vector<String> loadFile(String filename, Logger log) {
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
                log.severe("Failed to load " + filename + ": " + e);
            }
        }
        return networks;
    }

    public void addBan(BannedUser ban) {
        serverbans.add(ban);
    }

    public int syncBans(TeamTalkBase ttinst) {
        // WebLogin name (spambot@bearware.dk)
        UserAccount ua = new UserAccount();
        ttinst.getMyUserAccount(ua);

        // make list of bans created by spambot
        Vector<BannedUser> mybans = new Vector<>();
        Vector<String> ipaddrs = new Vector<>();
        for (BannedUser b : this.serverbans) {
            if (b.uBanTypes == BanType.BANTYPE_IPADDR && b.szOwner.equals(ua.szUsername)) {
                mybans.add(b);
                ipaddrs.add(b.szIPAddress);
            }
        }

        //remove bans no longer found in 'networks'
        for (BannedUser b : mybans) {
            if (!this.networks.contains(b.szIPAddress)) {
                this.serverbans.remove(b);
                return ttinst.doUnBanUserEx(b);
            }
        }

        // submit new bans not already on server
        for (++this.networkindex; this.networkindex < networks.size();++this.networkindex) {
            String ipaddr = networks.elementAt(this.networkindex);
            if (!ipaddrs.contains(ipaddr)) {
                BannedUser b = new BannedUser();
                b.szIPAddress = ipaddr;
                b.uBanTypes = BanType.BANTYPE_IPADDR;
                return ttinst.doBan(b);
            }
        }
        return 0;
    }
}
