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

#include "serverdlg.h"
#include "ui_serverdlg.h"
#include "appinfo.h"
#include "settings.h"

#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextStream>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

ServerDlg::ServerDlg(ServerDlgType type, const HostEntry& host, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ServerDlg)
    , m_encryptionTab(nullptr)
    , m_type(type)
    , m_hostentry(host)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(APPICON));

    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_SERVERWINDOWPOS).toByteArray());

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&Save and Close"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Close without saving"));

    m_encryptionTab = ui->tabWidget->widget(ui->tabWidget->indexOf(ui->tabEncryption));
    connect(ui->cryptChkBox, &QCheckBox::toggled, this, &ServerDlg::updateTabBar);
    connect(ui->cafileBtn, &QAbstractButton::clicked, this, &ServerDlg::setupNewCA);
    connect(ui->cafileBtn, &QAbstractButton::clicked, this, &ServerDlg::updateVerifyPeer);
    connect(ui->cafileresetBtn, &QAbstractButton::clicked, [&]()
    {
        m_hostentry.encryption.cacertdata.clear();
        ui->caEdit->clear();
        updateVerifyPeer();
    });

    connect(ui->clientcertfileBtn, &QAbstractButton::clicked, this, &ServerDlg::setupNewClientCertificate);
    connect(ui->clientcertfileresetBtn, &QAbstractButton::clicked, [&]()
    {
        m_hostentry.encryption.certdata.clear();
        ui->clientcertEdit->clear();
    });

    connect(ui->clientkeyfileBtn, &QAbstractButton::clicked, this, &ServerDlg::setupNewClientPrivateKey);
    connect(ui->clientkeyfileresetBtn, &QAbstractButton::clicked, [&]()
    {
        m_hostentry.encryption.privkeydata.clear();
        ui->clientkeyEdit->clear();
    });

    connect(ui->verifypeerChkBox, &QAbstractButton::clicked, [&](bool checked)
    {
        m_hostentry.encryption.verifypeer = checked;
    });

    if (m_hostentry.encryption.cacertdata.size())
    {
        QSslCertificate cert(m_hostentry.encryption.cacertdata.toUtf8());
        showCA(cert);
    }

    if (m_hostentry.encryption.certdata.size())
    {
        QSslCertificate cert(m_hostentry.encryption.certdata.toUtf8());
        showClientCertificate(cert);
    }

    if (m_hostentry.encryption.privkeydata.size())
    {
        QSslKey key(m_hostentry.encryption.privkeydata.toUtf8(), QSsl::Rsa);
        showClientPrivateKey(key);
    }

    updateVerifyPeer();
    connect(ui->bdkLogChkBox, &QCheckBox::toggled,
            this, &ServerDlg::slotToggledWebLogin);
    connect(ui->lastChanChkBox, &QCheckBox::toggled,
            this, &ServerDlg::slotToggledLastChannel);
    connect(ui->passwordChkBox, &QAbstractButton::clicked,
            this, [&](bool checked) { ui->passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password); } );
    connect(ui->chanpasswordChkBox, &QAbstractButton::clicked,
            this, [&](bool checked) { ui->chanpasswdEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password); } );

    switch(type)
    {
    case SERVER_CREATE :
        setWindowTitle(tr("Add Server"));
        this->setAccessibleDescription(tr("Add Server"));
        break;
    case SERVER_UPDATE :
    {
        setWindowTitle(tr("Edit Server"));
        this->setAccessibleDescription(tr("Edit Server %1").arg(host.name));
    }
    break;
    case SERVER_READONLY :
        setWindowTitle(tr("View Server Information"));
        this->setAccessibleDescription(tr("View %1 Information").arg(host.name));
        ui->nameEdit->setReadOnly(true);
        ui->hostaddrEdit->setReadOnly(true);
        ui->tcpportSpinbox->setReadOnly(true);
        ui->udpportSpinbox->setReadOnly(true);
        ui->cryptChkBox->setEnabled(false);
        ui->bdkLogChkBox->setEnabled(false);
        ui->usernameEdit->setReadOnly(true);
        ui->passwordEdit->setReadOnly(true);
        ui->nicknameEdit->setReadOnly(true);
        ui->lastChanChkBox->setEnabled(false);
        ui->channelEdit->setReadOnly(true);
        ui->chanpasswdEdit->setReadOnly(true);
        ui->caEdit->setReadOnly(true);
        ui->cafileBtn->setEnabled(false);
        ui->cafileresetBtn->setEnabled(false);
        ui->clientcertEdit->setReadOnly(true);
        ui->clientcertfileBtn->setEnabled(false);
        ui->clientcertfileresetBtn->setEnabled(false);
        ui->clientkeyEdit->setReadOnly(true);
        ui->clientkeyfileBtn->setEnabled(false);
        ui->clientkeyfileresetBtn->setEnabled(false);
        ui->connectSrvBox->setEnabled(false);
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
        ui->buttonBox->button(QDialogButtonBox::Close)->setText(tr("&Close"));
        break;
    }

    ui->nameEdit->setText(m_hostentry.name);
    ui->hostaddrEdit->setText(m_hostentry.ipaddr);
    ui->tcpportSpinbox->setValue(m_hostentry.tcpport);
    ui->udpportSpinbox->setValue(m_hostentry.udpport);
    ui->cryptChkBox->setChecked(m_hostentry.encrypted);
    ui->bdkLogChkBox->setChecked(isWebLogin(m_hostentry.username, true));
    ui->lastChanChkBox->setChecked(m_hostentry.lastChan);
    ui->usernameEdit->setText(m_hostentry.username);
    ui->passwordEdit->setText(m_hostentry.password);
    ui->nicknameEdit->setText(m_hostentry.nickname);
    ui->lastChanChkBox->setChecked(m_hostentry.lastChan);
    ui->channelEdit->setText(m_hostentry.channel);
    ui->chanpasswdEdit->setText(m_hostentry.chanpasswd);
    updateTabBar(ui->cryptChkBox->isChecked());
}

