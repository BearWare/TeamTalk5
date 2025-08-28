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

#include "bearwarelogindlg.h"
#include "ui_bearwarelogindlg.h"
#include "appinfo.h"
#include "settings.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QMessageBox>
#include <QDomDocument>
#include <QDesktopServices>
#include <QPushButton>
#include <QFile>

extern NonDefaultSettings* ttSettings;

QString BearWareLoginDlg::registerUrl = APPWEBSITE;

BearWareLoginDlg::BearWareLoginDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BearWareLoginDlg)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    connect(ui->registerButton, &QAbstractButton::clicked, this, &BearWareLoginDlg::slotRegister);
}

BearWareLoginDlg::~BearWareLoginDlg()
{
    delete ui;
}

void BearWareLoginDlg::slotRegister(bool)
{
    QDesktopServices::openUrl(QUrl(registerUrl));
}

void BearWareLoginDlg::accept()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    username = QUrl::toPercentEncoding(username);
    password = QUrl::toPercentEncoding(password);
    QString urlstr(WEBLOGIN_BEARWARE_URLAUTH(username, password));

    QUrl url(urlstr);
    auto http_manager = new QNetworkAccessManager(this);
    connect(http_manager, &QNetworkAccessManager::finished,
            this, &BearWareLoginDlg::slotHttpReply);

    QNetworkRequest request(url);
    http_manager->get(request);
}

void BearWareLoginDlg::slotHttpReply(QNetworkReply* reply)
{
    QString username, nickname, token;

    auto data = reply->readAll();

    QDomDocument doc("foo");
    if (doc.setContent(data))
    {
        auto child = doc.firstChildElement("teamtalk");
        if (!child.isNull())
        {
            child = child.firstChildElement("bearware");
            if(!child.isNull())
            {
                auto id = child.firstChildElement("username");
                if(!id.isNull())
                    username = id.text();
                auto name = child.firstChildElement("nickname");
                if(!name.isNull())
                    nickname = name.text();
                auto authtoken = child.firstChildElement("token");
                if (!authtoken.isNull())
                    token = authtoken.text();
            }
        }
    }

    if (username.isEmpty())
    {
        QMessageBox::critical(this, this->windowTitle(),
                              tr("Failed to authenticate"), QMessageBox::Ok);
    }
    else
    {
        ttSettings->setValue(SETTINGS_GENERAL_BEARWARE_USERNAME, username);
        ttSettings->setValue(SETTINGS_GENERAL_BEARWARE_TOKEN, token);

        QFile::setPermissions(ttSettings->fileName(), QFileDevice::ReadOwner | QFileDevice::WriteOwner);

        QMessageBox::information(this, this->windowTitle(),
                                 tr("%1, your username \"%2\" has been validated.").arg(nickname).arg(username), QMessageBox::Ok);
        
        QDialog::accept();
    }
}
