/********************************************************************************
** Form generated from reading UI file 'channel.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHANNEL_H
#define UI_CHANNEL_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChannelDlg
{
public:
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_6;
    QGroupBox *groupBox;
    QFormLayout *formLayout;
    QLabel *label_7;
    QLabel *chanpathLabel;
    QLabel *label;
    QLineEdit *nameEdit;
    QLabel *label_2;
    QPlainTextEdit *topicTextEdit;
    QLabel *label_3;
    QLineEdit *chanpasswdEdit;
    QLabel *label_4;
    QLineEdit *oppasswdEdit;
    QLabel *label_12;
    QSpinBox *maxusersSpinBox;
    QLabel *label_13;
    QSpinBox *diskquotaSpinBox;
    QVBoxLayout *verticalLayout_7;
    QCheckBox *staticchanBox;
    QCheckBox *singletxchanBox;
    QCheckBox *classroomchanBox;
    QCheckBox *oprecvonlychanBox;
    QCheckBox *novoiceactBox;
    QCheckBox *norecordBox;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_5;
    QComboBox *audiocodecBox;
    QStackedWidget *stackedWidget;
    QWidget *nocodecPage;
    QWidget *spxPage;
    QFormLayout *formLayout_3;
    QLabel *label_36;
    QHBoxLayout *horizontalLayout_22;
    QComboBox *spx_srateBox;
    QLabel *label_37;
    QSpacerItem *horizontalSpacer_6;
    QLabel *label_8;
    QHBoxLayout *horizontalLayout_8;
    QSlider *spx_qualitySlider;
    QLabel *spxQualityLabel;
    QSpacerItem *horizontalSpacer_7;
    QLabel *label_14;
    QHBoxLayout *horizontalLayout_3;
    QSpinBox *spx_txdelaySpinBox;
    QLabel *label_16;
    QSpacerItem *horizontalSpacer_8;
    QWidget *spxvbrPage;
    QFormLayout *formLayout_4;
    QLabel *label_38;
    QHBoxLayout *horizontalLayout_23;
    QComboBox *spxvbr_srateBox;
    QLabel *label_39;
    QSpacerItem *horizontalSpacer_9;
    QLabel *label_11;
    QHBoxLayout *horizontalLayout_7;
    QSlider *spxvbr_qualitySlider;
    QLabel *spxvbr_QualityLabel;
    QLabel *label_15;
    QHBoxLayout *horizontalLayout_9;
    QSpinBox *spxvbr_maxbpsSpinBox;
    QLabel *label_17;
    QCheckBox *spxvbr_dtxBox;
    QLabel *label_23;
    QHBoxLayout *horizontalLayout_24;
    QSpinBox *spxvbr_txdelaySpinBox;
    QLabel *label_24;
    QWidget *opusPage;
    QFormLayout *formLayout_5;
    QLabel *label_47;
    QHBoxLayout *horizontalLayout_27;
    QComboBox *opus_appBox;
    QSpacerItem *horizontalSpacer_12;
    QLabel *label_43;
    QHBoxLayout *horizontalLayout_26;
    QComboBox *opus_srateBox;
    QLabel *label_40;
    QSpacerItem *horizontalSpacer_11;
    QLabel *label_46;
    QHBoxLayout *horizontalLayout_25;
    QComboBox *opus_channelsBox;
    QSpacerItem *horizontalSpacer_10;
    QLabel *label_45;
    QHBoxLayout *horizontalLayout_29;
    QSpinBox *opus_bpsSpinBox;
    QLabel *label_42;
    QSpacerItem *horizontalSpacer_14;
    QCheckBox *opus_vbrCheckBox;
    QCheckBox *opus_dtxBox;
    QLabel *label_44;
    QHBoxLayout *horizontalLayout_28;
    QSpinBox *opus_txdelaySpinBox;
    QLabel *label_41;
    QSpacerItem *horizontalSpacer_13;
    QLabel *label_6;
    QComboBox *opus_framesizeComboBox;
    QLabel *label_10;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout;
    QCheckBox *agcBox;
    QHBoxLayout *horizontalLayout;
    QLabel *label_9;
    QSlider *gainlevelSlider;
    QLabel *gainlevelLabel;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ChannelDlg)
    {
        if (ChannelDlg->objectName().isEmpty())
            ChannelDlg->setObjectName(QString::fromUtf8("ChannelDlg"));
        ChannelDlg->resize(801, 602);
        ChannelDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        ChannelDlg->setModal(true);
        verticalLayout_3 = new QVBoxLayout(ChannelDlg);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        groupBox = new QGroupBox(ChannelDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        formLayout = new QFormLayout(groupBox);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_7);

        chanpathLabel = new QLabel(groupBox);
        chanpathLabel->setObjectName(QString::fromUtf8("chanpathLabel"));
        chanpathLabel->setText(QString::fromUtf8("TextLabel"));

        formLayout->setWidget(0, QFormLayout::FieldRole, chanpathLabel);

        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label);

        nameEdit = new QLineEdit(groupBox);
        nameEdit->setObjectName(QString::fromUtf8("nameEdit"));

        formLayout->setWidget(1, QFormLayout::FieldRole, nameEdit);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_2);

        topicTextEdit = new QPlainTextEdit(groupBox);
        topicTextEdit->setObjectName(QString::fromUtf8("topicTextEdit"));
        topicTextEdit->setTabChangesFocus(true);

        formLayout->setWidget(2, QFormLayout::FieldRole, topicTextEdit);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_3);

        chanpasswdEdit = new QLineEdit(groupBox);
        chanpasswdEdit->setObjectName(QString::fromUtf8("chanpasswdEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(chanpasswdEdit->sizePolicy().hasHeightForWidth());
        chanpasswdEdit->setSizePolicy(sizePolicy);

        formLayout->setWidget(3, QFormLayout::FieldRole, chanpasswdEdit);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout->setWidget(4, QFormLayout::LabelRole, label_4);

        oppasswdEdit = new QLineEdit(groupBox);
        oppasswdEdit->setObjectName(QString::fromUtf8("oppasswdEdit"));
        sizePolicy.setHeightForWidth(oppasswdEdit->sizePolicy().hasHeightForWidth());
        oppasswdEdit->setSizePolicy(sizePolicy);

        formLayout->setWidget(4, QFormLayout::FieldRole, oppasswdEdit);

        label_12 = new QLabel(groupBox);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        formLayout->setWidget(5, QFormLayout::LabelRole, label_12);

        maxusersSpinBox = new QSpinBox(groupBox);
        maxusersSpinBox->setObjectName(QString::fromUtf8("maxusersSpinBox"));
        sizePolicy.setHeightForWidth(maxusersSpinBox->sizePolicy().hasHeightForWidth());
        maxusersSpinBox->setSizePolicy(sizePolicy);
        maxusersSpinBox->setMaximum(1000);
        maxusersSpinBox->setValue(256);

        formLayout->setWidget(5, QFormLayout::FieldRole, maxusersSpinBox);

        label_13 = new QLabel(groupBox);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        formLayout->setWidget(6, QFormLayout::LabelRole, label_13);

        diskquotaSpinBox = new QSpinBox(groupBox);
        diskquotaSpinBox->setObjectName(QString::fromUtf8("diskquotaSpinBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(diskquotaSpinBox->sizePolicy().hasHeightForWidth());
        diskquotaSpinBox->setSizePolicy(sizePolicy1);
        diskquotaSpinBox->setMaximum(999999999);

        formLayout->setWidget(6, QFormLayout::FieldRole, diskquotaSpinBox);

        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setSpacing(6);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        staticchanBox = new QCheckBox(groupBox);
        staticchanBox->setObjectName(QString::fromUtf8("staticchanBox"));

        verticalLayout_7->addWidget(staticchanBox);

        singletxchanBox = new QCheckBox(groupBox);
        singletxchanBox->setObjectName(QString::fromUtf8("singletxchanBox"));

        verticalLayout_7->addWidget(singletxchanBox);

        classroomchanBox = new QCheckBox(groupBox);
        classroomchanBox->setObjectName(QString::fromUtf8("classroomchanBox"));

        verticalLayout_7->addWidget(classroomchanBox);

        oprecvonlychanBox = new QCheckBox(groupBox);
        oprecvonlychanBox->setObjectName(QString::fromUtf8("oprecvonlychanBox"));

        verticalLayout_7->addWidget(oprecvonlychanBox);

        novoiceactBox = new QCheckBox(groupBox);
        novoiceactBox->setObjectName(QString::fromUtf8("novoiceactBox"));

        verticalLayout_7->addWidget(novoiceactBox);

        norecordBox = new QCheckBox(groupBox);
        norecordBox->setObjectName(QString::fromUtf8("norecordBox"));

        verticalLayout_7->addWidget(norecordBox);


        formLayout->setLayout(7, QFormLayout::SpanningRole, verticalLayout_7);


        horizontalLayout_6->addWidget(groupBox);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox_2 = new QGroupBox(ChannelDlg);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_4 = new QVBoxLayout(groupBox_2);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_2->addWidget(label_5);

        audiocodecBox = new QComboBox(groupBox_2);
        audiocodecBox->setObjectName(QString::fromUtf8("audiocodecBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(audiocodecBox->sizePolicy().hasHeightForWidth());
        audiocodecBox->setSizePolicy(sizePolicy2);

        horizontalLayout_2->addWidget(audiocodecBox);


        verticalLayout_4->addLayout(horizontalLayout_2);

        stackedWidget = new QStackedWidget(groupBox_2);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        nocodecPage = new QWidget();
        nocodecPage->setObjectName(QString::fromUtf8("nocodecPage"));
        stackedWidget->addWidget(nocodecPage);
        spxPage = new QWidget();
        spxPage->setObjectName(QString::fromUtf8("spxPage"));
        formLayout_3 = new QFormLayout(spxPage);
        formLayout_3->setObjectName(QString::fromUtf8("formLayout_3"));
        label_36 = new QLabel(spxPage);
        label_36->setObjectName(QString::fromUtf8("label_36"));

        formLayout_3->setWidget(0, QFormLayout::LabelRole, label_36);

        horizontalLayout_22 = new QHBoxLayout();
        horizontalLayout_22->setObjectName(QString::fromUtf8("horizontalLayout_22"));
        spx_srateBox = new QComboBox(spxPage);
        spx_srateBox->setObjectName(QString::fromUtf8("spx_srateBox"));

        horizontalLayout_22->addWidget(spx_srateBox);

        label_37 = new QLabel(spxPage);
        label_37->setObjectName(QString::fromUtf8("label_37"));

        horizontalLayout_22->addWidget(label_37);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_22->addItem(horizontalSpacer_6);


        formLayout_3->setLayout(0, QFormLayout::FieldRole, horizontalLayout_22);

        label_8 = new QLabel(spxPage);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        formLayout_3->setWidget(1, QFormLayout::LabelRole, label_8);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        spx_qualitySlider = new QSlider(spxPage);
        spx_qualitySlider->setObjectName(QString::fromUtf8("spx_qualitySlider"));
        spx_qualitySlider->setMinimum(0);
        spx_qualitySlider->setMaximum(10);
        spx_qualitySlider->setValue(4);
        spx_qualitySlider->setOrientation(Qt::Horizontal);

        horizontalLayout_8->addWidget(spx_qualitySlider);

        spxQualityLabel = new QLabel(spxPage);
        spxQualityLabel->setObjectName(QString::fromUtf8("spxQualityLabel"));
        spxQualityLabel->setText(QString::fromUtf8("TextLabel"));

        horizontalLayout_8->addWidget(spxQualityLabel);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_7);


        formLayout_3->setLayout(1, QFormLayout::FieldRole, horizontalLayout_8);

        label_14 = new QLabel(spxPage);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        formLayout_3->setWidget(2, QFormLayout::LabelRole, label_14);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        spx_txdelaySpinBox = new QSpinBox(spxPage);
        spx_txdelaySpinBox->setObjectName(QString::fromUtf8("spx_txdelaySpinBox"));
        spx_txdelaySpinBox->setMinimum(20);
        spx_txdelaySpinBox->setMaximum(100);
        spx_txdelaySpinBox->setSingleStep(20);
        spx_txdelaySpinBox->setValue(40);

        horizontalLayout_3->addWidget(spx_txdelaySpinBox);

        label_16 = new QLabel(spxPage);
        label_16->setObjectName(QString::fromUtf8("label_16"));

        horizontalLayout_3->addWidget(label_16);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_8);


        formLayout_3->setLayout(2, QFormLayout::FieldRole, horizontalLayout_3);

        stackedWidget->addWidget(spxPage);
        spxvbrPage = new QWidget();
        spxvbrPage->setObjectName(QString::fromUtf8("spxvbrPage"));
        formLayout_4 = new QFormLayout(spxvbrPage);
        formLayout_4->setObjectName(QString::fromUtf8("formLayout_4"));
        label_38 = new QLabel(spxvbrPage);
        label_38->setObjectName(QString::fromUtf8("label_38"));

        formLayout_4->setWidget(0, QFormLayout::LabelRole, label_38);

        horizontalLayout_23 = new QHBoxLayout();
        horizontalLayout_23->setObjectName(QString::fromUtf8("horizontalLayout_23"));
        spxvbr_srateBox = new QComboBox(spxvbrPage);
        spxvbr_srateBox->setObjectName(QString::fromUtf8("spxvbr_srateBox"));

        horizontalLayout_23->addWidget(spxvbr_srateBox);

        label_39 = new QLabel(spxvbrPage);
        label_39->setObjectName(QString::fromUtf8("label_39"));

        horizontalLayout_23->addWidget(label_39);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_23->addItem(horizontalSpacer_9);


        formLayout_4->setLayout(0, QFormLayout::FieldRole, horizontalLayout_23);

        label_11 = new QLabel(spxvbrPage);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        formLayout_4->setWidget(1, QFormLayout::LabelRole, label_11);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        spxvbr_qualitySlider = new QSlider(spxvbrPage);
        spxvbr_qualitySlider->setObjectName(QString::fromUtf8("spxvbr_qualitySlider"));
        spxvbr_qualitySlider->setMinimum(0);
        spxvbr_qualitySlider->setMaximum(10);
        spxvbr_qualitySlider->setValue(4);
        spxvbr_qualitySlider->setOrientation(Qt::Horizontal);

        horizontalLayout_7->addWidget(spxvbr_qualitySlider);

        spxvbr_QualityLabel = new QLabel(spxvbrPage);
        spxvbr_QualityLabel->setObjectName(QString::fromUtf8("spxvbr_QualityLabel"));
        spxvbr_QualityLabel->setText(QString::fromUtf8("TextLabel"));

        horizontalLayout_7->addWidget(spxvbr_QualityLabel);


        formLayout_4->setLayout(1, QFormLayout::FieldRole, horizontalLayout_7);

        label_15 = new QLabel(spxvbrPage);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        formLayout_4->setWidget(2, QFormLayout::LabelRole, label_15);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        spxvbr_maxbpsSpinBox = new QSpinBox(spxvbrPage);
        spxvbr_maxbpsSpinBox->setObjectName(QString::fromUtf8("spxvbr_maxbpsSpinBox"));
        spxvbr_maxbpsSpinBox->setMinimum(0);
        spxvbr_maxbpsSpinBox->setMaximum(44000);
        spxvbr_maxbpsSpinBox->setSingleStep(1000);
        spxvbr_maxbpsSpinBox->setValue(0);

        horizontalLayout_9->addWidget(spxvbr_maxbpsSpinBox);

        label_17 = new QLabel(spxvbrPage);
        label_17->setObjectName(QString::fromUtf8("label_17"));

        horizontalLayout_9->addWidget(label_17);


        formLayout_4->setLayout(2, QFormLayout::FieldRole, horizontalLayout_9);

        spxvbr_dtxBox = new QCheckBox(spxvbrPage);
        spxvbr_dtxBox->setObjectName(QString::fromUtf8("spxvbr_dtxBox"));
        spxvbr_dtxBox->setChecked(true);

        formLayout_4->setWidget(3, QFormLayout::LabelRole, spxvbr_dtxBox);

        label_23 = new QLabel(spxvbrPage);
        label_23->setObjectName(QString::fromUtf8("label_23"));

        formLayout_4->setWidget(4, QFormLayout::LabelRole, label_23);

        horizontalLayout_24 = new QHBoxLayout();
        horizontalLayout_24->setObjectName(QString::fromUtf8("horizontalLayout_24"));
        spxvbr_txdelaySpinBox = new QSpinBox(spxvbrPage);
        spxvbr_txdelaySpinBox->setObjectName(QString::fromUtf8("spxvbr_txdelaySpinBox"));
        spxvbr_txdelaySpinBox->setMinimum(20);
        spxvbr_txdelaySpinBox->setMaximum(100);
        spxvbr_txdelaySpinBox->setSingleStep(20);
        spxvbr_txdelaySpinBox->setValue(40);

        horizontalLayout_24->addWidget(spxvbr_txdelaySpinBox);

        label_24 = new QLabel(spxvbrPage);
        label_24->setObjectName(QString::fromUtf8("label_24"));

        horizontalLayout_24->addWidget(label_24);


        formLayout_4->setLayout(4, QFormLayout::FieldRole, horizontalLayout_24);

        stackedWidget->addWidget(spxvbrPage);
        opusPage = new QWidget();
        opusPage->setObjectName(QString::fromUtf8("opusPage"));
        formLayout_5 = new QFormLayout(opusPage);
        formLayout_5->setObjectName(QString::fromUtf8("formLayout_5"));
        label_47 = new QLabel(opusPage);
        label_47->setObjectName(QString::fromUtf8("label_47"));

        formLayout_5->setWidget(0, QFormLayout::LabelRole, label_47);

        horizontalLayout_27 = new QHBoxLayout();
        horizontalLayout_27->setObjectName(QString::fromUtf8("horizontalLayout_27"));
        opus_appBox = new QComboBox(opusPage);
        opus_appBox->setObjectName(QString::fromUtf8("opus_appBox"));

        horizontalLayout_27->addWidget(opus_appBox);

        horizontalSpacer_12 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_27->addItem(horizontalSpacer_12);


        formLayout_5->setLayout(0, QFormLayout::FieldRole, horizontalLayout_27);

        label_43 = new QLabel(opusPage);
        label_43->setObjectName(QString::fromUtf8("label_43"));

        formLayout_5->setWidget(1, QFormLayout::LabelRole, label_43);

        horizontalLayout_26 = new QHBoxLayout();
        horizontalLayout_26->setObjectName(QString::fromUtf8("horizontalLayout_26"));
        opus_srateBox = new QComboBox(opusPage);
        opus_srateBox->setObjectName(QString::fromUtf8("opus_srateBox"));

        horizontalLayout_26->addWidget(opus_srateBox);

        label_40 = new QLabel(opusPage);
        label_40->setObjectName(QString::fromUtf8("label_40"));

        horizontalLayout_26->addWidget(label_40);

        horizontalSpacer_11 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_26->addItem(horizontalSpacer_11);


        formLayout_5->setLayout(1, QFormLayout::FieldRole, horizontalLayout_26);

        label_46 = new QLabel(opusPage);
        label_46->setObjectName(QString::fromUtf8("label_46"));

        formLayout_5->setWidget(2, QFormLayout::LabelRole, label_46);

        horizontalLayout_25 = new QHBoxLayout();
        horizontalLayout_25->setObjectName(QString::fromUtf8("horizontalLayout_25"));
        opus_channelsBox = new QComboBox(opusPage);
        opus_channelsBox->setObjectName(QString::fromUtf8("opus_channelsBox"));

        horizontalLayout_25->addWidget(opus_channelsBox);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_25->addItem(horizontalSpacer_10);


        formLayout_5->setLayout(2, QFormLayout::FieldRole, horizontalLayout_25);

        label_45 = new QLabel(opusPage);
        label_45->setObjectName(QString::fromUtf8("label_45"));

        formLayout_5->setWidget(3, QFormLayout::LabelRole, label_45);

        horizontalLayout_29 = new QHBoxLayout();
        horizontalLayout_29->setObjectName(QString::fromUtf8("horizontalLayout_29"));
        opus_bpsSpinBox = new QSpinBox(opusPage);
        opus_bpsSpinBox->setObjectName(QString::fromUtf8("opus_bpsSpinBox"));

        horizontalLayout_29->addWidget(opus_bpsSpinBox);

        label_42 = new QLabel(opusPage);
        label_42->setObjectName(QString::fromUtf8("label_42"));

        horizontalLayout_29->addWidget(label_42);

        horizontalSpacer_14 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_29->addItem(horizontalSpacer_14);

        opus_vbrCheckBox = new QCheckBox(opusPage);
        opus_vbrCheckBox->setObjectName(QString::fromUtf8("opus_vbrCheckBox"));

        horizontalLayout_29->addWidget(opus_vbrCheckBox);


        formLayout_5->setLayout(3, QFormLayout::FieldRole, horizontalLayout_29);

        opus_dtxBox = new QCheckBox(opusPage);
        opus_dtxBox->setObjectName(QString::fromUtf8("opus_dtxBox"));
        opus_dtxBox->setChecked(true);

        formLayout_5->setWidget(4, QFormLayout::SpanningRole, opus_dtxBox);

        label_44 = new QLabel(opusPage);
        label_44->setObjectName(QString::fromUtf8("label_44"));

        formLayout_5->setWidget(5, QFormLayout::LabelRole, label_44);

        horizontalLayout_28 = new QHBoxLayout();
        horizontalLayout_28->setObjectName(QString::fromUtf8("horizontalLayout_28"));
        opus_txdelaySpinBox = new QSpinBox(opusPage);
        opus_txdelaySpinBox->setObjectName(QString::fromUtf8("opus_txdelaySpinBox"));

        horizontalLayout_28->addWidget(opus_txdelaySpinBox);

        label_41 = new QLabel(opusPage);
        label_41->setObjectName(QString::fromUtf8("label_41"));

        horizontalLayout_28->addWidget(label_41);

        horizontalSpacer_13 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_28->addItem(horizontalSpacer_13);

        label_6 = new QLabel(opusPage);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_28->addWidget(label_6);

        opus_framesizeComboBox = new QComboBox(opusPage);
        opus_framesizeComboBox->setObjectName(QString::fromUtf8("opus_framesizeComboBox"));

        horizontalLayout_28->addWidget(opus_framesizeComboBox);

        label_10 = new QLabel(opusPage);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        horizontalLayout_28->addWidget(label_10);


        formLayout_5->setLayout(5, QFormLayout::FieldRole, horizontalLayout_28);

        stackedWidget->addWidget(opusPage);

        verticalLayout_4->addWidget(stackedWidget);


        verticalLayout_2->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(ChannelDlg);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        verticalLayout = new QVBoxLayout(groupBox_3);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        agcBox = new QCheckBox(groupBox_3);
        agcBox->setObjectName(QString::fromUtf8("agcBox"));

        verticalLayout->addWidget(agcBox);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_9 = new QLabel(groupBox_3);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        horizontalLayout->addWidget(label_9);

        gainlevelSlider = new QSlider(groupBox_3);
        gainlevelSlider->setObjectName(QString::fromUtf8("gainlevelSlider"));
        gainlevelSlider->setMinimum(1);
        gainlevelSlider->setMaximum(32);
        gainlevelSlider->setValue(8);
        gainlevelSlider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(gainlevelSlider);

        gainlevelLabel = new QLabel(groupBox_3);
        gainlevelLabel->setObjectName(QString::fromUtf8("gainlevelLabel"));
        gainlevelLabel->setText(QString::fromUtf8("TextLabel"));

        horizontalLayout->addWidget(gainlevelLabel);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_2->addWidget(groupBox_3);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);


        horizontalLayout_6->addLayout(verticalLayout_2);


        verticalLayout_3->addLayout(horizontalLayout_6);

        buttonBox = new QDialogButtonBox(ChannelDlg);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_3->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        label->setBuddy(nameEdit);
        label_2->setBuddy(topicTextEdit);
        label_3->setBuddy(chanpasswdEdit);
        label_4->setBuddy(oppasswdEdit);
        label_12->setBuddy(maxusersSpinBox);
        label_13->setBuddy(diskquotaSpinBox);
        label_5->setBuddy(audiocodecBox);
        label_8->setBuddy(spx_qualitySlider);
        label_14->setBuddy(spx_txdelaySpinBox);
        label_11->setBuddy(spx_qualitySlider);
        label_15->setBuddy(spxvbr_maxbpsSpinBox);
        label_23->setBuddy(spx_txdelaySpinBox);
        label_45->setBuddy(opus_bpsSpinBox);
        label_44->setBuddy(opus_txdelaySpinBox);
        label_6->setBuddy(opus_framesizeComboBox);
        label_9->setBuddy(gainlevelSlider);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(nameEdit, topicTextEdit);
        QWidget::setTabOrder(topicTextEdit, chanpasswdEdit);
        QWidget::setTabOrder(chanpasswdEdit, oppasswdEdit);
        QWidget::setTabOrder(oppasswdEdit, maxusersSpinBox);
        QWidget::setTabOrder(maxusersSpinBox, diskquotaSpinBox);
        QWidget::setTabOrder(diskquotaSpinBox, staticchanBox);
        QWidget::setTabOrder(staticchanBox, singletxchanBox);
        QWidget::setTabOrder(singletxchanBox, classroomchanBox);
        QWidget::setTabOrder(classroomchanBox, oprecvonlychanBox);
        QWidget::setTabOrder(oprecvonlychanBox, novoiceactBox);
        QWidget::setTabOrder(novoiceactBox, norecordBox);
        QWidget::setTabOrder(norecordBox, audiocodecBox);
        QWidget::setTabOrder(audiocodecBox, spx_srateBox);
        QWidget::setTabOrder(spx_srateBox, spx_qualitySlider);
        QWidget::setTabOrder(spx_qualitySlider, spx_txdelaySpinBox);
        QWidget::setTabOrder(spx_txdelaySpinBox, spxvbr_srateBox);
        QWidget::setTabOrder(spxvbr_srateBox, spxvbr_qualitySlider);
        QWidget::setTabOrder(spxvbr_qualitySlider, spxvbr_maxbpsSpinBox);
        QWidget::setTabOrder(spxvbr_maxbpsSpinBox, spxvbr_dtxBox);
        QWidget::setTabOrder(spxvbr_dtxBox, spxvbr_txdelaySpinBox);
        QWidget::setTabOrder(spxvbr_txdelaySpinBox, opus_appBox);
        QWidget::setTabOrder(opus_appBox, opus_srateBox);
        QWidget::setTabOrder(opus_srateBox, opus_channelsBox);
        QWidget::setTabOrder(opus_channelsBox, opus_bpsSpinBox);
        QWidget::setTabOrder(opus_bpsSpinBox, opus_vbrCheckBox);
        QWidget::setTabOrder(opus_vbrCheckBox, opus_dtxBox);
        QWidget::setTabOrder(opus_dtxBox, opus_txdelaySpinBox);
        QWidget::setTabOrder(opus_txdelaySpinBox, opus_framesizeComboBox);
        QWidget::setTabOrder(opus_framesizeComboBox, agcBox);
        QWidget::setTabOrder(agcBox, gainlevelSlider);

        retranslateUi(ChannelDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), ChannelDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ChannelDlg, SLOT(reject()));

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ChannelDlg);
    } // setupUi

    void retranslateUi(QDialog *ChannelDlg)
    {
        ChannelDlg->setWindowTitle(QCoreApplication::translate("ChannelDlg", "Channel", nullptr));
        groupBox->setTitle(QCoreApplication::translate("ChannelDlg", "Channel Information", nullptr));
        label_7->setText(QCoreApplication::translate("ChannelDlg", "Channel path", nullptr));
        label->setText(QCoreApplication::translate("ChannelDlg", "Channel name", nullptr));
        label_2->setText(QCoreApplication::translate("ChannelDlg", "Topic", nullptr));
        label_3->setText(QCoreApplication::translate("ChannelDlg", "Password", nullptr));
        label_4->setText(QCoreApplication::translate("ChannelDlg", "Operator password", nullptr));
        label_12->setText(QCoreApplication::translate("ChannelDlg", "Max users", nullptr));
        label_13->setText(QCoreApplication::translate("ChannelDlg", "Disk quota (KBytes)", nullptr));
        staticchanBox->setText(QCoreApplication::translate("ChannelDlg", "Permanent channel (stored on server)", nullptr));
        singletxchanBox->setText(QCoreApplication::translate("ChannelDlg", "No interruptions (no simultaneous voice transmission)", nullptr));
        classroomchanBox->setText(QCoreApplication::translate("ChannelDlg", "Classroom (operator-controlled transmissions)", nullptr));
        oprecvonlychanBox->setText(QCoreApplication::translate("ChannelDlg", "Operator receive only (only operator see and hear users)", nullptr));
        novoiceactBox->setText(QCoreApplication::translate("ChannelDlg", "No voice activation (only Push-to-Talk allowed)", nullptr));
        norecordBox->setText(QCoreApplication::translate("ChannelDlg", "No audio recording allowed (save to disk not allowed)", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("ChannelDlg", "Audio Codec", nullptr));
        label_5->setText(QCoreApplication::translate("ChannelDlg", "Codec type", nullptr));
        label_36->setText(QCoreApplication::translate("ChannelDlg", "Sample rate", nullptr));
        label_37->setText(QCoreApplication::translate("ChannelDlg", "Hz", nullptr));
        label_8->setText(QCoreApplication::translate("ChannelDlg", "Quality", nullptr));
        label_14->setText(QCoreApplication::translate("ChannelDlg", "Transmit interval", nullptr));
        label_16->setText(QCoreApplication::translate("ChannelDlg", "msec", nullptr));
        label_38->setText(QCoreApplication::translate("ChannelDlg", "Sample rate", nullptr));
        label_39->setText(QCoreApplication::translate("ChannelDlg", "Hz", nullptr));
        label_11->setText(QCoreApplication::translate("ChannelDlg", "Quality", nullptr));
        label_15->setText(QCoreApplication::translate("ChannelDlg", "Max bitrate", nullptr));
        label_17->setText(QCoreApplication::translate("ChannelDlg", "bps", nullptr));
        spxvbr_dtxBox->setText(QCoreApplication::translate("ChannelDlg", "Ignore silence (DTX)", nullptr));
        label_23->setText(QCoreApplication::translate("ChannelDlg", "Transmit interval", nullptr));
        label_24->setText(QCoreApplication::translate("ChannelDlg", "msec", nullptr));
        label_47->setText(QCoreApplication::translate("ChannelDlg", "Application", nullptr));
        label_43->setText(QCoreApplication::translate("ChannelDlg", "Sample rate", nullptr));
        label_40->setText(QCoreApplication::translate("ChannelDlg", "Hz", nullptr));
        label_46->setText(QCoreApplication::translate("ChannelDlg", "Audio channels", nullptr));
        label_45->setText(QCoreApplication::translate("ChannelDlg", "Bitrate", nullptr));
        label_42->setText(QCoreApplication::translate("ChannelDlg", "Kbps", nullptr));
        opus_vbrCheckBox->setText(QCoreApplication::translate("ChannelDlg", "Variable bitrate", nullptr));
        opus_dtxBox->setText(QCoreApplication::translate("ChannelDlg", "Ignore silence (DTX)", nullptr));
        label_44->setText(QCoreApplication::translate("ChannelDlg", "Transmit interval", nullptr));
        label_41->setText(QCoreApplication::translate("ChannelDlg", "msec", nullptr));
        label_6->setText(QCoreApplication::translate("ChannelDlg", "Frame size", nullptr));
        label_10->setText(QCoreApplication::translate("ChannelDlg", "msec", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("ChannelDlg", "Audio Configuration", nullptr));
#if QT_CONFIG(tooltip)
        agcBox->setToolTip(QCoreApplication::translate("ChannelDlg", "Ensure all users in the channel have the same audio volume", nullptr));
#endif // QT_CONFIG(tooltip)
        agcBox->setText(QCoreApplication::translate("ChannelDlg", "Enable fixed audio volume for all users", nullptr));
        label_9->setText(QCoreApplication::translate("ChannelDlg", "Volume level", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChannelDlg: public Ui_ChannelDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHANNEL_H
