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
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.util.Vector;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class IPBan {
    // bans already on server (doListBans() result)
    Vector<BannedUser> serverbans = new Vector<>();
    // IPs/networks to ban (loaded from file)
    Vector<String> networks;
    int networkindex = -1;
    int banDurationSecs;
    Logger logger;

    public IPBan(Vector<String> networks, int banDurationSecs, Logger log) {
        this.networks = networks;
        this.banDurationSecs = banDurationSecs;
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
        if (ban.szBanTime.isEmpty()) {
            ZonedDateTime now = ZonedDateTime.now(ZoneId.of("UTC"));
            ban.szBanTime = String.format("%04d/%02d/%02d %02d:%02d",
                                          now.getYear(),
                                          now.getMonthValue(),
                                          now.getDayOfMonth(),
                                          now.getHour(),
                                          now.getMinute());
        }
        serverbans.add(ban);
    }

    private void removeBan(BannedUser ban) {
        this.serverbans.remove(ban);
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
                this.logger.info(String.format("Removed missing network ban: 0x%x IP: %s Username: %s Channel: %s",
                                               b.uBanTypes, b.szIPAddress, b.szUsername, b.szChannelPath));
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
                this.logger.info(String.format("Added new network ban: 0x%x IP: %s Username: %s Channel: %s",
                                               b.uBanTypes, b.szIPAddress, b.szUsername, b.szChannelPath));
                return ttinst.doBan(b);
            }
        }

        // remove expired bans
        if (this.banDurationSecs > 0) {
            ZonedDateTime bannedBefore = ZonedDateTime.now(ZoneId.of("UTC")).minusSeconds(this.banDurationSecs);
            for (BannedUser b : getBannedBySpamBot(ua, BanType.BANTYPE_IPADDR | BanType.BANTYPE_USERNAME,
                                                   bannedBefore)) {
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
            if (b.szOwner.equals(ua.szUsername) &&
                b.uBanTypes == uBanTypes) {
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
