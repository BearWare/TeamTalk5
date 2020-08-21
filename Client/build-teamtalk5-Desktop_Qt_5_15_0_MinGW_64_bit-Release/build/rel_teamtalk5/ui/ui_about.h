/********************************************************************************
** Form generated from reading UI file 'about.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUT_H
#define UI_ABOUT_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AboutDlg
{
public:
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QLabel *versionLabel;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *translationLabel;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_2;
    QLabel *compileLabel;
    QSpacerItem *verticalSpacer;
    QLabel *iconLabel;

    void setupUi(QDialog *AboutDlg)
    {
        if (AboutDlg->objectName().isEmpty())
            AboutDlg->setObjectName(QString::fromUtf8("AboutDlg"));
        AboutDlg->resize(333, 351);
        AboutDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        horizontalLayout_2 = new QHBoxLayout(AboutDlg);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(AboutDlg);
        label->setObjectName(QString::fromUtf8("label"));
        QFont font;
        font.setPointSize(32);
        font.setBold(true);
        font.setWeight(75);
        label->setFont(font);
        label->setText(QString::fromUtf8("TeamTalk"));

        verticalLayout->addWidget(label);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        versionLabel = new QLabel(AboutDlg);
        versionLabel->setObjectName(QString::fromUtf8("versionLabel"));
        QFont font1;
        font1.setPointSize(24);
        font1.setBold(true);
        font1.setWeight(75);
        versionLabel->setFont(font1);
        versionLabel->setText(QString::fromUtf8("5.0"));
        versionLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(versionLabel);


        verticalLayout->addLayout(horizontalLayout);

        label_3 = new QLabel(AboutDlg);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setWordWrap(true);

        verticalLayout->addWidget(label_3);

        label_4 = new QLabel(AboutDlg);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setText(QString::fromUtf8("Copyright (c) 2005-2020, BearWare.dk"));
        label_4->setWordWrap(true);

        verticalLayout->addWidget(label_4);

        translationLabel = new QLabel(AboutDlg);
        translationLabel->setObjectName(QString::fromUtf8("translationLabel"));

        verticalLayout->addWidget(translationLabel);

        label_5 = new QLabel(AboutDlg);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setWordWrap(true);

        verticalLayout->addWidget(label_5);

        label_6 = new QLabel(AboutDlg);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        verticalLayout->addWidget(label_6);

        label_2 = new QLabel(AboutDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setWordWrap(true);

        verticalLayout->addWidget(label_2);

        compileLabel = new QLabel(AboutDlg);
        compileLabel->setObjectName(QString::fromUtf8("compileLabel"));
        compileLabel->setText(QString::fromUtf8("Compile date"));
        compileLabel->setWordWrap(true);

        verticalLayout->addWidget(compileLabel);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        horizontalLayout_2->addLayout(verticalLayout);

        iconLabel = new QLabel(AboutDlg);
        iconLabel->setObjectName(QString::fromUtf8("iconLabel"));
        iconLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(iconLabel);


        retranslateUi(AboutDlg);

        QMetaObject::connectSlotsByName(AboutDlg);
    } // setupUi

    void retranslateUi(QDialog *AboutDlg)
    {
        AboutDlg->setWindowTitle(QCoreApplication::translate("AboutDlg", "About", nullptr));
        label_3->setText(QCoreApplication::translate("AboutDlg", "Written by Bjoern D. Rasmussen", nullptr));
        translationLabel->setText(QCoreApplication::translate("AboutDlg", "Translated by Bjoern D. Rasmussen", nullptr));
        label_5->setText(QCoreApplication::translate("AboutDlg", "Speex codec developed by Jean-Marc Valin, www.speex.org", nullptr));
        label_6->setText(QCoreApplication::translate("AboutDlg", "OPUS codec developed by Opus project, www.opus-codec.org", nullptr));
        label_2->setText(QCoreApplication::translate("AboutDlg", "WebM VP8 codec developed by WebM project, www.webmproject.org", nullptr));
        iconLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class AboutDlg: public Ui_AboutDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUT_H
