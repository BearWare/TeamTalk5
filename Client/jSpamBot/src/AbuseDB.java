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

import java.io.IOException;
import java.net.URI;
import java.net.URLEncoder;
import java.net.http.HttpClient;
import java.net.http.HttpRequest.BodyPublishers;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse.BodyHandlers;
import java.net.http.HttpResponse;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.StringJoiner;
import java.util.logging.Logger;

import org.json.JSONException;
import org.json.JSONObject;

public class AbuseDB {

    String abuseIPDBKey;
    Set<String> whitelist = new HashSet<>();
    Set<String> badlist = new HashSet<>();
    int totalReports, distinctUsers, confidenceScore;
    Logger logger;

    public AbuseDB(String apikey, int totalReports, int distinctUsers, int confidenceScore, Logger log) {
        this.abuseIPDBKey = apikey;
        this.totalReports = totalReports;
        this.distinctUsers = distinctUsers;
        this.confidenceScore = confidenceScore;
        this.logger = log;
    }

    public void addWhiteListIPAddr(String ipaddr) {
        whitelist.add(ipaddr);
    }

    public void report(String ipaddr, String comment) {
        try {
            Map<String,String> arguments = new HashMap<>();
            arguments.put("ip", ipaddr);
            arguments.put("categories", "13");
            arguments.put("comment", comment);
            StringJoiner sj = new StringJoiner("&");
            for(Map.Entry<String,String> entry : arguments.entrySet())
                sj.add(URLEncoder.encode(entry.getKey(), "UTF-8") + "="
                       + URLEncoder.encode(entry.getValue(), "UTF-8"));

            HttpRequest request = HttpRequest.newBuilder()
                .uri(URI.create("https://api.abuseipdb.com/api/v2/report"))
                .header("Accept", "application/json")
                .header("Key", this.abuseIPDBKey)
                .header("Content-Type", "application/x-www-form-urlencoded")
                .POST(BodyPublishers.ofString(sj.toString()))
                .build();

            var client = HttpClient.newHttpClient();
            var response = client.send(request, BodyHandlers.ofString());
            if (response.statusCode() >= 300) {
                logger.severe("Failed to report: " + response.body());
            }
        }
        catch (IOException | InterruptedException e) {
            logger.severe("Failed to submit IP-address: " + ipaddr);
        }
    }

    public boolean checkForReported(String ipaddr) {
        if (whitelist.contains(ipaddr))
            return false;
        if (badlist.contains(ipaddr))
            return true;

        HttpRequest request = HttpRequest.newBuilder()
            .uri(URI.create(String.format("https://api.abuseipdb.com/api/v2/check?ipAddress=%s&maxAgeInDays=%d", ipaddr, 365)))
            .header("Accept", "application/json")
            .header("Key", this.abuseIPDBKey)
            .GET()
            .build();

        try {
            var client = HttpClient.newHttpClient();
            var response = client.send(request, BodyHandlers.ofString());

            JSONObject json = new JSONObject(response.body());
            json = json.getJSONObject("data");
            if (json.getInt("totalReports") >= this.totalReports &&
                json.getInt("numDistinctUsers") >= this.distinctUsers &&
                json.getInt("abuseConfidenceScore") >= this.confidenceScore &&
                json.getBoolean("isWhitelisted") == false) {
                badlist.add(ipaddr);
            }
            else {
                whitelist.add(ipaddr);
            }
        }
        catch (IOException | InterruptedException | JSONException e) {
            logger.severe("Failed to check IP-address: " + ipaddr);
        }

        return badlist.contains(ipaddr);
    }
}
