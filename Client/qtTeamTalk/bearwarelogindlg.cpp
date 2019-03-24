#include "bearwarelogindlg.h"
#include "ui_bearwarelogindlg.h"
#include "appinfo.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QMessageBox>

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
    QString urlstr(WEBLOGIN_URL);

    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    urlstr += "service=bearware";
    urlstr += "&username=" + QUrl::toPercentEncoding(username);
    urlstr += "&password=" + QUrl::toPercentEncoding(password);
    urlstr += "&client=" APPNAME_SHORT "&version=" APPVERSION_SHORT;

    QUrl url(urlstr);
    auto http_manager = new QNetworkAccessManager(this);
    connect(http_manager, SIGNAL(finished(QNetworkReply*)),
            SLOT(slotHttpReply(QNetworkReply*)));

    QNetworkRequest request(url);
    http_manager->get(request);
}

void BearWareLoginDlg::slotHttpReply(QNetworkReply* reply)
{
    qDebug() << "Error: " << reply->error();
    qDebug() << reply->readAll();
    if (reply->error())
    {
        QMessageBox::critical(this, tr("Failed to authenticate"),
                              QMessageBox::Ok, )
    }
}
