/********************************************************************************
** Form generated from reading UI file 'bannedusers.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BANNEDUSERS_H
#define UI_BANNEDUSERS_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_BannedUsersDlg
{
public:
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QTreeView *bannedTreeView;
    QHBoxLayout *horizontalLayout_2;
    QComboBox *bantypeBox;
    QLineEdit *banEdit;
    QToolButton *newbanBtn;
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QToolButton *rightButton;
    QToolButton *leftButton;
    QSpacerItem *verticalSpacer_2;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_3;
    QTreeView *unbannedTreeView;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *BannedUsersDlg)
    {
        if (BannedUsersDlg->objectName().isEmpty())
            BannedUsersDlg->setObjectName(QString::fromUtf8("BannedUsersDlg"));
        BannedUsersDlg->resize(749, 506);
        BannedUsersDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        BannedUsersDlg->setSizeGripEnabled(true);
        verticalLayout_4 = new QVBoxLayout(BannedUsersDlg);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        groupBox = new QGroupBox(BannedUsersDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        bannedTreeView = new QTreeView(groupBox);
        bannedTreeView->setObjectName(QString::fromUtf8("bannedTreeView"));
        QFont font;
        font.setPointSize(8);
        bannedTreeView->setFont(font);
        bannedTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        bannedTreeView->setProperty("showDropIndicator", QVariant(false));
        bannedTreeView->setAlternatingRowColors(true);
        bannedTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        bannedTreeView->setIndentation(0);
        bannedTreeView->setItemsExpandable(false);
        bannedTreeView->setSortingEnabled(true);
        bannedTreeView->setExpandsOnDoubleClick(false);

        verticalLayout_2->addWidget(bannedTreeView);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        bantypeBox = new QComboBox(groupBox);
        bantypeBox->setObjectName(QString::fromUtf8("bantypeBox"));

        horizontalLayout_2->addWidget(bantypeBox);

        banEdit = new QLineEdit(groupBox);
        banEdit->setObjectName(QString::fromUtf8("banEdit"));

        horizontalLayout_2->addWidget(banEdit);

        newbanBtn = new QToolButton(groupBox);
        newbanBtn->setObjectName(QString::fromUtf8("newbanBtn"));

        horizontalLayout_2->addWidget(newbanBtn);


        verticalLayout_2->addLayout(horizontalLayout_2);

        verticalLayout_2->setStretch(0, 1);

        horizontalLayout->addWidget(groupBox);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        rightButton = new QToolButton(BannedUsersDlg);
        rightButton->setObjectName(QString::fromUtf8("rightButton"));
        rightButton->setText(QString::fromUtf8(">"));

        verticalLayout->addWidget(rightButton);

        leftButton = new QToolButton(BannedUsersDlg);
        leftButton->setObjectName(QString::fromUtf8("leftButton"));
        leftButton->setText(QString::fromUtf8("<"));

        verticalLayout->addWidget(leftButton);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);


        horizontalLayout->addLayout(verticalLayout);

        groupBox_2 = new QGroupBox(BannedUsersDlg);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_3 = new QVBoxLayout(groupBox_2);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        unbannedTreeView = new QTreeView(groupBox_2);
        unbannedTreeView->setObjectName(QString::fromUtf8("unbannedTreeView"));
        unbannedTreeView->setFont(font);
        unbannedTreeView->setAlternatingRowColors(true);
        unbannedTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        unbannedTreeView->setIndentation(0);
        unbannedTreeView->setItemsExpandable(false);
        unbannedTreeView->setSortingEnabled(true);
        unbannedTreeView->setExpandsOnDoubleClick(false);

        verticalLayout_3->addWidget(unbannedTreeView);


        horizontalLayout->addWidget(groupBox_2);


        verticalLayout_4->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(BannedUsersDlg);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_4->addWidget(buttonBox);

        QWidget::setTabOrder(bannedTreeView, bantypeBox);
        QWidget::setTabOrder(bantypeBox, banEdit);
        QWidget::setTabOrder(banEdit, newbanBtn);
        QWidget::setTabOrder(newbanBtn, rightButton);
        QWidget::setTabOrder(rightButton, leftButton);
        QWidget::setTabOrder(leftButton, unbannedTreeView);

        retranslateUi(BannedUsersDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), BannedUsersDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), BannedUsersDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(BannedUsersDlg);
    } // setupUi

    void retranslateUi(QDialog *BannedUsersDlg)
    {
        BannedUsersDlg->setWindowTitle(QCoreApplication::translate("BannedUsersDlg", "Banned Users", nullptr));
        groupBox->setTitle(QCoreApplication::translate("BannedUsersDlg", "Banned Users", nullptr));
#if QT_CONFIG(tooltip)
        newbanBtn->setToolTip(QCoreApplication::translate("BannedUsersDlg", "Add to list of banned users", nullptr));
#endif // QT_CONFIG(tooltip)
        newbanBtn->setText(QCoreApplication::translate("BannedUsersDlg", "Add", nullptr));
#if QT_CONFIG(tooltip)
        rightButton->setToolTip(QCoreApplication::translate("BannedUsersDlg", "Remove banned user", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        leftButton->setToolTip(QCoreApplication::translate("BannedUsersDlg", "Put back to banned users", nullptr));
#endif // QT_CONFIG(tooltip)
        groupBox_2->setTitle(QCoreApplication::translate("BannedUsersDlg", "Unbanned Users", nullptr));
    } // retranslateUi

};

namespace Ui {
    class BannedUsersDlg: public Ui_BannedUsersDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BANNEDUSERS_H
