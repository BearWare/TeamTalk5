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
#include "encryptionsetupdlg.h"
#include "settings.h"

#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>

extern TTInstance* ttInst;
extern NonDefaultSettings* ttSettings;

ServerDlg::ServerDlg(ServerDlgType type, const HostEntry& host, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ServerDlg)
    , m_type(type)
    , m_hostentry(host)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(APPICON));

    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_SERVERWINDOWPOS).toByteArray());

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&Save and Close"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Close without saving"));

    connect(ui->cryptChkBox, &QCheckBox::toggled, ui->encsetupBtn, &QAbstractButton::setEnabled);
    connect(ui->encsetupBtn, &QAbstractButton::clicked, [&]()
    {
        HostEntry copyentry = m_hostentry;
        if (EncryptionSetupDlg(copyentry.encryption, this).exec())
            m_hostentry = copyentry;
    });
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
        ui->encsetupBtn->setEnabled(false);
        ui->bdkLogChkBox->setEnabled(false);
        ui->usernameEdit->setReadOnly(true);
        ui->passwordEdit->setReadOnly(true);
        ui->nicknameEdit->setReadOnly(true);
        ui->statusmsgEdit->setReadOnly(true);
        ui->lastChanChkBox->setEnabled(false);
        ui->channelEdit->setReadOnly(true);
        ui->chanpasswdEdit->setReadOnly(true);
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
    ui->statusmsgEdit->setText(m_hostentry.statusmsg);
    ui->lastChanChkBox->setChecked(m_hostentry.lastChan);
    ui->channelEdit->setText(m_hostentry.channel);
    ui->chanpasswdEdit->setText(m_hostentry.chanpasswd);
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
    newhostentry.statusmsg = ui->statusmsgEdit->text();
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
