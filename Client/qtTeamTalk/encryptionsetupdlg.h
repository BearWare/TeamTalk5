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

#ifndef ENCRYPTIONSETUPDLG_H
#define ENCRYPTIONSETUPDLG_H

#include "ui_encryptionsetupdlg.h"

#include "common.h"

#include <QSslCertificate>
#include <QSslKey>

class EncryptionSetupDlg : public QDialog
{
    Q_OBJECT
public:
    explicit EncryptionSetupDlg(HostEncryption& enc, QWidget *parent = nullptr);

private:
    QString getFile(const QString& fileext);
    QString readFile(const QString& filename);

    void showCA(const QSslCertificate& cert);
    void setupNewCA();
    void updateVerifyPeer();

    void showClientCertificate(const QSslCertificate& cert);
    void setupNewClientCertificate();
    void showClientPrivateKey(const QSslKey& key);
    void setupNewClientPrivateKey();

    void setEncryption();
    HostEncryption &m_org_encryption, m_encryption;
    Ui::EncryptionSetupDlg ui;
};

#endif // ENCRYPTIONSETUPDLG_H
