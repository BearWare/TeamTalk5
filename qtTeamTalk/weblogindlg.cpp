#include "weblogindlg.h"
#include "appinfo.h"

#include <QUrl>
#include <QRegExp>
#include <QDebug>
#include <QKeyEvent>
#include <QMessageBox>

#if defined(Q_OS_WIN32)
#include <QAxWidget>
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#include <QWebEngineView>
#elif (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
#include <QtWebKit>
#endif

WebLoginDlg::WebLoginDlg(QWidget *parent) :
    QDialog(parent)
    , m_timerid(-1)
    , m_cancelled(false)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

#if defined(Q_OS_WIN32)
    m_webView = new QAxWidget(this);
    m_webView->setControl(QString::fromUtf8("{8856F961-340A-11D0-A96B-00C04FD705A2}"));
    connect(m_webView, SIGNAL(NavigateComplete2(IDispatch*, QVariant&)),
            this, SLOT( slotNavigateComplete(IDispatch*, QVariant&)));
    ui.horizontalLayout->addWidget(m_webView);
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    m_webView = new QWebEngineView(this);
    m_webView->setObjectName(QStringLiteral("webView"));

    connect(m_webView, SIGNAL(urlChanged(const QUrl&)),
            SLOT(slotUrlChanged(const QUrl&)));

    ui.horizontalLayout->addWidget(m_webView);
#elif (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
    m_webView = new QWebView(this);
    m_webView->setObjectName("webView");

    connect(m_webView, SIGNAL(urlChanged(const QUrl&)),
            SLOT(slotUrlChanged(const QUrl&)));

    ui.horizontalLayout->addWidget(m_webView);
#endif
    navigate(WEBLOGIN_FACEBOOK_URL);
}

WebLoginDlg::~WebLoginDlg()
{
}

void WebLoginDlg::navigate(const QString& url)
{
#if defined(Q_OS_WIN32)
    m_webView->dynamicCall("Navigate(const QString&)", url);
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    m_webView->load(QUrl(url));
#elif (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
    m_webView->load(QUrl(url));
#endif
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
    if (urlstr.startsWith(WEBLOGIN_FACEBOOK_REDIRECT))
    {
        QRegExp rxtoken("#access_token=([A-Za-z0-9\\-_]*)");
        if(rxtoken.indexIn(urlstr) >= 0)
        {
            m_token = rxtoken.cap(1);
        }

        QRegExp rxcode("&code=([A-Za-z0-9\\-_]*)");
        if(rxcode.indexIn(urlstr) >= 0)
        {
            m_password = WEBLOGIN_FACEBOOK_PASSWDPREFIX + rxcode.cap(1);
            m_timerid = startTimer(1000);
            // Web control has focus so hitting escape has no effect. 
            // Therefore set focus to dialog.
            this->setFocus();
        }
    }

    if(urlstr == WEBLOGIN_FACEBOOK_LOGOUT_REDIRECT)
        this->reject();
}

void WebLoginDlg::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        if(m_timerid<0)
            killTimer(m_timerid);
        m_timerid = -1;
        m_cancelled = true;

        QMessageBox::StandardButton answer = QMessageBox::question(this,
            tr("Facebook Login"), tr("Do Facebook logout??"),
            QMessageBox::Yes | QMessageBox::No);

        if(answer == QMessageBox::Yes)
        {
            event->ignore();

            navigate(QString("%1next=%2&access_token=%3")
                .arg(WEBLOGIN_FACEBOOK_LOGOUT_URL)
                .arg(WEBLOGIN_FACEBOOK_LOGOUT_REDIRECT)
                .arg(m_token));
        }
        else if(m_password.size())
            this->accept();
    }
    else
    {
        QDialog::keyPressEvent(event);
    }
}

void WebLoginDlg::timerEvent(QTimerEvent *event)
{
    QDialog::timerEvent(event);
    
    if(!m_cancelled)
        this->accept();

    killTimer(m_timerid);
}
