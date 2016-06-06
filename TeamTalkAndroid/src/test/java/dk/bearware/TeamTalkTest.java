package dk.bearware;

import org.junit.Test;

import java.util.regex.Pattern;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class TeamTalkTest extends TeamTalkTestCaseBase {

    @Test
    public void test_mytest() {
        TeamTalkBase ttclient = newClientInstance();
        assertTrue(ttclient != null);
    }
}