ServerDlg::~ServerDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_SERVERWINDOWPOS, saveGeometry());
    delete ui;
}

HostEntry ServerDlg::GetHostEntry() const
{
    HostEntry newhostentry = m_hostentry;
    newhostentry.name = ui->nameEdit->text();
    newhostentry.ipaddr = ui->hostaddrEdit->text();
    newhostentry.tcpport = ui->tcpportSpinbox->value();
    newhostentry.udpport = ui->udpportSpinbox->value();
    newhostentry.encrypted = ui->cryptChkBox->isChecked();
    newhostentry.username = ui->usernameEdit->text();
    newhostentry.password = isWebLogin(ui->usernameEdit->text(), true)?"":ui->passwordEdit->text();
    newhostentry.nickname = ui->nicknameEdit->text();
    newhostentry.lastChan = ui->lastChanChkBox->isChecked();
    newhostentry.channel = ui->channelEdit->text();
    newhostentry.chanpasswd = ui->chanpasswdEdit->text();

    return newhostentry;
}

bool ServerDlg::connectToServer() const
{
    return ui->connectSrvBox->isChecked();
}

void ServerDlg::generateEntryName()
{
    ui->nameEdit->setText(GetHostEntry().generateEntryName());
}

void ServerDlg::accept()
{
    if (ui->nameEdit->text().trimmed().isEmpty())
    {
        generateEntryName();
        ui->nameEdit->setFocus();
        return;
    }
    if (m_type == SERVER_CREATE && !isServerNameUnique(ui->nameEdit->text()))
    {
        QMessageBox::critical(this, tr("Name already used"), tr("Another server with this name already exists. Please use a different name."));
        return;
    }
    if (ui->hostaddrEdit->text().trimmed().isEmpty())
    {
        QMessageBox::critical(this, tr("Missing information"), tr("Please fill in \"Host IP-address\" field"));
        return;
    }
    
    QDialog::accept();
}

void ServerDlg::slotToggledWebLogin()
{
    ui->usernameEdit->setReadOnly(ui->bdkLogChkBox->isChecked());
    if (ui->bdkLogChkBox->isChecked())
        ui->usernameEdit->setText(WEBLOGIN_BEARWARE_USERNAME);
    else
        ui->usernameEdit->setText(m_hostentry.username);
    ui->usernameLabel->setVisible(!ui->bdkLogChkBox->isChecked());
    ui->usernameEdit->setVisible(!ui->bdkLogChkBox->isChecked());
    ui->passwdLabel->setVisible(!ui->bdkLogChkBox->isChecked());
    ui->passwordEdit->setVisible(!ui->bdkLogChkBox->isChecked());
    ui->passwordChkBox->setVisible(!ui->bdkLogChkBox->isChecked());
}

