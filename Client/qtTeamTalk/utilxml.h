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
