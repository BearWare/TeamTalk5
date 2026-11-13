import org.junit.jupiter.api.Test;

import java.time.Instant;
import java.time.ZoneId;
import java.time.ZonedDateTime;

import static org.junit.jupiter.api.Assertions.*;

public class TimeTest {
    @Test
    public void timeUtilConvert() {
        Instant now = Instant.now();
        long nowInMinutes = now.getEpochSecond() - (now.getEpochSecond() % 60);
        now = Instant.ofEpochSecond(nowInMinutes);

        String banTime = TimeUtil.instantToBanTime(now);
        Instant was = TimeUtil.banTimeToInstant(banTime);
        assertEquals(was, now);
    }

    @Test
    public void banTimeString() {
        Instant banTime = TimeUtil.banTimeToInstant("2024/05/29 23:09");
        ZonedDateTime zdt = banTime.atZone(ZoneId.of("UTC"));
        assertEquals(2024, zdt.getYear());
        assertEquals(5, zdt.getMonthValue());
        assertEquals(29, zdt.getDayOfMonth());
        assertEquals(23, zdt.getHour());
        assertEquals(9, zdt.getMinute());
    }
}
