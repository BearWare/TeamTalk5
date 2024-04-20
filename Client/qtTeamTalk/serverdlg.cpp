#include "serverdlg.h"
#include "ui_serverdlg.h"

ServerDlg::ServerDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ServerDlg)
{
    ui->setupUi(this);
}

ServerDlg::~ServerDlg()
{
    delete ui;
}
