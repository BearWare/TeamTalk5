/********************************************************************************
** Form generated from reading UI file 'desktopaccess.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DESKTOPACCESS_H
#define UI_DESKTOPACCESS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DesktopAccessDlg
{
public:
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_6;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QListView *listView;
    QHBoxLayout *horizontalLayout;
    QPushButton *delBtn;
    QSpacerItem *horizontalSpacer;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_4;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLineEdit *hostaddrEdit;
    QLabel *label_6;
    QSpinBox *tcpportSpinBox;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QComboBox *channelsComboBox;
    QToolButton *addchanBtn;
    QToolButton *delchanBtn;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_3;
    QComboBox *usernamesComboBox;
    QToolButton *adduserBtn;
    QToolButton *deluserBtn;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *clearBtn;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *addBtn;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DesktopAccessDlg)
    {
        if (DesktopAccessDlg->objectName().isEmpty())
            DesktopAccessDlg->setObjectName(QString::fromUtf8("DesktopAccessDlg"));
        DesktopAccessDlg->resize(799, 467);
        verticalLayout_4 = new QVBoxLayout(DesktopAccessDlg);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        groupBox = new QGroupBox(DesktopAccessDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        listView = new QListView(groupBox);
        listView->setObjectName(QString::fromUtf8("listView"));

        verticalLayout->addWidget(listView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        delBtn = new QPushButton(groupBox);
        delBtn->setObjectName(QString::fromUtf8("delBtn"));

        horizontalLayout->addWidget(delBtn);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);


        horizontalLayout_6->addWidget(groupBox);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox_2 = new QGroupBox(DesktopAccessDlg);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_3 = new QVBoxLayout(groupBox_2);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setWordWrap(true);

        verticalLayout_3->addWidget(label_4);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        hostaddrEdit = new QLineEdit(groupBox_2);
        hostaddrEdit->setObjectName(QString::fromUtf8("hostaddrEdit"));

        horizontalLayout_2->addWidget(hostaddrEdit);

        label_6 = new QLabel(groupBox_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_2->addWidget(label_6);

        tcpportSpinBox = new QSpinBox(groupBox_2);
        tcpportSpinBox->setObjectName(QString::fromUtf8("tcpportSpinBox"));
        tcpportSpinBox->setMinimum(1);
        tcpportSpinBox->setMaximum(65535);
        tcpportSpinBox->setValue(10333);

        horizontalLayout_2->addWidget(tcpportSpinBox);


        verticalLayout_3->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label = new QLabel(groupBox_2);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_3->addWidget(label);

        channelsComboBox = new QComboBox(groupBox_2);
        channelsComboBox->setObjectName(QString::fromUtf8("channelsComboBox"));
        channelsComboBox->setEditable(true);
        channelsComboBox->setInsertPolicy(QComboBox::InsertAlphabetically);

        horizontalLayout_3->addWidget(channelsComboBox);

        addchanBtn = new QToolButton(groupBox_2);
        addchanBtn->setObjectName(QString::fromUtf8("addchanBtn"));

        horizontalLayout_3->addWidget(addchanBtn);

        delchanBtn = new QToolButton(groupBox_2);
        delchanBtn->setObjectName(QString::fromUtf8("delchanBtn"));

        horizontalLayout_3->addWidget(delchanBtn);


        verticalLayout_3->addLayout(horizontalLayout_3);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_5->addWidget(label_3);

        usernamesComboBox = new QComboBox(groupBox_2);
        usernamesComboBox->setObjectName(QString::fromUtf8("usernamesComboBox"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(usernamesComboBox->sizePolicy().hasHeightForWidth());
        usernamesComboBox->setSizePolicy(sizePolicy);
        usernamesComboBox->setEditable(true);
        usernamesComboBox->setInsertPolicy(QComboBox::InsertAlphabetically);

        horizontalLayout_5->addWidget(usernamesComboBox);

        adduserBtn = new QToolButton(groupBox_2);
        adduserBtn->setObjectName(QString::fromUtf8("adduserBtn"));

        horizontalLayout_5->addWidget(adduserBtn);

        deluserBtn = new QToolButton(groupBox_2);
        deluserBtn->setObjectName(QString::fromUtf8("deluserBtn"));

        horizontalLayout_5->addWidget(deluserBtn);

        horizontalLayout_5->setStretch(1, 1);

        verticalLayout_3->addLayout(horizontalLayout_5);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        clearBtn = new QPushButton(groupBox_2);
        clearBtn->setObjectName(QString::fromUtf8("clearBtn"));

        horizontalLayout_4->addWidget(clearBtn);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_3);

        addBtn = new QPushButton(groupBox_2);
        addBtn->setObjectName(QString::fromUtf8("addBtn"));

        horizontalLayout_4->addWidget(addBtn);


        verticalLayout_3->addLayout(horizontalLayout_4);


        verticalLayout_2->addWidget(groupBox_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);


        horizontalLayout_6->addLayout(verticalLayout_2);

        horizontalLayout_6->setStretch(1, 1);

        verticalLayout_4->addLayout(horizontalLayout_6);

        buttonBox = new QDialogButtonBox(DesktopAccessDlg);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_4->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        label_2->setBuddy(hostaddrEdit);
        label->setBuddy(channelsComboBox);
        label_3->setBuddy(usernamesComboBox);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(listView, delBtn);
        QWidget::setTabOrder(delBtn, hostaddrEdit);
        QWidget::setTabOrder(hostaddrEdit, tcpportSpinBox);
        QWidget::setTabOrder(tcpportSpinBox, channelsComboBox);
        QWidget::setTabOrder(channelsComboBox, addchanBtn);
        QWidget::setTabOrder(addchanBtn, delchanBtn);
        QWidget::setTabOrder(delchanBtn, usernamesComboBox);
        QWidget::setTabOrder(usernamesComboBox, adduserBtn);
        QWidget::setTabOrder(adduserBtn, deluserBtn);
        QWidget::setTabOrder(deluserBtn, addBtn);
        QWidget::setTabOrder(addBtn, clearBtn);
        QWidget::setTabOrder(clearBtn, buttonBox);

        retranslateUi(DesktopAccessDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), DesktopAccessDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DesktopAccessDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(DesktopAccessDlg);
    } // setupUi

    void retranslateUi(QDialog *DesktopAccessDlg)
    {
        DesktopAccessDlg->setWindowTitle(QCoreApplication::translate("DesktopAccessDlg", "Safe List For Automatic Desktop Access", nullptr));
        groupBox->setTitle(QCoreApplication::translate("DesktopAccessDlg", "Safe List For Desktop Access", nullptr));
        delBtn->setText(QCoreApplication::translate("DesktopAccessDlg", "&Delete", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("DesktopAccessDlg", "Desktop Access Entry", nullptr));
        label_4->setText(QCoreApplication::translate("DesktopAccessDlg", "Here it is possible to automatically give desktop access to a selected group of users on a server. This way it is not required to click \"Allow Desktop Access\" every time a user logs on.", nullptr));
        label_2->setText(QCoreApplication::translate("DesktopAccessDlg", "Host IP-address", nullptr));
        label_6->setText(QCoreApplication::translate("DesktopAccessDlg", "TCP port", nullptr));
        label->setText(QCoreApplication::translate("DesktopAccessDlg", "Enable desktop access to all users in channel", nullptr));
        addchanBtn->setText(QCoreApplication::translate("DesktopAccessDlg", "&Add", nullptr));
        delchanBtn->setText(QCoreApplication::translate("DesktopAccessDlg", "&Remove", nullptr));
        label_3->setText(QCoreApplication::translate("DesktopAccessDlg", "Enable desktop access to users with username", nullptr));
        adduserBtn->setText(QCoreApplication::translate("DesktopAccessDlg", "A&dd", nullptr));
        deluserBtn->setText(QCoreApplication::translate("DesktopAccessDlg", "R&emove", nullptr));
        clearBtn->setText(QCoreApplication::translate("DesktopAccessDlg", "&Clear", nullptr));
        addBtn->setText(QCoreApplication::translate("DesktopAccessDlg", "Add to &Safe List", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DesktopAccessDlg: public Ui_DesktopAccessDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DESKTOPACCESS_H
