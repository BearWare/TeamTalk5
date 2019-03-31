#include "bearwarelogindlg.h"
#include "ui_bearwarelogindlg.h"
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

    QString urlstr(WEBLOGIN_URL);
    urlstr += "service=bearware";
    urlstr += "&username=" + QUrl::toPercentEncoding(username);
    urlstr += "&password=" + QUrl::toPercentEncoding(password);

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
        this->username = username;
        this->token = "";
        QDialog::accept();
    }
}
