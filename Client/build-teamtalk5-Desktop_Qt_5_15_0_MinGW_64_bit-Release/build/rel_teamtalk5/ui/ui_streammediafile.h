/********************************************************************************
** Form generated from reading UI file 'streammediafile.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STREAMMEDIAFILE_H
#define UI_STREAMMEDIAFILE_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StreamMediaFileDlg
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *mediafileComboBox;
    QToolButton *toolButton;
    QToolButton *refreshBtn;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLabel *audioLabel;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_3;
    QLabel *videoLabel;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_4;
    QLabel *durationLabel;
    QCheckBox *loopChkBox;
    QGroupBox *playbackGroupBox;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_5;
    QComboBox *preprocessorComboBox;
    QPushButton *preprocessButton;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_6;
    QSlider *playbackOffsetSlider;
    QLabel *playbackTimeLabel;
    QHBoxLayout *horizontalLayout_8;
    QLabel *label_7;
    QToolButton *stopToolButton;
    QToolButton *startToolButton;
    QSpacerItem *horizontalSpacer_3;
    QGroupBox *vidgroupBox;
    QVBoxLayout *verticalLayout_14;
    QHBoxLayout *horizontalLayout_19;
    QLabel *label_33;
    QComboBox *vidcodecBox;
    QSpacerItem *horizontalSpacer_21;
    QStackedWidget *vidcodecStackedWidget;
    QWidget *vp8_page;
    QVBoxLayout *verticalLayout_15;
    QHBoxLayout *horizontalLayout_21;
    QLabel *label_42;
    QSpinBox *vp8bitrateSpinBox;
    QLabel *label_43;
    QSpacerItem *horizontalSpacer_22;
    QSpacerItem *verticalSpacer_9;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer;
    QPushButton *okBtn;
    QPushButton *cancelBtn;

    void setupUi(QDialog *StreamMediaFileDlg)
    {
        if (StreamMediaFileDlg->objectName().isEmpty())
            StreamMediaFileDlg->setObjectName(QString::fromUtf8("StreamMediaFileDlg"));
        StreamMediaFileDlg->resize(511, 523);
        verticalLayout_2 = new QVBoxLayout(StreamMediaFileDlg);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox = new QGroupBox(StreamMediaFileDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        mediafileComboBox = new QComboBox(groupBox);
        mediafileComboBox->setObjectName(QString::fromUtf8("mediafileComboBox"));
        mediafileComboBox->setEditable(true);

        horizontalLayout->addWidget(mediafileComboBox);

        toolButton = new QToolButton(groupBox);
        toolButton->setObjectName(QString::fromUtf8("toolButton"));

        horizontalLayout->addWidget(toolButton);

        refreshBtn = new QToolButton(groupBox);
        refreshBtn->setObjectName(QString::fromUtf8("refreshBtn"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/images/refresh.png"), QSize(), QIcon::Normal, QIcon::Off);
        refreshBtn->setIcon(icon);

        horizontalLayout->addWidget(refreshBtn);

        horizontalLayout->setStretch(1, 1);

        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        audioLabel = new QLabel(groupBox);
        audioLabel->setObjectName(QString::fromUtf8("audioLabel"));

        horizontalLayout_2->addWidget(audioLabel);

        horizontalLayout_2->setStretch(1, 1);

        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_3->addWidget(label_3);

        videoLabel = new QLabel(groupBox);
        videoLabel->setObjectName(QString::fromUtf8("videoLabel"));

        horizontalLayout_3->addWidget(videoLabel);

        horizontalLayout_3->setStretch(1, 1);

        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_5->addWidget(label_4);

        durationLabel = new QLabel(groupBox);
        durationLabel->setObjectName(QString::fromUtf8("durationLabel"));

        horizontalLayout_5->addWidget(durationLabel);

        horizontalLayout_5->setStretch(1, 1);

        verticalLayout->addLayout(horizontalLayout_5);

        loopChkBox = new QCheckBox(groupBox);
        loopChkBox->setObjectName(QString::fromUtf8("loopChkBox"));

        verticalLayout->addWidget(loopChkBox);


        verticalLayout_2->addWidget(groupBox);

        playbackGroupBox = new QGroupBox(StreamMediaFileDlg);
        playbackGroupBox->setObjectName(QString::fromUtf8("playbackGroupBox"));
        verticalLayout_3 = new QVBoxLayout(playbackGroupBox);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_5 = new QLabel(playbackGroupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_7->addWidget(label_5);

        preprocessorComboBox = new QComboBox(playbackGroupBox);
        preprocessorComboBox->setObjectName(QString::fromUtf8("preprocessorComboBox"));

        horizontalLayout_7->addWidget(preprocessorComboBox);

        preprocessButton = new QPushButton(playbackGroupBox);
        preprocessButton->setObjectName(QString::fromUtf8("preprocessButton"));
        preprocessButton->setAutoDefault(false);

        horizontalLayout_7->addWidget(preprocessButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_2);


        verticalLayout_3->addLayout(horizontalLayout_7);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_6 = new QLabel(playbackGroupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_6->addWidget(label_6);

        playbackOffsetSlider = new QSlider(playbackGroupBox);
        playbackOffsetSlider->setObjectName(QString::fromUtf8("playbackOffsetSlider"));
        playbackOffsetSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_6->addWidget(playbackOffsetSlider);

        playbackTimeLabel = new QLabel(playbackGroupBox);
        playbackTimeLabel->setObjectName(QString::fromUtf8("playbackTimeLabel"));
        playbackTimeLabel->setText(QString::fromUtf8("0:00:00.000"));

        horizontalLayout_6->addWidget(playbackTimeLabel);


        verticalLayout_3->addLayout(horizontalLayout_6);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        label_7 = new QLabel(playbackGroupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_8->addWidget(label_7);

        stopToolButton = new QToolButton(playbackGroupBox);
        stopToolButton->setObjectName(QString::fromUtf8("stopToolButton"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/images/stop.png"), QSize(), QIcon::Normal, QIcon::Off);
        stopToolButton->setIcon(icon1);

        horizontalLayout_8->addWidget(stopToolButton);

        startToolButton = new QToolButton(playbackGroupBox);
        startToolButton->setObjectName(QString::fromUtf8("startToolButton"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/images/play.png"), QSize(), QIcon::Normal, QIcon::Off);
        startToolButton->setIcon(icon2);
        startToolButton->setCheckable(false);

        horizontalLayout_8->addWidget(startToolButton);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_3);


        verticalLayout_3->addLayout(horizontalLayout_8);


        verticalLayout_2->addWidget(playbackGroupBox);

        vidgroupBox = new QGroupBox(StreamMediaFileDlg);
        vidgroupBox->setObjectName(QString::fromUtf8("vidgroupBox"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(vidgroupBox->sizePolicy().hasHeightForWidth());
        vidgroupBox->setSizePolicy(sizePolicy);
        verticalLayout_14 = new QVBoxLayout(vidgroupBox);
        verticalLayout_14->setObjectName(QString::fromUtf8("verticalLayout_14"));
        horizontalLayout_19 = new QHBoxLayout();
        horizontalLayout_19->setObjectName(QString::fromUtf8("horizontalLayout_19"));
        label_33 = new QLabel(vidgroupBox);
        label_33->setObjectName(QString::fromUtf8("label_33"));

        horizontalLayout_19->addWidget(label_33);

        vidcodecBox = new QComboBox(vidgroupBox);
        vidcodecBox->setObjectName(QString::fromUtf8("vidcodecBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(vidcodecBox->sizePolicy().hasHeightForWidth());
        vidcodecBox->setSizePolicy(sizePolicy1);

        horizontalLayout_19->addWidget(vidcodecBox);

        horizontalSpacer_21 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_19->addItem(horizontalSpacer_21);


        verticalLayout_14->addLayout(horizontalLayout_19);

        vidcodecStackedWidget = new QStackedWidget(vidgroupBox);
        vidcodecStackedWidget->setObjectName(QString::fromUtf8("vidcodecStackedWidget"));
        vp8_page = new QWidget();
        vp8_page->setObjectName(QString::fromUtf8("vp8_page"));
        verticalLayout_15 = new QVBoxLayout(vp8_page);
        verticalLayout_15->setObjectName(QString::fromUtf8("verticalLayout_15"));
        verticalLayout_15->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_21 = new QHBoxLayout();
        horizontalLayout_21->setObjectName(QString::fromUtf8("horizontalLayout_21"));
        label_42 = new QLabel(vp8_page);
        label_42->setObjectName(QString::fromUtf8("label_42"));

        horizontalLayout_21->addWidget(label_42);

        vp8bitrateSpinBox = new QSpinBox(vp8_page);
        vp8bitrateSpinBox->setObjectName(QString::fromUtf8("vp8bitrateSpinBox"));
        sizePolicy1.setHeightForWidth(vp8bitrateSpinBox->sizePolicy().hasHeightForWidth());
        vp8bitrateSpinBox->setSizePolicy(sizePolicy1);
        vp8bitrateSpinBox->setMinimum(0);
        vp8bitrateSpinBox->setMaximum(1000);
        vp8bitrateSpinBox->setValue(0);

        horizontalLayout_21->addWidget(vp8bitrateSpinBox);

        label_43 = new QLabel(vp8_page);
        label_43->setObjectName(QString::fromUtf8("label_43"));
        label_43->setText(QString::fromUtf8("kbps"));

        horizontalLayout_21->addWidget(label_43);

        horizontalSpacer_22 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_21->addItem(horizontalSpacer_22);


        verticalLayout_15->addLayout(horizontalLayout_21);

        vidcodecStackedWidget->addWidget(vp8_page);

        verticalLayout_14->addWidget(vidcodecStackedWidget);


        verticalLayout_2->addWidget(vidgroupBox);

        verticalSpacer_9 = new QSpacerItem(20, 73, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_9);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);

        okBtn = new QPushButton(StreamMediaFileDlg);
        okBtn->setObjectName(QString::fromUtf8("okBtn"));

        horizontalLayout_4->addWidget(okBtn);

        cancelBtn = new QPushButton(StreamMediaFileDlg);
        cancelBtn->setObjectName(QString::fromUtf8("cancelBtn"));

        horizontalLayout_4->addWidget(cancelBtn);


        verticalLayout_2->addLayout(horizontalLayout_4);

#if QT_CONFIG(shortcut)
        playbackTimeLabel->setBuddy(playbackOffsetSlider);
        label_33->setBuddy(vidcodecBox);
        label_42->setBuddy(vp8bitrateSpinBox);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(mediafileComboBox, toolButton);
        QWidget::setTabOrder(toolButton, refreshBtn);
        QWidget::setTabOrder(refreshBtn, loopChkBox);
        QWidget::setTabOrder(loopChkBox, preprocessorComboBox);
        QWidget::setTabOrder(preprocessorComboBox, preprocessButton);
        QWidget::setTabOrder(preprocessButton, playbackOffsetSlider);
        QWidget::setTabOrder(playbackOffsetSlider, stopToolButton);
        QWidget::setTabOrder(stopToolButton, startToolButton);
        QWidget::setTabOrder(startToolButton, vidcodecBox);
        QWidget::setTabOrder(vidcodecBox, vp8bitrateSpinBox);
        QWidget::setTabOrder(vp8bitrateSpinBox, okBtn);
        QWidget::setTabOrder(okBtn, cancelBtn);

        retranslateUi(StreamMediaFileDlg);
        QObject::connect(okBtn, SIGNAL(pressed()), StreamMediaFileDlg, SLOT(accept()));
        QObject::connect(cancelBtn, SIGNAL(pressed()), StreamMediaFileDlg, SLOT(reject()));

        vidcodecStackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(StreamMediaFileDlg);
    } // setupUi

    void retranslateUi(QDialog *StreamMediaFileDlg)
    {
        StreamMediaFileDlg->setWindowTitle(QCoreApplication::translate("StreamMediaFileDlg", "Stream Media File To Channel", nullptr));
        groupBox->setTitle(QCoreApplication::translate("StreamMediaFileDlg", "Media File Properties", nullptr));
        label->setText(QCoreApplication::translate("StreamMediaFileDlg", "Media file", nullptr));
#if QT_CONFIG(accessibility)
        toolButton->setAccessibleName(QCoreApplication::translate("StreamMediaFileDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
        toolButton->setText(QCoreApplication::translate("StreamMediaFileDlg", "...", nullptr));
#if QT_CONFIG(accessibility)
        refreshBtn->setAccessibleName(QCoreApplication::translate("StreamMediaFileDlg", "Refresh", nullptr));
#endif // QT_CONFIG(accessibility)
        refreshBtn->setText(QCoreApplication::translate("StreamMediaFileDlg", "...", nullptr));
        label_2->setText(QCoreApplication::translate("StreamMediaFileDlg", "Audio format:", nullptr));
        audioLabel->setText(QString());
        label_3->setText(QCoreApplication::translate("StreamMediaFileDlg", "Video format:", nullptr));
        videoLabel->setText(QString());
        label_4->setText(QCoreApplication::translate("StreamMediaFileDlg", "Duration:", nullptr));
        durationLabel->setText(QString());
        loopChkBox->setText(QCoreApplication::translate("StreamMediaFileDlg", "Continuously play media file", nullptr));
        playbackGroupBox->setTitle(QCoreApplication::translate("StreamMediaFileDlg", "Playback Settings", nullptr));
        label_5->setText(QCoreApplication::translate("StreamMediaFileDlg", "Audio preprocessor", nullptr));
        preprocessButton->setText(QCoreApplication::translate("StreamMediaFileDlg", "Setup", nullptr));
        label_6->setText(QCoreApplication::translate("StreamMediaFileDlg", "Start position", nullptr));
        label_7->setText(QCoreApplication::translate("StreamMediaFileDlg", "Test playback", nullptr));
        stopToolButton->setText(QCoreApplication::translate("StreamMediaFileDlg", "Stop", nullptr));
        startToolButton->setText(QCoreApplication::translate("StreamMediaFileDlg", "Play", nullptr));
#if QT_CONFIG(accessibility)
        vidgroupBox->setAccessibleName(QCoreApplication::translate("StreamMediaFileDlg", "Video Codec Settings", nullptr));
#endif // QT_CONFIG(accessibility)
        vidgroupBox->setTitle(QCoreApplication::translate("StreamMediaFileDlg", "Video Codec Settings", nullptr));
        label_33->setText(QCoreApplication::translate("StreamMediaFileDlg", "Codec", nullptr));
        label_42->setText(QCoreApplication::translate("StreamMediaFileDlg", "Bitrate", nullptr));
        okBtn->setText(QCoreApplication::translate("StreamMediaFileDlg", "&OK", nullptr));
        cancelBtn->setText(QCoreApplication::translate("StreamMediaFileDlg", "&Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class StreamMediaFileDlg: public Ui_StreamMediaFileDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STREAMMEDIAFILE_H
