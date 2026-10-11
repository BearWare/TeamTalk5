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
import dk.bearware.UserAccount;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.time.Duration;
import java.time.Instant;
import java.util.Vector;
import java.util.logging.Logger;

public class IPBan {
    TimeProvider time;
    // bans already on server (doListBans() result)
    Vector<BannedUser> serverbans = new Vector<>();
    // IPs/networks to ban (loaded from file)
    Vector<String> networks;
    int networkindex = -1;
    Duration banDuration;
    Logger logger;
    // Ban types made by spambot (ban network and ban user)
    final int SPAMBOT_NETWORK_BAN_TYPE = BanType.BANTYPE_IPADDR;
    final int SPAMBOT_USER_BAN_TYPE = BanType.BANTYPE_IPADDR | BanType.BANTYPE_USERNAME;

    public IPBan(TimeProvider time, Vector<String> networks, Duration banDuration, Logger log) {
        this.time = time;
        this.networks = networks;
        this.banDuration = banDuration;
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

    private void addBan(BannedUser ban) {
        if (ban.szBanTime.isEmpty()) {
            logger.severe("Ban time is not set on ban owner " + ban.szOwner + " IP addr: " + ban.szIPAddress);
        }
        if (ban.szOwner.isEmpty()) {
            logger.severe("Ban owner is not set on ban type " + ban.uBanTypes + " IP addr: " + ban.szIPAddress);
        }
        if (ban.uBanTypes == BanType.BANTYPE_NONE) {
            logger.severe("Ban type is not set, IP addr: " + ban.szIPAddress);
        }
        serverbans.add(ban);
    }

    // a "local ban" is a ban issued by the spambot
    public void addLocalBan(BannedUser ban, BanProvider banner) {
        ban.szOwner = banner.banOwner().szUsername;
        ban.szBanTime = TimeUtil.instantToBanTime(time.now());
        addRemoteBan(ban);
    }

    // a "remote ban" that exists on the remote server
    public void addRemoteBan(BannedUser ban) {
        addBan(ban);
    }

    private void removeBan(BannedUser ban) {
        this.serverbans.remove(ban);
    }

    public int syncBans(BanProvider banner) {

        // make list of network bans created by spambot
        Vector<BannedUser> mybans = new Vector<>();
        Vector<String> ipaddrs = new Vector<>();
        for (BannedUser b : this.serverbans) {
            if (b.uBanTypes == SPAMBOT_NETWORK_BAN_TYPE && b.szOwner.equals(banner.banOwner().szUsername)) {
                mybans.add(b);
                ipaddrs.add(b.szIPAddress);
            }
        }

        //remove network bans no longer found in 'networks'
        for (BannedUser b : mybans) {
            if (!this.networks.contains(b.szIPAddress)) {
                this.serverbans.remove(b);
                this.logger.info(String.format("Removed missing network ban: 0x%x IP: %s Username: %s Channel: %s",
                                               b.uBanTypes, b.szIPAddress, b.szUsername, b.szChannelPath));
                return banner.issueRemoveBan(b);
            }
        }

        // submit new network bans not already on server
        for (++this.networkindex; this.networkindex < networks.size();++this.networkindex) {
            String ipaddr = networks.elementAt(this.networkindex);
            if (!ipaddrs.contains(ipaddr)) {
                BannedUser b = new BannedUser();
                b.szIPAddress = ipaddr;
                b.uBanTypes = SPAMBOT_NETWORK_BAN_TYPE;
                this.logger.info(String.format("Added new network ban: 0x%x IP: %s Username: %s Channel: %s",
                                               b.uBanTypes, b.szIPAddress, b.szUsername, b.szChannelPath));
                // store in local ban list
                int cmdid = banner.issueAddBan(b);
                if (cmdid > 0) {
                    addLocalBan(b, banner);
                }
                return cmdid;
            }
        }

        // remove expired user bans
        if (!this.banDuration.isZero()) {
            Instant bannedBefore = time.now().minus(this.banDuration);
            for (BannedUser b : getBannedBySpamBot(banner.banOwner(), SPAMBOT_USER_BAN_TYPE, bannedBefore)) {
                removeBan(b);
                this.logger.info(String.format("Removed expired ban: 0x%x IP: %s Username: %s Channel: %s",
                                               b.uBanTypes, b.szIPAddress, b.szUsername, b.szChannelPath));
                return banner.issueRemoveBan(b);
            }
        }

        return 0;
    }

    private Vector<BannedUser> getBannedBySpamBot(UserAccount ua, int uBanTypes, Instant before) {
        Vector<BannedUser> result = new Vector<>();
        for (BannedUser b : getBannedUsersBefore(before)) {
            if (b.szOwner.equals(ua.szUsername) && b.uBanTypes == uBanTypes) {
                result.add(b);
            }
        }
        return result;
    }

    private Vector<BannedUser> getBannedUsersBefore(Instant when) {
        Vector<BannedUser> result = new Vector<>();
        for (BannedUser b : this.serverbans) {
            if (TimeUtil.banTimeToInstant(b.szBanTime).isBefore(when)) {
                result.add(b);
            }
        }
        return result;
    }
}
