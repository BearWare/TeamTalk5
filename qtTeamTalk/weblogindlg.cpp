#include "weblogindlg.h"
#include "ui_weblogin.h"

WebLoginDlg::WebLoginDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WebLoginDlg)
{
    ui->setupUi(this);
}

WebLoginDlg::~WebLoginDlg()
{
    delete ui;
}
