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

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

public class Abuse {

    Map< String, Vector<Long> > iplogins = new HashMap<>();
    Map< String, Vector<Long> > ipjoins = new HashMap<>();
    Map< String, Vector<Long> > ipkicks = new HashMap<>();

    int ipJoinCount, ipLoginCount, ipKickCount;
    long durationSec;

    public Abuse(int ipJoinCount, int ipLoginCount, int ipKickCount, long durationSec) {
        this.ipJoinCount = ipJoinCount;
        this.ipLoginCount = ipLoginCount;
        this.ipKickCount = ipKickCount;
        this.durationSec = durationSec;
    }

    private static void inc(Map< String, Vector<Long> > ipmap, String ipaddr) {
        Vector<Long> timestamps = ipmap.get(ipaddr);
        if (timestamps == null) {
            timestamps = new Vector<Long>();
            ipmap.put(ipaddr, timestamps);
        }
        timestamps.add(System.nanoTime());
    }

    public void incLogin(String ipaddr) {
        inc(iplogins, ipaddr);
    }

    public void incJoins(String ipaddr) {
        inc(ipjoins, ipaddr);
    }

    public void incKicks(String ipaddr) {
        inc(ipkicks, ipaddr);
    }
    
    private static void clean(Map< String, Vector<Long> > history, long durationSec) {
        Vector<String> removeme = new Vector<>();
        for (String key : history.keySet()) {
            var occurTimes = history.get(key);
            while (occurTimes.size() > 0 && occurTimes.firstElement() + durationSec * 1E9 <= System.nanoTime()) {
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
        clean(iplogins, this.durationSec);
        return iplogins.get(ipaddr) != null ? iplogins.get(ipaddr).size() >= ipLoginCount : false;
    }

    public boolean checkJoinAbuse(String ipaddr) {
        clean(ipjoins, this.durationSec);
        return ipjoins.get(ipaddr) != null ? ipjoins.get(ipaddr).size() >= ipJoinCount : false;
    }

    public boolean checkKickAbuse(String ipaddr) {
        clean(ipkicks, this.durationSec);
        return ipkicks.get(ipaddr) != null ? ipkicks.get(ipaddr).size() >= ipKickCount : false;
    }
}
