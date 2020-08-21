/********************************************************************************
** Form generated from reading UI file 'mediastorage.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MEDIASTORAGE_H
#define UI_MEDIASTORAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_MediaStorageDlg
{
public:
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label_3;
    QCheckBox *singleCheckBox;
    QCheckBox *multipleCheckBox;
    QGridLayout *gridLayout_2;
    QLabel *label_2;
    QComboBox *affComboBox;
    QLabel *label;
    QLineEdit *audiopathEdit;
    QToolButton *audioToolButton;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_3;
    QLabel *label_4;
    QLineEdit *chanlogEdit;
    QToolButton *chanlogToolButton;
    QLabel *label_5;
    QLineEdit *usertextEdit;
    QToolButton *usertextToolButton;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *MediaStorageDlg)
    {
        if (MediaStorageDlg->objectName().isEmpty())
            MediaStorageDlg->setObjectName(QString::fromUtf8("MediaStorageDlg"));
        MediaStorageDlg->resize(429, 288);
        verticalLayout_3 = new QVBoxLayout(MediaStorageDlg);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        groupBox = new QGroupBox(MediaStorageDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 0, 0, 1, 1);

        singleCheckBox = new QCheckBox(groupBox);
        singleCheckBox->setObjectName(QString::fromUtf8("singleCheckBox"));

        gridLayout->addWidget(singleCheckBox, 0, 1, 1, 1);

        multipleCheckBox = new QCheckBox(groupBox);
        multipleCheckBox->setObjectName(QString::fromUtf8("multipleCheckBox"));

        gridLayout->addWidget(multipleCheckBox, 1, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout_2->addWidget(label_2, 0, 0, 1, 1);

        affComboBox = new QComboBox(groupBox);
        affComboBox->setObjectName(QString::fromUtf8("affComboBox"));

        gridLayout_2->addWidget(affComboBox, 0, 1, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_2->addWidget(label, 1, 0, 1, 1);

        audiopathEdit = new QLineEdit(groupBox);
        audiopathEdit->setObjectName(QString::fromUtf8("audiopathEdit"));

        gridLayout_2->addWidget(audiopathEdit, 1, 1, 1, 1);

        audioToolButton = new QToolButton(groupBox);
        audioToolButton->setObjectName(QString::fromUtf8("audioToolButton"));

        gridLayout_2->addWidget(audioToolButton, 1, 2, 1, 1);


        verticalLayout->addLayout(gridLayout_2);


        verticalLayout_3->addWidget(groupBox);

        groupBox_2 = new QGroupBox(MediaStorageDlg);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout_3 = new QGridLayout(groupBox_2);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout_3->addWidget(label_4, 0, 0, 1, 1);

        chanlogEdit = new QLineEdit(groupBox_2);
        chanlogEdit->setObjectName(QString::fromUtf8("chanlogEdit"));

        gridLayout_3->addWidget(chanlogEdit, 0, 1, 1, 1);

        chanlogToolButton = new QToolButton(groupBox_2);
        chanlogToolButton->setObjectName(QString::fromUtf8("chanlogToolButton"));

        gridLayout_3->addWidget(chanlogToolButton, 0, 2, 1, 1);

        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout_3->addWidget(label_5, 1, 0, 1, 1);

        usertextEdit = new QLineEdit(groupBox_2);
        usertextEdit->setObjectName(QString::fromUtf8("usertextEdit"));

        gridLayout_3->addWidget(usertextEdit, 1, 1, 1, 1);

        usertextToolButton = new QToolButton(groupBox_2);
        usertextToolButton->setObjectName(QString::fromUtf8("usertextToolButton"));

        gridLayout_3->addWidget(usertextToolButton, 1, 2, 1, 1);


        verticalLayout_3->addWidget(groupBox_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(MediaStorageDlg);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_3->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        label_3->setBuddy(singleCheckBox);
        label_2->setBuddy(affComboBox);
        label->setBuddy(audiopathEdit);
        label_4->setBuddy(chanlogEdit);
        label_5->setBuddy(usertextEdit);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(singleCheckBox, multipleCheckBox);
        QWidget::setTabOrder(multipleCheckBox, affComboBox);
        QWidget::setTabOrder(affComboBox, audiopathEdit);
        QWidget::setTabOrder(audiopathEdit, audioToolButton);
        QWidget::setTabOrder(audioToolButton, chanlogEdit);
        QWidget::setTabOrder(chanlogEdit, chanlogToolButton);
        QWidget::setTabOrder(chanlogToolButton, usertextEdit);
        QWidget::setTabOrder(usertextEdit, usertextToolButton);
        QWidget::setTabOrder(usertextToolButton, buttonBox);

        retranslateUi(MediaStorageDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), MediaStorageDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), MediaStorageDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(MediaStorageDlg);
    } // setupUi

    void retranslateUi(QDialog *MediaStorageDlg)
    {
        MediaStorageDlg->setWindowTitle(QCoreApplication::translate("MediaStorageDlg", "Record Conversations to Disk", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MediaStorageDlg", "Store audio conversations", nullptr));
        label_3->setText(QCoreApplication::translate("MediaStorageDlg", "Storage mode", nullptr));
        singleCheckBox->setText(QCoreApplication::translate("MediaStorageDlg", "Single audio file for all users", nullptr));
        multipleCheckBox->setText(QCoreApplication::translate("MediaStorageDlg", "Separate audio file for each user", nullptr));
        label_2->setText(QCoreApplication::translate("MediaStorageDlg", "Audio file format", nullptr));
        label->setText(QCoreApplication::translate("MediaStorageDlg", "Folder for audio files", nullptr));
        audioToolButton->setText(QCoreApplication::translate("MediaStorageDlg", "...", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("MediaStorageDlg", "Store text message conversations", nullptr));
        label_4->setText(QCoreApplication::translate("MediaStorageDlg", "Folder for channel log files", nullptr));
        chanlogToolButton->setText(QCoreApplication::translate("MediaStorageDlg", "...", nullptr));
        label_5->setText(QCoreApplication::translate("MediaStorageDlg", "Folder for user-to-user log files", nullptr));
        usertextToolButton->setText(QCoreApplication::translate("MediaStorageDlg", "...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MediaStorageDlg: public Ui_MediaStorageDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MEDIASTORAGE_H
