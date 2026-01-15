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

import dk.bearware.TeamTalk5;
import java.io.IOException;
import java.net.URISyntaxException;
import java.util.Vector;
import java.util.logging.FileHandler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;
import javax.xml.xpath.XPathExpressionException;

public class Main {

    public static void main(String[] args) throws IOException,
        InterruptedException, URISyntaxException, XPathExpressionException {

        System.out.println("TeamTalk 5 SpamBot for Java");

        Logger logger = Logger.getLogger("dk.bearware");
        FileHandler fh = new FileHandler("spambot.log");
        fh.setFormatter(new SimpleFormatter());
        logger.addHandler(fh);
        logger.setLevel(Level.ALL);
        logger.setUseParentHandlers(false);

        logger.info("Starting SpamBot");

        String username, passwd;
        username = System.getProperty("dk.bearware.username");
        if (username == null)
            username = getInput("Type username", "spambot");
        passwd = System.getProperty("dk.bearware.password");
        if (passwd == null)
            passwd = new String(System.console().readPassword());

        int iplogins = Integer.parseInt(System.getProperty("dk.bearware.iplogincount", "10"));
        int ipjoins = Integer.parseInt(System.getProperty("dk.bearware.ipjoinscount", "10"));
        int ipkicks = Integer.parseInt(System.getProperty("dk.bearware.ipkickscount", "10"));
        int ipcmdduration = Integer.parseInt(System.getProperty("dk.bearware.ipcmdduration", "60"));
        int ipv4banprefix = Integer.parseInt(System.getProperty("dk.bearware.ipv4banprefix", "32"));
        int ipv6banprefix = Integer.parseInt(System.getProperty("dk.bearware.ipv6banprefix", "128"));
        String abuseIPDBKey = System.getProperty("dk.bearware.abuseipdbkey", "");
        int abuseIPDBTotalReports = Integer.parseInt(System.getProperty("dk.bearware.abuseipdbtotalreports", "2"));
        int abuseIPDBDistinctUsers = Integer.parseInt(System.getProperty("dk.bearware.abuseipdbdistinctusers", "2"));
        int abuseIPDBConfidenceScore = Integer.parseInt(System.getProperty("dk.bearware.abuseipdbconfidencescore", "2"));
        int banDurationSeconds = Integer.parseInt(System.getProperty("dk.bearware.bandurationsecs", "0"));

        TeamTalk5.loadLibrary();
        String regname = System.getProperty("dk.bearware.regname", "");
        String regkey = System.getProperty("dk.bearware.regkey", "");
        TeamTalk5.setLicenseInformation(regname, regkey);

        BadWords badwords = new BadWords(logger);
        badwords.loadFile("", "badwords.txt");
        badwords.loadFile("english", "badwords.txt");
        badwords.loadFile("french", "badwords_french.txt");

        var bannetworks = IPBan.loadFile("vpnips.txt", logger);

        AbuseDB abusedb = new AbuseDB(abuseIPDBKey, abuseIPDBTotalReports,
                                      abuseIPDBDistinctUsers, abuseIPDBConfidenceScore,
                                      logger);

        var sessions = new Vector<SpamBotSession>();
        var lastServers = new Vector<TeamTalkServer>();
        var serverlistUpdateTimeout = System.nanoTime();
        var connectionUpdateTimeout = System.nanoTime();
        while (true) {

            // update list of spambot servers
            if (System.nanoTime() >= serverlistUpdateTimeout) {
                logger.info("Updating server list...");
                var servers = getServerList();
                if (servers.size() == 0) {
                    try {
                        servers = new WebLogin(username, passwd, logger).getServerList();
                    }
                    catch (IOException e) {
                        logger.severe("Failed to retrieve server list: " + e);
                    }
                }

                if (!lastServers.equals(servers)) {
                    logger.info("Dirty server list. Updating...");
                    for (var session : sessions) {
                        session.close();
                    }
                    sessions.clear();
                    for (var server : servers) {
                        try {
                            var spambot = new SpamBotSession(server,
                                                             new WebLogin(username, passwd, logger),
                                                             new IPBan(bannetworks, banDurationSeconds, logger),
                                                             badwords,
                                                             new Abuse(ipjoins, iplogins, ipkicks, ipcmdduration),
                                                             abusedb, ipv4banprefix, ipv6banprefix, logger);
                            sessions.add(spambot);
                        }
                        catch (IOException e) {
                            logger.severe("Failed to add spambot: " + e);
                        }
                    }
                    lastServers = servers;
                }
                // get server list every minute
                serverlistUpdateTimeout = System.nanoTime() + 1 * 60 * 1000000000l;
            }

            // initiate connection (if not already open)
            if (System.nanoTime() >= connectionUpdateTimeout) {
                logger.info("Updating connections...");

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
        System.out.println("Got");
        if(input.length() == 0)
            return def;
        return input;
    }

    static String getInput(String prompt, String def) {
        System.out.print(prompt);
        return getInput(def);
    }

    static Vector<TeamTalkServer> getServerList() {
        Vector<TeamTalkServer> servers = new Vector<>();
        String ipaddr = System.getProperty("dk.bearware.test.server.ipaddr");
        if (ipaddr != null) {
            int tcpport = Integer.parseInt(System.getProperty("dk.bearware.test.server.tcpport"));
            int udpport = Integer.parseInt(System.getProperty("dk.bearware.test.server.udpport"));
            int encrypted = Integer.parseInt(System.getProperty("dk.bearware.test.server.encrypted"));
            TeamTalkServer s = new TeamTalkServer(ipaddr, tcpport, udpport, encrypted != 0);
            servers.add(s);
        }
        return servers;
    }
}
