/********************************************************************************
** Form generated from reading UI file 'serverlist.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERVERLIST_H
#define UI_SERVERLIST_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ServerListDlg
{
public:
    QHBoxLayout *horizontalLayout_5;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_3;
    QListWidget *listWidget;
    QCheckBox *freeserverChkBox;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *delButton;
    QSpacerItem *horizontalSpacer;
    QVBoxLayout *verticalLayout_5;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QLabel *label_6;
    QLineEdit *tcpportEdit;
    QLabel *label_5;
    QLabel *label_8;
    QLineEdit *udpportEdit;
    QLabel *label_7;
    QHBoxLayout *horizontalLayout_6;
    QComboBox *hostaddrBox;
    QToolButton *deleteBtn;
    QCheckBox *cryptChkBox;
    QGroupBox *groupBox_3;
    QFormLayout *formLayout;
    QLabel *label_10;
    QLabel *label_11;
    QLineEdit *passwordEdit;
    QComboBox *usernameBox;
    QGroupBox *groupBox_4;
    QFormLayout *formLayout_2;
    QLabel *label_4;
    QLineEdit *channelEdit;
    QLabel *label_12;
    QLineEdit *chanpasswdEdit;
    QHBoxLayout *horizontalLayout;
    QPushButton *clearButton;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *connectButton;
    QGroupBox *groupBox_5;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label;
    QLineEdit *nameEdit;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *genttButton;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *addupdButton;
    QSpacerItem *verticalSpacer;

    void setupUi(QDialog *ServerListDlg)
    {
        if (ServerListDlg->objectName().isEmpty())
            ServerListDlg->setObjectName(QString::fromUtf8("ServerListDlg"));
        ServerListDlg->resize(659, 572);
        ServerListDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        ServerListDlg->setSizeGripEnabled(true);
        horizontalLayout_5 = new QHBoxLayout(ServerListDlg);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        groupBox = new QGroupBox(ServerListDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_3 = new QVBoxLayout(groupBox);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        listWidget = new QListWidget(groupBox);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));

        verticalLayout_3->addWidget(listWidget);

        freeserverChkBox = new QCheckBox(groupBox);
        freeserverChkBox->setObjectName(QString::fromUtf8("freeserverChkBox"));

        verticalLayout_3->addWidget(freeserverChkBox);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        delButton = new QPushButton(groupBox);
        delButton->setObjectName(QString::fromUtf8("delButton"));

        horizontalLayout_3->addWidget(delButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);


        verticalLayout_3->addLayout(horizontalLayout_3);


        horizontalLayout_5->addWidget(groupBox);

        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        groupBox_2 = new QGroupBox(ServerListDlg);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout = new QVBoxLayout(groupBox_2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 0, 0, 1, 1);

        label_6 = new QLabel(groupBox_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 1, 0, 1, 1);

        tcpportEdit = new QLineEdit(groupBox_2);
        tcpportEdit->setObjectName(QString::fromUtf8("tcpportEdit"));

        gridLayout->addWidget(tcpportEdit, 1, 1, 1, 1);

        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 1, 2, 1, 1);

        label_8 = new QLabel(groupBox_2);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout->addWidget(label_8, 2, 0, 1, 1);

        udpportEdit = new QLineEdit(groupBox_2);
        udpportEdit->setObjectName(QString::fromUtf8("udpportEdit"));

        gridLayout->addWidget(udpportEdit, 2, 1, 1, 1);

        label_7 = new QLabel(groupBox_2);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout->addWidget(label_7, 2, 2, 1, 1);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        hostaddrBox = new QComboBox(groupBox_2);
        hostaddrBox->setObjectName(QString::fromUtf8("hostaddrBox"));
        hostaddrBox->setEditable(true);
        hostaddrBox->setDuplicatesEnabled(true);

        horizontalLayout_6->addWidget(hostaddrBox);

        deleteBtn = new QToolButton(groupBox_2);
        deleteBtn->setObjectName(QString::fromUtf8("deleteBtn"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/images/delete.png"), QSize(), QIcon::Normal, QIcon::On);
        deleteBtn->setIcon(icon);

        horizontalLayout_6->addWidget(deleteBtn);


        gridLayout->addLayout(horizontalLayout_6, 0, 1, 1, 2);

        cryptChkBox = new QCheckBox(groupBox_2);
        cryptChkBox->setObjectName(QString::fromUtf8("cryptChkBox"));

        gridLayout->addWidget(cryptChkBox, 3, 1, 1, 2);


        verticalLayout->addLayout(gridLayout);

        groupBox_3 = new QGroupBox(groupBox_2);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        formLayout = new QFormLayout(groupBox_3);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        label_10 = new QLabel(groupBox_3);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_10);

        label_11 = new QLabel(groupBox_3);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_11);

        passwordEdit = new QLineEdit(groupBox_3);
        passwordEdit->setObjectName(QString::fromUtf8("passwordEdit"));
        passwordEdit->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(1, QFormLayout::FieldRole, passwordEdit);

        usernameBox = new QComboBox(groupBox_3);
        usernameBox->setObjectName(QString::fromUtf8("usernameBox"));
        usernameBox->setEditable(true);

        formLayout->setWidget(0, QFormLayout::FieldRole, usernameBox);


        verticalLayout->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(groupBox_2);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        formLayout_2 = new QFormLayout(groupBox_4);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        label_4 = new QLabel(groupBox_4);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label_4);

        channelEdit = new QLineEdit(groupBox_4);
        channelEdit->setObjectName(QString::fromUtf8("channelEdit"));

        formLayout_2->setWidget(0, QFormLayout::FieldRole, channelEdit);

        label_12 = new QLabel(groupBox_4);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        formLayout_2->setWidget(1, QFormLayout::LabelRole, label_12);

        chanpasswdEdit = new QLineEdit(groupBox_4);
        chanpasswdEdit->setObjectName(QString::fromUtf8("chanpasswdEdit"));
        chanpasswdEdit->setEchoMode(QLineEdit::Password);

        formLayout_2->setWidget(1, QFormLayout::FieldRole, chanpasswdEdit);


        verticalLayout->addWidget(groupBox_4);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        clearButton = new QPushButton(groupBox_2);
        clearButton->setObjectName(QString::fromUtf8("clearButton"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(clearButton->sizePolicy().hasHeightForWidth());
        clearButton->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(clearButton);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_4);

        connectButton = new QPushButton(groupBox_2);
        connectButton->setObjectName(QString::fromUtf8("connectButton"));

        horizontalLayout->addWidget(connectButton);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_5->addWidget(groupBox_2);

        groupBox_5 = new QGroupBox(ServerListDlg);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        verticalLayout_2 = new QVBoxLayout(groupBox_5);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label = new QLabel(groupBox_5);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_4->addWidget(label);

        nameEdit = new QLineEdit(groupBox_5);
        nameEdit->setObjectName(QString::fromUtf8("nameEdit"));

        horizontalLayout_4->addWidget(nameEdit);


        verticalLayout_2->addLayout(horizontalLayout_4);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        genttButton = new QPushButton(groupBox_5);
        genttButton->setObjectName(QString::fromUtf8("genttButton"));

        horizontalLayout_2->addWidget(genttButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        addupdButton = new QPushButton(groupBox_5);
        addupdButton->setObjectName(QString::fromUtf8("addupdButton"));
        addupdButton->setEnabled(false);
        sizePolicy.setHeightForWidth(addupdButton->sizePolicy().hasHeightForWidth());
        addupdButton->setSizePolicy(sizePolicy);

        horizontalLayout_2->addWidget(addupdButton);


        verticalLayout_2->addLayout(horizontalLayout_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);


        verticalLayout_5->addWidget(groupBox_5);


        horizontalLayout_5->addLayout(verticalLayout_5);

#if QT_CONFIG(shortcut)
        label_2->setBuddy(hostaddrBox);
        label_6->setBuddy(tcpportEdit);
        label_8->setBuddy(udpportEdit);
        label_10->setBuddy(usernameBox);
        label_11->setBuddy(passwordEdit);
        label_4->setBuddy(channelEdit);
        label_12->setBuddy(chanpasswdEdit);
        label->setBuddy(nameEdit);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(hostaddrBox, deleteBtn);
        QWidget::setTabOrder(deleteBtn, tcpportEdit);
        QWidget::setTabOrder(tcpportEdit, udpportEdit);
        QWidget::setTabOrder(udpportEdit, cryptChkBox);
        QWidget::setTabOrder(cryptChkBox, usernameBox);
        QWidget::setTabOrder(usernameBox, passwordEdit);
        QWidget::setTabOrder(passwordEdit, channelEdit);
        QWidget::setTabOrder(channelEdit, chanpasswdEdit);
        QWidget::setTabOrder(chanpasswdEdit, connectButton);
        QWidget::setTabOrder(connectButton, clearButton);
        QWidget::setTabOrder(clearButton, nameEdit);
        QWidget::setTabOrder(nameEdit, addupdButton);
        QWidget::setTabOrder(addupdButton, genttButton);
        QWidget::setTabOrder(genttButton, listWidget);
        QWidget::setTabOrder(listWidget, freeserverChkBox);
        QWidget::setTabOrder(freeserverChkBox, delButton);

        retranslateUi(ServerListDlg);

        QMetaObject::connectSlotsByName(ServerListDlg);
    } // setupUi

    void retranslateUi(QDialog *ServerListDlg)
    {
        ServerListDlg->setWindowTitle(QCoreApplication::translate("ServerListDlg", "Connect to a Server", nullptr));
        groupBox->setTitle(QCoreApplication::translate("ServerListDlg", "Server List", nullptr));
        freeserverChkBox->setText(QCoreApplication::translate("ServerListDlg", "Include public servers (green ones)", nullptr));
        delButton->setText(QCoreApplication::translate("ServerListDlg", "&Delete", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("ServerListDlg", "Server Information", nullptr));
        label_2->setText(QCoreApplication::translate("ServerListDlg", "Host IP-address", nullptr));
        label_6->setText(QCoreApplication::translate("ServerListDlg", "TCP port", nullptr));
        label_5->setText(QCoreApplication::translate("ServerListDlg", "Default: 10333", nullptr));
        label_8->setText(QCoreApplication::translate("ServerListDlg", "UDP port", nullptr));
        label_7->setText(QCoreApplication::translate("ServerListDlg", "Default: 10333", nullptr));
#if QT_CONFIG(tooltip)
        deleteBtn->setToolTip(QCoreApplication::translate("ServerListDlg", "Remove entry", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(accessibility)
        deleteBtn->setAccessibleName(QCoreApplication::translate("ServerListDlg", "Remove entry", nullptr));
#endif // QT_CONFIG(accessibility)
        deleteBtn->setText(QString());
        cryptChkBox->setText(QCoreApplication::translate("ServerListDlg", "Encrypted server", nullptr));
#if QT_CONFIG(accessibility)
        groupBox_3->setAccessibleName(QCoreApplication::translate("ServerListDlg", "Authentication (optional)", nullptr));
#endif // QT_CONFIG(accessibility)
        groupBox_3->setTitle(QCoreApplication::translate("ServerListDlg", "Authentication", nullptr));
        label_10->setText(QCoreApplication::translate("ServerListDlg", "Username", nullptr));
        label_11->setText(QCoreApplication::translate("ServerListDlg", "Password", nullptr));
#if QT_CONFIG(accessibility)
        groupBox_4->setAccessibleName(QCoreApplication::translate("ServerListDlg", "Join specific channel after authentication (optional)", nullptr));
#endif // QT_CONFIG(accessibility)
        groupBox_4->setTitle(QCoreApplication::translate("ServerListDlg", "Join specific channel after authentication (optional)", nullptr));
        label_4->setText(QCoreApplication::translate("ServerListDlg", "Channel", nullptr));
        label_12->setText(QCoreApplication::translate("ServerListDlg", "Password", nullptr));
        clearButton->setText(QCoreApplication::translate("ServerListDlg", "C&lear", nullptr));
        connectButton->setText(QCoreApplication::translate("ServerListDlg", "&Connect", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("ServerListDlg", "Save Server Information", nullptr));
        label->setText(QCoreApplication::translate("ServerListDlg", "Entry name", nullptr));
#if QT_CONFIG(tooltip)
        genttButton->setToolTip(QCoreApplication::translate("ServerListDlg", ".tt files can be double-clicked and the client will automatically start and connect to the specified server", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        genttButton->setWhatsThis(QCoreApplication::translate("ServerListDlg", ".tt files can be double-clicked and the client will automatically start and connect to the specified server", nullptr));
#endif // QT_CONFIG(whatsthis)
        genttButton->setText(QCoreApplication::translate("ServerListDlg", "&Generate .tt File", nullptr));
        addupdButton->setText(QCoreApplication::translate("ServerListDlg", "&Save", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ServerListDlg: public Ui_ServerListDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERVERLIST_H
