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

#include <QDialog>
#include <QSslCertificate>
#include <QSslKey>

namespace Ui {
class ServerDlg;
}

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
    explicit ServerDlg(ServerDlgType type, const HostEntry& host, QWidget *parent = nullptr);
    ~ServerDlg();

    HostEntry GetHostEntry() const;
    bool connectToServer() const;

protected:
    void accept() override;

private:
    Ui::ServerDlg *ui;
    QWidget* m_encryptionTab;
    HostEntry m_hostentry;
    ServerDlgType m_type;
    void generateEntryName();
    void slotToggledWebLogin();
    void slotToggledLastChannel();
    bool isServerNameUnique(const QString& serverName);

    QString getFile(const QString& fileext);
    QString readFile(const QString& filename);

    void showCA(const QSslCertificate& cert);
    void setupNewCA();
    void updateVerifyPeer();

    void showClientCertificate(const QSslCertificate& cert);
    void setupNewClientCertificate();
    void showClientPrivateKey(const QSslKey& key);
    void setupNewClientPrivateKey();
    void updateTabBar(bool checked);
};

#endif // SERVERDLG_H
