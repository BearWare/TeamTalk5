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
#include "utilui.h"
#include "encryptionsetupdlg.h"
#include "settings.h"

#include <QPushButton>
#include <QInputDialog>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

ServerDlg::ServerDlg(ServerDlgType type, const HostEntry& host, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ServerDlg)
    , m_hostentry(host)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(APPICON));

    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_SERVERWINDOWPOS).toByteArray());

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    ui->usernameBox->addItem(WEBLOGIN_BEARWARE_USERNAME);

    connect(ui->cryptChkBox, &QCheckBox::toggled, ui->encsetupBtn, &QAbstractButton::setEnabled);
    connect(ui->encsetupBtn, &QAbstractButton::clicked, [&]()
    {
        HostEntry copyentry = m_hostentry;
        if (EncryptionSetupDlg(copyentry.encryption, this).exec())
            m_hostentry = copyentry;
    });
    connect(ui->passwordChkBox, &QAbstractButton::clicked,
            this, [&](bool checked) { ui->passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password); } );
    connect(ui->chanpasswordChkBox, &QAbstractButton::clicked,
            this, [&](bool checked) { ui->chanpasswdEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password); } );

    switch(type)
    {
    case SERVER_CREATE :
        setWindowTitle(tr("Add Server"));
        this->setAccessibleDescription(tr("Add server"));
        break;
    case SERVER_UPDATE :
    {
        setWindowTitle(tr("Edit Server"));
        this->setAccessibleDescription(tr("Edit Server %1").arg(host.name));
    }
    break;
    case SERVER_READONLY :
        setWindowTitle(tr("View Server Information"));
        this->setAccessibleDescription(tr("View %1 information").arg(host.name));
        ui->nameEdit->setReadOnly(true);
        ui->hostaddrEdit->setReadOnly(true);
        ui->tcpportEdit->setReadOnly(true);
        ui->udpportEdit->setReadOnly(true);
        ui->cryptChkBox->setEnabled(false);
        ui->encsetupBtn->setEnabled(false);
        ui->usernameBox->lineEdit()->setReadOnly(true);
        ui->passwordEdit->setReadOnly(true);
        ui->nicknameEdit->setReadOnly(true);
        ui->channelEdit->setReadOnly(true);
        ui->chanpasswdEdit->setReadOnly(true);
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
        ui->buttonBox->button(QDialogButtonBox::Close)->setText(tr("&Close"));
        break;
    }

    ui->nameEdit->setText(m_hostentry.name);
    ui->hostaddrEdit->setText(m_hostentry.ipaddr);
    ui->tcpportEdit->setText(QString::number(m_hostentry.tcpport));
    ui->udpportEdit->setText(QString::number(m_hostentry.udpport));
    ui->cryptChkBox->setChecked(m_hostentry.encrypted);
    ui->usernameBox->lineEdit()->setText(m_hostentry.username);
    ui->passwordEdit->setText(m_hostentry.password);
    ui->nicknameEdit->setText(m_hostentry.nickname);
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
    newhostentry.tcpport = ui->tcpportEdit->text().toInt();
    newhostentry.udpport = ui->udpportEdit->text().toInt();
    newhostentry.encrypted = ui->cryptChkBox->isChecked();
    newhostentry.username = ui->usernameBox->lineEdit()->text();
    newhostentry.password = ui->passwordEdit->text();
    newhostentry.nickname = ui->nicknameEdit->text();
    newhostentry.channel = ui->channelEdit->text();
    newhostentry.chanpasswd = ui->chanpasswdEdit->text();

    return newhostentry;
}
