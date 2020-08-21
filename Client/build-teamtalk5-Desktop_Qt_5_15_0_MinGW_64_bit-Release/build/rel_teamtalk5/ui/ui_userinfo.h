/********************************************************************************
** Form generated from reading UI file 'userinfo.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USERINFO_H
#define UI_USERINFO_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_UserInfoDlg
{
public:
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *userid;
    QLabel *label_3;
    QLineEdit *nickname;
    QLabel *label_5;
    QHBoxLayout *horizontalLayout;
    QLineEdit *username;
    QPushButton *profileBtn;
    QLabel *label_11;
    QLineEdit *clientname;
    QLabel *label_7;
    QLineEdit *statusmode;
    QLabel *label_9;
    QLineEdit *statusmsg;
    QLabel *label_15;
    QLineEdit *usertype;
    QLabel *label_8;
    QLineEdit *ipaddr;
    QLabel *label_4;
    QLineEdit *version;
    QLabel *label_13;
    QLineEdit *voicepacketloss;
    QLabel *label_6;
    QLineEdit *vidpacketloss;
    QLabel *label_2;
    QLineEdit *mediaaudpacketloss;
    QLabel *label_10;
    QLineEdit *mediavidpacketloss;

    void setupUi(QDialog *UserInfoDlg)
    {
        if (UserInfoDlg->objectName().isEmpty())
            UserInfoDlg->setObjectName(QString::fromUtf8("UserInfoDlg"));
        UserInfoDlg->resize(359, 386);
        UserInfoDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        formLayout = new QFormLayout(UserInfoDlg);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        label = new QLabel(UserInfoDlg);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        userid = new QLineEdit(UserInfoDlg);
        userid->setObjectName(QString::fromUtf8("userid"));
        userid->setReadOnly(true);

        formLayout->setWidget(0, QFormLayout::FieldRole, userid);

        label_3 = new QLabel(UserInfoDlg);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_3);

        nickname = new QLineEdit(UserInfoDlg);
        nickname->setObjectName(QString::fromUtf8("nickname"));
        nickname->setReadOnly(true);

        formLayout->setWidget(1, QFormLayout::FieldRole, nickname);

        label_5 = new QLabel(UserInfoDlg);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_5);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        username = new QLineEdit(UserInfoDlg);
        username->setObjectName(QString::fromUtf8("username"));
        username->setReadOnly(true);

        horizontalLayout->addWidget(username);

        profileBtn = new QPushButton(UserInfoDlg);
        profileBtn->setObjectName(QString::fromUtf8("profileBtn"));

        horizontalLayout->addWidget(profileBtn);


        formLayout->setLayout(2, QFormLayout::FieldRole, horizontalLayout);

        label_11 = new QLabel(UserInfoDlg);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_11);

        clientname = new QLineEdit(UserInfoDlg);
        clientname->setObjectName(QString::fromUtf8("clientname"));
        clientname->setReadOnly(true);

        formLayout->setWidget(3, QFormLayout::FieldRole, clientname);

        label_7 = new QLabel(UserInfoDlg);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        formLayout->setWidget(4, QFormLayout::LabelRole, label_7);

        statusmode = new QLineEdit(UserInfoDlg);
        statusmode->setObjectName(QString::fromUtf8("statusmode"));
        statusmode->setReadOnly(true);

        formLayout->setWidget(4, QFormLayout::FieldRole, statusmode);

        label_9 = new QLabel(UserInfoDlg);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        formLayout->setWidget(5, QFormLayout::LabelRole, label_9);

        statusmsg = new QLineEdit(UserInfoDlg);
        statusmsg->setObjectName(QString::fromUtf8("statusmsg"));
        statusmsg->setReadOnly(true);

        formLayout->setWidget(5, QFormLayout::FieldRole, statusmsg);

        label_15 = new QLabel(UserInfoDlg);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setFrameShape(QFrame::NoFrame);

        formLayout->setWidget(6, QFormLayout::LabelRole, label_15);

        usertype = new QLineEdit(UserInfoDlg);
        usertype->setObjectName(QString::fromUtf8("usertype"));
        usertype->setReadOnly(true);

        formLayout->setWidget(6, QFormLayout::FieldRole, usertype);

        label_8 = new QLabel(UserInfoDlg);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        formLayout->setWidget(7, QFormLayout::LabelRole, label_8);

        ipaddr = new QLineEdit(UserInfoDlg);
        ipaddr->setObjectName(QString::fromUtf8("ipaddr"));
        ipaddr->setReadOnly(true);

        formLayout->setWidget(7, QFormLayout::FieldRole, ipaddr);

        label_4 = new QLabel(UserInfoDlg);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout->setWidget(8, QFormLayout::LabelRole, label_4);

        version = new QLineEdit(UserInfoDlg);
        version->setObjectName(QString::fromUtf8("version"));
        version->setReadOnly(true);

        formLayout->setWidget(8, QFormLayout::FieldRole, version);

        label_13 = new QLabel(UserInfoDlg);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        formLayout->setWidget(9, QFormLayout::LabelRole, label_13);

        voicepacketloss = new QLineEdit(UserInfoDlg);
        voicepacketloss->setObjectName(QString::fromUtf8("voicepacketloss"));
        voicepacketloss->setReadOnly(true);

        formLayout->setWidget(9, QFormLayout::FieldRole, voicepacketloss);

        label_6 = new QLabel(UserInfoDlg);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        formLayout->setWidget(10, QFormLayout::LabelRole, label_6);

        vidpacketloss = new QLineEdit(UserInfoDlg);
        vidpacketloss->setObjectName(QString::fromUtf8("vidpacketloss"));
        vidpacketloss->setReadOnly(true);

        formLayout->setWidget(10, QFormLayout::FieldRole, vidpacketloss);

        label_2 = new QLabel(UserInfoDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(11, QFormLayout::LabelRole, label_2);

        mediaaudpacketloss = new QLineEdit(UserInfoDlg);
        mediaaudpacketloss->setObjectName(QString::fromUtf8("mediaaudpacketloss"));

        formLayout->setWidget(11, QFormLayout::FieldRole, mediaaudpacketloss);

        label_10 = new QLabel(UserInfoDlg);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        formLayout->setWidget(12, QFormLayout::LabelRole, label_10);

        mediavidpacketloss = new QLineEdit(UserInfoDlg);
        mediavidpacketloss->setObjectName(QString::fromUtf8("mediavidpacketloss"));

        formLayout->setWidget(12, QFormLayout::FieldRole, mediavidpacketloss);

#if QT_CONFIG(shortcut)
        label->setBuddy(userid);
        label_3->setBuddy(nickname);
        label_5->setBuddy(username);
        label_11->setBuddy(clientname);
        label_7->setBuddy(statusmode);
        label_9->setBuddy(statusmsg);
        label_15->setBuddy(usertype);
        label_8->setBuddy(ipaddr);
        label_4->setBuddy(version);
        label_13->setBuddy(voicepacketloss);
        label_6->setBuddy(vidpacketloss);
        label_2->setBuddy(mediaaudpacketloss);
        label_10->setBuddy(mediavidpacketloss);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(userid, nickname);
        QWidget::setTabOrder(nickname, username);
        QWidget::setTabOrder(username, clientname);
        QWidget::setTabOrder(clientname, statusmode);
        QWidget::setTabOrder(statusmode, statusmsg);
        QWidget::setTabOrder(statusmsg, usertype);
        QWidget::setTabOrder(usertype, ipaddr);
        QWidget::setTabOrder(ipaddr, version);
        QWidget::setTabOrder(version, voicepacketloss);
        QWidget::setTabOrder(voicepacketloss, vidpacketloss);
        QWidget::setTabOrder(vidpacketloss, mediaaudpacketloss);
        QWidget::setTabOrder(mediaaudpacketloss, mediavidpacketloss);

        retranslateUi(UserInfoDlg);

        QMetaObject::connectSlotsByName(UserInfoDlg);
    } // setupUi

    void retranslateUi(QDialog *UserInfoDlg)
    {
        UserInfoDlg->setWindowTitle(QCoreApplication::translate("UserInfoDlg", "User Information", nullptr));
        label->setText(QCoreApplication::translate("UserInfoDlg", "User ID", nullptr));
        userid->setText(QString());
        label_3->setText(QCoreApplication::translate("UserInfoDlg", "Nickname", nullptr));
        nickname->setText(QString());
        label_5->setText(QCoreApplication::translate("UserInfoDlg", "Username", nullptr));
        username->setText(QString());
        profileBtn->setText(QCoreApplication::translate("UserInfoDlg", "Profile", nullptr));
        label_11->setText(QCoreApplication::translate("UserInfoDlg", "Client name", nullptr));
        label_7->setText(QCoreApplication::translate("UserInfoDlg", "Status mode", nullptr));
        statusmode->setText(QString());
        label_9->setText(QCoreApplication::translate("UserInfoDlg", "Status message", nullptr));
        statusmsg->setText(QString());
        label_15->setText(QCoreApplication::translate("UserInfoDlg", "User type", nullptr));
        usertype->setText(QString());
        label_8->setText(QCoreApplication::translate("UserInfoDlg", "IP-address", nullptr));
        ipaddr->setText(QString());
        label_4->setText(QCoreApplication::translate("UserInfoDlg", "Version", nullptr));
        version->setText(QString());
        label_13->setText(QCoreApplication::translate("UserInfoDlg", "Voice packet loss", nullptr));
        voicepacketloss->setText(QString());
        label_6->setText(QCoreApplication::translate("UserInfoDlg", "Video frame loss", nullptr));
        vidpacketloss->setText(QString());
        label_2->setText(QCoreApplication::translate("UserInfoDlg", "Audio file packets loss", nullptr));
        label_10->setText(QCoreApplication::translate("UserInfoDlg", "Video file frame loss", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UserInfoDlg: public Ui_UserInfoDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERINFO_H
