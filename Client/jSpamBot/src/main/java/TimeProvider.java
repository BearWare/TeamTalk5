import java.time.Instant;

public interface TimeProvider {
    Instant now();
}
