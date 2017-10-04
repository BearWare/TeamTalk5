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

    QString m_password;
private slots:
    void slotUrlChanged(const QUrl &url);

private:
    Ui::WebLoginDlg ui;
};

#endif // WEBLOGINDLG_H
