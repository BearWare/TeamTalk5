/********************************************************************************
** Form generated from reading UI file 'weblogin.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WEBLOGIN_H
#define UI_WEBLOGIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_WebLoginDlg
{
public:
    QHBoxLayout *horizontalLayout;

    void setupUi(QDialog *WebLoginDlg)
    {
        if (WebLoginDlg->objectName().isEmpty())
            WebLoginDlg->setObjectName(QString::fromUtf8("WebLoginDlg"));
        WebLoginDlg->resize(800, 600);
        WebLoginDlg->setSizeGripEnabled(true);
        WebLoginDlg->setModal(true);
        horizontalLayout = new QHBoxLayout(WebLoginDlg);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);

        retranslateUi(WebLoginDlg);

        QMetaObject::connectSlotsByName(WebLoginDlg);
    } // setupUi

    void retranslateUi(QDialog *WebLoginDlg)
    {
        WebLoginDlg->setWindowTitle(QCoreApplication::translate("WebLoginDlg", "Web Login", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WebLoginDlg: public Ui_WebLoginDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WEBLOGIN_H
