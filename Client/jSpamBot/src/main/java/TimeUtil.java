import java.time.Instant;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class TimeUtil {
    public static String instantToBanTime(Instant instant) {
        ZonedDateTime zdt = instant.atZone(ZoneId.of("UTC"));
        return String.format("%04d/%02d/%02d %02d:%02d",
                zdt.getYear(),
                zdt.getMonthValue(),
                zdt.getDayOfMonth(),
                zdt.getHour(),
                zdt.getMinute());
    }
    public static Instant banTimeToInstant(String banTime) {
        Pattern pattern = Pattern.compile("^(\\d+)/(\\d+)/(\\d+) (\\d+):(\\d+)$");
        Matcher datematch = pattern.matcher(banTime);
        if (datematch.find()) {
            ZonedDateTime zdt = ZonedDateTime.of(Integer.parseInt(datematch.group(1)), //year
                    Integer.parseInt(datematch.group(2)), //month
                    Integer.parseInt(datematch.group(3)), //day
                    Integer.parseInt(datematch.group(4)), //hour
                    Integer.parseInt(datematch.group(5)), //minute
                    0, 0, //seconds, nanos
                    ZoneId.of("UTC"));
            return zdt.toInstant();
        }
        return Instant.MIN;
    }
}
