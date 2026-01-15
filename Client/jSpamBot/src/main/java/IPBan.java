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
import java.time.Duration;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.util.Vector;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

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

    static void setBanTime(BannedUser ban) {
        ZonedDateTime now = ZonedDateTime.now(ZoneId.of("UTC"));
        ban.szBanTime = String.format("%04d/%02d/%02d %02d:%02d",
                now.getYear(),
                now.getMonthValue(),
                now.getDayOfMonth(),
                now.getHour(),
                now.getMinute());
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

    public void addLocalBan(BannedUser ban, TeamTalkBase ttinst) {
        UserAccount ua = new UserAccount();
        ttinst.getMyUserAccount(ua);
        ban.szOwner = ua.szUsername;
        setBanTime(ban);
        addRemoteBan(ban);
    }

    public void addRemoteBan(BannedUser ban) {
        addBan(ban);
    }

    private void removeBan(BannedUser ban) {
        this.serverbans.remove(ban);
    }

    public int syncBans(TeamTalkBase ttinst) {
        // WebLogin name (spambot@bearware.dk)
        UserAccount ua = new UserAccount();
        ttinst.getMyUserAccount(ua);

        // make list of network bans created by spambot
        Vector<BannedUser> mybans = new Vector<>();
        Vector<String> ipaddrs = new Vector<>();
        for (BannedUser b : this.serverbans) {
            if (b.uBanTypes == SPAMBOT_NETWORK_BAN_TYPE && b.szOwner.equals(ua.szUsername)) {
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
                return ttinst.doUnBanUserEx(b);
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
                int cmdid = ttinst.doBan(b);
                if (cmdid > 0) {
                    addLocalBan(b, ttinst);
                }
                return cmdid;
            }
        }

        // remove expired user bans
        if (!this.banDuration.isZero()) {
            ZonedDateTime bannedBefore = ZonedDateTime.now(ZoneId.of("UTC")).minus(this.banDuration);
            for (BannedUser b : getBannedBySpamBot(ua, SPAMBOT_USER_BAN_TYPE, bannedBefore)) {
                removeBan(b);
                this.logger.info(String.format("Removed expired ban: 0x%x IP: %s Username: %s Channel: %s",
                                               b.uBanTypes, b.szIPAddress, b.szUsername, b.szChannelPath));
                return ttinst.doUnBanUserEx(b);
            }
        }

        return 0;
    }

    private Vector<BannedUser> getBannedBySpamBot(UserAccount ua, int uBanTypes, ZonedDateTime before) {
        Vector<BannedUser> result = new Vector<>();
        for (BannedUser b : getBannedUsersBefore(before)) {
            if (b.szOwner.equals(ua.szUsername) && b.uBanTypes == uBanTypes) {
                result.add(b);
            }
        }
        return result;
    }

    private Vector<BannedUser> getBannedUsersBefore(ZonedDateTime zdt) {
        Vector<BannedUser> result = new Vector<>();
        for (BannedUser b : this.serverbans) {
            Pattern pattern = Pattern.compile("^(\\d+)/(\\d+)/(\\d+) (\\d+):(\\d+)$");
            Matcher datematch = pattern.matcher(b.szBanTime);
            if (datematch.find()) {
                ZonedDateTime bantime = ZonedDateTime.of(Integer.parseInt(datematch.group(1)), //year
                                                         Integer.parseInt(datematch.group(2)), //month
                                                         Integer.parseInt(datematch.group(3)), //day
                                                         Integer.parseInt(datematch.group(4)), //hour
                                                         Integer.parseInt(datematch.group(5)), //minute
                                                         0, 0, //seconds, nanos
                                                         ZoneId.of("UTC"));
                if (bantime.isBefore(zdt)) {
                    result.add(b);
                }
            }
            else {
                this.logger.info(String.format("Date format %s does not match: ", b.szBanTime));
            }
        }
        return result;
    }
}
