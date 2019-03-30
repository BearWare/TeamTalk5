#ifndef BEARWARELOGINDLG_H
#define BEARWARELOGINDLG_H

#include <QDialog>
#include <QNetworkReply>

namespace Ui {
class BearWareLoginDlg;
}

class BearWareLoginDlg : public QDialog
{
    Q_OBJECT

public:
    explicit BearWareLoginDlg(QWidget *parent = 0);
    ~BearWareLoginDlg();

    void accept();

    QString username;
    QString token;

private slots:
    void slotHttpReply(QNetworkReply* reply);

private:
    Ui::BearWareLoginDlg *ui;
};

#endif // BEARWARELOGINDLG_H
