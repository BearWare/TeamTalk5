package dk.bearware;

/**
 * Created by bdr on 3-06-16.
 */
public class MyTest extends TeamTalkTestCaseBase {

    protected void setUp() throws Exception {
        super.setUp();

        this.ADMIN_USERNAME = "admin";
        this.ADMIN_PASSWORD = "admin";

        this.IPADDR = "127.0.0.1";
    }

    public void test_This() {
        TeamTalkBase ttclient = newClientInstance();
        assertTrue(ttclient != null);
        connect(ttclient);
    }
}
