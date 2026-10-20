import org.junit.platform.engine.TestExecutionResult;
import org.junit.platform.launcher.TestExecutionListener;
import org.junit.platform.launcher.TestIdentifier;

public class SummaryTestExecutionListener implements TestExecutionListener {
    private boolean testFailed = false;

    @Override
    public void executionFinished(TestIdentifier testIdentifier, TestExecutionResult testExecutionResult) {
        if (testExecutionResult.getStatus() == TestExecutionResult.Status.FAILED) {
            testFailed = true;
            Throwable throwable = testExecutionResult.getThrowable().orElse(null);
            if (throwable != null) {
                System.out.println(testIdentifier.getDisplayName() + " FAILED");
                throwable.printStackTrace();
            }
        }
    }

    public boolean testsFailed() {
        return testFailed;
    }
}
