/********************************************************************************
** Form generated from reading UI file 'uservideo.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USERVIDEO_H
#define UI_USERVIDEO_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_UserVideoDlg
{
public:
    QHBoxLayout *horizontalLayout;

    void setupUi(QDialog *UserVideoDlg)
    {
        if (UserVideoDlg->objectName().isEmpty())
            UserVideoDlg->setObjectName(QString::fromUtf8("UserVideoDlg"));
        UserVideoDlg->resize(463, 337);
        UserVideoDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        UserVideoDlg->setSizeGripEnabled(true);
        horizontalLayout = new QHBoxLayout(UserVideoDlg);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

        retranslateUi(UserVideoDlg);

        QMetaObject::connectSlotsByName(UserVideoDlg);
    } // setupUi

    void retranslateUi(QDialog *UserVideoDlg)
    {
        UserVideoDlg->setWindowTitle(QCoreApplication::translate("UserVideoDlg", "Video", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UserVideoDlg: public Ui_UserVideoDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERVIDEO_H
