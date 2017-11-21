#ifndef WEBLOGINDLG_H
#define WEBLOGINDLG_H

#include <QDialog>
#include "ui_weblogin.h"

class WebLoginDlg : public QDialog
{
    Q_OBJECT

public:
    WebLoginDlg(QWidget *parent = 0);
    ~WebLoginDlg();

    QString m_password, m_token;

private slots:
    void navigate(const QString& url);

#if defined(Q_OS_WIN32)
    void slotNavigateComplete(struct IDispatch*, QVariant&);
#endif

    void slotUrlChanged(const QUrl &url);

private:
    Ui::WebLoginDlg ui;
#if defined(Q_OS_WIN32)
    class QAxWidget* m_webView;
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    class QWebEngineView* m_webView;
#elif (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
    class QWebView* m_webView;
#endif
    int m_timerid;
    bool m_cancelled;

protected:
    void keyPressEvent(QKeyEvent *event);
    void timerEvent(QTimerEvent *event);
};

#endif // WEBLOGINDLG_H
