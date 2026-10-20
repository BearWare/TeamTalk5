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

package dk.bearware;

import org.junit.jupiter.api.extension.BeforeTestExecutionCallback;
import org.junit.jupiter.api.extension.AfterTestExecutionCallback;
import org.junit.jupiter.api.extension.TestWatcher;
import org.junit.jupiter.api.extension.ExtensionContext;
import java.util.Optional;

/**
 * JUnit 5 Extension that replaces the JUnit 4 @Rule Stopwatch.
 * Measures test execution time and prints the duration after each test completes.
 */
public class TestTimingExtension implements BeforeTestExecutionCallback, AfterTestExecutionCallback, TestWatcher {

    private static final String START_TIME_KEY = "TestTimingExtension:startTime";

    @Override
    public void beforeTestExecution(ExtensionContext context) throws Exception {
        context.getStore(ExtensionContext.Namespace.create(TestTimingExtension.class))
                .put(START_TIME_KEY, System.nanoTime());
    }

    @Override
    public void afterTestExecution(ExtensionContext context) throws Exception {
        long startTime = context.getStore(ExtensionContext.Namespace.create(TestTimingExtension.class))
                .get(START_TIME_KEY, Long.class);
        long endTime = System.nanoTime();
        long durationNanos = endTime - startTime;
        long durationMillis = durationNanos / 1000000;
        System.out.println(context.getDisplayName() + " Duration: " + durationMillis + " msec");
    }

    @Override
    public void testDisabled(ExtensionContext context, Optional<String> reason) {
        // Don't print anything for disabled tests
    }
}
