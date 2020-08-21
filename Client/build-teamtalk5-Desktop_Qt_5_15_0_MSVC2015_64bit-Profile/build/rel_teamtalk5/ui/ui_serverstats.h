/********************************************************************************
** Form generated from reading UI file 'serverstats.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERVERSTATS_H
#define UI_SERVERSTATS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ServerStatsDlg
{
public:
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label_6;
    QLabel *label_10;
    QLineEdit *uptimeEdit;
    QLabel *label_4;
    QLineEdit *kbpsEdit;
    QLabel *label_2;
    QLabel *label_12;
    QLabel *label_13;
    QSpinBox *msecSpinBox;
    QCheckBox *updateChkBox;
    QLabel *label_14;
    QLineEdit *mediafileEdit;
    QLineEdit *filesEdit;
    QLabel *label_15;
    QLabel *label;
    QLineEdit *totalEdit;
    QLabel *label_7;
    QLabel *label_3;
    QLineEdit *voiceEdit;
    QLabel *label_9;
    QLabel *label_5;
    QLineEdit *videoEdit;
    QLabel *label_11;
    QLabel *label_8;
    QLineEdit *desktopEdit;
    QLabel *label_16;
    QLabel *label_17;
    QLineEdit *usersservedEdit;
    QLabel *label_19;
    QLineEdit *userspeakEdit;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *closeBtn;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QDialog *ServerStatsDlg)
    {
        if (ServerStatsDlg->objectName().isEmpty())
            ServerStatsDlg->setObjectName(QString::fromUtf8("ServerStatsDlg"));
        ServerStatsDlg->resize(486, 346);
        verticalLayout = new QVBoxLayout(ServerStatsDlg);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_6 = new QLabel(ServerStatsDlg);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 9, 0, 1, 1);

        label_10 = new QLabel(ServerStatsDlg);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout->addWidget(label_10, 3, 0, 1, 1);

        uptimeEdit = new QLineEdit(ServerStatsDlg);
        uptimeEdit->setObjectName(QString::fromUtf8("uptimeEdit"));
        uptimeEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        uptimeEdit->setReadOnly(true);

        gridLayout->addWidget(uptimeEdit, 9, 1, 1, 1);

        label_4 = new QLabel(ServerStatsDlg);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 5, 2, 1, 1);

        kbpsEdit = new QLineEdit(ServerStatsDlg);
        kbpsEdit->setObjectName(QString::fromUtf8("kbpsEdit"));
        kbpsEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        kbpsEdit->setReadOnly(true);

        gridLayout->addWidget(kbpsEdit, 5, 1, 1, 1);

        label_2 = new QLabel(ServerStatsDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 5, 0, 1, 1);

        label_12 = new QLabel(ServerStatsDlg);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        gridLayout->addWidget(label_12, 4, 2, 1, 1);

        label_13 = new QLabel(ServerStatsDlg);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        gridLayout->addWidget(label_13, 11, 2, 1, 1);

        msecSpinBox = new QSpinBox(ServerStatsDlg);
        msecSpinBox->setObjectName(QString::fromUtf8("msecSpinBox"));
        msecSpinBox->setEnabled(false);
        msecSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        msecSpinBox->setMinimum(250);
        msecSpinBox->setMaximum(10000);
        msecSpinBox->setSingleStep(50);
        msecSpinBox->setValue(1000);

        gridLayout->addWidget(msecSpinBox, 11, 1, 1, 1);

        updateChkBox = new QCheckBox(ServerStatsDlg);
        updateChkBox->setObjectName(QString::fromUtf8("updateChkBox"));

        gridLayout->addWidget(updateChkBox, 11, 0, 1, 1);

        label_14 = new QLabel(ServerStatsDlg);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        gridLayout->addWidget(label_14, 3, 2, 1, 1);

        mediafileEdit = new QLineEdit(ServerStatsDlg);
        mediafileEdit->setObjectName(QString::fromUtf8("mediafileEdit"));
        mediafileEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(mediafileEdit, 3, 1, 1, 1);

        filesEdit = new QLineEdit(ServerStatsDlg);
        filesEdit->setObjectName(QString::fromUtf8("filesEdit"));
        filesEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        filesEdit->setReadOnly(true);

        gridLayout->addWidget(filesEdit, 6, 1, 1, 1);

        label_15 = new QLabel(ServerStatsDlg);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        gridLayout->addWidget(label_15, 6, 0, 1, 1);

        label = new QLabel(ServerStatsDlg);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        totalEdit = new QLineEdit(ServerStatsDlg);
        totalEdit->setObjectName(QString::fromUtf8("totalEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(totalEdit->sizePolicy().hasHeightForWidth());
        totalEdit->setSizePolicy(sizePolicy);
        totalEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        totalEdit->setReadOnly(true);

        gridLayout->addWidget(totalEdit, 0, 1, 1, 1);

        label_7 = new QLabel(ServerStatsDlg);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout->addWidget(label_7, 0, 2, 1, 1);

        label_3 = new QLabel(ServerStatsDlg);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        voiceEdit = new QLineEdit(ServerStatsDlg);
        voiceEdit->setObjectName(QString::fromUtf8("voiceEdit"));
        sizePolicy.setHeightForWidth(voiceEdit->sizePolicy().hasHeightForWidth());
        voiceEdit->setSizePolicy(sizePolicy);
        voiceEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        voiceEdit->setReadOnly(true);

        gridLayout->addWidget(voiceEdit, 1, 1, 1, 1);

        label_9 = new QLabel(ServerStatsDlg);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout->addWidget(label_9, 1, 2, 1, 1);

        label_5 = new QLabel(ServerStatsDlg);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 2, 0, 1, 1);

        videoEdit = new QLineEdit(ServerStatsDlg);
        videoEdit->setObjectName(QString::fromUtf8("videoEdit"));
        sizePolicy.setHeightForWidth(videoEdit->sizePolicy().hasHeightForWidth());
        videoEdit->setSizePolicy(sizePolicy);
        videoEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        videoEdit->setReadOnly(true);

        gridLayout->addWidget(videoEdit, 2, 1, 1, 1);

        label_11 = new QLabel(ServerStatsDlg);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout->addWidget(label_11, 2, 2, 1, 1);

        label_8 = new QLabel(ServerStatsDlg);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout->addWidget(label_8, 4, 0, 1, 1);

        desktopEdit = new QLineEdit(ServerStatsDlg);
        desktopEdit->setObjectName(QString::fromUtf8("desktopEdit"));
        sizePolicy.setHeightForWidth(desktopEdit->sizePolicy().hasHeightForWidth());
        desktopEdit->setSizePolicy(sizePolicy);
        desktopEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        desktopEdit->setReadOnly(true);

        gridLayout->addWidget(desktopEdit, 4, 1, 1, 1);

        label_16 = new QLabel(ServerStatsDlg);
        label_16->setObjectName(QString::fromUtf8("label_16"));

        gridLayout->addWidget(label_16, 6, 2, 1, 1);

        label_17 = new QLabel(ServerStatsDlg);
        label_17->setObjectName(QString::fromUtf8("label_17"));

        gridLayout->addWidget(label_17, 7, 0, 1, 1);

        usersservedEdit = new QLineEdit(ServerStatsDlg);
        usersservedEdit->setObjectName(QString::fromUtf8("usersservedEdit"));
        usersservedEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        usersservedEdit->setReadOnly(true);

        gridLayout->addWidget(usersservedEdit, 7, 1, 1, 1);

        label_19 = new QLabel(ServerStatsDlg);
        label_19->setObjectName(QString::fromUtf8("label_19"));

        gridLayout->addWidget(label_19, 8, 0, 1, 1);

        userspeakEdit = new QLineEdit(ServerStatsDlg);
        userspeakEdit->setObjectName(QString::fromUtf8("userspeakEdit"));
        userspeakEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        userspeakEdit->setReadOnly(true);

        gridLayout->addWidget(userspeakEdit, 8, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(74, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        closeBtn = new QPushButton(ServerStatsDlg);
        closeBtn->setObjectName(QString::fromUtf8("closeBtn"));

        horizontalLayout->addWidget(closeBtn);

        horizontalSpacer_2 = new QSpacerItem(65, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout);

#if QT_CONFIG(shortcut)
        label_6->setBuddy(uptimeEdit);
        label_2->setBuddy(kbpsEdit);
        label_15->setBuddy(filesEdit);
        label->setBuddy(totalEdit);
        label_3->setBuddy(voiceEdit);
        label_5->setBuddy(videoEdit);
        label_8->setBuddy(desktopEdit);
        label_17->setBuddy(usersservedEdit);
        label_19->setBuddy(userspeakEdit);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(totalEdit, voiceEdit);
        QWidget::setTabOrder(voiceEdit, videoEdit);
        QWidget::setTabOrder(videoEdit, mediafileEdit);
        QWidget::setTabOrder(mediafileEdit, desktopEdit);
        QWidget::setTabOrder(desktopEdit, kbpsEdit);
        QWidget::setTabOrder(kbpsEdit, filesEdit);
        QWidget::setTabOrder(filesEdit, usersservedEdit);
        QWidget::setTabOrder(usersservedEdit, userspeakEdit);
        QWidget::setTabOrder(userspeakEdit, uptimeEdit);
        QWidget::setTabOrder(uptimeEdit, updateChkBox);
        QWidget::setTabOrder(updateChkBox, msecSpinBox);
        QWidget::setTabOrder(msecSpinBox, closeBtn);

        retranslateUi(ServerStatsDlg);

        QMetaObject::connectSlotsByName(ServerStatsDlg);
    } // setupUi

    void retranslateUi(QDialog *ServerStatsDlg)
    {
        ServerStatsDlg->setWindowTitle(QCoreApplication::translate("ServerStatsDlg", "Server Statistics", nullptr));
        label_6->setText(QCoreApplication::translate("ServerStatsDlg", "Uptime", nullptr));
        label_10->setText(QCoreApplication::translate("ServerStatsDlg", "Media File RX/TX", nullptr));
        label_4->setText(QCoreApplication::translate("ServerStatsDlg", "KBytes/sec", nullptr));
        label_2->setText(QCoreApplication::translate("ServerStatsDlg", "Throughput RX/TX", nullptr));
        label_12->setText(QCoreApplication::translate("ServerStatsDlg", "KBytes", nullptr));
        label_13->setText(QCoreApplication::translate("ServerStatsDlg", "msec", nullptr));
        updateChkBox->setText(QCoreApplication::translate("ServerStatsDlg", "Update interval", nullptr));
        label_14->setText(QCoreApplication::translate("ServerStatsDlg", "KBytes", nullptr));
        label_15->setText(QCoreApplication::translate("ServerStatsDlg", "Files RX/TX", nullptr));
        label->setText(QCoreApplication::translate("ServerStatsDlg", "Total RX/TX", nullptr));
        label_7->setText(QCoreApplication::translate("ServerStatsDlg", "KBytes", nullptr));
        label_3->setText(QCoreApplication::translate("ServerStatsDlg", "Voice RX/TX", nullptr));
        label_9->setText(QCoreApplication::translate("ServerStatsDlg", "KBytes", nullptr));
        label_5->setText(QCoreApplication::translate("ServerStatsDlg", "Video RX/TX", nullptr));
        label_11->setText(QCoreApplication::translate("ServerStatsDlg", "KBytes", nullptr));
        label_8->setText(QCoreApplication::translate("ServerStatsDlg", "Desktop RX/TX", nullptr));
        label_16->setText(QCoreApplication::translate("ServerStatsDlg", "KBytes", nullptr));
        label_17->setText(QCoreApplication::translate("ServerStatsDlg", "Users served", nullptr));
        label_19->setText(QCoreApplication::translate("ServerStatsDlg", "Users peak", nullptr));
        closeBtn->setText(QCoreApplication::translate("ServerStatsDlg", "&Close", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ServerStatsDlg: public Ui_ServerStatsDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERVERSTATS_H
