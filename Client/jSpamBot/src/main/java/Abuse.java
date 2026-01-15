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

import java.time.Duration;
import java.time.Instant;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

public class Abuse {

    Map< String, Vector<Instant> > iplogins = new HashMap<>();
    Map< String, Vector<Instant> > ipjoins = new HashMap<>();
    Map< String, Vector<Instant> > ipkicks = new HashMap<>();

    TimeProvider time;
    int ipJoinCount, ipLoginCount, ipKickCount;
    Duration banDuration;

    public Abuse(TimeProvider time, int ipJoinCount, int ipLoginCount, int ipKickCount, Duration banDuration) {
        this.time = time;
        this.ipJoinCount = ipJoinCount;
        this.ipLoginCount = ipLoginCount;
        this.ipKickCount = ipKickCount;
        this.banDuration = banDuration;
    }

    private static void inc(TimeProvider time, Map< String, Vector<Instant> > ipmap, String ipaddr) {
        Vector<Instant> timestamps = ipmap.computeIfAbsent(ipaddr, k -> new Vector<Instant>());
        timestamps.add(time.now());
    }

    public void incLogin(String ipaddr) {
        inc(time, iplogins, ipaddr);
    }

    public void incJoins(String ipaddr) {
        inc(time, ipjoins, ipaddr);
    }

    public void incKicks(String ipaddr) {
        inc(time, ipkicks, ipaddr);
    }
    
    private static void clean(TimeProvider time, Map< String, Vector<Instant> > history, Duration banDuration) {
        Vector<String> removeme = new Vector<>();
        for (String key : history.keySet()) {
            var occurTimes = history.get(key);
            while (!occurTimes.isEmpty() && Duration.between(occurTimes.firstElement(), time.now()).compareTo(banDuration) > 0) {
                occurTimes.remove(0);
            }
            if (occurTimes.isEmpty())
                removeme.add(key);
        }
        for (String key : removeme)
            history.remove(key);
    }

    public void removeIpaddr(String ipaddr) {
        iplogins.remove(ipaddr);
        ipjoins.remove(ipaddr);
        ipkicks.remove(ipaddr);
    }

    public boolean checkLoginAbuse(String ipaddr) {
        clean(time, iplogins, this.banDuration);
        return iplogins.get(ipaddr) != null ? iplogins.get(ipaddr).size() >= ipLoginCount : false;
    }

    public boolean checkJoinAbuse(String ipaddr) {
        clean(time, ipjoins, this.banDuration);
        return ipjoins.get(ipaddr) != null ? ipjoins.get(ipaddr).size() >= ipJoinCount : false;
    }

    public boolean checkKickAbuse(String ipaddr) {
        clean(time, ipkicks, this.banDuration);
        return ipkicks.get(ipaddr) != null ? ipkicks.get(ipaddr).size() >= ipKickCount : false;
    }
}
