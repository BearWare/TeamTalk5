/********************************************************************************
** Form generated from reading UI file 'keycomp.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_KEYCOMP_H
#define UI_KEYCOMP_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_KeyCompDlg
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QLineEdit *keycompEdit;
    QLabel *label_3;

    void setupUi(QDialog *KeyCompDlg)
    {
        if (KeyCompDlg->objectName().isEmpty())
            KeyCompDlg->setObjectName(QString::fromUtf8("KeyCompDlg"));
        KeyCompDlg->setWindowModality(Qt::NonModal);
        KeyCompDlg->resize(320, 232);
        KeyCompDlg->setFocusPolicy(Qt::StrongFocus);
        KeyCompDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        KeyCompDlg->setModal(true);
        verticalLayout = new QVBoxLayout(KeyCompDlg);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox = new QGroupBox(KeyCompDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setWordWrap(true);

        verticalLayout_2->addWidget(label);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        keycompEdit = new QLineEdit(groupBox);
        keycompEdit->setObjectName(QString::fromUtf8("keycompEdit"));
        keycompEdit->setEnabled(false);
        keycompEdit->setReadOnly(true);

        horizontalLayout->addWidget(keycompEdit);


        verticalLayout_2->addLayout(horizontalLayout);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setWordWrap(true);

        verticalLayout_2->addWidget(label_3);


        verticalLayout->addWidget(groupBox);

#if QT_CONFIG(shortcut)
        label_2->setBuddy(keycompEdit);
#endif // QT_CONFIG(shortcut)

        retranslateUi(KeyCompDlg);

        QMetaObject::connectSlotsByName(KeyCompDlg);
    } // setupUi

    void retranslateUi(QDialog *KeyCompDlg)
    {
        KeyCompDlg->setWindowTitle(QCoreApplication::translate("KeyCompDlg", "Key Combination", nullptr));
        groupBox->setTitle(QCoreApplication::translate("KeyCompDlg", "Setup Hotkey", nullptr));
        label->setText(QCoreApplication::translate("KeyCompDlg", "Hold down the keys which should be used as a hot key. Modifier keys like Shift, Ctrl and Alt can be used in combination with other keys", nullptr));
        label_2->setText(QCoreApplication::translate("KeyCompDlg", "Current key combination", nullptr));
        label_3->setText(QCoreApplication::translate("KeyCompDlg", "This dialog closes when you have released all keys", nullptr));
    } // retranslateUi

};

namespace Ui {
    class KeyCompDlg: public Ui_KeyCompDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_KEYCOMP_H
