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

import org.junit.runner.JUnitCore;
import org.junit.runner.Request;
import org.junit.runner.Result;
import org.junit.runner.notification.Failure;

public class SingleJUnitTestRunner {
    public static void main(String... args) throws ClassNotFoundException {
        String[] classAndMethod = args[0].split("#");
        Request request = Request.method(Class.forName(classAndMethod[0]),
                classAndMethod[1]);

        Result result = new JUnitCore().run(request);

        for (Failure failure : result.getFailures()) {
            System.out.println(failure.toString());
        }
        System.exit(result.wasSuccessful() ? 0 : 1);
    }
}