void ServerDlg::slotToggledLastChannel()
{
    if (!ui->lastChanChkBox->isChecked())
    {
        ui->channelEdit->setText("");
        ui->chanpasswdEdit->setText("");
    }
    ui->channel_label->setVisible(!ui->lastChanChkBox->isChecked());
    ui->channelEdit->setVisible(!ui->lastChanChkBox->isChecked());
    ui->chanpsw_label->setVisible(!ui->lastChanChkBox->isChecked());
    ui->chanpasswdEdit->setVisible(!ui->lastChanChkBox->isChecked());
    ui->chanpasswordChkBox->setVisible(!ui->lastChanChkBox->isChecked());
}

bool ServerDlg::isServerNameUnique(const QString& serverName)
{
    ttSettings->beginGroup("serverentries");
    const QStringList keys = ttSettings->allKeys();
    foreach (const QString &key, keys)
    {
        if (key.endsWith("_name"))
        {
            if (ttSettings->value(key).toString().compare(serverName, Qt::CaseInsensitive) == 0)
            {
                ttSettings->endGroup();
                return false;
            }
        }
    }

    ttSettings->endGroup();
    return true;
}

void ServerDlg::showCA(const QSslCertificate& cert)
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

    ui->caEdit->setPlainText(*certStream.string());
}

void ServerDlg::setupNewCA()
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
                m_hostentry.encryption.cacertdata = readFile(filename);
            }
            else
            {
                QMessageBox::critical(this, tr("Setup Certificate Authority"),
                    tr("The file %1 does not contain a valid certificate authority").arg(QDir::toNativeSeparators(filename)));
            }
        }
    }
}

void ServerDlg::showClientCertificate(const QSslCertificate& cert)
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

    ui->clientcertEdit->setPlainText(*certStream.string());
}

void ServerDlg::setupNewClientCertificate()
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
                m_hostentry.encryption.certdata = readFile(filename);
            }
            else
            {
                QMessageBox::critical(this, tr("Setup Client Certificate"),
                    tr("The file %1 does not contain a valid client certificate").arg(QDir::toNativeSeparators(filename)));
            }
        }
    }
}

void ServerDlg::showClientPrivateKey(const QSslKey& key)
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

    ui->clientkeyEdit->setPlainText(*certStream.string());
}

void ServerDlg::setupNewClientPrivateKey()
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
                m_hostentry.encryption.privkeydata = readFile(filename);
            }
            else
            {
                QMessageBox::critical(this, tr("Setup Client Private Key"),
                    tr("The file %1 does not contain a valid client private key").arg(QDir::toNativeSeparators(filename)));
            }
        }
    }
}

QString ServerDlg::getFile(const QString& fileext)
{
    QString start_dir = ttSettings->value(SETTINGS_LAST_DIRECTORY).toString();
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), start_dir, fileext);
    if (filename.size())
        ttSettings->setValue(SETTINGS_LAST_DIRECTORY, QFileInfo(filename).absolutePath());
    return filename;
}

QString ServerDlg::readFile(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";

    return QTextStream(&file).readAll();
}

void ServerDlg::updateVerifyPeer()
{
    ui->verifypeerChkBox->setEnabled(m_hostentry.encryption.cacertdata.size() > 0 && m_type != SERVER_READONLY);
    if (m_hostentry.encryption.cacertdata.size() == 0)
        m_hostentry.encryption.verifypeer = false;

    ui->verifypeerChkBox->setChecked(m_hostentry.encryption.verifypeer);
}

void ServerDlg::updateTabBar(bool checked)
{
    if (checked)
    {
        if (ui->tabWidget->indexOf(m_encryptionTab) == -1)
        {
            ui->tabWidget->addTab(m_encryptionTab, tr("Encryption Setup"));
        }
    }
    else
    {
        int index = ui->tabWidget->indexOf(m_encryptionTab);
        if (index != -1)
        {
            ui->tabWidget->removeTab(index);
        }
    }
}
