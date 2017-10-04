#include "weblogindlg.h"
#include "appinfo.h"

#include <QUrl>
#include <QRegExp>

WebLoginDlg::WebLoginDlg(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(ui.webView, SIGNAL(urlChanged(const QUrl&)),
            SLOT(slotUrlChanged(const QUrl&)));

    ui.webView->load(QUrl(WEBLOGIN_FACEBOOK_URL));
}

WebLoginDlg::~WebLoginDlg()
{
}

void WebLoginDlg::slotUrlChanged(const QUrl &url)
{
    QString urlstr = url.toString();
    qDebug() << urlstr;
    if (urlstr.startsWith(WEBLOGIN_FACEBOOK_REDIRECT))
    {
        QRegExp rx("&code=([A-Za-z0-9\\-_]*)");
        if(rx.indexIn(urlstr) >= 0)
        {
            m_password = rx.cap(1);
            this->accept();
        }
    }
}
