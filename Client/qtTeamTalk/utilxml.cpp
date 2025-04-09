/*
 * Copyright (C) 2023, Bjørn D. Rasmussen, BearWare.dk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "utilxml.h"
#include "appinfo.h"

void processTrustedXML(const QDomElement& hostElement, HostEntry& entry)
{
    QDomElement trusted = hostElement.firstChildElement("trusted-certificate");
    if (!trusted.isNull())
    {
        QDomElement tmp = trusted.firstChildElement("certificate-authority-pem");
        if (!tmp.isNull())
            entry.encryption.cacertdata = tmp.text();
        tmp = trusted.firstChildElement("client-certificate-pem");
        if (!tmp.isNull())
            entry.encryption.certdata = tmp.text();
        tmp = trusted.firstChildElement("client-private-key-pem");
        if (!tmp.isNull())
            entry.encryption.privkeydata = tmp.text();
        tmp = trusted.firstChildElement("verify-peer");
        if (!tmp.isNull())
            entry.encryption.verifypeer = tmp.text() == "true";
    }
}

void processAuthXML(const QDomElement& hostElement, HostEntry& entry)
{
    QDomElement auth = hostElement.firstChildElement("auth");
    if (!auth.isNull())
    {
        QDomElement tmp = auth.firstChildElement("username");
        if (!tmp.isNull())
            entry.username = tmp.text();

        tmp = auth.firstChildElement("password");
        if (!tmp.isNull())
            entry.password = tmp.text();

        tmp = auth.firstChildElement("nickname");
        if (!tmp.isNull())
            entry.nickname = tmp.text();
    }
}

void processJoinXML(const QDomElement& hostElement, HostEntry& entry)
{
    QDomElement join = hostElement.firstChildElement("join");
    if(!join.isNull())
    {
        QDomElement tmp = join.firstChildElement("channel");
        if (!tmp.isNull())
            entry.channel = tmp.text();
        tmp = join.firstChildElement("password");
        if (!tmp.isNull())
            entry.chanpasswd = tmp.text();
        tmp = join.firstChildElement("join-last-channel");
        if (!tmp.isNull())
            entry.lastChan = tmp.text() == "true";
    }
}

void processClientSetupXML(const QDomElement& hostElement, HostEntry& entry)
{
    QDomElement client = hostElement.firstChildElement(CLIENTSETUP_TAG);
    if (!client.isNull())
    {
        QDomElement tmp = client.firstChildElement("nickname");
        if (!tmp.isNull())
            entry.nickname = tmp.text();

        tmp = client.firstChildElement("gender");
        if(!tmp.isNull())
        {
            switch (tmp.text().toInt())
            {
            case GENDER_MALE :
                entry.gender = GENDER_MALE;
                break;
            case GENDER_FEMALE :
                entry.gender = GENDER_FEMALE;
                break;
            case GENDER_NEUTRAL :
            default:
                entry.gender = GENDER_NEUTRAL;
                break;
            }
        }
#if defined(Q_OS_WIN32)
        tmp = client.firstChildElement("win-hotkey");
#elif defined(Q_OS_DARWIN)
        tmp = client.firstChildElement("mac-hotkey");
#elif defined(Q_OS_LINUX)
        tmp = client.firstChildElement("x11-hotkey");
#else
#error Unknown OS
#endif
        if(!tmp.isNull())
        {
            tmp = tmp.firstChildElement("key");
            while(!tmp.isNull())
            {
                entry.hotkey.push_back(tmp.text().toInt());
                tmp = tmp.nextSiblingElement("key");
            }
        }
        tmp = client.firstChildElement("voice-activated");
        if(!tmp.isNull())
            entry.voiceact = tmp.text().toInt();
        tmp = client.firstChildElement("videoformat");
        if(!tmp.isNull())
        {
            QDomElement cap = tmp.firstChildElement("width");
            if(!cap.isNull())
                entry.capformat.nWidth = cap.text().toInt();
            cap = tmp.firstChildElement("height");
            if(!cap.isNull())
                entry.capformat.nHeight = cap.text().toInt();

            cap = tmp.firstChildElement("fps-numerator");
            if(!cap.isNull())
                entry.capformat.nFPS_Numerator = cap.text().toInt();
            cap = tmp.firstChildElement("fps-denominator");
            if(!cap.isNull())
                entry.capformat.nFPS_Denominator = cap.text().toInt();

            cap = tmp.firstChildElement("fourcc");
            if(!cap.isNull())
                entry.capformat.picFourCC = (FourCC)cap.text().toInt();
        }
        tmp = client.firstChildElement("videocodec");
        if(!tmp.isNull())
        {
            QDomElement vid = tmp.firstChildElement("codec");
            if(!vid.isNull())
                entry.vidcodec.nCodec = (Codec)vid.text().toInt();
            switch(entry.vidcodec.nCodec)
            {
            case WEBM_VP8_CODEC :
                vid = tmp.firstChildElement("webm-vp8-bitrate");
                if(!vid.isNull())
                {
                    entry.vidcodec.webm_vp8.nRcTargetBitrate = vid.text().toInt();
                    entry.vidcodec.webm_vp8.nEncodeDeadline = DEFAULT_WEBMVP8_DEADLINE;
                }
                break;
            case SPEEX_CODEC :
            case SPEEX_VBR_CODEC :
            case OPUS_CODEC :
            case NO_CODEC :
                break;
            }
        }
    }
}

bool getServerEntry(const QDomElement& hostElement, HostEntry& entry)
{
    Q_ASSERT(hostElement.tagName() == "host");
    bool ok = true;
    QDomElement tmp = hostElement.firstChildElement("name");
    if(!tmp.isNull())
        entry.name = tmp.text();
    else ok = false;

    tmp = hostElement.firstChildElement("address");
    if(!tmp.isNull())
        entry.ipaddr = tmp.text();
    else ok = false;

    tmp = hostElement.firstChildElement("tcpport");
    if(!tmp.isNull())
        entry.tcpport = tmp.text().toInt();
    else ok = false;

    tmp = hostElement.firstChildElement("udpport");
    if(!tmp.isNull())
        entry.udpport = tmp.text().toInt();
    else ok = false;

    tmp = hostElement.firstChildElement("encrypted");
    if(!tmp.isNull())
        entry.encrypted = (tmp.text().toLower() == "true" || tmp.text() == "1");

    processTrustedXML(hostElement, entry);
    processAuthXML(hostElement, entry);
    processJoinXML(hostElement, entry);
    processClientSetupXML(hostElement, entry);

    return ok;
}

QByteArray generateTTFile(const HostEntry& entry)
{
    QDomDocument doc(TTFILE_ROOT);
    QDomProcessingInstruction pi = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" ");
    doc.appendChild(pi);

    QDomElement root = doc.createElement(TTFILE_ROOT);
    root.setAttribute("version", TTFILE_VERSION);
    doc.appendChild(root);

    QDomElement host = doc.createElement("host");

    QDomElement name = doc.createElement("name");
    name.appendChild(doc.createTextNode(entry.name));

    QDomElement address = doc.createElement("address");
    address.appendChild(doc.createTextNode(entry.ipaddr));

    QDomElement tcpport = doc.createElement("tcpport");
    tcpport.appendChild(doc.createTextNode(QString::number(entry.tcpport)));

    QDomElement udpport = doc.createElement("udpport");
    udpport.appendChild(doc.createTextNode(QString::number(entry.udpport)));

    QDomElement encrypted = doc.createElement("encrypted");
    encrypted.appendChild(doc.createTextNode(entry.encrypted?"true":"false"));

    host.appendChild(name);
    host.appendChild(address);
    host.appendChild(tcpport);
    host.appendChild(udpport);
    host.appendChild(encrypted);

    if (entry.encrypted)
    {
        QDomElement trusted = doc.createElement("trusted-certificate");
        QDomElement ca_pem = doc.createElement("certificate-authority-pem");
        ca_pem.appendChild(doc.createTextNode(entry.encryption.cacertdata));
        QDomElement cert_pem = doc.createElement("client-certificate-pem");
        cert_pem.appendChild(doc.createTextNode(entry.encryption.certdata));
        QDomElement key_pem = doc.createElement("client-private-key-pem");
        key_pem.appendChild(doc.createTextNode(entry.encryption.privkeydata));
        QDomElement verifypeer = doc.createElement("verify-peer");
        verifypeer.appendChild(doc.createTextNode(entry.encryption.verifypeer ? "true" : "false"));

        trusted.appendChild(ca_pem);
        trusted.appendChild(cert_pem);
        trusted.appendChild(key_pem);
        trusted.appendChild(verifypeer);
        host.appendChild(trusted);
    }

    if(entry.username.size())
    {
        QDomElement auth = doc.createElement("auth");

        QDomElement username = doc.createElement("username");
        username.appendChild(doc.createTextNode(entry.username));

        QDomElement password = doc.createElement("password");
        password.appendChild(doc.createTextNode(entry.password));

        QDomElement nickname = doc.createElement("nickname");
        nickname.appendChild(doc.createTextNode(entry.nickname));

        auth.appendChild(username);
        auth.appendChild(password);
        auth.appendChild(nickname);

        host.appendChild(auth);
    }

    if(entry.channel.size())
    {
        QDomElement join = doc.createElement("join");

        QDomElement channel = doc.createElement("channel");
        channel.appendChild(doc.createTextNode(entry.channel));
        join.appendChild(channel);

        QDomElement password = doc.createElement("password");
        password.appendChild(doc.createTextNode(entry.chanpasswd));
        join.appendChild(password);

        QDomElement joinlast = doc.createElement("join-last-channel");
        joinlast.appendChild(doc.createTextNode(entry.lastChan ? "true" : "false"));
        join.appendChild(joinlast);

        host.appendChild(join);
    }

    QDomElement client = doc.createElement(CLIENTSETUP_TAG);

    if(entry.nickname.size())
    {
        QDomElement nickname = doc.createElement("nickname");
        nickname.appendChild(doc.createTextNode(entry.nickname));
        client.appendChild(nickname);
    }
    if (entry.gender != GENDER_NONE)
    {
        QDomElement gender = doc.createElement("gender");
        gender.appendChild(doc.createTextNode(QString::number(entry.gender)));
        client.appendChild(gender);
    }
    if(entry.hotkey.size())
    {
#if defined(Q_OS_WIN32)
        QDomElement hotkey = doc.createElement("win-hotkey");
#elif defined(Q_OS_DARWIN)
        QDomElement hotkey = doc.createElement("mac-hotkey");
#elif defined(Q_OS_LINUX)
        QDomElement hotkey = doc.createElement("x11-hotkey");
#else
#error Unknown OS
#endif
        for (int k : entry.hotkey)
        {
            QDomElement key = doc.createElement("key");
            key.appendChild(doc.createTextNode(QString::number(k)));
            hotkey.appendChild(key);
        }
        client.appendChild(hotkey);
    }
    if(entry.voiceact >= 0)
    {
        QDomElement vox = doc.createElement("voice-activated");
        vox.appendChild(doc.createTextNode(QString::number(entry.voiceact)));
        client.appendChild(vox);
    }
    if(entry.capformat.nWidth)
    {
        QDomElement cap = doc.createElement("videoformat");

        QDomElement newElement = doc.createElement("width");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.nWidth)));
        cap.appendChild(newElement);
        newElement = doc.createElement("height");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.nHeight)));
        cap.appendChild(newElement);

        newElement = doc.createElement("fps-numerator");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.nFPS_Numerator)));
        cap.appendChild(newElement);
        newElement = doc.createElement("fps-denominator");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.nFPS_Denominator)));
        cap.appendChild(newElement);

        newElement = doc.createElement("fourcc");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.picFourCC)));
        cap.appendChild(newElement);

        client.appendChild(cap);
    }
    if(entry.vidcodec.nCodec != NO_CODEC)
    {
        QDomElement vidcodec = doc.createElement("videocodec");

        QDomElement newElement = doc.createElement("codec");
        newElement.appendChild(doc.createTextNode(QString::number(entry.vidcodec.nCodec)));
        vidcodec.appendChild(newElement);
        switch(entry.vidcodec.nCodec)
        {
        case WEBM_VP8_CODEC :
            newElement = doc.createElement("webm-vp8-bitrate");
            newElement.appendChild(doc.createTextNode(QString::number(entry.vidcodec.webm_vp8.nRcTargetBitrate)));
            vidcodec.appendChild(newElement);
            break;
        case SPEEX_CODEC :
        case SPEEX_VBR_CODEC :
        case OPUS_CODEC :
        case NO_CODEC :
            break;
        }
        client.appendChild(vidcodec);
    }

    if(client.hasChildNodes())
        host.appendChild(client);

    root.appendChild(host);

    return doc.toByteArray();
}

QString parseXML(const QDomDocument& doc, QString elements)
{
    QDomElement element(doc.documentElement());
    QStringList tokens = elements.split("/");
    while (tokens.size())
    {
        if (element.isNull() || element.nodeName() != tokens[0])
            return QString();

        tokens.removeFirst();
        if (tokens.isEmpty())
            return element.text();

        element = element.firstChildElement(tokens[0]);
    }
    return QString();
}

QString newVersionAvailable(const QDomDocument& updateDoc)
{
    return parseXML(updateDoc, "teamtalk/update/name");
}

QString downloadUpdateURL(const QDomDocument& updateDoc)
{
    return parseXML(updateDoc, "teamtalk/update/download-url");
}

QString newBetaVersionAvailable(const QDomDocument& updateDoc)
{
    return parseXML(updateDoc, "teamtalk/beta/name");
}

QString downloadBetaUpdateURL(const QDomDocument& updateDoc)
{
    return parseXML(updateDoc, "teamtalk/beta/download-url");
}

QString getBearWareRegistrationUrl(const QDomDocument& doc)
{
    return parseXML(doc, "teamtalk/bearware/register-url");
}
