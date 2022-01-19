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

import javax.xml.xpath.XPathExpressionException;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.net.URISyntaxException;
import java.util.Arrays;
import java.util.Vector;

public class Main {

    public static void main(String[] args) throws IOException, InterruptedException, URISyntaxException, XPathExpressionException {

        System.out.println("TeamTalk 5 SpamBot for Java");

        String username, passwd;
        username = System.getProperty("dk.bearware.username");
        if (username == null)
            username = getInput("Type username", "spambot");
        passwd = System.getProperty("dk.bearware.password");
        if (passwd == null)
            passwd = new String(System.console().readPassword());

        WebLogin weblogin = new WebLogin(username, passwd);

        Vector<String> badwords = new Vector<>();
        File file = new File("badwords.txt");
        if (file.exists()) {
            try (BufferedReader br = new BufferedReader(new FileReader(file))) {
                String line;
                while ((line = br.readLine()) != null) {
                    badwords.addAll(Arrays.asList(line.split(",")));
                }

                while (badwords.remove(""));
            }
        }

        var sessions = new Vector<SpamBotSession>();
        var lastServers = new Vector<TeamTalkServer>();
        var serverlistUpdateTimeout = System.nanoTime();
        var connectionUpdateTimeout = System.nanoTime();
        while (true) {

            // update list of spambot servers
            if (System.nanoTime() >= serverlistUpdateTimeout) {
                System.out.println("Updating server list...");
                var servers = weblogin.getServerList();
                if (!lastServers.equals(servers)) {
                    System.out.println("Dirty server list. Updating...");
                    for (var session : sessions) {
                        session.close();
                    }
                    sessions.clear();
                    for (var server : servers) {
                        sessions.add(new SpamBotSession(server, weblogin, badwords));
                    }
                    lastServers = servers;
                }
                // get server list every minute
                serverlistUpdateTimeout = System.nanoTime() + 1 * 60 * 1000000000l;
            }

            // initiate connection (if not already open)
            if (System.nanoTime() >= connectionUpdateTimeout) {
                System.out.println("Updating connections...");
                for (var session : sessions) {
                    session.runConnectionEventLoop();
                }
                // try connecting again every 5 minutes
                connectionUpdateTimeout = System.nanoTime() + 5 * 60 * 1000000000l;
            }

            // run event loop on all active sessions
            for (var session : sessions) {
                session.runEventLoop(0);
            }

            if (sessions.isEmpty()) {
                Thread.sleep(5 * 60 * 1000);
            }
            else {
                Thread.sleep(100);
            }
        }
    }

    static String getInput(String def) {
        if(def.length()>0)
            System.out.print(" (" + def + "): ");
        else
            System.out.print(": ");
        String input = System.console().readLine();
        if(input.length() == 0)
            return def;
        return input;
    }

    static String getInput(String prompt, String def) {
        System.out.print(prompt);
        return getInput(def);
    }
}
