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

#ifndef SERVERDLG_H
#define SERVERDLG_H

#include "common.h"
#include "ui_server.h"

class ServerDlg : public QDialog
{
    Q_OBJECT

public:
    enum ServerDlgType
    {
        SERVER_CREATE,
        SERVER_UPDATE,
        SERVER_READONLY
    };

    ServerDlg(ServerDlgType type, const HostEntry& host, QWidget * parent = 0);
    ~ServerDlg();

    HostEntry GetHostEntry() const;

private:
    void slotGenerateEntryName(const QString&);

private:
    Ui::ServerDlg ui;
    HostEntry m_hostentry;
};

#endif
