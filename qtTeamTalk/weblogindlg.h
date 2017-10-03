#ifndef WEBLOGINDLG_H
#define WEBLOGINDLG_H

#include <QDialog>

namespace Ui {
class WebLoginDlg;
}

class WebLoginDlg : public QDialog
{
    Q_OBJECT

public:
    explicit WebLoginDlg(QWidget *parent = 0);
    ~WebLoginDlg();

private:
    Ui::WebLoginDlg *ui;
};

#endif // WEBLOGINDLG_H
