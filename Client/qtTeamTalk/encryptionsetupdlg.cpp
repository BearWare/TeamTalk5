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

#include "encryptionsetupdlg.h"
#include "appinfo.h"
#include "settings.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>

extern QSettings* ttSettings;

EncryptionSetupDlg::EncryptionSetupDlg(HostEncryption& enc, QWidget *parent)
    : QDialog(parent)
    , m_org_encryption(enc)
    , m_encryption(enc)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(this, &QDialog::accepted, this, &EncryptionSetupDlg::setEncryption);
    connect(ui.cafileBtn, &QAbstractButton::clicked, this, &EncryptionSetupDlg::setupNewCA);
    connect(ui.cafileBtn, &QAbstractButton::clicked, this, &EncryptionSetupDlg::updateVerifyPeer);
    connect(ui.cafileresetBtn, &QAbstractButton::clicked, [&]() { m_encryption.cacertdata.clear(); ui.caEdit->clear(); updateVerifyPeer(); });

    connect(ui.clientcertfileBtn, &QAbstractButton::clicked, this, &EncryptionSetupDlg::setupNewClientCertificate);
    connect(ui.clientcertfileresetBtn, &QAbstractButton::clicked, [&]() { m_encryption.certdata.clear(); ui.clientcertEdit->clear();  });

    connect(ui.clientkeyfileBtn, &QAbstractButton::clicked, this, &EncryptionSetupDlg::setupNewClientPrivateKey);
    connect(ui.clientkeyfileresetBtn, &QAbstractButton::clicked, [&]() { m_encryption.privkeydata.clear(); ui.clientkeyEdit->clear(); });

    connect(ui.verifypeerChkBox, &QAbstractButton::clicked, [&](bool checked) { m_encryption.verifypeer = checked; });

    if (m_encryption.cacertdata.size())
    {
        QSslCertificate cert(m_encryption.cacertdata.toUtf8());
        showCA(cert);
    }

    if (m_encryption.certdata.size())
    {
        QSslCertificate cert(m_encryption.certdata.toUtf8());
        showClientCertificate(cert);
    }

    if (m_encryption.privkeydata.size())
    {
        QSslKey key(m_encryption.privkeydata.toUtf8(), QSsl::Rsa);
        showClientPrivateKey(key);
    }

    updateVerifyPeer();
}

void EncryptionSetupDlg::showCA(const QSslCertificate& cert)
{
    QString text;
    QTextStream certStream(&text);
    if (!cert.isNull())
    {
        certStream << tr("Issuer: %1").arg(cert.issuerDisplayName()) << Qt::endl;
        certStream << tr("Subject: %1").arg(cert.subjectDisplayName()) << Qt::endl;
        certStream << tr("Effective date: %1").arg(cert.effectiveDate().toLocalTime().toString()) << Qt::endl;
        certStream << tr("Expiration date: %1").arg(cert.expiryDate().toLocalTime().toString()) << Qt::endl;
    }
    else
    {
        certStream << "Certificate Authority is invalid" << Qt::endl;
    }

    ui.caEdit->setPlainText(*certStream.string());
}

void EncryptionSetupDlg::setupNewCA()
{
    QString filename = getFile(tr("Certificate Authority (*.cer)"));
    if (filename.size())
    {
        QFile f(filename);
        if (f.open(QFile::ReadOnly))
        {
            QSslCertificate cert(&f);
            if (!cert.isNull())
            {
                showCA(cert);
                m_encryption.cacertdata = readFile(filename);
            }
            else
            {
                QMessageBox::critical(this, tr("Setup Certificate Autority"),
                    tr("The file %1 does not contain a valid certificate authority").arg(QDir::toNativeSeparators(filename)));
            }
        }
    }
}

void EncryptionSetupDlg::showClientCertificate(const QSslCertificate& cert)
{
    QString text;
    QTextStream certStream(&text);

    if (!cert.isNull())
    {
        certStream << tr("Issuer: %1").arg(cert.issuerDisplayName()) << Qt::endl;
        certStream << tr("Subject: %1").arg(cert.subjectDisplayName()) << Qt::endl;
        certStream << tr("Effective date: %1").arg(cert.effectiveDate().toLocalTime().toString()) << Qt::endl;
        certStream << tr("Expiration date: %1").arg(cert.expiryDate().toLocalTime().toString()) << Qt::endl;
    }
    else
    {
        certStream << "Client certificate is invalid" << Qt::endl;
    }

    ui.clientcertEdit->setPlainText(*certStream.string());
}

void EncryptionSetupDlg::setupNewClientCertificate()
{
    QString filename = getFile(tr("Client Certificate (*.pem)"));
    if (filename.size())
    {
        QFile f(filename);
        if (f.open(QFile::ReadOnly))
        {
            QSslCertificate cert(&f);
            if (!cert.isNull())
            {
                showClientCertificate(cert);
                m_encryption.certdata = readFile(filename);
            }
            else
            {
                QMessageBox::critical(this, tr("Setup Client Certificate"),
                    tr("The file %1 does not contain a valid client certificate").arg(QDir::toNativeSeparators(filename)));
            }
        }
    }
}

void EncryptionSetupDlg::showClientPrivateKey(const QSslKey& key)
{
    QString text;
    QTextStream certStream(&text);

    if (!key.isNull())
    {
        certStream << tr("RSA encryption") << Qt::endl;
        if (key.type() == QSsl::PrivateKey)
            certStream << tr("Private key: %1 bits").arg(key.length()) << Qt::endl;
    }
    else
    {
        certStream << "Client private key is invalid" << Qt::endl;
    }

    ui.clientkeyEdit->setPlainText(*certStream.string());
}

void EncryptionSetupDlg::setupNewClientPrivateKey()
{
    QString filename = getFile(tr("Client Private Key (*.pem)"));
    if (filename.size())
    {
        QFile f(filename);
        if (f.open(QFile::ReadOnly))
        {
            QSslKey key(&f, QSsl::Rsa);
            if (!key.isNull())
            {
                showClientPrivateKey(key);
                m_encryption.privkeydata = readFile(filename);
            }
            else
            {
                QMessageBox::critical(this, tr("Setup Client Private Key"),
                    tr("The file %1 does not contain a valid client private key").arg(QDir::toNativeSeparators(filename)));
            }
        }
    }
}

QString EncryptionSetupDlg::getFile(const QString& fileext)
{
    QString start_dir = ttSettings->value(SETTINGS_LAST_DIRECTORY).toString();
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), start_dir, fileext);
    if (filename.size())
        ttSettings->setValue(SETTINGS_LAST_DIRECTORY, QFileInfo(filename).absolutePath());
    return filename;
}

QString EncryptionSetupDlg::readFile(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";

    return QTextStream(&file).readAll();
}

void EncryptionSetupDlg::setEncryption()
{
    m_org_encryption = m_encryption;
}

void EncryptionSetupDlg::updateVerifyPeer()
{
    ui.verifypeerChkBox->setEnabled(m_encryption.cacertdata.size() > 0);
    if (m_encryption.cacertdata.size() == 0)
        m_encryption.verifypeer = false;

    ui.verifypeerChkBox->setChecked(m_encryption.verifypeer);
}
