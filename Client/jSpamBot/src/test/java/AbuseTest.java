import org.junit.jupiter.api.Test;

import java.time.Duration;
import java.time.Instant;

import static org.junit.jupiter.api.Assertions.*;

class AbuseTest {
    static class MyTimeProvider implements  TimeProvider {
        private Instant now = Instant.now();
        public void setNow(Instant now) {this.now = now;}
        @Override
        public Instant now() {
            return this.now;
        }
    }
    @Test
    public void joinAbuse() {
        TimeProvider time = Instant::now;
        Abuse a = new Abuse(time, 2, 3, 4, Duration.ofSeconds(2));
        var ipaddr = "192.160.0.1";
        a.incJoins(ipaddr);
        assertFalse(a.checkJoinAbuse(ipaddr));
        a.incJoins(ipaddr);
        assertTrue(a.checkJoinAbuse(ipaddr));
    }

    @Test
    public void joinAbuseTimeout() {
        MyTimeProvider time = new MyTimeProvider();
        Duration banDuration = Duration.ofSeconds(2);
        Abuse a = new Abuse(time, 2, 3, 4, banDuration);
        var ipaddr = "192.160.0.1";
        a.incJoins(ipaddr);
        assertFalse(a.checkJoinAbuse(ipaddr));
        time.setNow(Instant.now().plus(banDuration.plusSeconds(1)));
        a.incJoins(ipaddr);
        assertFalse(a.checkJoinAbuse(ipaddr));
    }
}