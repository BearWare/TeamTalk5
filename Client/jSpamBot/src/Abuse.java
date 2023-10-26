import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

public class Abuse {

    Map< String, Vector<Long> > iplogins = new HashMap<>();
    Map< String, Vector<Long> > ipjoins = new HashMap<>();

    int ipjoinCount, ipLoginCount;
    long durationSec;

    public Abuse(int ipjoinCount, int ipLoginCount, long durationSec) {
        this.ipjoinCount = ipjoinCount;
        this.ipLoginCount = ipLoginCount;
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
    }

    public boolean checkLoginAbuse(String ipaddr) {
        clean(iplogins, this.durationSec);
        return iplogins.get(ipaddr) != null ? iplogins.get(ipaddr).size() >= ipLoginCount : false;
    }

    public boolean checkJoinAbuse(String ipaddr) {
        clean(ipjoins, this.durationSec);
        return ipjoins.get(ipaddr) != null ? ipjoins.get(ipaddr).size() >= ipjoinCount : false;
    }

}
