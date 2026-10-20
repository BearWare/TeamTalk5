import org.junit.jupiter.api.Test;

import dk.bearware.BanType;
import dk.bearware.BannedUser;
import dk.bearware.UserAccount;

import java.io.IOException;
import java.time.Duration;
import java.time.Instant;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.logging.*;
import java.util.Vector;

import static org.junit.jupiter.api.Assertions.*;

class IPBanTest {

    static class MyTimeProvider implements TimeProvider {
        private Instant now = Instant.now();

        public void setNow(Instant now) {
            this.now = now;
        }

        @Override
        public Instant now() {
            return this.now;
        }
    }

    @Test
    public void testNetworkBans() {
        MyTimeProvider time = new MyTimeProvider();
        final var BANDURATION = Duration.ofSeconds(60);
        final UserAccount myuseraccount = new UserAccount();
        myuseraccount.szUsername = "spambot@bearware.dk";
        Vector<String> networkBans = new Vector<>();
        final String BANNED_NETWORK1 = "129.2.4.0/24";
        final String BANNED_NETWORK2 = "192.5.0.7/26";
        networkBans.add(BANNED_NETWORK1);
        networkBans.add(BANNED_NETWORK2);
        IPBan ipban = new IPBan(time, networkBans, BANDURATION, Logger.getLogger("dk.bearware"));

        Vector<BannedUser> networkBannedUsers = new Vector<>();
        Vector<BannedUser> removedBannedUsers = new Vector<>();

        BanProvider banner = new BanProvider() {
            int cmdid = 0;
            public int issueAddBan(BannedUser ban) {
                networkBannedUsers.add(ban);
                return ++cmdid;
            }

            public int issueRemoveBan(BannedUser ban) {
                removedBannedUsers.add(ban);
                return ++cmdid;
            }

            public UserAccount banOwner() {
                return myuseraccount;
            }
        };
        assertEquals(1, ipban.syncBans(banner));
        assertEquals(1, networkBannedUsers.size());
        assertEquals(2, ipban.syncBans(banner));
        assertEquals(2, networkBannedUsers.size());
        assertEquals(0, ipban.syncBans(banner));
        assertEquals(BANNED_NETWORK1, networkBannedUsers.elementAt(0).szIPAddress);
        assertEquals(myuseraccount.szUsername, networkBannedUsers.elementAt(0).szOwner);
        assertEquals(BanType.BANTYPE_IPADDR, networkBannedUsers.elementAt(0).uBanTypes);
        assertEquals(BANNED_NETWORK2, networkBannedUsers.elementAt(1).szIPAddress);
        assertEquals(myuseraccount.szUsername, networkBannedUsers.elementAt(1).szOwner);
        assertEquals(BanType.BANTYPE_IPADDR, networkBannedUsers.elementAt(1).uBanTypes);
        assertEquals(0, removedBannedUsers.size());

        BannedUser newBan = new BannedUser();
        newBan.szIPAddress = "45.6.7.8";
        newBan.szOwner = "someother";
        newBan.szBanTime = TimeUtil.instantToBanTime(time.now().minus(BANDURATION));
        newBan.uBanTypes = BanType.BANTYPE_CHANNEL | BanType.BANTYPE_IPADDR;
        newBan.szChannelPath = "/some channel";
        ipban.addRemoteBan(newBan);
        assertEquals(0, ipban.syncBans(banner));
        assertEquals(2, networkBannedUsers.size());
        assertEquals(0, removedBannedUsers.size());

        time.setNow(time.now().plus(BANDURATION));
        assertEquals(0, ipban.syncBans(banner));
        assertEquals(2, networkBannedUsers.size());
        assertEquals(0, removedBannedUsers.size());
    }

    @Test
    public void testRemoteBanExpire() {
        MyTimeProvider time = new MyTimeProvider();
        final var BANDURATION = Duration.ofSeconds(60);
        final UserAccount myuseraccount = new UserAccount();
        myuseraccount.szUsername = "spambot@bearware.dk";
        Vector<String> networkBans = new Vector<>();
        IPBan ipban = new IPBan(time, networkBans, BANDURATION, Logger.getLogger("dk.bearware"));

        Vector<BannedUser> bannedUsers = new Vector<>();
        Vector<BannedUser> removedBannedUsers = new Vector<>();

        BanProvider banner = new BanProvider() {
            int cmdid = 0;

            public int issueAddBan(BannedUser ban) {
                bannedUsers.add(ban);
                return ++cmdid;
            }

            public int issueRemoveBan(BannedUser ban) {
                removedBannedUsers.add(ban);
                return ++cmdid;
            }

            public UserAccount banOwner() {
                return myuseraccount;
            }
        };
        assertEquals(0, ipban.syncBans(banner));

        // this ban should be removed by syncBans()
        BannedUser expiredBan = new BannedUser();
        expiredBan.szIPAddress = "45.6.7.8";
        expiredBan.szBanTime = TimeUtil.instantToBanTime(time.now().minus(BANDURATION).minus(BANDURATION));
        expiredBan.szOwner = myuseraccount.szUsername;
        expiredBan.uBanTypes = BanType.BANTYPE_IPADDR | BanType.BANTYPE_USERNAME;
        expiredBan.szUsername = "expiredBan";
        ipban.addRemoteBan(expiredBan);

        // this ban should NOT be removed by syncBans()
        BannedUser activeBan = new BannedUser();
        activeBan.szIPAddress = "45.6.7.9";
        activeBan.szBanTime = TimeUtil.instantToBanTime(time.now());
        activeBan.szOwner = myuseraccount.szUsername;
        activeBan.uBanTypes = BanType.BANTYPE_IPADDR | BanType.BANTYPE_USERNAME;
        activeBan.szUsername = "activeBan";
        ipban.addRemoteBan(activeBan);

        assertEquals(1, ipban.syncBans(banner));
        assertEquals(removedBannedUsers.firstElement().uBanTypes, expiredBan.uBanTypes);
        assertEquals(removedBannedUsers.firstElement().szIPAddress, expiredBan.szIPAddress);
        assertEquals(removedBannedUsers.firstElement().szUsername, expiredBan.szUsername);

        assertEquals(0, bannedUsers.size());
        assertEquals(0, ipban.syncBans(banner));

        time.setNow(time.now().plus(BANDURATION).plus(BANDURATION));
        assertEquals(2, ipban.syncBans(banner));
        assertEquals(removedBannedUsers.lastElement().uBanTypes, activeBan.uBanTypes);
        assertEquals(removedBannedUsers.lastElement().szIPAddress, activeBan.szIPAddress);
        assertEquals(removedBannedUsers.lastElement().szUsername, activeBan.szUsername);
    }
}
