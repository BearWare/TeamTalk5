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
