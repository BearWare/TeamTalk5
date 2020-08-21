/********************************************************************************
** Form generated from reading UI file 'changestatus.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHANGESTATUS_H
#define UI_CHANGESTATUS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ChangeStatusDlg
{
public:
    QVBoxLayout *verticalLayout_2;
    QFormLayout *formLayout;
    QLabel *label;
    QVBoxLayout *verticalLayout;
    QRadioButton *availBtn;
    QRadioButton *awayBtn;
    QRadioButton *questionBtn;
    QLabel *label_2;
    QLineEdit *msgEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ChangeStatusDlg)
    {
        if (ChangeStatusDlg->objectName().isEmpty())
            ChangeStatusDlg->setObjectName(QString::fromUtf8("ChangeStatusDlg"));
        ChangeStatusDlg->resize(277, 145);
        ChangeStatusDlg->setModal(true);
        verticalLayout_2 = new QVBoxLayout(ChangeStatusDlg);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        label = new QLabel(ChangeStatusDlg);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        availBtn = new QRadioButton(ChangeStatusDlg);
        availBtn->setObjectName(QString::fromUtf8("availBtn"));

        verticalLayout->addWidget(availBtn);

        awayBtn = new QRadioButton(ChangeStatusDlg);
        awayBtn->setObjectName(QString::fromUtf8("awayBtn"));

        verticalLayout->addWidget(awayBtn);

        questionBtn = new QRadioButton(ChangeStatusDlg);
        questionBtn->setObjectName(QString::fromUtf8("questionBtn"));

        verticalLayout->addWidget(questionBtn);


        formLayout->setLayout(0, QFormLayout::FieldRole, verticalLayout);

        label_2 = new QLabel(ChangeStatusDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        msgEdit = new QLineEdit(ChangeStatusDlg);
        msgEdit->setObjectName(QString::fromUtf8("msgEdit"));

        formLayout->setWidget(1, QFormLayout::FieldRole, msgEdit);


        verticalLayout_2->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(ChangeStatusDlg);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_2->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        label->setBuddy(availBtn);
        label_2->setBuddy(msgEdit);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(availBtn, awayBtn);
        QWidget::setTabOrder(awayBtn, questionBtn);
        QWidget::setTabOrder(questionBtn, msgEdit);
        QWidget::setTabOrder(msgEdit, buttonBox);

        retranslateUi(ChangeStatusDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), ChangeStatusDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ChangeStatusDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(ChangeStatusDlg);
    } // setupUi

    void retranslateUi(QDialog *ChangeStatusDlg)
    {
        ChangeStatusDlg->setWindowTitle(QCoreApplication::translate("ChangeStatusDlg", "Change Status", nullptr));
#if QT_CONFIG(tooltip)
        ChangeStatusDlg->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        ChangeStatusDlg->setWhatsThis(QString());
#endif // QT_CONFIG(whatsthis)
        label->setText(QCoreApplication::translate("ChangeStatusDlg", "Status mode", nullptr));
        availBtn->setText(QCoreApplication::translate("ChangeStatusDlg", "&Available", nullptr));
        awayBtn->setText(QCoreApplication::translate("ChangeStatusDlg", "A&way", nullptr));
        questionBtn->setText(QCoreApplication::translate("ChangeStatusDlg", "&Question", nullptr));
        label_2->setText(QCoreApplication::translate("ChangeStatusDlg", "Message", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChangeStatusDlg: public Ui_ChangeStatusDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHANGESTATUS_H
