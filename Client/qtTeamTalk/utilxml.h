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

#ifndef UTILXML_H
#define UTILXML_H

#include "common.h"

#include <QDomDocument>

//server entry from XML
bool getServerEntry(const QDomElement& hostElement, HostEntry& entry);
#define CLIENTSETUP_TAG "clientsetup"
QByteArray generateTTFile(const HostEntry& entry);

QString parseXML(const QDomDocument& doc, QString elements);
QString newVersionAvailable(const QDomDocument& updateDoc);
QString downloadUpdateURL(const QDomDocument& updateDoc);
QString newBetaVersionAvailable(const QDomDocument& updateDoc);
QString downloadBetaUpdateURL(const QDomDocument& updateDoc);
QString getBearWareRegistrationUrl(const QDomDocument& doc);

#endif // UTILXML_H
