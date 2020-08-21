/********************************************************************************
** Form generated from reading UI file 'serverproperties.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERVERPROPERTIES_H
#define UI_SERVERPROPERTIES_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ServerPropertiesDlg
{
public:
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *servernameEdit;
    QLabel *label_7;
    QSpinBox *maxusersSpinBox;
    QLabel *label_2;
    QPlainTextEdit *motdTextEdit;
    QCheckBox *motdChkBox;
    QLabel *label_4;
    QLabel *label_5;
    QSpinBox *tcpportSpinBox;
    QLabel *label_6;
    QSpinBox *udpportSpinBox;
    QLabel *label_9;
    QSpinBox *usertimeoutSpinBox;
    QCheckBox *autosaveBox;
    QSpacerItem *verticalSpacer_2;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox_5;
    QGridLayout *gridLayout;
    QLabel *label_16;
    QSpinBox *videotxSpinBox;
    QLabel *label_17;
    QSpinBox *totaltxSpinBox;
    QLabel *label_13;
    QLabel *label_14;
    QLabel *label_15;
    QSpinBox *audtxSpinBox;
    QLabel *label_12;
    QLabel *label_18;
    QSpinBox *desktoptxSpinBox;
    QLabel *label_19;
    QLabel *label_24;
    QSpinBox *mediafiletxSpinBox;
    QLabel *label_25;
    QGroupBox *groupBox_6;
    QGridLayout *gridLayout_2;
    QSpinBox *maxloginattemptsSpinBox;
    QLabel *label_23;
    QSpinBox *maxiploginsSpinBox;
    QLabel *label_22;
    QLabel *label_20;
    QLabel *label_21;
    QLabel *label_3;
    QSpinBox *logindelaySpinBox;
    QLabel *label_10;
    QGroupBox *groupBox_3;
    QFormLayout *formLayout_2;
    QLabel *label_8;
    QLineEdit *serverversionEdit;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ServerPropertiesDlg)
    {
        if (ServerPropertiesDlg->objectName().isEmpty())
            ServerPropertiesDlg->setObjectName(QString::fromUtf8("ServerPropertiesDlg"));
        ServerPropertiesDlg->resize(751, 674);
        ServerPropertiesDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        horizontalLayout = new QHBoxLayout(ServerPropertiesDlg);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        groupBox = new QGroupBox(ServerPropertiesDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        formLayout = new QFormLayout(groupBox);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        servernameEdit = new QLineEdit(groupBox);
        servernameEdit->setObjectName(QString::fromUtf8("servernameEdit"));

        formLayout->setWidget(0, QFormLayout::FieldRole, servernameEdit);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_7);

        maxusersSpinBox = new QSpinBox(groupBox);
        maxusersSpinBox->setObjectName(QString::fromUtf8("maxusersSpinBox"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(maxusersSpinBox->sizePolicy().hasHeightForWidth());
        maxusersSpinBox->setSizePolicy(sizePolicy);
        maxusersSpinBox->setMaximum(1000);
        maxusersSpinBox->setValue(1000);

        formLayout->setWidget(1, QFormLayout::FieldRole, maxusersSpinBox);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setWordWrap(false);

        formLayout->setWidget(2, QFormLayout::LabelRole, label_2);

        motdTextEdit = new QPlainTextEdit(groupBox);
        motdTextEdit->setObjectName(QString::fromUtf8("motdTextEdit"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(motdTextEdit->sizePolicy().hasHeightForWidth());
        motdTextEdit->setSizePolicy(sizePolicy1);

        formLayout->setWidget(2, QFormLayout::FieldRole, motdTextEdit);

        motdChkBox = new QCheckBox(groupBox);
        motdChkBox->setObjectName(QString::fromUtf8("motdChkBox"));

        formLayout->setWidget(3, QFormLayout::LabelRole, motdChkBox);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setWordWrap(true);

        formLayout->setWidget(3, QFormLayout::FieldRole, label_4);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        formLayout->setWidget(4, QFormLayout::LabelRole, label_5);

        tcpportSpinBox = new QSpinBox(groupBox);
        tcpportSpinBox->setObjectName(QString::fromUtf8("tcpportSpinBox"));
        sizePolicy.setHeightForWidth(tcpportSpinBox->sizePolicy().hasHeightForWidth());
        tcpportSpinBox->setSizePolicy(sizePolicy);
        tcpportSpinBox->setMinimum(1);
        tcpportSpinBox->setMaximum(65535);
        tcpportSpinBox->setValue(10333);

        formLayout->setWidget(4, QFormLayout::FieldRole, tcpportSpinBox);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        formLayout->setWidget(5, QFormLayout::LabelRole, label_6);

        udpportSpinBox = new QSpinBox(groupBox);
        udpportSpinBox->setObjectName(QString::fromUtf8("udpportSpinBox"));
        sizePolicy.setHeightForWidth(udpportSpinBox->sizePolicy().hasHeightForWidth());
        udpportSpinBox->setSizePolicy(sizePolicy);
        udpportSpinBox->setMinimum(1);
        udpportSpinBox->setMaximum(65535);
        udpportSpinBox->setValue(10333);

        formLayout->setWidget(5, QFormLayout::FieldRole, udpportSpinBox);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        formLayout->setWidget(6, QFormLayout::LabelRole, label_9);

        usertimeoutSpinBox = new QSpinBox(groupBox);
        usertimeoutSpinBox->setObjectName(QString::fromUtf8("usertimeoutSpinBox"));
        sizePolicy.setHeightForWidth(usertimeoutSpinBox->sizePolicy().hasHeightForWidth());
        usertimeoutSpinBox->setSizePolicy(sizePolicy);
        usertimeoutSpinBox->setMinimum(1);
        usertimeoutSpinBox->setMaximum(99999);

        formLayout->setWidget(6, QFormLayout::FieldRole, usertimeoutSpinBox);

        autosaveBox = new QCheckBox(groupBox);
        autosaveBox->setObjectName(QString::fromUtf8("autosaveBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(autosaveBox->sizePolicy().hasHeightForWidth());
        autosaveBox->setSizePolicy(sizePolicy2);

        formLayout->setWidget(8, QFormLayout::SpanningRole, autosaveBox);


        verticalLayout_3->addWidget(groupBox);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_2);


        horizontalLayout->addLayout(verticalLayout_3);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox_5 = new QGroupBox(ServerPropertiesDlg);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        gridLayout = new QGridLayout(groupBox_5);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_16 = new QLabel(groupBox_5);
        label_16->setObjectName(QString::fromUtf8("label_16"));

        gridLayout->addWidget(label_16, 1, 0, 1, 1);

        videotxSpinBox = new QSpinBox(groupBox_5);
        videotxSpinBox->setObjectName(QString::fromUtf8("videotxSpinBox"));
        videotxSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        videotxSpinBox->setMaximum(1000000);

        gridLayout->addWidget(videotxSpinBox, 1, 1, 1, 1);

        label_17 = new QLabel(groupBox_5);
        label_17->setObjectName(QString::fromUtf8("label_17"));

        gridLayout->addWidget(label_17, 1, 2, 1, 1);

        totaltxSpinBox = new QSpinBox(groupBox_5);
        totaltxSpinBox->setObjectName(QString::fromUtf8("totaltxSpinBox"));
        totaltxSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        totaltxSpinBox->setMaximum(1000000);

        gridLayout->addWidget(totaltxSpinBox, 4, 1, 1, 1);

        label_13 = new QLabel(groupBox_5);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        gridLayout->addWidget(label_13, 4, 2, 1, 1);

        label_14 = new QLabel(groupBox_5);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        gridLayout->addWidget(label_14, 0, 0, 1, 1);

        label_15 = new QLabel(groupBox_5);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        gridLayout->addWidget(label_15, 0, 2, 1, 1);

        audtxSpinBox = new QSpinBox(groupBox_5);
        audtxSpinBox->setObjectName(QString::fromUtf8("audtxSpinBox"));
        audtxSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        audtxSpinBox->setMaximum(1000000);

        gridLayout->addWidget(audtxSpinBox, 0, 1, 1, 1);

        label_12 = new QLabel(groupBox_5);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        gridLayout->addWidget(label_12, 4, 0, 1, 1);

        label_18 = new QLabel(groupBox_5);
        label_18->setObjectName(QString::fromUtf8("label_18"));

        gridLayout->addWidget(label_18, 3, 0, 1, 1);

        desktoptxSpinBox = new QSpinBox(groupBox_5);
        desktoptxSpinBox->setObjectName(QString::fromUtf8("desktoptxSpinBox"));
        desktoptxSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        desktoptxSpinBox->setMaximum(1000000);

        gridLayout->addWidget(desktoptxSpinBox, 3, 1, 1, 1);

        label_19 = new QLabel(groupBox_5);
        label_19->setObjectName(QString::fromUtf8("label_19"));

        gridLayout->addWidget(label_19, 3, 2, 1, 1);

        label_24 = new QLabel(groupBox_5);
        label_24->setObjectName(QString::fromUtf8("label_24"));

        gridLayout->addWidget(label_24, 2, 0, 1, 1);

        mediafiletxSpinBox = new QSpinBox(groupBox_5);
        mediafiletxSpinBox->setObjectName(QString::fromUtf8("mediafiletxSpinBox"));
        mediafiletxSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        mediafiletxSpinBox->setMaximum(1000000);

        gridLayout->addWidget(mediafiletxSpinBox, 2, 1, 1, 1);

        label_25 = new QLabel(groupBox_5);
        label_25->setObjectName(QString::fromUtf8("label_25"));

        gridLayout->addWidget(label_25, 2, 2, 1, 1);


        verticalLayout_2->addWidget(groupBox_5);

        groupBox_6 = new QGroupBox(ServerPropertiesDlg);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        gridLayout_2 = new QGridLayout(groupBox_6);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        maxloginattemptsSpinBox = new QSpinBox(groupBox_6);
        maxloginattemptsSpinBox->setObjectName(QString::fromUtf8("maxloginattemptsSpinBox"));
        sizePolicy.setHeightForWidth(maxloginattemptsSpinBox->sizePolicy().hasHeightForWidth());
        maxloginattemptsSpinBox->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(maxloginattemptsSpinBox, 0, 1, 1, 1);

        label_23 = new QLabel(groupBox_6);
        label_23->setObjectName(QString::fromUtf8("label_23"));

        gridLayout_2->addWidget(label_23, 0, 2, 1, 1);

        maxiploginsSpinBox = new QSpinBox(groupBox_6);
        maxiploginsSpinBox->setObjectName(QString::fromUtf8("maxiploginsSpinBox"));
        sizePolicy.setHeightForWidth(maxiploginsSpinBox->sizePolicy().hasHeightForWidth());
        maxiploginsSpinBox->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(maxiploginsSpinBox, 1, 1, 1, 1);

        label_22 = new QLabel(groupBox_6);
        label_22->setObjectName(QString::fromUtf8("label_22"));

        gridLayout_2->addWidget(label_22, 0, 0, 1, 1);

        label_20 = new QLabel(groupBox_6);
        label_20->setObjectName(QString::fromUtf8("label_20"));

        gridLayout_2->addWidget(label_20, 1, 0, 1, 1);

        label_21 = new QLabel(groupBox_6);
        label_21->setObjectName(QString::fromUtf8("label_21"));

        gridLayout_2->addWidget(label_21, 1, 2, 1, 1);

        label_3 = new QLabel(groupBox_6);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout_2->addWidget(label_3, 2, 0, 1, 1);

        logindelaySpinBox = new QSpinBox(groupBox_6);
        logindelaySpinBox->setObjectName(QString::fromUtf8("logindelaySpinBox"));
        logindelaySpinBox->setMaximum(100000);

        gridLayout_2->addWidget(logindelaySpinBox, 2, 1, 1, 1);

        label_10 = new QLabel(groupBox_6);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout_2->addWidget(label_10, 2, 2, 1, 1);


        verticalLayout_2->addWidget(groupBox_6);

        groupBox_3 = new QGroupBox(ServerPropertiesDlg);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        formLayout_2 = new QFormLayout(groupBox_3);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        formLayout_2->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        label_8 = new QLabel(groupBox_3);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label_8);

        serverversionEdit = new QLineEdit(groupBox_3);
        serverversionEdit->setObjectName(QString::fromUtf8("serverversionEdit"));
        sizePolicy.setHeightForWidth(serverversionEdit->sizePolicy().hasHeightForWidth());
        serverversionEdit->setSizePolicy(sizePolicy);
        serverversionEdit->setReadOnly(true);

        formLayout_2->setWidget(0, QFormLayout::FieldRole, serverversionEdit);


        verticalLayout_2->addWidget(groupBox_3);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(ServerPropertiesDlg);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_2->addWidget(buttonBox);


        horizontalLayout->addLayout(verticalLayout_2);

#if QT_CONFIG(shortcut)
        label->setBuddy(servernameEdit);
        label_7->setBuddy(maxusersSpinBox);
        label_2->setBuddy(motdTextEdit);
        label_4->setBuddy(motdTextEdit);
        label_5->setBuddy(tcpportSpinBox);
        label_6->setBuddy(udpportSpinBox);
        label_9->setBuddy(usertimeoutSpinBox);
        label_16->setBuddy(videotxSpinBox);
        label_14->setBuddy(audtxSpinBox);
        label_12->setBuddy(totaltxSpinBox);
        label_18->setBuddy(desktoptxSpinBox);
        label_24->setBuddy(videotxSpinBox);
        label_8->setBuddy(serverversionEdit);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(servernameEdit, maxusersSpinBox);
        QWidget::setTabOrder(maxusersSpinBox, motdTextEdit);
        QWidget::setTabOrder(motdTextEdit, motdChkBox);
        QWidget::setTabOrder(motdChkBox, tcpportSpinBox);
        QWidget::setTabOrder(tcpportSpinBox, udpportSpinBox);
        QWidget::setTabOrder(udpportSpinBox, autosaveBox);
        QWidget::setTabOrder(autosaveBox, usertimeoutSpinBox);
        QWidget::setTabOrder(usertimeoutSpinBox, audtxSpinBox);
        QWidget::setTabOrder(audtxSpinBox, videotxSpinBox);
        QWidget::setTabOrder(videotxSpinBox, mediafiletxSpinBox);
        QWidget::setTabOrder(mediafiletxSpinBox, desktoptxSpinBox);
        QWidget::setTabOrder(desktoptxSpinBox, totaltxSpinBox);
        QWidget::setTabOrder(totaltxSpinBox, maxloginattemptsSpinBox);
        QWidget::setTabOrder(maxloginattemptsSpinBox, maxiploginsSpinBox);
        QWidget::setTabOrder(maxiploginsSpinBox, logindelaySpinBox);
        QWidget::setTabOrder(logindelaySpinBox, serverversionEdit);

        retranslateUi(ServerPropertiesDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), ServerPropertiesDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ServerPropertiesDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(ServerPropertiesDlg);
    } // setupUi

    void retranslateUi(QDialog *ServerPropertiesDlg)
    {
        ServerPropertiesDlg->setWindowTitle(QCoreApplication::translate("ServerPropertiesDlg", "Server Properties", nullptr));
        groupBox->setTitle(QCoreApplication::translate("ServerPropertiesDlg", "Server Properties", nullptr));
        label->setText(QCoreApplication::translate("ServerPropertiesDlg", "Server name", nullptr));
        label_7->setText(QCoreApplication::translate("ServerPropertiesDlg", "Max users", nullptr));
        label_2->setText(QCoreApplication::translate("ServerPropertiesDlg", "Message of the day", nullptr));
        motdChkBox->setText(QCoreApplication::translate("ServerPropertiesDlg", "Show variables", nullptr));
        label_4->setText(QCoreApplication::translate("ServerPropertiesDlg", "MOTD variables: %users% number of users on server, %admins% number of admins on server, %uptime% server's time online, %voicetx% KBytes received, %voicerx% KBytes sent and %lastuser% last user to log on.", nullptr));
        label_5->setText(QCoreApplication::translate("ServerPropertiesDlg", "TCP port", nullptr));
        label_6->setText(QCoreApplication::translate("ServerPropertiesDlg", "UDP port", nullptr));
        label_9->setText(QCoreApplication::translate("ServerPropertiesDlg", "User timeout", nullptr));
        autosaveBox->setText(QCoreApplication::translate("ServerPropertiesDlg", "Auto save server changes", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("ServerPropertiesDlg", "Server Bandwidth Limitations", nullptr));
        label_16->setText(QCoreApplication::translate("ServerPropertiesDlg", "Video TX max", nullptr));
        label_17->setText(QCoreApplication::translate("ServerPropertiesDlg", "KBytes/sec (0 = disabled)", nullptr));
        label_13->setText(QCoreApplication::translate("ServerPropertiesDlg", "KBytes/sec (0 = disabled)", nullptr));
        label_14->setText(QCoreApplication::translate("ServerPropertiesDlg", "Voice TX max", nullptr));
        label_15->setText(QCoreApplication::translate("ServerPropertiesDlg", "KBytes/sec (0 = disabled)", nullptr));
        label_12->setText(QCoreApplication::translate("ServerPropertiesDlg", "Total TX max", nullptr));
        label_18->setText(QCoreApplication::translate("ServerPropertiesDlg", "Desktop TX max", nullptr));
        label_19->setText(QCoreApplication::translate("ServerPropertiesDlg", "KBytes/sec (0 = disabled)", nullptr));
        label_24->setText(QCoreApplication::translate("ServerPropertiesDlg", "Media File TX max", nullptr));
        label_25->setText(QCoreApplication::translate("ServerPropertiesDlg", "KBytes/sec (0 = disabled)", nullptr));
        groupBox_6->setTitle(QCoreApplication::translate("ServerPropertiesDlg", "Server Abuse", nullptr));
        label_23->setText(QCoreApplication::translate("ServerPropertiesDlg", "(0 = disabled)", nullptr));
        label_22->setText(QCoreApplication::translate("ServerPropertiesDlg", "Max login attempts before ban", nullptr));
        label_20->setText(QCoreApplication::translate("ServerPropertiesDlg", "Max logins per IP-address", nullptr));
        label_21->setText(QCoreApplication::translate("ServerPropertiesDlg", "(0 = disabled)", nullptr));
        label_3->setText(QCoreApplication::translate("ServerPropertiesDlg", "Login delay per IP-address", nullptr));
        label_10->setText(QCoreApplication::translate("ServerPropertiesDlg", "msec (0 = disabled)", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("ServerPropertiesDlg", "Server Information", nullptr));
        label_8->setText(QCoreApplication::translate("ServerPropertiesDlg", "Server version", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ServerPropertiesDlg: public Ui_ServerPropertiesDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERVERPROPERTIES_H
