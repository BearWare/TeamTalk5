/********************************************************************************
** Form generated from reading UI file 'onlineusers.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ONLINEUSERS_H
#define UI_ONLINEUSERS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_OnlineUsersDlg
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QTreeView *treeView;

    void setupUi(QDialog *OnlineUsersDlg)
    {
        if (OnlineUsersDlg->objectName().isEmpty())
            OnlineUsersDlg->setObjectName(QString::fromUtf8("OnlineUsersDlg"));
        OnlineUsersDlg->resize(588, 617);
        verticalLayout = new QVBoxLayout(OnlineUsersDlg);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox = new QGroupBox(OnlineUsersDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        treeView = new QTreeView(groupBox);
        treeView->setObjectName(QString::fromUtf8("treeView"));
        treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        treeView->setAlternatingRowColors(true);
        treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        treeView->setIndentation(0);
        treeView->setRootIsDecorated(false);
        treeView->setItemsExpandable(false);
        treeView->setSortingEnabled(true);
        treeView->setExpandsOnDoubleClick(false);

        verticalLayout_2->addWidget(treeView);


        verticalLayout->addWidget(groupBox);


        retranslateUi(OnlineUsersDlg);

        QMetaObject::connectSlotsByName(OnlineUsersDlg);
    } // setupUi

    void retranslateUi(QDialog *OnlineUsersDlg)
    {
        OnlineUsersDlg->setWindowTitle(QCoreApplication::translate("OnlineUsersDlg", "Online Users", nullptr));
        groupBox->setTitle(QCoreApplication::translate("OnlineUsersDlg", "Users Currently on Server", nullptr));
    } // retranslateUi

};

namespace Ui {
    class OnlineUsersDlg: public Ui_OnlineUsersDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ONLINEUSERS_H
