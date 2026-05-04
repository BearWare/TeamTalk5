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

import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import java.io.IOException;
import java.io.StringReader;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URLEncoder;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.Vector;
import java.util.logging.Logger;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

public class WebLogin {

    String mUsername;
    String mToken;
    Logger logger;

    public String getUsername() {
        return mUsername;
    }

    public WebLogin(String username, String passwd, Logger log) throws IOException, InterruptedException, URISyntaxException, XPathExpressionException {
        this.logger = log;
        String encodedUsername = URLEncoder.encode(username, StandardCharsets.UTF_8);
        String encodedPasswd = URLEncoder.encode(passwd, StandardCharsets.UTF_8);
        String url = String.format("https://www.bearware.dk/teamtalk/weblogin.php?service=bearware&action=auth&username=%s&password=%s", encodedUsername, encodedPasswd);
        HttpRequest request = HttpRequest.newBuilder()
                .uri(new URI(url))
                .timeout(Duration.ofSeconds(20))
                .GET()
                .build();

        HttpResponse<String> response = HttpClient.newBuilder()
                .followRedirects(HttpClient.Redirect.ALWAYS)
                .build()
                .send(request, HttpResponse.BodyHandlers.ofString());

        if (response.statusCode() > 300) {
            logger.severe("Response: " + response.body());
            throw new IOException("Invalid password for " + username + ". Error code: " + response.statusCode());
        }

        XPath xPath = XPathFactory.newInstance().newXPath();
        mUsername = xPath.evaluate("/teamtalk/bearware/username", new InputSource(new StringReader(response.body())));
        mToken = xPath.evaluate("/teamtalk/bearware/token", new InputSource(new StringReader(response.body())));
    }

    public boolean registerLogin(String accesstoken) {
        String encodeAccessToken = URLEncoder.encode(accesstoken, StandardCharsets.UTF_8);
        String encodeToken = URLEncoder.encode(mToken, StandardCharsets.UTF_8);
        String encodedUsername = URLEncoder.encode(mUsername, StandardCharsets.UTF_8);
        String url = String.format("https://www.bearware.dk/teamtalk/weblogin.php?service=bearware&action=clientauth&accesstoken=%s&token=%s&username=%s", encodeAccessToken, encodeToken, encodedUsername);

        try {
            HttpRequest request = HttpRequest.newBuilder()
                    .uri(new URI(url))
                    .timeout(Duration.ofSeconds(20))
                    .GET()
                    .build();

            HttpResponse<String> response = HttpClient.newBuilder()
                    .followRedirects(HttpClient.Redirect.ALWAYS)
                    .build()
                    .send(request, HttpResponse.BodyHandlers.ofString());

            if (response.statusCode() > 300) {
                logger.severe("WebLogin with token failed: " + response.body());
                logger.severe("Invalid token for " + mUsername + ". Error code: " + response.statusCode());
                return false;
            }

            return true;
        }
        catch (IOException | InterruptedException | URISyntaxException e) {
            logger.severe("Failed to get token for " + mUsername + ". "+ e);
            return false;
        }
    }

    public Vector<TeamTalkServer> getServerList() {
        String encodeToken = URLEncoder.encode(mToken, StandardCharsets.UTF_8);
        String encodedUsername = URLEncoder.encode(mUsername, StandardCharsets.UTF_8);
        String url = String.format("https://www.bearware.dk/teamtalk/weblogin.php?service=bearware&action=spambotservers&token=%s&username=%s", encodeToken, encodedUsername);

        var servers = new Vector<TeamTalkServer>();
        try {
            HttpRequest request = HttpRequest.newBuilder()
                    .uri(new URI(url))
                    .timeout(Duration.ofSeconds(20))
                    .GET()
                    .build();

            HttpResponse<String> response = HttpClient.newBuilder()
                    .followRedirects(HttpClient.Redirect.ALWAYS)
                    .build()
                    .send(request, HttpResponse.BodyHandlers.ofString());

            if (response.statusCode() > 300) {
                logger.severe("Invalid token for " + mUsername + ". Error code: " + response.statusCode());
                return servers;
            }

            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document doc = builder.parse(new InputSource(new StringReader(response.body())));

            XPath xPath = XPathFactory.newInstance().newXPath();
            NodeList ipaddrList = (NodeList) xPath.evaluate("/teamtalk/host/address/text()", doc, XPathConstants.NODESET);
            NodeList tcpportList = (NodeList) xPath.evaluate("/teamtalk/host/tcpport/text()", doc, XPathConstants.NODESET);
            NodeList udpportList = (NodeList) xPath.evaluate("/teamtalk/host/udpport/text()", doc, XPathConstants.NODESET);
            NodeList encryptedList = (NodeList) xPath.evaluate("/teamtalk/host/encrypted/text()", doc, XPathConstants.NODESET);

            for (int i=0;i<ipaddrList.getLength();++i) {
                TeamTalkServer srv = new TeamTalkServer(ipaddrList.item(i).getNodeValue(),
                                        Integer.parseInt(tcpportList.item(i).getNodeValue()),
                                        Integer.parseInt(udpportList.item(i).getNodeValue()),
                                        encryptedList.item(i).getNodeValue().equals("true"));
                servers.add(srv);
            }
        }
        catch (IOException | InterruptedException | URISyntaxException | XPathExpressionException | ParserConfigurationException | SAXException e) {
            logger.severe("Failed to get server list for " + mUsername + ". "+ e);
        }
        return servers;
    }
}
