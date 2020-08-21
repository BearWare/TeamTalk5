/********************************************************************************
** Form generated from reading UI file 'audiopreprocessor.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AUDIOPREPROCESSOR_H
#define UI_AUDIOPREPROCESSOR_H

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
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AudioPreprocessorDlg
{
public:
    QVBoxLayout *verticalLayout;
    QStackedWidget *stackedWidget;
    QWidget *page_0;
    QLabel *label_7;
    QWidget *page_1;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_7;
    QCheckBox *agcCheckBox;
    QSpacerItem *horizontalSpacer;
    QPushButton *spxDefaultButton;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QSpinBox *gainlevelSpinBox;
    QLabel *label_5;
    QSpinBox *maxgainSpinBox;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QSpinBox *maxincSpinBox;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_4;
    QSpinBox *maxdecSpinBox;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout_8;
    QCheckBox *denoiseCheckBox;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_6;
    QSpinBox *maxdenoiseSpinBox;
    QWidget *page_2;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QSlider *gainlevelSlider;
    QPushButton *ttdefaultButton;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *muteleftCheckBox;
    QCheckBox *muteRightCheckBox;
    QSpacerItem *verticalSpacer;
    QSpacerItem *verticalSpacer_2;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *AudioPreprocessorDlg)
    {
        if (AudioPreprocessorDlg->objectName().isEmpty())
            AudioPreprocessorDlg->setObjectName(QString::fromUtf8("AudioPreprocessorDlg"));
        AudioPreprocessorDlg->resize(371, 341);
        verticalLayout = new QVBoxLayout(AudioPreprocessorDlg);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        stackedWidget = new QStackedWidget(AudioPreprocessorDlg);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        page_0 = new QWidget();
        page_0->setObjectName(QString::fromUtf8("page_0"));
        label_7 = new QLabel(page_0);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(130, 110, 181, 61));
        label_7->setText(QString::fromUtf8("No Audio Preprocessor"));
        stackedWidget->addWidget(page_0);
        page_1 = new QWidget();
        page_1->setObjectName(QString::fromUtf8("page_1"));
        verticalLayout_3 = new QVBoxLayout(page_1);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        groupBox = new QGroupBox(page_1);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_4 = new QVBoxLayout(groupBox);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        agcCheckBox = new QCheckBox(groupBox);
        agcCheckBox->setObjectName(QString::fromUtf8("agcCheckBox"));

        horizontalLayout_7->addWidget(agcCheckBox);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer);

        spxDefaultButton = new QPushButton(groupBox);
        spxDefaultButton->setObjectName(QString::fromUtf8("spxDefaultButton"));

        horizontalLayout_7->addWidget(spxDefaultButton);


        verticalLayout_4->addLayout(horizontalLayout_7);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_3->addWidget(label_2);

        gainlevelSpinBox = new QSpinBox(groupBox);
        gainlevelSpinBox->setObjectName(QString::fromUtf8("gainlevelSpinBox"));

        horizontalLayout_3->addWidget(gainlevelSpinBox);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_3->addWidget(label_5);

        maxgainSpinBox = new QSpinBox(groupBox);
        maxgainSpinBox->setObjectName(QString::fromUtf8("maxgainSpinBox"));

        horizontalLayout_3->addWidget(maxgainSpinBox);


        verticalLayout_4->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_4->addWidget(label_3);

        maxincSpinBox = new QSpinBox(groupBox);
        maxincSpinBox->setObjectName(QString::fromUtf8("maxincSpinBox"));

        horizontalLayout_4->addWidget(maxincSpinBox);


        verticalLayout_4->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_5->addWidget(label_4);

        maxdecSpinBox = new QSpinBox(groupBox);
        maxdecSpinBox->setObjectName(QString::fromUtf8("maxdecSpinBox"));

        horizontalLayout_5->addWidget(maxdecSpinBox);


        verticalLayout_4->addLayout(horizontalLayout_5);


        verticalLayout_3->addWidget(groupBox);

        groupBox_2 = new QGroupBox(page_1);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_5 = new QVBoxLayout(groupBox_2);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        denoiseCheckBox = new QCheckBox(groupBox_2);
        denoiseCheckBox->setObjectName(QString::fromUtf8("denoiseCheckBox"));

        horizontalLayout_8->addWidget(denoiseCheckBox);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_2);


        verticalLayout_5->addLayout(horizontalLayout_8);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_6 = new QLabel(groupBox_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_6->addWidget(label_6);

        maxdenoiseSpinBox = new QSpinBox(groupBox_2);
        maxdenoiseSpinBox->setObjectName(QString::fromUtf8("maxdenoiseSpinBox"));

        horizontalLayout_6->addWidget(maxdenoiseSpinBox);


        verticalLayout_5->addLayout(horizontalLayout_6);


        verticalLayout_3->addWidget(groupBox_2);

        stackedWidget->addWidget(page_1);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        verticalLayout_2 = new QVBoxLayout(page_2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(page_2);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        gainlevelSlider = new QSlider(page_2);
        gainlevelSlider->setObjectName(QString::fromUtf8("gainlevelSlider"));
        gainlevelSlider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(gainlevelSlider);

        ttdefaultButton = new QPushButton(page_2);
        ttdefaultButton->setObjectName(QString::fromUtf8("ttdefaultButton"));

        horizontalLayout->addWidget(ttdefaultButton);


        verticalLayout_2->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        muteleftCheckBox = new QCheckBox(page_2);
        muteleftCheckBox->setObjectName(QString::fromUtf8("muteleftCheckBox"));

        horizontalLayout_2->addWidget(muteleftCheckBox);

        muteRightCheckBox = new QCheckBox(page_2);
        muteRightCheckBox->setObjectName(QString::fromUtf8("muteRightCheckBox"));

        horizontalLayout_2->addWidget(muteRightCheckBox);


        verticalLayout_2->addLayout(horizontalLayout_2);

        verticalSpacer = new QSpacerItem(266, 176, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        stackedWidget->addWidget(page_2);

        verticalLayout->addWidget(stackedWidget);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        buttonBox = new QDialogButtonBox(AudioPreprocessorDlg);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        label_2->setBuddy(gainlevelSpinBox);
        label_5->setBuddy(maxgainSpinBox);
        label_3->setBuddy(maxincSpinBox);
        label_4->setBuddy(maxdecSpinBox);
        label_6->setBuddy(maxdenoiseSpinBox);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(agcCheckBox, spxDefaultButton);
        QWidget::setTabOrder(spxDefaultButton, gainlevelSpinBox);
        QWidget::setTabOrder(gainlevelSpinBox, maxgainSpinBox);
        QWidget::setTabOrder(maxgainSpinBox, maxincSpinBox);
        QWidget::setTabOrder(maxincSpinBox, maxdecSpinBox);
        QWidget::setTabOrder(maxdecSpinBox, denoiseCheckBox);
        QWidget::setTabOrder(denoiseCheckBox, maxdenoiseSpinBox);
        QWidget::setTabOrder(maxdenoiseSpinBox, gainlevelSlider);
        QWidget::setTabOrder(gainlevelSlider, ttdefaultButton);
        QWidget::setTabOrder(ttdefaultButton, muteleftCheckBox);
        QWidget::setTabOrder(muteleftCheckBox, muteRightCheckBox);

        retranslateUi(AudioPreprocessorDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), AudioPreprocessorDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AudioPreprocessorDlg, SLOT(reject()));

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(AudioPreprocessorDlg);
    } // setupUi

    void retranslateUi(QDialog *AudioPreprocessorDlg)
    {
        AudioPreprocessorDlg->setWindowTitle(QCoreApplication::translate("AudioPreprocessorDlg", "Audio Preprocessor Setup", nullptr));
        groupBox->setTitle(QCoreApplication::translate("AudioPreprocessorDlg", "Automatic Gain Control (AGC)", nullptr));
        agcCheckBox->setText(QCoreApplication::translate("AudioPreprocessorDlg", "Enable AGC", nullptr));
        spxDefaultButton->setText(QCoreApplication::translate("AudioPreprocessorDlg", "Default", nullptr));
        label_2->setText(QCoreApplication::translate("AudioPreprocessorDlg", "Gain Level", nullptr));
        label_5->setText(QCoreApplication::translate("AudioPreprocessorDlg", "Max Gain dB", nullptr));
        label_3->setText(QCoreApplication::translate("AudioPreprocessorDlg", "Max Gain Increase Per Sec (dB)", nullptr));
        label_4->setText(QCoreApplication::translate("AudioPreprocessorDlg", "Max Gain Decrease Per Sec (dB)", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("AudioPreprocessorDlg", "Denoising", nullptr));
        denoiseCheckBox->setText(QCoreApplication::translate("AudioPreprocessorDlg", "Enable Denoising", nullptr));
        label_6->setText(QCoreApplication::translate("AudioPreprocessorDlg", "Max Attenuation of noise (dB)", nullptr));
        label->setText(QCoreApplication::translate("AudioPreprocessorDlg", "Gain Level", nullptr));
        ttdefaultButton->setText(QCoreApplication::translate("AudioPreprocessorDlg", "&Default", nullptr));
        muteleftCheckBox->setText(QCoreApplication::translate("AudioPreprocessorDlg", "Mute left channel", nullptr));
        muteRightCheckBox->setText(QCoreApplication::translate("AudioPreprocessorDlg", "Mute right channel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AudioPreprocessorDlg: public Ui_AudioPreprocessorDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUDIOPREPROCESSOR_H
