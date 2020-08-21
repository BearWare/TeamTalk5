/********************************************************************************
** Form generated from reading UI file 'uservolume.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USERVOLUME_H
#define UI_USERVOLUME_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_UserVolumeDlg
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QSlider *voicevolSlider;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QCheckBox *voiceleftChkBox;
    QCheckBox *voicerightChkBox;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_2;
    QSlider *mfvolSlider;
    QGroupBox *groupBox_4;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *mfleftChkBox;
    QCheckBox *mfrightChkBox;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *defaultsButton;
    QSpacerItem *horizontalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *UserVolumeDlg)
    {
        if (UserVolumeDlg->objectName().isEmpty())
            UserVolumeDlg->setObjectName(QString::fromUtf8("UserVolumeDlg"));
        UserVolumeDlg->resize(323, 340);
        UserVolumeDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        verticalLayout_2 = new QVBoxLayout(UserVolumeDlg);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox_2 = new QGroupBox(UserVolumeDlg);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout = new QVBoxLayout(groupBox_2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(groupBox_2);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        voicevolSlider = new QSlider(groupBox_2);
        voicevolSlider->setObjectName(QString::fromUtf8("voicevolSlider"));
        voicevolSlider->setMaximum(100);
        voicevolSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(voicevolSlider);


        verticalLayout->addLayout(horizontalLayout_2);

        groupBox = new QGroupBox(groupBox_2);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        voiceleftChkBox = new QCheckBox(groupBox);
        voiceleftChkBox->setObjectName(QString::fromUtf8("voiceleftChkBox"));

        horizontalLayout->addWidget(voiceleftChkBox);

        voicerightChkBox = new QCheckBox(groupBox);
        voicerightChkBox->setObjectName(QString::fromUtf8("voicerightChkBox"));

        horizontalLayout->addWidget(voicerightChkBox);


        verticalLayout->addWidget(groupBox);


        verticalLayout_2->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(UserVolumeDlg);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        verticalLayout_3 = new QVBoxLayout(groupBox_3);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_2 = new QLabel(groupBox_3);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_4->addWidget(label_2);

        mfvolSlider = new QSlider(groupBox_3);
        mfvolSlider->setObjectName(QString::fromUtf8("mfvolSlider"));
        mfvolSlider->setMaximum(100);
        mfvolSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_4->addWidget(mfvolSlider);


        verticalLayout_3->addLayout(horizontalLayout_4);

        groupBox_4 = new QGroupBox(groupBox_3);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        horizontalLayout_3 = new QHBoxLayout(groupBox_4);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        mfleftChkBox = new QCheckBox(groupBox_4);
        mfleftChkBox->setObjectName(QString::fromUtf8("mfleftChkBox"));

        horizontalLayout_3->addWidget(mfleftChkBox);

        mfrightChkBox = new QCheckBox(groupBox_4);
        mfrightChkBox->setObjectName(QString::fromUtf8("mfrightChkBox"));

        horizontalLayout_3->addWidget(mfrightChkBox);


        verticalLayout_3->addWidget(groupBox_4);


        verticalLayout_2->addWidget(groupBox_3);

        verticalSpacer = new QSpacerItem(20, 3, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        defaultsButton = new QPushButton(UserVolumeDlg);
        defaultsButton->setObjectName(QString::fromUtf8("defaultsButton"));

        horizontalLayout_5->addWidget(defaultsButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer);

        buttonBox = new QDialogButtonBox(UserVolumeDlg);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok);
        buttonBox->setCenterButtons(false);

        horizontalLayout_5->addWidget(buttonBox);


        verticalLayout_2->addLayout(horizontalLayout_5);

#if QT_CONFIG(shortcut)
        label->setBuddy(voicevolSlider);
        label_2->setBuddy(mfvolSlider);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(voicevolSlider, voiceleftChkBox);
        QWidget::setTabOrder(voiceleftChkBox, voicerightChkBox);
        QWidget::setTabOrder(voicerightChkBox, mfvolSlider);
        QWidget::setTabOrder(mfvolSlider, mfleftChkBox);
        QWidget::setTabOrder(mfleftChkBox, mfrightChkBox);
        QWidget::setTabOrder(mfrightChkBox, buttonBox);

        retranslateUi(UserVolumeDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), UserVolumeDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), UserVolumeDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(UserVolumeDlg);
    } // setupUi

    void retranslateUi(QDialog *UserVolumeDlg)
    {
        UserVolumeDlg->setWindowTitle(QCoreApplication::translate("UserVolumeDlg", "Volume", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("UserVolumeDlg", "Voice Volume Settings", nullptr));
        label->setText(QCoreApplication::translate("UserVolumeDlg", "Volume", nullptr));
        groupBox->setTitle(QCoreApplication::translate("UserVolumeDlg", "Stereo Option", nullptr));
        voiceleftChkBox->setText(QCoreApplication::translate("UserVolumeDlg", "Mute left", nullptr));
        voicerightChkBox->setText(QCoreApplication::translate("UserVolumeDlg", "Mute right", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("UserVolumeDlg", "Media File Volume Settings", nullptr));
        label_2->setText(QCoreApplication::translate("UserVolumeDlg", "Volume", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("UserVolumeDlg", "Stereo Option", nullptr));
        mfleftChkBox->setText(QCoreApplication::translate("UserVolumeDlg", "Mute left", nullptr));
        mfrightChkBox->setText(QCoreApplication::translate("UserVolumeDlg", "Mute right", nullptr));
        defaultsButton->setText(QCoreApplication::translate("UserVolumeDlg", "&Default", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UserVolumeDlg: public Ui_UserVolumeDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERVOLUME_H
