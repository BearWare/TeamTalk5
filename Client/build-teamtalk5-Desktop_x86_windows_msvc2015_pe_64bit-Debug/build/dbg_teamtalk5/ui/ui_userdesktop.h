/********************************************************************************
** Form generated from reading UI file 'userdesktop.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USERDESKTOP_H
#define UI_USERDESKTOP_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include "userdesktopwidget.h"

QT_BEGIN_NAMESPACE

class Ui_UserDesktopDlg
{
public:
    QHBoxLayout *horizontalLayout;
    UserDesktopWidget *userdesktopWidget;

    void setupUi(QDialog *UserDesktopDlg)
    {
        if (UserDesktopDlg->objectName().isEmpty())
            UserDesktopDlg->setObjectName(QString::fromUtf8("UserDesktopDlg"));
        UserDesktopDlg->resize(463, 337);
        UserDesktopDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        UserDesktopDlg->setSizeGripEnabled(true);
        horizontalLayout = new QHBoxLayout(UserDesktopDlg);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        userdesktopWidget = new UserDesktopWidget(UserDesktopDlg);
        userdesktopWidget->setObjectName(QString::fromUtf8("userdesktopWidget"));
        userdesktopWidget->setFocusPolicy(Qt::StrongFocus);

        horizontalLayout->addWidget(userdesktopWidget);


        retranslateUi(UserDesktopDlg);

        QMetaObject::connectSlotsByName(UserDesktopDlg);
    } // setupUi

    void retranslateUi(QDialog *UserDesktopDlg)
    {
        UserDesktopDlg->setWindowTitle(QCoreApplication::translate("UserDesktopDlg", "Desktop", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UserDesktopDlg: public Ui_UserDesktopDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERDESKTOP_H
