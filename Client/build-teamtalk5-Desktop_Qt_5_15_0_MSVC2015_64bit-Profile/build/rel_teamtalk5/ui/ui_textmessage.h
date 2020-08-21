/********************************************************************************
** Form generated from reading UI file 'textmessage.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEXTMESSAGE_H
#define UI_TEXTMESSAGE_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include "chattextedit.h"
#include "sendtextedit.h"

QT_BEGIN_NAMESPACE

class Ui_TextMessageDlg
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    ChatTextEdit *historyTextEdit;
    QLabel *newmsgLabel;
    SendTextEdit *newmsgTextEdit;
    QHBoxLayout *horizontalLayout;
    QPushButton *cancelButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *sendButton;

    void setupUi(QDialog *TextMessageDlg)
    {
        if (TextMessageDlg->objectName().isEmpty())
            TextMessageDlg->setObjectName(QString::fromUtf8("TextMessageDlg"));
        TextMessageDlg->resize(272, 300);
        TextMessageDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        TextMessageDlg->setSizeGripEnabled(true);
        verticalLayout = new QVBoxLayout(TextMessageDlg);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(TextMessageDlg);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        historyTextEdit = new ChatTextEdit(TextMessageDlg);
        historyTextEdit->setObjectName(QString::fromUtf8("historyTextEdit"));
        historyTextEdit->setTabChangesFocus(true);
        historyTextEdit->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse|Qt::TextBrowserInteraction|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout->addWidget(historyTextEdit);

        newmsgLabel = new QLabel(TextMessageDlg);
        newmsgLabel->setObjectName(QString::fromUtf8("newmsgLabel"));

        verticalLayout->addWidget(newmsgLabel);

        newmsgTextEdit = new SendTextEdit(TextMessageDlg);
        newmsgTextEdit->setObjectName(QString::fromUtf8("newmsgTextEdit"));
        newmsgTextEdit->setTabChangesFocus(true);

        verticalLayout->addWidget(newmsgTextEdit);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        cancelButton = new QPushButton(TextMessageDlg);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(cancelButton->sizePolicy().hasHeightForWidth());
        cancelButton->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(cancelButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        sendButton = new QPushButton(TextMessageDlg);
        sendButton->setObjectName(QString::fromUtf8("sendButton"));
        sizePolicy.setHeightForWidth(sendButton->sizePolicy().hasHeightForWidth());
        sendButton->setSizePolicy(sizePolicy);
        sendButton->setFlat(false);

        horizontalLayout->addWidget(sendButton);


        verticalLayout->addLayout(horizontalLayout);

        verticalLayout->setStretch(3, 1);
#if QT_CONFIG(shortcut)
        label->setBuddy(historyTextEdit);
        newmsgLabel->setBuddy(newmsgTextEdit);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(newmsgTextEdit, sendButton);
        QWidget::setTabOrder(sendButton, cancelButton);
        QWidget::setTabOrder(cancelButton, historyTextEdit);

        retranslateUi(TextMessageDlg);

        sendButton->setDefault(true);


        QMetaObject::connectSlotsByName(TextMessageDlg);
    } // setupUi

    void retranslateUi(QDialog *TextMessageDlg)
    {
        TextMessageDlg->setWindowTitle(QCoreApplication::translate("TextMessageDlg", "Messages", nullptr));
        label->setText(QCoreApplication::translate("TextMessageDlg", "History", nullptr));
        newmsgLabel->setText(QCoreApplication::translate("TextMessageDlg", "New message", nullptr));
        cancelButton->setText(QCoreApplication::translate("TextMessageDlg", "&Cancel", nullptr));
        sendButton->setText(QCoreApplication::translate("TextMessageDlg", "&Send", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TextMessageDlg: public Ui_TextMessageDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEXTMESSAGE_H
