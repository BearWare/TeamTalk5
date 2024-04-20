#ifndef SERVERDLG_H
#define SERVERDLG_H

#include <QDialog>

namespace Ui {
class ServerDlg;
}

class ServerDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ServerDlg(QWidget *parent = nullptr);
    ~ServerDlg();

private:
    Ui::ServerDlg *ui;
};

#endif // SERVERDLG_H
