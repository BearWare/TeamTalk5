#include "weblogindlg.h"
#include "appinfo.h"

#include <QUrl>
#include <QRegExp>
#include <QDebug>

#if defined(Q_OS_WIN32)
#include <QAxWidget>
#else
#include <QWebEngineView>
#endif

WebLoginDlg::WebLoginDlg(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

#if defined(Q_OS_WIN32)
    QAxWidget *webView = new QAxWidget(this);
    webView->setControl(QString::fromUtf8("{8856F961-340A-11D0-A96B-00C04FD705A2}"));
    connect(webView, SIGNAL(NavigateComplete2(IDispatch*, QVariant&)),
            this, SLOT( slotNavigateComplete(IDispatch*, QVariant&)));
    webView->dynamicCall( "Navigate(const QString&)", WEBLOGIN_FACEBOOK_URL);
    ui.horizontalLayout->addWidget(webView);
#else
    QWebEngineView *webView = new QWebEngineView(this);
    webView->setObjectName(QStringLiteral("webView"));

    connect(webView, SIGNAL(urlChanged(const QUrl&)),
            SLOT(slotUrlChanged(const QUrl&)));

    ui.horizontalLayout->addWidget(webView);
    webView->load(QUrl(WEBLOGIN_FACEBOOK_URL));
#endif
}

WebLoginDlg::~WebLoginDlg()
{
}

#if defined(Q_OS_WIN32)
void WebLoginDlg::slotNavigateComplete(IDispatch*, QVariant& url)
{
    slotUrlChanged(QUrl(url.toString()));
}
#endif

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
