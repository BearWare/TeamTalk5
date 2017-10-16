#ifndef WEBLOGINDLG_H
#define WEBLOGINDLG_H

#include <QDialog>
#include "ui_weblogin.h"

#if defined(Q_OS_WIN32)
#include <QAxWidget>
#endif

class WebLoginDlg : public QDialog
{
    Q_OBJECT

public:
    WebLoginDlg(QWidget *parent = 0);
    ~WebLoginDlg();

    QString m_password;
private slots:
#if defined(Q_OS_WIN32)
    void slotNavigateComplete(IDispatch*, QVariant&);
#endif

    void slotUrlChanged(const QUrl &url);

private:
    Ui::WebLoginDlg ui;
};

#endif // WEBLOGINDLG_H
