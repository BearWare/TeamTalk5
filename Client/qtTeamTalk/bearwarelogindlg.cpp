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

#include "bearwarelogindlg.h"
#include "ui_bearwarelogindlg.h"
#include "common.h"
#include "appinfo.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QMessageBox>
#include <QDomDocument>

BearWareLoginDlg::BearWareLoginDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BearWareLoginDlg)
{
    ui->setupUi(this);
}

BearWareLoginDlg::~BearWareLoginDlg()
{
    delete ui;
}

void BearWareLoginDlg::accept()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    username = QUrl::toPercentEncoding(username);
    password = QUrl::toPercentEncoding(password);
    QString urlstr(WEBLOGIN_BEARWARE_URL(username, password));

    QUrl url(urlstr);
    auto http_manager = new QNetworkAccessManager(this);
    connect(http_manager, SIGNAL(finished(QNetworkReply*)),
            SLOT(slotHttpReply(QNetworkReply*)));

    QNetworkRequest request(url);
    http_manager->get(request);
}

void BearWareLoginDlg::slotHttpReply(QNetworkReply* reply)
{
    QString username, nickname;

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
                auto id = child.firstChildElement("id");
                if(!id.isNull())
                    username = id.text();
                auto name = child.firstChildElement("name");
                if(!name.isNull())
                    nickname = name.text();
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
        m_username = username;
        m_token = "";

        QMessageBox::information(this, this->windowTitle(),
                                 tr("%1, your username \"%2\" has been validated.").arg(nickname).arg(username), QMessageBox::Ok);
        
        QDialog::accept();
    }
}
