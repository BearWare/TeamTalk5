/********************************************************************************
** Form generated from reading UI file 'useraccounts.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USERACCOUNTS_H
#define UI_USERACCOUNTS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UserAccountsDlg
{
public:
    QVBoxLayout *verticalLayout_4;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_6;
    QTreeView *usersTreeView;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_6;
    QHBoxLayout *horizontalLayout_5;
    QGroupBox *groupBox_6;
    QFormLayout *formLayout_2;
    QLabel *label_3;
    QVBoxLayout *verticalLayout_2;
    QRadioButton *defaultuserBtn;
    QRadioButton *adminBtn;
    QLabel *label;
    QLineEdit *usernameEdit;
    QLabel *label_2;
    QLineEdit *passwordEdit;
    QLabel *label_4;
    QPlainTextEdit *noteEdit;
    QLabel *label_5;
    QComboBox *channelComboBox;
    QSpacerItem *verticalSpacer_4;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox_3;
    QFormLayout *formLayout;
    QLabel *label_7;
    QListWidget *opchannelsListWidget;
    QLabel *label_6;
    QHBoxLayout *horizontalLayout_3;
    QComboBox *opchanComboBox;
    QToolButton *addopBtn;
    QToolButton *rmopBtn;
    QSpacerItem *verticalSpacer_2;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout_5;
    QGroupBox *groupBox_5;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_10;
    QSpinBox *audmaxbpsSpinBox;
    QLabel *label_11;
    QSpacerItem *verticalSpacer_3;
    QWidget *tab_3;
    QVBoxLayout *verticalLayout_8;
    QGroupBox *groupBox_7;
    QVBoxLayout *verticalLayout_7;
    QLabel *label_9;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_8;
    QComboBox *limitcmdComboBox;
    QSpacerItem *verticalSpacer_5;
    QGroupBox *groupBox_4;
    QFormLayout *formLayout_3;
    QCheckBox *multiloginBox;
    QCheckBox *uploadfilesBox;
    QCheckBox *chnickBox;
    QCheckBox *downloadfilesBox;
    QCheckBox *viewallusersBox;
    QCheckBox *permchannelsBox;
    QCheckBox *tempchannelsBox;
    QCheckBox *clientbroadcastBox;
    QCheckBox *kickusersBox;
    QCheckBox *banusersBox;
    QCheckBox *moveusersBox;
    QCheckBox *chanopBox;
    QCheckBox *transmitdesktopaccessBox;
    QCheckBox *transmitdesktopBox;
    QCheckBox *transmitvideofileBox;
    QCheckBox *transmitaudiofileBox;
    QCheckBox *transmitvideoBox;
    QCheckBox *transmitvoiceBox;
    QCheckBox *srvpropBox;
    QCheckBox *recordBox;
    QHBoxLayout *horizontalLayout;
    QPushButton *delButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *newButton;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *addButton;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *closeBtn;
    QSpacerItem *verticalSpacer;

    void setupUi(QDialog *UserAccountsDlg)
    {
        if (UserAccountsDlg->objectName().isEmpty())
            UserAccountsDlg->setObjectName(QString::fromUtf8("UserAccountsDlg"));
        UserAccountsDlg->resize(815, 866);
        UserAccountsDlg->setSizeGripEnabled(true);
        verticalLayout_4 = new QVBoxLayout(UserAccountsDlg);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        groupBox = new QGroupBox(UserAccountsDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        horizontalLayout_6 = new QHBoxLayout(groupBox);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        usersTreeView = new QTreeView(groupBox);
        usersTreeView->setObjectName(QString::fromUtf8("usersTreeView"));
        usersTreeView->setMinimumSize(QSize(200, 0));
        QFont font;
        font.setPointSize(8);
        usersTreeView->setFont(font);
        usersTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        usersTreeView->setProperty("showDropIndicator", QVariant(true));
        usersTreeView->setDragDropOverwriteMode(false);
        usersTreeView->setAlternatingRowColors(true);
        usersTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        usersTreeView->setRootIsDecorated(false);
        usersTreeView->setItemsExpandable(false);
        usersTreeView->setSortingEnabled(true);
        usersTreeView->setExpandsOnDoubleClick(false);

        horizontalLayout_6->addWidget(usersTreeView);


        verticalLayout_4->addWidget(groupBox);

        groupBox_2 = new QGroupBox(UserAccountsDlg);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_6 = new QVBoxLayout(groupBox_2);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        groupBox_6 = new QGroupBox(groupBox_2);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        formLayout_2 = new QFormLayout(groupBox_6);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        label_3 = new QLabel(groupBox_6);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label_3);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        defaultuserBtn = new QRadioButton(groupBox_6);
        defaultuserBtn->setObjectName(QString::fromUtf8("defaultuserBtn"));
        defaultuserBtn->setChecked(true);

        verticalLayout_2->addWidget(defaultuserBtn);

        adminBtn = new QRadioButton(groupBox_6);
        adminBtn->setObjectName(QString::fromUtf8("adminBtn"));

        verticalLayout_2->addWidget(adminBtn);


        formLayout_2->setLayout(0, QFormLayout::FieldRole, verticalLayout_2);

        label = new QLabel(groupBox_6);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout_2->setWidget(1, QFormLayout::LabelRole, label);

        usernameEdit = new QLineEdit(groupBox_6);
        usernameEdit->setObjectName(QString::fromUtf8("usernameEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(usernameEdit->sizePolicy().hasHeightForWidth());
        usernameEdit->setSizePolicy(sizePolicy);

        formLayout_2->setWidget(1, QFormLayout::FieldRole, usernameEdit);

        label_2 = new QLabel(groupBox_6);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout_2->setWidget(2, QFormLayout::LabelRole, label_2);

        passwordEdit = new QLineEdit(groupBox_6);
        passwordEdit->setObjectName(QString::fromUtf8("passwordEdit"));
        sizePolicy.setHeightForWidth(passwordEdit->sizePolicy().hasHeightForWidth());
        passwordEdit->setSizePolicy(sizePolicy);

        formLayout_2->setWidget(2, QFormLayout::FieldRole, passwordEdit);

        label_4 = new QLabel(groupBox_6);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout_2->setWidget(3, QFormLayout::LabelRole, label_4);

        noteEdit = new QPlainTextEdit(groupBox_6);
        noteEdit->setObjectName(QString::fromUtf8("noteEdit"));

        formLayout_2->setWidget(3, QFormLayout::FieldRole, noteEdit);

        label_5 = new QLabel(groupBox_6);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        formLayout_2->setWidget(4, QFormLayout::LabelRole, label_5);

        channelComboBox = new QComboBox(groupBox_6);
        channelComboBox->setObjectName(QString::fromUtf8("channelComboBox"));
        channelComboBox->setEditable(true);

        formLayout_2->setWidget(4, QFormLayout::FieldRole, channelComboBox);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        formLayout_2->setItem(5, QFormLayout::FieldRole, verticalSpacer_4);


        horizontalLayout_5->addWidget(groupBox_6);

        tabWidget = new QTabWidget(groupBox_2);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayout_3 = new QVBoxLayout(tab);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        groupBox_3 = new QGroupBox(tab);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        formLayout = new QFormLayout(groupBox_3);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        label_7 = new QLabel(groupBox_3);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_7);

        opchannelsListWidget = new QListWidget(groupBox_3);
        opchannelsListWidget->setObjectName(QString::fromUtf8("opchannelsListWidget"));
        opchannelsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        opchannelsListWidget->setUniformItemSizes(false);

        formLayout->setWidget(0, QFormLayout::FieldRole, opchannelsListWidget);

        label_6 = new QLabel(groupBox_3);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_6);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        opchanComboBox = new QComboBox(groupBox_3);
        opchanComboBox->setObjectName(QString::fromUtf8("opchanComboBox"));
        opchanComboBox->setEditable(false);

        horizontalLayout_3->addWidget(opchanComboBox);

        addopBtn = new QToolButton(groupBox_3);
        addopBtn->setObjectName(QString::fromUtf8("addopBtn"));

        horizontalLayout_3->addWidget(addopBtn);

        rmopBtn = new QToolButton(groupBox_3);
        rmopBtn->setObjectName(QString::fromUtf8("rmopBtn"));

        horizontalLayout_3->addWidget(rmopBtn);

        horizontalLayout_3->setStretch(0, 1);

        formLayout->setLayout(1, QFormLayout::FieldRole, horizontalLayout_3);


        verticalLayout_3->addWidget(groupBox_3);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_2);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        verticalLayout_5 = new QVBoxLayout(tab_2);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        groupBox_5 = new QGroupBox(tab_2);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        horizontalLayout_2 = new QHBoxLayout(groupBox_5);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_10 = new QLabel(groupBox_5);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        horizontalLayout_2->addWidget(label_10);

        audmaxbpsSpinBox = new QSpinBox(groupBox_5);
        audmaxbpsSpinBox->setObjectName(QString::fromUtf8("audmaxbpsSpinBox"));
        audmaxbpsSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        audmaxbpsSpinBox->setMaximum(2048);
        audmaxbpsSpinBox->setSingleStep(1);

        horizontalLayout_2->addWidget(audmaxbpsSpinBox);

        label_11 = new QLabel(groupBox_5);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        horizontalLayout_2->addWidget(label_11);


        verticalLayout_5->addWidget(groupBox_5);

        verticalSpacer_3 = new QSpacerItem(20, 158, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer_3);

        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        verticalLayout_8 = new QVBoxLayout(tab_3);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        groupBox_7 = new QGroupBox(tab_3);
        groupBox_7->setObjectName(QString::fromUtf8("groupBox_7"));
        verticalLayout_7 = new QVBoxLayout(groupBox_7);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        label_9 = new QLabel(groupBox_7);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setWordWrap(true);

        verticalLayout_7->addWidget(label_9);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_8 = new QLabel(groupBox_7);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        horizontalLayout_7->addWidget(label_8);

        limitcmdComboBox = new QComboBox(groupBox_7);
        limitcmdComboBox->setObjectName(QString::fromUtf8("limitcmdComboBox"));

        horizontalLayout_7->addWidget(limitcmdComboBox);


        verticalLayout_7->addLayout(horizontalLayout_7);


        verticalLayout_8->addWidget(groupBox_7);

        verticalSpacer_5 = new QSpacerItem(20, 100, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_8->addItem(verticalSpacer_5);

        tabWidget->addTab(tab_3, QString());

        horizontalLayout_5->addWidget(tabWidget);


        verticalLayout_6->addLayout(horizontalLayout_5);

        groupBox_4 = new QGroupBox(groupBox_2);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        formLayout_3 = new QFormLayout(groupBox_4);
        formLayout_3->setObjectName(QString::fromUtf8("formLayout_3"));
        multiloginBox = new QCheckBox(groupBox_4);
        multiloginBox->setObjectName(QString::fromUtf8("multiloginBox"));

        formLayout_3->setWidget(0, QFormLayout::LabelRole, multiloginBox);

        uploadfilesBox = new QCheckBox(groupBox_4);
        uploadfilesBox->setObjectName(QString::fromUtf8("uploadfilesBox"));

        formLayout_3->setWidget(0, QFormLayout::FieldRole, uploadfilesBox);

        chnickBox = new QCheckBox(groupBox_4);
        chnickBox->setObjectName(QString::fromUtf8("chnickBox"));

        formLayout_3->setWidget(1, QFormLayout::LabelRole, chnickBox);

        downloadfilesBox = new QCheckBox(groupBox_4);
        downloadfilesBox->setObjectName(QString::fromUtf8("downloadfilesBox"));

        formLayout_3->setWidget(1, QFormLayout::FieldRole, downloadfilesBox);

        viewallusersBox = new QCheckBox(groupBox_4);
        viewallusersBox->setObjectName(QString::fromUtf8("viewallusersBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(viewallusersBox->sizePolicy().hasHeightForWidth());
        viewallusersBox->setSizePolicy(sizePolicy1);

        formLayout_3->setWidget(2, QFormLayout::LabelRole, viewallusersBox);

        permchannelsBox = new QCheckBox(groupBox_4);
        permchannelsBox->setObjectName(QString::fromUtf8("permchannelsBox"));

        formLayout_3->setWidget(3, QFormLayout::LabelRole, permchannelsBox);

        tempchannelsBox = new QCheckBox(groupBox_4);
        tempchannelsBox->setObjectName(QString::fromUtf8("tempchannelsBox"));
        sizePolicy1.setHeightForWidth(tempchannelsBox->sizePolicy().hasHeightForWidth());
        tempchannelsBox->setSizePolicy(sizePolicy1);

        formLayout_3->setWidget(4, QFormLayout::LabelRole, tempchannelsBox);

        clientbroadcastBox = new QCheckBox(groupBox_4);
        clientbroadcastBox->setObjectName(QString::fromUtf8("clientbroadcastBox"));
        sizePolicy1.setHeightForWidth(clientbroadcastBox->sizePolicy().hasHeightForWidth());
        clientbroadcastBox->setSizePolicy(sizePolicy1);

        formLayout_3->setWidget(5, QFormLayout::LabelRole, clientbroadcastBox);

        kickusersBox = new QCheckBox(groupBox_4);
        kickusersBox->setObjectName(QString::fromUtf8("kickusersBox"));

        formLayout_3->setWidget(6, QFormLayout::LabelRole, kickusersBox);

        banusersBox = new QCheckBox(groupBox_4);
        banusersBox->setObjectName(QString::fromUtf8("banusersBox"));

        formLayout_3->setWidget(7, QFormLayout::LabelRole, banusersBox);

        moveusersBox = new QCheckBox(groupBox_4);
        moveusersBox->setObjectName(QString::fromUtf8("moveusersBox"));

        formLayout_3->setWidget(8, QFormLayout::LabelRole, moveusersBox);

        chanopBox = new QCheckBox(groupBox_4);
        chanopBox->setObjectName(QString::fromUtf8("chanopBox"));

        formLayout_3->setWidget(9, QFormLayout::LabelRole, chanopBox);

        transmitdesktopaccessBox = new QCheckBox(groupBox_4);
        transmitdesktopaccessBox->setObjectName(QString::fromUtf8("transmitdesktopaccessBox"));

        formLayout_3->setWidget(9, QFormLayout::FieldRole, transmitdesktopaccessBox);

        transmitdesktopBox = new QCheckBox(groupBox_4);
        transmitdesktopBox->setObjectName(QString::fromUtf8("transmitdesktopBox"));

        formLayout_3->setWidget(8, QFormLayout::FieldRole, transmitdesktopBox);

        transmitvideofileBox = new QCheckBox(groupBox_4);
        transmitvideofileBox->setObjectName(QString::fromUtf8("transmitvideofileBox"));

        formLayout_3->setWidget(7, QFormLayout::FieldRole, transmitvideofileBox);

        transmitaudiofileBox = new QCheckBox(groupBox_4);
        transmitaudiofileBox->setObjectName(QString::fromUtf8("transmitaudiofileBox"));

        formLayout_3->setWidget(6, QFormLayout::FieldRole, transmitaudiofileBox);

        transmitvideoBox = new QCheckBox(groupBox_4);
        transmitvideoBox->setObjectName(QString::fromUtf8("transmitvideoBox"));

        formLayout_3->setWidget(5, QFormLayout::FieldRole, transmitvideoBox);

        transmitvoiceBox = new QCheckBox(groupBox_4);
        transmitvoiceBox->setObjectName(QString::fromUtf8("transmitvoiceBox"));

        formLayout_3->setWidget(4, QFormLayout::FieldRole, transmitvoiceBox);

        srvpropBox = new QCheckBox(groupBox_4);
        srvpropBox->setObjectName(QString::fromUtf8("srvpropBox"));

        formLayout_3->setWidget(3, QFormLayout::FieldRole, srvpropBox);

        recordBox = new QCheckBox(groupBox_4);
        recordBox->setObjectName(QString::fromUtf8("recordBox"));

        formLayout_3->setWidget(2, QFormLayout::FieldRole, recordBox);


        verticalLayout_6->addWidget(groupBox_4);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        delButton = new QPushButton(groupBox_2);
        delButton->setObjectName(QString::fromUtf8("delButton"));

        horizontalLayout->addWidget(delButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        newButton = new QPushButton(groupBox_2);
        newButton->setObjectName(QString::fromUtf8("newButton"));

        horizontalLayout->addWidget(newButton);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_4);

        addButton = new QPushButton(groupBox_2);
        addButton->setObjectName(QString::fromUtf8("addButton"));

        horizontalLayout->addWidget(addButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        closeBtn = new QPushButton(groupBox_2);
        closeBtn->setObjectName(QString::fromUtf8("closeBtn"));

        horizontalLayout->addWidget(closeBtn);


        verticalLayout_6->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(236, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_6->addItem(verticalSpacer);


        verticalLayout_4->addWidget(groupBox_2);

#if QT_CONFIG(shortcut)
        label_3->setBuddy(defaultuserBtn);
        label->setBuddy(usernameEdit);
        label_2->setBuddy(passwordEdit);
        label_4->setBuddy(noteEdit);
        label_5->setBuddy(channelComboBox);
        label_7->setBuddy(opchannelsListWidget);
        label_6->setBuddy(opchanComboBox);
        label_10->setBuddy(audmaxbpsSpinBox);
        label_8->setBuddy(limitcmdComboBox);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(usersTreeView, defaultuserBtn);
        QWidget::setTabOrder(defaultuserBtn, adminBtn);
        QWidget::setTabOrder(adminBtn, usernameEdit);
        QWidget::setTabOrder(usernameEdit, passwordEdit);
        QWidget::setTabOrder(passwordEdit, noteEdit);
        QWidget::setTabOrder(noteEdit, channelComboBox);
        QWidget::setTabOrder(channelComboBox, multiloginBox);
        QWidget::setTabOrder(multiloginBox, chnickBox);
        QWidget::setTabOrder(chnickBox, viewallusersBox);
        QWidget::setTabOrder(viewallusersBox, permchannelsBox);
        QWidget::setTabOrder(permchannelsBox, tempchannelsBox);
        QWidget::setTabOrder(tempchannelsBox, clientbroadcastBox);
        QWidget::setTabOrder(clientbroadcastBox, kickusersBox);
        QWidget::setTabOrder(kickusersBox, banusersBox);
        QWidget::setTabOrder(banusersBox, moveusersBox);
        QWidget::setTabOrder(moveusersBox, chanopBox);
        QWidget::setTabOrder(chanopBox, uploadfilesBox);
        QWidget::setTabOrder(uploadfilesBox, downloadfilesBox);
        QWidget::setTabOrder(downloadfilesBox, recordBox);
        QWidget::setTabOrder(recordBox, srvpropBox);
        QWidget::setTabOrder(srvpropBox, transmitvoiceBox);
        QWidget::setTabOrder(transmitvoiceBox, transmitvideoBox);
        QWidget::setTabOrder(transmitvideoBox, transmitaudiofileBox);
        QWidget::setTabOrder(transmitaudiofileBox, transmitvideofileBox);
        QWidget::setTabOrder(transmitvideofileBox, transmitdesktopBox);
        QWidget::setTabOrder(transmitdesktopBox, transmitdesktopaccessBox);
        QWidget::setTabOrder(transmitdesktopaccessBox, tabWidget);
        QWidget::setTabOrder(tabWidget, opchannelsListWidget);
        QWidget::setTabOrder(opchannelsListWidget, opchanComboBox);
        QWidget::setTabOrder(opchanComboBox, addopBtn);
        QWidget::setTabOrder(addopBtn, rmopBtn);
        QWidget::setTabOrder(rmopBtn, audmaxbpsSpinBox);
        QWidget::setTabOrder(audmaxbpsSpinBox, limitcmdComboBox);
        QWidget::setTabOrder(limitcmdComboBox, delButton);
        QWidget::setTabOrder(delButton, newButton);
        QWidget::setTabOrder(newButton, addButton);
        QWidget::setTabOrder(addButton, closeBtn);

        retranslateUi(UserAccountsDlg);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(UserAccountsDlg);
    } // setupUi

    void retranslateUi(QDialog *UserAccountsDlg)
    {
        UserAccountsDlg->setWindowTitle(QCoreApplication::translate("UserAccountsDlg", "User Accounts", nullptr));
        groupBox->setTitle(QCoreApplication::translate("UserAccountsDlg", "Active Users", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("UserAccountsDlg", "Modify User Account", nullptr));
        groupBox_6->setTitle(QCoreApplication::translate("UserAccountsDlg", "Basic Account Properties", nullptr));
        label_3->setText(QCoreApplication::translate("UserAccountsDlg", "User type", nullptr));
        defaultuserBtn->setText(QCoreApplication::translate("UserAccountsDlg", "Default user", nullptr));
        adminBtn->setText(QCoreApplication::translate("UserAccountsDlg", "Administrator", nullptr));
        label->setText(QCoreApplication::translate("UserAccountsDlg", "Username", nullptr));
        label_2->setText(QCoreApplication::translate("UserAccountsDlg", "Password", nullptr));
        label_4->setText(QCoreApplication::translate("UserAccountsDlg", "Note", nullptr));
        label_5->setText(QCoreApplication::translate("UserAccountsDlg", "Initial Channel", nullptr));
#if QT_CONFIG(tooltip)
        channelComboBox->setToolTip(QCoreApplication::translate("UserAccountsDlg", "User should automatically join this channel after login", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(accessibility)
        groupBox_3->setAccessibleName(QCoreApplication::translate("UserAccountsDlg", "Auto-Operator Channels", nullptr));
#endif // QT_CONFIG(accessibility)
        groupBox_3->setTitle(QCoreApplication::translate("UserAccountsDlg", "Auto-Operator Channels", nullptr));
        label_7->setText(QCoreApplication::translate("UserAccountsDlg", "Selected Channels", nullptr));
#if QT_CONFIG(tooltip)
        opchannelsListWidget->setToolTip(QCoreApplication::translate("UserAccountsDlg", "User will automatically become operator when joining these channels", nullptr));
#endif // QT_CONFIG(tooltip)
        label_6->setText(QCoreApplication::translate("UserAccountsDlg", "Available Channels", nullptr));
        addopBtn->setText(QCoreApplication::translate("UserAccountsDlg", "Add", nullptr));
        rmopBtn->setText(QCoreApplication::translate("UserAccountsDlg", "Remove", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("UserAccountsDlg", "Channel Operator", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("UserAccountsDlg", "Audio Codec Limitations", nullptr));
        label_10->setText(QCoreApplication::translate("UserAccountsDlg", "Max bitrate for audio codecs", nullptr));
        label_11->setText(QCoreApplication::translate("UserAccountsDlg", "kbps (0 = disabled)", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("UserAccountsDlg", "Codec Limitations", nullptr));
        groupBox_7->setTitle(QCoreApplication::translate("UserAccountsDlg", "Flood Protection", nullptr));
        label_9->setText(QCoreApplication::translate("UserAccountsDlg", "Prevent user from e.g. spamming text messages by limiting the number of commands they can issue within a given timeframe.", nullptr));
        label_8->setText(QCoreApplication::translate("UserAccountsDlg", "Limit issued commands", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QCoreApplication::translate("UserAccountsDlg", "Abuse Prevention", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("UserAccountsDlg", "User Rights", nullptr));
        multiloginBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can log in multiple times", nullptr));
        uploadfilesBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can upload files", nullptr));
        chnickBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can change nickname", nullptr));
        downloadfilesBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can download files", nullptr));
        viewallusersBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can see users in all channels", nullptr));
        permchannelsBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can create/modify all channels", nullptr));
        tempchannelsBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can create temporary channels", nullptr));
        clientbroadcastBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can broadcast text messages", nullptr));
        kickusersBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can kick users off the server", nullptr));
        banusersBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can ban users from server", nullptr));
        moveusersBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can move users between channels", nullptr));
        chanopBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can make other users channel operator", nullptr));
        transmitdesktopaccessBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can get remote access to desktop sessions", nullptr));
        transmitdesktopBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can transmit desktop sessions (shared desktop)", nullptr));
        transmitvideofileBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can transmit video files (avi, mp4 files)", nullptr));
        transmitaudiofileBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can transmit audio files (wav, mp3 files)", nullptr));
        transmitvideoBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can transmit video data (webcam)", nullptr));
        transmitvoiceBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can transmit voice data (microphone)", nullptr));
        srvpropBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can update server properties", nullptr));
        recordBox->setText(QCoreApplication::translate("UserAccountsDlg", "User can record voice in all channels", nullptr));
        delButton->setText(QCoreApplication::translate("UserAccountsDlg", "&Delete", nullptr));
        newButton->setText(QCoreApplication::translate("UserAccountsDlg", "&Clear", nullptr));
        addButton->setText(QCoreApplication::translate("UserAccountsDlg", "&Add/Update", nullptr));
        closeBtn->setText(QCoreApplication::translate("UserAccountsDlg", "C&lose", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UserAccountsDlg: public Ui_UserAccountsDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERACCOUNTS_H
