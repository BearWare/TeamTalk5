/********************************************************************************
** Form generated from reading UI file 'preferences.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PREFERENCES_H
#define UI_PREFERENCES_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PreferencesDlg
{
public:
    QGridLayout *gridLayout_6;
    QHBoxLayout *horizontalLayout_5;
    QSpacerItem *horizontalSpacer_7;
    QDialogButtonBox *buttonBox;
    QSpacerItem *horizontalSpacer_8;
    QTabWidget *tabWidget;
    QWidget *generalTab;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *nicknameEdit;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_7;
    QSpacerItem *horizontalSpacer_12;
    QRadioButton *maleRadioButton;
    QRadioButton *femaleRadioButton;
    QSpacerItem *horizontalSpacer_13;
    QHBoxLayout *horizontalLayout_22;
    QLabel *label_35;
    QLineEdit *bearwareidEdit;
    QPushButton *setupBearWareLoginButton;
    QSpacerItem *horizontalSpacer_24;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QSpinBox *awaySpinBox;
    QLabel *label_3;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *groupBox_2;
    QFormLayout *formLayout;
    QCheckBox *pttChkBox;
    QPushButton *setupkeysButton;
    QLabel *label_4;
    QLineEdit *keycompEdit;
    QCheckBox *voiceactChkBox;
    QSpacerItem *verticalSpacer;
    QWidget *displayTab;
    QVBoxLayout *verticalLayout_9;
    QGroupBox *groupBox_3;
    QHBoxLayout *horizontalLayout_18;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_17;
    QLabel *label_5;
    QComboBox *languageBox;
    QCheckBox *startminimizedChkBox;
    QCheckBox *trayChkBox;
    QCheckBox *alwaysontopChkBox;
    QCheckBox *vumeterChkBox;
    QCheckBox *usercountChkBox;
    QCheckBox *showusernameChkBox;
    QCheckBox *lasttalkChkBox;
    QCheckBox *msgpopupChkBox;
    QCheckBox *videodlgChkBox;
    QCheckBox *returnvidChkBox;
    QCheckBox *desktopdlgChkBox;
    QSpacerItem *horizontalSpacer_20;
    QVBoxLayout *verticalLayout_4;
    QCheckBox *msgtimestampChkBox;
    QCheckBox *logstatusbarChkBox;
    QCheckBox *updatesChkBox;
    QHBoxLayout *horizontalLayout_15;
    QCheckBox *vidtextChkBox;
    QToolButton *vidtextsrcToolBtn;
    QSpacerItem *horizontalSpacer_19;
    QHBoxLayout *horizontalLayout_16;
    QLabel *label_39;
    QSpinBox *maxtextSpinBox;
    QSpacerItem *horizontalSpacer_18;
    QSpacerItem *verticalSpacer_8;
    QSpacerItem *verticalSpacer_6;
    QWidget *connectionTab;
    QVBoxLayout *verticalLayout_13;
    QGroupBox *groupBox_9;
    QVBoxLayout *verticalLayout_5;
    QCheckBox *autoconnectChkBox;
    QCheckBox *reconnectChkBox;
    QCheckBox *autojoinChkBox;
    QCheckBox *maxpayloadChkBox;
    QCheckBox *winfwChkBox;
    QGroupBox *groupBox_21;
    QHBoxLayout *horizontalLayout_13;
    QGridLayout *gridLayout;
    QCheckBox *subusermsgChkBox;
    QCheckBox *subchanmsgChkBox;
    QCheckBox *subbcastmsgChkBox;
    QCheckBox *subvoiceChkBox;
    QCheckBox *subvidcapChkBox;
    QCheckBox *subdesktopChkBox;
    QHBoxLayout *horizontalLayout_20;
    QLabel *label_41;
    QToolButton *subdeskinputBtn;
    QCheckBox *submediafileChkBox;
    QSpacerItem *horizontalSpacer_15;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_2;
    QLabel *label_8;
    QLabel *label_10;
    QLabel *label_9;
    QLabel *label_11;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *horizontalSpacer_4;
    QSpinBox *tcpportSpinBox;
    QSpinBox *udpportSpinBox;
    QSpacerItem *verticalSpacer_2;
    QWidget *soundTab;
    QVBoxLayout *verticalLayout_6;
    QGroupBox *groupBox_5;
    QVBoxLayout *verticalLayout_7;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer_16;
    QRadioButton *wasapiButton;
    QRadioButton *dsoundButton;
    QRadioButton *winmmButton;
    QRadioButton *alsaButton;
    QRadioButton *coreaudioButton;
    QSpacerItem *horizontalSpacer_17;
    QGridLayout *gridLayout_3;
    QLabel *label_12;
    QComboBox *inputdevBox;
    QLabel *inputinfoLabel;
    QLabel *label_14;
    QComboBox *outputdevBox;
    QLabel *outputinfoLabel;
    QToolButton *refreshoutputButton;
    QToolButton *refreshinputButton;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer_5;
    QPushButton *sndtestButton;
    QSpacerItem *horizontalSpacer_6;
    QCheckBox *sndduplexBox;
    QCheckBox *echocancelBox;
    QCheckBox *agcBox;
    QCheckBox *denoisingBox;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_34;
    QSlider *mediavsvoiceSlider;
    QHBoxLayout *horizontalLayout_6;
    QSpacerItem *horizontalSpacer_9;
    QPushButton *snddefaultButton;
    QSpacerItem *verticalSpacer_3;
    QWidget *eventsTab;
    QVBoxLayout *verticalLayout_8;
    QGroupBox *groupBox_6;
    QHBoxLayout *horizontalLayout_14;
    QGridLayout *gridLayout_4;
    QToolButton *rmuserButton;
    QToolButton *newuserButton;
    QLabel *label_38;
    QLabel *label_36;
    QLabel *label_18;
    QLineEdit *questionmodeEdit;
    QLineEdit *usermsgEdit;
    QLabel *label_19;
    QToolButton *hotkeyButton;
    QToolButton *desktopaccessBtn;
    QToolButton *questionmodeBtn;
    QToolButton *videosessionButton;
    QLabel *label_21;
    QLabel *label_23;
    QLineEdit *newuserEdit;
    QLabel *label_37;
    QLineEdit *videosessionEdit;
    QLineEdit *chanmsgEdit;
    QLabel *label_32;
    QLineEdit *voiceactonEdit;
    QLabel *label_320;
    QToolButton *voiceactonButton;
    QLineEdit *voiceactoffEdit;
    QLabel *label_321;
    QToolButton *voiceactoffButton;
    QToolButton *transferdoneButton;
    QToolButton *srvlostButton;
    QLineEdit *rmuserEdit;
    QLineEdit *chansilentEdit;
    QLineEdit *srvlostEdit;
    QToolButton *desktopsessionButton;
    QLabel *label_40;
    QLineEdit *desktopaccessEdit;
    QLabel *label_24;
    QLabel *label_16;
    QLineEdit *fileupdEdit;
    QToolButton *usermsgButton;
    QLabel *label_20;
    QLineEdit *desktopsessionEdit;
    QLabel *label_17;
    QLineEdit *hotkeyEdit;
    QToolButton *chansilentButton;
    QLineEdit *transferdoneEdit;
    QToolButton *fileupdButton;
    QToolButton *chanmsgButton;
    QLabel *label_22;
    QLineEdit *bcastmsgEdit;
    QToolButton *bcastmsgButton;
    QSpacerItem *verticalSpacer_7;
    QWidget *shortcutsTab;
    QVBoxLayout *verticalLayout_10;
    QGroupBox *groupBox_7;
    QVBoxLayout *verticalLayout_11;
    QGridLayout *gridLayout_5;
    QLabel *label_25;
    QLineEdit *voiceactEdit;
    QPushButton *voiceactButton;
    QLabel *label_26;
    QLineEdit *volumeincEdit;
    QPushButton *volumeincButton;
    QLabel *label_27;
    QLineEdit *volumedecEdit;
    QPushButton *volumedecButton;
    QLabel *label_28;
    QLineEdit *muteallEdit;
    QPushButton *muteallButton;
    QLabel *label_29;
    QLineEdit *voicegainincEdit;
    QPushButton *voicegainincButton;
    QLabel *label_30;
    QLineEdit *voicegaindecEdit;
    QPushButton *voicegaindecButton;
    QLabel *label_6;
    QLineEdit *videotxEdit;
    QPushButton *videotxButton;
    QSpacerItem *verticalSpacer_4;
    QWidget *videoTab;
    QVBoxLayout *verticalLayout_22;
    QGroupBox *groupBox_8;
    QFormLayout *formLayout_2;
    QLabel *label_13;
    QComboBox *vidcapdevicesBox;
    QLabel *label_15;
    QHBoxLayout *horizontalLayout_9;
    QComboBox *captureformatsBox;
    QToolButton *vidfmtToolButton;
    QHBoxLayout *horizontalLayout_11;
    QLabel *label_31;
    QRadioButton *vidrgb32RadioButton;
    QRadioButton *vidi420RadioButton;
    QRadioButton *vidyuy2RadioButton;
    QHBoxLayout *horizontalLayout_8;
    QSpacerItem *horizontalSpacer_10;
    QPushButton *vidtestButton;
    QSpacerItem *horizontalSpacer_11;
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
    QHBoxLayout *horizontalLayout_12;
    QSpacerItem *horizontalSpacer_14;
    QPushButton *viddefaultButton;
    QSpacerItem *verticalSpacer_5;

    void setupUi(QDialog *PreferencesDlg)
    {
        if (PreferencesDlg->objectName().isEmpty())
            PreferencesDlg->setObjectName(QString::fromUtf8("PreferencesDlg"));
        PreferencesDlg->resize(758, 621);
        PreferencesDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        PreferencesDlg->setSizeGripEnabled(true);
        PreferencesDlg->setModal(true);
        gridLayout_6 = new QGridLayout(PreferencesDlg);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_7);

        buttonBox = new QDialogButtonBox(PreferencesDlg);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        horizontalLayout_5->addWidget(buttonBox);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_8);


        gridLayout_6->addLayout(horizontalLayout_5, 1, 0, 1, 1);

        tabWidget = new QTabWidget(PreferencesDlg);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setUsesScrollButtons(true);
        generalTab = new QWidget();
        generalTab->setObjectName(QString::fromUtf8("generalTab"));
        verticalLayout_2 = new QVBoxLayout(generalTab);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox = new QGroupBox(generalTab);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_3 = new QVBoxLayout(groupBox);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        nicknameEdit = new QLineEdit(groupBox);
        nicknameEdit->setObjectName(QString::fromUtf8("nicknameEdit"));

        horizontalLayout->addWidget(nicknameEdit);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout_3->addLayout(horizontalLayout);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        horizontalLayout_7->setContentsMargins(0, -1, -1, -1);
        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_7->addWidget(label_7);

        horizontalSpacer_12 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_12);

        maleRadioButton = new QRadioButton(groupBox);
        maleRadioButton->setObjectName(QString::fromUtf8("maleRadioButton"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(maleRadioButton->sizePolicy().hasHeightForWidth());
        maleRadioButton->setSizePolicy(sizePolicy);
        maleRadioButton->setLayoutDirection(Qt::LeftToRight);
        maleRadioButton->setChecked(true);

        horizontalLayout_7->addWidget(maleRadioButton);

        femaleRadioButton = new QRadioButton(groupBox);
        femaleRadioButton->setObjectName(QString::fromUtf8("femaleRadioButton"));

        horizontalLayout_7->addWidget(femaleRadioButton);

        horizontalSpacer_13 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_13);


        verticalLayout_3->addLayout(horizontalLayout_7);

        horizontalLayout_22 = new QHBoxLayout();
        horizontalLayout_22->setObjectName(QString::fromUtf8("horizontalLayout_22"));
        label_35 = new QLabel(groupBox);
        label_35->setObjectName(QString::fromUtf8("label_35"));

        horizontalLayout_22->addWidget(label_35);

        bearwareidEdit = new QLineEdit(groupBox);
        bearwareidEdit->setObjectName(QString::fromUtf8("bearwareidEdit"));
        bearwareidEdit->setReadOnly(true);

        horizontalLayout_22->addWidget(bearwareidEdit);

        setupBearWareLoginButton = new QPushButton(groupBox);
        setupBearWareLoginButton->setObjectName(QString::fromUtf8("setupBearWareLoginButton"));

        horizontalLayout_22->addWidget(setupBearWareLoginButton);

        horizontalSpacer_24 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_22->addItem(horizontalSpacer_24);


        verticalLayout_3->addLayout(horizontalLayout_22);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        awaySpinBox = new QSpinBox(groupBox);
        awaySpinBox->setObjectName(QString::fromUtf8("awaySpinBox"));
        awaySpinBox->setMaximum(3600);

        horizontalLayout_2->addWidget(awaySpinBox);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_2->addWidget(label_3);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout_3->addLayout(horizontalLayout_2);


        verticalLayout_2->addWidget(groupBox);

        groupBox_2 = new QGroupBox(generalTab);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        formLayout = new QFormLayout(groupBox_2);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
        formLayout->setLabelAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        pttChkBox = new QCheckBox(groupBox_2);
        pttChkBox->setObjectName(QString::fromUtf8("pttChkBox"));

        formLayout->setWidget(0, QFormLayout::LabelRole, pttChkBox);

        setupkeysButton = new QPushButton(groupBox_2);
        setupkeysButton->setObjectName(QString::fromUtf8("setupkeysButton"));

        formLayout->setWidget(0, QFormLayout::FieldRole, setupkeysButton);

        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_4);

        keycompEdit = new QLineEdit(groupBox_2);
        keycompEdit->setObjectName(QString::fromUtf8("keycompEdit"));
        keycompEdit->setReadOnly(true);

        formLayout->setWidget(1, QFormLayout::FieldRole, keycompEdit);

        voiceactChkBox = new QCheckBox(groupBox_2);
        voiceactChkBox->setObjectName(QString::fromUtf8("voiceactChkBox"));

        formLayout->setWidget(2, QFormLayout::LabelRole, voiceactChkBox);


        verticalLayout_2->addWidget(groupBox_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        tabWidget->addTab(generalTab, QString());
        displayTab = new QWidget();
        displayTab->setObjectName(QString::fromUtf8("displayTab"));
        verticalLayout_9 = new QVBoxLayout(displayTab);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        groupBox_3 = new QGroupBox(displayTab);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        horizontalLayout_18 = new QHBoxLayout(groupBox_3);
        horizontalLayout_18->setObjectName(QString::fromUtf8("horizontalLayout_18"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName(QString::fromUtf8("horizontalLayout_17"));
        label_5 = new QLabel(groupBox_3);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_17->addWidget(label_5);

        languageBox = new QComboBox(groupBox_3);
        languageBox->setObjectName(QString::fromUtf8("languageBox"));

        horizontalLayout_17->addWidget(languageBox);


        verticalLayout->addLayout(horizontalLayout_17);

        startminimizedChkBox = new QCheckBox(groupBox_3);
        startminimizedChkBox->setObjectName(QString::fromUtf8("startminimizedChkBox"));

        verticalLayout->addWidget(startminimizedChkBox);

        trayChkBox = new QCheckBox(groupBox_3);
        trayChkBox->setObjectName(QString::fromUtf8("trayChkBox"));

        verticalLayout->addWidget(trayChkBox);

        alwaysontopChkBox = new QCheckBox(groupBox_3);
        alwaysontopChkBox->setObjectName(QString::fromUtf8("alwaysontopChkBox"));

        verticalLayout->addWidget(alwaysontopChkBox);

        vumeterChkBox = new QCheckBox(groupBox_3);
        vumeterChkBox->setObjectName(QString::fromUtf8("vumeterChkBox"));

        verticalLayout->addWidget(vumeterChkBox);

        usercountChkBox = new QCheckBox(groupBox_3);
        usercountChkBox->setObjectName(QString::fromUtf8("usercountChkBox"));

        verticalLayout->addWidget(usercountChkBox);

        showusernameChkBox = new QCheckBox(groupBox_3);
        showusernameChkBox->setObjectName(QString::fromUtf8("showusernameChkBox"));

        verticalLayout->addWidget(showusernameChkBox);

        lasttalkChkBox = new QCheckBox(groupBox_3);
        lasttalkChkBox->setObjectName(QString::fromUtf8("lasttalkChkBox"));

        verticalLayout->addWidget(lasttalkChkBox);

        msgpopupChkBox = new QCheckBox(groupBox_3);
        msgpopupChkBox->setObjectName(QString::fromUtf8("msgpopupChkBox"));

        verticalLayout->addWidget(msgpopupChkBox);

        videodlgChkBox = new QCheckBox(groupBox_3);
        videodlgChkBox->setObjectName(QString::fromUtf8("videodlgChkBox"));

        verticalLayout->addWidget(videodlgChkBox);

        returnvidChkBox = new QCheckBox(groupBox_3);
        returnvidChkBox->setObjectName(QString::fromUtf8("returnvidChkBox"));

        verticalLayout->addWidget(returnvidChkBox);

        desktopdlgChkBox = new QCheckBox(groupBox_3);
        desktopdlgChkBox->setObjectName(QString::fromUtf8("desktopdlgChkBox"));

        verticalLayout->addWidget(desktopdlgChkBox);


        horizontalLayout_18->addLayout(verticalLayout);

        horizontalSpacer_20 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_18->addItem(horizontalSpacer_20);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        msgtimestampChkBox = new QCheckBox(groupBox_3);
        msgtimestampChkBox->setObjectName(QString::fromUtf8("msgtimestampChkBox"));

        verticalLayout_4->addWidget(msgtimestampChkBox);

        logstatusbarChkBox = new QCheckBox(groupBox_3);
        logstatusbarChkBox->setObjectName(QString::fromUtf8("logstatusbarChkBox"));

        verticalLayout_4->addWidget(logstatusbarChkBox);

        updatesChkBox = new QCheckBox(groupBox_3);
        updatesChkBox->setObjectName(QString::fromUtf8("updatesChkBox"));

        verticalLayout_4->addWidget(updatesChkBox);

        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setSpacing(6);
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        vidtextChkBox = new QCheckBox(groupBox_3);
        vidtextChkBox->setObjectName(QString::fromUtf8("vidtextChkBox"));

        horizontalLayout_15->addWidget(vidtextChkBox);

        vidtextsrcToolBtn = new QToolButton(groupBox_3);
        vidtextsrcToolBtn->setObjectName(QString::fromUtf8("vidtextsrcToolBtn"));

        horizontalLayout_15->addWidget(vidtextsrcToolBtn);

        horizontalSpacer_19 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_15->addItem(horizontalSpacer_19);


        verticalLayout_4->addLayout(horizontalLayout_15);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        label_39 = new QLabel(groupBox_3);
        label_39->setObjectName(QString::fromUtf8("label_39"));

        horizontalLayout_16->addWidget(label_39);

        maxtextSpinBox = new QSpinBox(groupBox_3);
        maxtextSpinBox->setObjectName(QString::fromUtf8("maxtextSpinBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(maxtextSpinBox->sizePolicy().hasHeightForWidth());
        maxtextSpinBox->setSizePolicy(sizePolicy1);
        maxtextSpinBox->setMinimum(5);
        maxtextSpinBox->setMaximum(512);

        horizontalLayout_16->addWidget(maxtextSpinBox);

        horizontalSpacer_18 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_16->addItem(horizontalSpacer_18);


        verticalLayout_4->addLayout(horizontalLayout_16);

        verticalSpacer_8 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer_8);


        horizontalLayout_18->addLayout(verticalLayout_4);


        verticalLayout_9->addWidget(groupBox_3);

        verticalSpacer_6 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_9->addItem(verticalSpacer_6);

        tabWidget->addTab(displayTab, QString());
        connectionTab = new QWidget();
        connectionTab->setObjectName(QString::fromUtf8("connectionTab"));
        verticalLayout_13 = new QVBoxLayout(connectionTab);
        verticalLayout_13->setObjectName(QString::fromUtf8("verticalLayout_13"));
        groupBox_9 = new QGroupBox(connectionTab);
        groupBox_9->setObjectName(QString::fromUtf8("groupBox_9"));
        verticalLayout_5 = new QVBoxLayout(groupBox_9);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        autoconnectChkBox = new QCheckBox(groupBox_9);
        autoconnectChkBox->setObjectName(QString::fromUtf8("autoconnectChkBox"));

        verticalLayout_5->addWidget(autoconnectChkBox);

        reconnectChkBox = new QCheckBox(groupBox_9);
        reconnectChkBox->setObjectName(QString::fromUtf8("reconnectChkBox"));

        verticalLayout_5->addWidget(reconnectChkBox);

        autojoinChkBox = new QCheckBox(groupBox_9);
        autojoinChkBox->setObjectName(QString::fromUtf8("autojoinChkBox"));

        verticalLayout_5->addWidget(autojoinChkBox);

        maxpayloadChkBox = new QCheckBox(groupBox_9);
        maxpayloadChkBox->setObjectName(QString::fromUtf8("maxpayloadChkBox"));

        verticalLayout_5->addWidget(maxpayloadChkBox);

        winfwChkBox = new QCheckBox(groupBox_9);
        winfwChkBox->setObjectName(QString::fromUtf8("winfwChkBox"));

        verticalLayout_5->addWidget(winfwChkBox);

        groupBox_21 = new QGroupBox(groupBox_9);
        groupBox_21->setObjectName(QString::fromUtf8("groupBox_21"));
        horizontalLayout_13 = new QHBoxLayout(groupBox_21);
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        subusermsgChkBox = new QCheckBox(groupBox_21);
        subusermsgChkBox->setObjectName(QString::fromUtf8("subusermsgChkBox"));

        gridLayout->addWidget(subusermsgChkBox, 0, 0, 1, 1);

        subchanmsgChkBox = new QCheckBox(groupBox_21);
        subchanmsgChkBox->setObjectName(QString::fromUtf8("subchanmsgChkBox"));

        gridLayout->addWidget(subchanmsgChkBox, 0, 1, 1, 1);

        subbcastmsgChkBox = new QCheckBox(groupBox_21);
        subbcastmsgChkBox->setObjectName(QString::fromUtf8("subbcastmsgChkBox"));

        gridLayout->addWidget(subbcastmsgChkBox, 0, 2, 1, 1);

        subvoiceChkBox = new QCheckBox(groupBox_21);
        subvoiceChkBox->setObjectName(QString::fromUtf8("subvoiceChkBox"));

        gridLayout->addWidget(subvoiceChkBox, 1, 0, 1, 1);

        subvidcapChkBox = new QCheckBox(groupBox_21);
        subvidcapChkBox->setObjectName(QString::fromUtf8("subvidcapChkBox"));

        gridLayout->addWidget(subvidcapChkBox, 1, 1, 1, 1);

        subdesktopChkBox = new QCheckBox(groupBox_21);
        subdesktopChkBox->setObjectName(QString::fromUtf8("subdesktopChkBox"));

        gridLayout->addWidget(subdesktopChkBox, 1, 2, 1, 1);

        horizontalLayout_20 = new QHBoxLayout();
        horizontalLayout_20->setSpacing(6);
        horizontalLayout_20->setObjectName(QString::fromUtf8("horizontalLayout_20"));
        label_41 = new QLabel(groupBox_21);
        label_41->setObjectName(QString::fromUtf8("label_41"));

        horizontalLayout_20->addWidget(label_41);

        subdeskinputBtn = new QToolButton(groupBox_21);
        subdeskinputBtn->setObjectName(QString::fromUtf8("subdeskinputBtn"));

        horizontalLayout_20->addWidget(subdeskinputBtn);


        gridLayout->addLayout(horizontalLayout_20, 1, 3, 1, 1);

        submediafileChkBox = new QCheckBox(groupBox_21);
        submediafileChkBox->setObjectName(QString::fromUtf8("submediafileChkBox"));

        gridLayout->addWidget(submediafileChkBox, 0, 3, 1, 1);


        horizontalLayout_13->addLayout(gridLayout);

        horizontalSpacer_15 = new QSpacerItem(265, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_13->addItem(horizontalSpacer_15);


        verticalLayout_5->addWidget(groupBox_21);

        groupBox_4 = new QGroupBox(groupBox_9);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        gridLayout_2 = new QGridLayout(groupBox_4);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_8 = new QLabel(groupBox_4);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout_2->addWidget(label_8, 0, 0, 1, 1);

        label_10 = new QLabel(groupBox_4);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout_2->addWidget(label_10, 0, 2, 1, 1);

        label_9 = new QLabel(groupBox_4);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout_2->addWidget(label_9, 1, 0, 1, 1);

        label_11 = new QLabel(groupBox_4);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout_2->addWidget(label_11, 1, 2, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_3, 0, 3, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_4, 1, 3, 1, 1);

        tcpportSpinBox = new QSpinBox(groupBox_4);
        tcpportSpinBox->setObjectName(QString::fromUtf8("tcpportSpinBox"));
        sizePolicy.setHeightForWidth(tcpportSpinBox->sizePolicy().hasHeightForWidth());
        tcpportSpinBox->setSizePolicy(sizePolicy);
        tcpportSpinBox->setMaximum(65535);

        gridLayout_2->addWidget(tcpportSpinBox, 0, 1, 1, 1);

        udpportSpinBox = new QSpinBox(groupBox_4);
        udpportSpinBox->setObjectName(QString::fromUtf8("udpportSpinBox"));
        udpportSpinBox->setMaximum(65535);

        gridLayout_2->addWidget(udpportSpinBox, 1, 1, 1, 1);


        verticalLayout_5->addWidget(groupBox_4);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer_2);


        verticalLayout_13->addWidget(groupBox_9);

        tabWidget->addTab(connectionTab, QString());
        soundTab = new QWidget();
        soundTab->setObjectName(QString::fromUtf8("soundTab"));
        verticalLayout_6 = new QVBoxLayout(soundTab);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        groupBox_5 = new QGroupBox(soundTab);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        verticalLayout_7 = new QVBoxLayout(groupBox_5);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalSpacer_16 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_16);

        wasapiButton = new QRadioButton(groupBox_5);
        wasapiButton->setObjectName(QString::fromUtf8("wasapiButton"));

        horizontalLayout_3->addWidget(wasapiButton);

        dsoundButton = new QRadioButton(groupBox_5);
        dsoundButton->setObjectName(QString::fromUtf8("dsoundButton"));

        horizontalLayout_3->addWidget(dsoundButton);

        winmmButton = new QRadioButton(groupBox_5);
        winmmButton->setObjectName(QString::fromUtf8("winmmButton"));

        horizontalLayout_3->addWidget(winmmButton);

        alsaButton = new QRadioButton(groupBox_5);
        alsaButton->setObjectName(QString::fromUtf8("alsaButton"));

        horizontalLayout_3->addWidget(alsaButton);

        coreaudioButton = new QRadioButton(groupBox_5);
        coreaudioButton->setObjectName(QString::fromUtf8("coreaudioButton"));

        horizontalLayout_3->addWidget(coreaudioButton);

        horizontalSpacer_17 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_17);


        verticalLayout_7->addLayout(horizontalLayout_3);

        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        label_12 = new QLabel(groupBox_5);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        gridLayout_3->addWidget(label_12, 0, 0, 1, 1);

        inputdevBox = new QComboBox(groupBox_5);
        inputdevBox->setObjectName(QString::fromUtf8("inputdevBox"));

        gridLayout_3->addWidget(inputdevBox, 0, 1, 1, 1);

        inputinfoLabel = new QLabel(groupBox_5);
        inputinfoLabel->setObjectName(QString::fromUtf8("inputinfoLabel"));
        inputinfoLabel->setText(QString::fromUtf8("TextLabel"));
        inputinfoLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        inputinfoLabel->setWordWrap(true);

        gridLayout_3->addWidget(inputinfoLabel, 1, 1, 1, 1);

        label_14 = new QLabel(groupBox_5);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        gridLayout_3->addWidget(label_14, 2, 0, 1, 1);

        outputdevBox = new QComboBox(groupBox_5);
        outputdevBox->setObjectName(QString::fromUtf8("outputdevBox"));

        gridLayout_3->addWidget(outputdevBox, 2, 1, 1, 1);

        outputinfoLabel = new QLabel(groupBox_5);
        outputinfoLabel->setObjectName(QString::fromUtf8("outputinfoLabel"));
        outputinfoLabel->setText(QString::fromUtf8("TextLabel"));
        outputinfoLabel->setWordWrap(true);

        gridLayout_3->addWidget(outputinfoLabel, 3, 1, 1, 1);

        refreshoutputButton = new QToolButton(groupBox_5);
        refreshoutputButton->setObjectName(QString::fromUtf8("refreshoutputButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/images/refresh.png"), QSize(), QIcon::Normal, QIcon::Off);
        refreshoutputButton->setIcon(icon);
        refreshoutputButton->setIconSize(QSize(20, 20));

        gridLayout_3->addWidget(refreshoutputButton, 2, 2, 1, 1);

        refreshinputButton = new QToolButton(groupBox_5);
        refreshinputButton->setObjectName(QString::fromUtf8("refreshinputButton"));
        refreshinputButton->setIcon(icon);
        refreshinputButton->setIconSize(QSize(20, 20));

        gridLayout_3->addWidget(refreshinputButton, 0, 2, 1, 1);

        gridLayout_3->setColumnStretch(1, 1);

        verticalLayout_7->addLayout(gridLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_5);

        sndtestButton = new QPushButton(groupBox_5);
        sndtestButton->setObjectName(QString::fromUtf8("sndtestButton"));
        sndtestButton->setCheckable(true);

        horizontalLayout_4->addWidget(sndtestButton);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_6);


        verticalLayout_7->addLayout(horizontalLayout_4);

        sndduplexBox = new QCheckBox(groupBox_5);
        sndduplexBox->setObjectName(QString::fromUtf8("sndduplexBox"));

        verticalLayout_7->addWidget(sndduplexBox);

        echocancelBox = new QCheckBox(groupBox_5);
        echocancelBox->setObjectName(QString::fromUtf8("echocancelBox"));

        verticalLayout_7->addWidget(echocancelBox);

        agcBox = new QCheckBox(groupBox_5);
        agcBox->setObjectName(QString::fromUtf8("agcBox"));

        verticalLayout_7->addWidget(agcBox);

        denoisingBox = new QCheckBox(groupBox_5);
        denoisingBox->setObjectName(QString::fromUtf8("denoisingBox"));

        verticalLayout_7->addWidget(denoisingBox);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        label_34 = new QLabel(groupBox_5);
        label_34->setObjectName(QString::fromUtf8("label_34"));

        horizontalLayout_10->addWidget(label_34);

        mediavsvoiceSlider = new QSlider(groupBox_5);
        mediavsvoiceSlider->setObjectName(QString::fromUtf8("mediavsvoiceSlider"));
        mediavsvoiceSlider->setMaximum(200);
        mediavsvoiceSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_10->addWidget(mediavsvoiceSlider);


        verticalLayout_7->addLayout(horizontalLayout_10);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_9);

        snddefaultButton = new QPushButton(groupBox_5);
        snddefaultButton->setObjectName(QString::fromUtf8("snddefaultButton"));

        horizontalLayout_6->addWidget(snddefaultButton);


        verticalLayout_7->addLayout(horizontalLayout_6);


        verticalLayout_6->addWidget(groupBox_5);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_6->addItem(verticalSpacer_3);

        tabWidget->addTab(soundTab, QString());
        eventsTab = new QWidget();
        eventsTab->setObjectName(QString::fromUtf8("eventsTab"));
        verticalLayout_8 = new QVBoxLayout(eventsTab);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        groupBox_6 = new QGroupBox(eventsTab);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        horizontalLayout_14 = new QHBoxLayout(groupBox_6);
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        gridLayout_4 = new QGridLayout();
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        rmuserButton = new QToolButton(groupBox_6);
        rmuserButton->setObjectName(QString::fromUtf8("rmuserButton"));
        rmuserButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(rmuserButton, 1, 2, 1, 1);

        newuserButton = new QToolButton(groupBox_6);
        newuserButton->setObjectName(QString::fromUtf8("newuserButton"));
        sizePolicy.setHeightForWidth(newuserButton->sizePolicy().hasHeightForWidth());
        newuserButton->setSizePolicy(sizePolicy);
        newuserButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(newuserButton, 0, 2, 1, 1);

        label_38 = new QLabel(groupBox_6);
        label_38->setObjectName(QString::fromUtf8("label_38"));

        gridLayout_4->addWidget(label_38, 12, 0, 1, 1);

        label_36 = new QLabel(groupBox_6);
        label_36->setObjectName(QString::fromUtf8("label_36"));

        gridLayout_4->addWidget(label_36, 5, 0, 1, 1);

        label_18 = new QLabel(groupBox_6);
        label_18->setObjectName(QString::fromUtf8("label_18"));

        gridLayout_4->addWidget(label_18, 2, 0, 1, 1);

        questionmodeEdit = new QLineEdit(groupBox_6);
        questionmodeEdit->setObjectName(QString::fromUtf8("questionmodeEdit"));

        gridLayout_4->addWidget(questionmodeEdit, 12, 1, 1, 1);

        usermsgEdit = new QLineEdit(groupBox_6);
        usermsgEdit->setObjectName(QString::fromUtf8("usermsgEdit"));

        gridLayout_4->addWidget(usermsgEdit, 3, 1, 1, 1);

        label_19 = new QLabel(groupBox_6);
        label_19->setObjectName(QString::fromUtf8("label_19"));

        gridLayout_4->addWidget(label_19, 3, 0, 1, 1);

        hotkeyButton = new QToolButton(groupBox_6);
        hotkeyButton->setObjectName(QString::fromUtf8("hotkeyButton"));
        hotkeyButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(hotkeyButton, 6, 2, 1, 1);

        desktopaccessBtn = new QToolButton(groupBox_6);
        desktopaccessBtn->setObjectName(QString::fromUtf8("desktopaccessBtn"));
        desktopaccessBtn->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(desktopaccessBtn, 13, 2, 1, 1);

        questionmodeBtn = new QToolButton(groupBox_6);
        questionmodeBtn->setObjectName(QString::fromUtf8("questionmodeBtn"));
        questionmodeBtn->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(questionmodeBtn, 12, 2, 1, 1);

        videosessionButton = new QToolButton(groupBox_6);
        videosessionButton->setObjectName(QString::fromUtf8("videosessionButton"));
        videosessionButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(videosessionButton, 10, 2, 1, 1);

        label_21 = new QLabel(groupBox_6);
        label_21->setObjectName(QString::fromUtf8("label_21"));

        gridLayout_4->addWidget(label_21, 6, 0, 1, 1);

        label_23 = new QLabel(groupBox_6);
        label_23->setObjectName(QString::fromUtf8("label_23"));

        gridLayout_4->addWidget(label_23, 8, 0, 1, 1);

        newuserEdit = new QLineEdit(groupBox_6);
        newuserEdit->setObjectName(QString::fromUtf8("newuserEdit"));

        gridLayout_4->addWidget(newuserEdit, 0, 1, 1, 1);

        label_37 = new QLabel(groupBox_6);
        label_37->setObjectName(QString::fromUtf8("label_37"));

        gridLayout_4->addWidget(label_37, 11, 0, 1, 1);

        videosessionEdit = new QLineEdit(groupBox_6);
        videosessionEdit->setObjectName(QString::fromUtf8("videosessionEdit"));

        gridLayout_4->addWidget(videosessionEdit, 10, 1, 1, 1);

        chanmsgEdit = new QLineEdit(groupBox_6);
        chanmsgEdit->setObjectName(QString::fromUtf8("chanmsgEdit"));

        gridLayout_4->addWidget(chanmsgEdit, 4, 1, 1, 1);

        label_32 = new QLabel(groupBox_6);
        label_32->setObjectName(QString::fromUtf8("label_32"));

        gridLayout_4->addWidget(label_32, 10, 0, 1, 1);

        voiceactonEdit = new QLineEdit(groupBox_6);
        voiceactonEdit->setObjectName(QString::fromUtf8("voiceactonEdit"));

        gridLayout_4->addWidget(voiceactonEdit, 20, 1, 1, 1);

        label_320 = new QLabel(groupBox_6);
        label_320->setObjectName(QString::fromUtf8("label_320"));

        gridLayout_4->addWidget(label_320, 20, 0, 1, 1);

        voiceactonButton = new QToolButton(groupBox_6);
        voiceactonButton->setObjectName(QString::fromUtf8("voiceactonButton"));
        voiceactonButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(voiceactonButton, 20, 2, 1, 1);

        voiceactoffEdit = new QLineEdit(groupBox_6);
        voiceactoffEdit->setObjectName(QString::fromUtf8("voiceactoffEdit"));

        gridLayout_4->addWidget(voiceactoffEdit, 21, 1, 1, 1);

        label_321 = new QLabel(groupBox_6);
        label_321->setObjectName(QString::fromUtf8("label_321"));

        gridLayout_4->addWidget(label_321, 21, 0, 1, 1);

        voiceactoffButton = new QToolButton(groupBox_6);
        voiceactoffButton->setObjectName(QString::fromUtf8("voiceactoffButton"));
        voiceactoffButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(voiceactoffButton, 21, 2, 1, 1);

        transferdoneButton = new QToolButton(groupBox_6);
        transferdoneButton->setObjectName(QString::fromUtf8("transferdoneButton"));
        transferdoneButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(transferdoneButton, 9, 2, 1, 1);

        srvlostButton = new QToolButton(groupBox_6);
        srvlostButton->setObjectName(QString::fromUtf8("srvlostButton"));
        srvlostButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(srvlostButton, 2, 2, 1, 1);

        rmuserEdit = new QLineEdit(groupBox_6);
        rmuserEdit->setObjectName(QString::fromUtf8("rmuserEdit"));

        gridLayout_4->addWidget(rmuserEdit, 1, 1, 1, 1);

        chansilentEdit = new QLineEdit(groupBox_6);
        chansilentEdit->setObjectName(QString::fromUtf8("chansilentEdit"));

        gridLayout_4->addWidget(chansilentEdit, 7, 1, 1, 1);

        srvlostEdit = new QLineEdit(groupBox_6);
        srvlostEdit->setObjectName(QString::fromUtf8("srvlostEdit"));

        gridLayout_4->addWidget(srvlostEdit, 2, 1, 1, 1);

        desktopsessionButton = new QToolButton(groupBox_6);
        desktopsessionButton->setObjectName(QString::fromUtf8("desktopsessionButton"));
        desktopsessionButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(desktopsessionButton, 11, 2, 1, 1);

        label_40 = new QLabel(groupBox_6);
        label_40->setObjectName(QString::fromUtf8("label_40"));

        gridLayout_4->addWidget(label_40, 13, 0, 1, 1);

        desktopaccessEdit = new QLineEdit(groupBox_6);
        desktopaccessEdit->setObjectName(QString::fromUtf8("desktopaccessEdit"));

        gridLayout_4->addWidget(desktopaccessEdit, 13, 1, 1, 1);

        label_24 = new QLabel(groupBox_6);
        label_24->setObjectName(QString::fromUtf8("label_24"));

        gridLayout_4->addWidget(label_24, 9, 0, 1, 1);

        label_16 = new QLabel(groupBox_6);
        label_16->setObjectName(QString::fromUtf8("label_16"));

        gridLayout_4->addWidget(label_16, 0, 0, 1, 1);

        fileupdEdit = new QLineEdit(groupBox_6);
        fileupdEdit->setObjectName(QString::fromUtf8("fileupdEdit"));

        gridLayout_4->addWidget(fileupdEdit, 8, 1, 1, 1);

        usermsgButton = new QToolButton(groupBox_6);
        usermsgButton->setObjectName(QString::fromUtf8("usermsgButton"));
        usermsgButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(usermsgButton, 3, 2, 1, 1);

        label_20 = new QLabel(groupBox_6);
        label_20->setObjectName(QString::fromUtf8("label_20"));

        gridLayout_4->addWidget(label_20, 4, 0, 1, 1);

        desktopsessionEdit = new QLineEdit(groupBox_6);
        desktopsessionEdit->setObjectName(QString::fromUtf8("desktopsessionEdit"));

        gridLayout_4->addWidget(desktopsessionEdit, 11, 1, 1, 1);

        label_17 = new QLabel(groupBox_6);
        label_17->setObjectName(QString::fromUtf8("label_17"));

        gridLayout_4->addWidget(label_17, 1, 0, 1, 1);

        hotkeyEdit = new QLineEdit(groupBox_6);
        hotkeyEdit->setObjectName(QString::fromUtf8("hotkeyEdit"));

        gridLayout_4->addWidget(hotkeyEdit, 6, 1, 1, 1);

        chansilentButton = new QToolButton(groupBox_6);
        chansilentButton->setObjectName(QString::fromUtf8("chansilentButton"));
        chansilentButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(chansilentButton, 7, 2, 1, 1);

        transferdoneEdit = new QLineEdit(groupBox_6);
        transferdoneEdit->setObjectName(QString::fromUtf8("transferdoneEdit"));

        gridLayout_4->addWidget(transferdoneEdit, 9, 1, 1, 1);

        fileupdButton = new QToolButton(groupBox_6);
        fileupdButton->setObjectName(QString::fromUtf8("fileupdButton"));
        fileupdButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(fileupdButton, 8, 2, 1, 1);

        chanmsgButton = new QToolButton(groupBox_6);
        chanmsgButton->setObjectName(QString::fromUtf8("chanmsgButton"));
        chanmsgButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(chanmsgButton, 4, 2, 1, 1);

        label_22 = new QLabel(groupBox_6);
        label_22->setObjectName(QString::fromUtf8("label_22"));

        gridLayout_4->addWidget(label_22, 7, 0, 1, 1);

        bcastmsgEdit = new QLineEdit(groupBox_6);
        bcastmsgEdit->setObjectName(QString::fromUtf8("bcastmsgEdit"));

        gridLayout_4->addWidget(bcastmsgEdit, 5, 1, 1, 1);

        bcastmsgButton = new QToolButton(groupBox_6);
        bcastmsgButton->setObjectName(QString::fromUtf8("bcastmsgButton"));
        bcastmsgButton->setText(QString::fromUtf8("..."));

        gridLayout_4->addWidget(bcastmsgButton, 5, 2, 1, 1);


        horizontalLayout_14->addLayout(gridLayout_4);


        verticalLayout_8->addWidget(groupBox_6);

        verticalSpacer_7 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_8->addItem(verticalSpacer_7);

        tabWidget->addTab(eventsTab, QString());
        shortcutsTab = new QWidget();
        shortcutsTab->setObjectName(QString::fromUtf8("shortcutsTab"));
        verticalLayout_10 = new QVBoxLayout(shortcutsTab);
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        groupBox_7 = new QGroupBox(shortcutsTab);
        groupBox_7->setObjectName(QString::fromUtf8("groupBox_7"));
        verticalLayout_11 = new QVBoxLayout(groupBox_7);
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        gridLayout_5 = new QGridLayout();
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        label_25 = new QLabel(groupBox_7);
        label_25->setObjectName(QString::fromUtf8("label_25"));

        gridLayout_5->addWidget(label_25, 0, 0, 1, 1);

        voiceactEdit = new QLineEdit(groupBox_7);
        voiceactEdit->setObjectName(QString::fromUtf8("voiceactEdit"));
        voiceactEdit->setReadOnly(true);

        gridLayout_5->addWidget(voiceactEdit, 0, 1, 1, 1);

        voiceactButton = new QPushButton(groupBox_7);
        voiceactButton->setObjectName(QString::fromUtf8("voiceactButton"));
        voiceactButton->setCheckable(true);

        gridLayout_5->addWidget(voiceactButton, 0, 2, 1, 1);

        label_26 = new QLabel(groupBox_7);
        label_26->setObjectName(QString::fromUtf8("label_26"));

        gridLayout_5->addWidget(label_26, 1, 0, 1, 1);

        volumeincEdit = new QLineEdit(groupBox_7);
        volumeincEdit->setObjectName(QString::fromUtf8("volumeincEdit"));
        volumeincEdit->setReadOnly(true);

        gridLayout_5->addWidget(volumeincEdit, 1, 1, 1, 1);

        volumeincButton = new QPushButton(groupBox_7);
        volumeincButton->setObjectName(QString::fromUtf8("volumeincButton"));
        volumeincButton->setCheckable(true);

        gridLayout_5->addWidget(volumeincButton, 1, 2, 1, 1);

        label_27 = new QLabel(groupBox_7);
        label_27->setObjectName(QString::fromUtf8("label_27"));

        gridLayout_5->addWidget(label_27, 2, 0, 1, 1);

        volumedecEdit = new QLineEdit(groupBox_7);
        volumedecEdit->setObjectName(QString::fromUtf8("volumedecEdit"));
        volumedecEdit->setReadOnly(true);

        gridLayout_5->addWidget(volumedecEdit, 2, 1, 1, 1);

        volumedecButton = new QPushButton(groupBox_7);
        volumedecButton->setObjectName(QString::fromUtf8("volumedecButton"));
        volumedecButton->setCheckable(true);

        gridLayout_5->addWidget(volumedecButton, 2, 2, 1, 1);

        label_28 = new QLabel(groupBox_7);
        label_28->setObjectName(QString::fromUtf8("label_28"));

        gridLayout_5->addWidget(label_28, 3, 0, 1, 1);

        muteallEdit = new QLineEdit(groupBox_7);
        muteallEdit->setObjectName(QString::fromUtf8("muteallEdit"));
        muteallEdit->setReadOnly(true);

        gridLayout_5->addWidget(muteallEdit, 3, 1, 1, 1);

        muteallButton = new QPushButton(groupBox_7);
        muteallButton->setObjectName(QString::fromUtf8("muteallButton"));
        muteallButton->setCheckable(true);

        gridLayout_5->addWidget(muteallButton, 3, 2, 1, 1);

        label_29 = new QLabel(groupBox_7);
        label_29->setObjectName(QString::fromUtf8("label_29"));

        gridLayout_5->addWidget(label_29, 4, 0, 1, 1);

        voicegainincEdit = new QLineEdit(groupBox_7);
        voicegainincEdit->setObjectName(QString::fromUtf8("voicegainincEdit"));
        voicegainincEdit->setReadOnly(true);

        gridLayout_5->addWidget(voicegainincEdit, 4, 1, 1, 1);

        voicegainincButton = new QPushButton(groupBox_7);
        voicegainincButton->setObjectName(QString::fromUtf8("voicegainincButton"));
        voicegainincButton->setCheckable(true);

        gridLayout_5->addWidget(voicegainincButton, 4, 2, 1, 1);

        label_30 = new QLabel(groupBox_7);
        label_30->setObjectName(QString::fromUtf8("label_30"));

        gridLayout_5->addWidget(label_30, 5, 0, 1, 1);

        voicegaindecEdit = new QLineEdit(groupBox_7);
        voicegaindecEdit->setObjectName(QString::fromUtf8("voicegaindecEdit"));
        voicegaindecEdit->setReadOnly(true);

        gridLayout_5->addWidget(voicegaindecEdit, 5, 1, 1, 1);

        voicegaindecButton = new QPushButton(groupBox_7);
        voicegaindecButton->setObjectName(QString::fromUtf8("voicegaindecButton"));
        voicegaindecButton->setCheckable(true);

        gridLayout_5->addWidget(voicegaindecButton, 5, 2, 1, 1);

        label_6 = new QLabel(groupBox_7);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout_5->addWidget(label_6, 6, 0, 1, 1);

        videotxEdit = new QLineEdit(groupBox_7);
        videotxEdit->setObjectName(QString::fromUtf8("videotxEdit"));

        gridLayout_5->addWidget(videotxEdit, 6, 1, 1, 1);

        videotxButton = new QPushButton(groupBox_7);
        videotxButton->setObjectName(QString::fromUtf8("videotxButton"));
        videotxButton->setCheckable(true);

        gridLayout_5->addWidget(videotxButton, 6, 2, 1, 1);


        verticalLayout_11->addLayout(gridLayout_5);


        verticalLayout_10->addWidget(groupBox_7);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_10->addItem(verticalSpacer_4);

        tabWidget->addTab(shortcutsTab, QString());
        videoTab = new QWidget();
        videoTab->setObjectName(QString::fromUtf8("videoTab"));
        verticalLayout_22 = new QVBoxLayout(videoTab);
        verticalLayout_22->setObjectName(QString::fromUtf8("verticalLayout_22"));
        groupBox_8 = new QGroupBox(videoTab);
        groupBox_8->setObjectName(QString::fromUtf8("groupBox_8"));
        formLayout_2 = new QFormLayout(groupBox_8);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        label_13 = new QLabel(groupBox_8);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label_13);

        vidcapdevicesBox = new QComboBox(groupBox_8);
        vidcapdevicesBox->setObjectName(QString::fromUtf8("vidcapdevicesBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(vidcapdevicesBox->sizePolicy().hasHeightForWidth());
        vidcapdevicesBox->setSizePolicy(sizePolicy2);

        formLayout_2->setWidget(0, QFormLayout::FieldRole, vidcapdevicesBox);

        label_15 = new QLabel(groupBox_8);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        formLayout_2->setWidget(1, QFormLayout::LabelRole, label_15);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        captureformatsBox = new QComboBox(groupBox_8);
        captureformatsBox->setObjectName(QString::fromUtf8("captureformatsBox"));

        horizontalLayout_9->addWidget(captureformatsBox);

        vidfmtToolButton = new QToolButton(groupBox_8);
        vidfmtToolButton->setObjectName(QString::fromUtf8("vidfmtToolButton"));

        horizontalLayout_9->addWidget(vidfmtToolButton);


        formLayout_2->setLayout(1, QFormLayout::FieldRole, horizontalLayout_9);

        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        label_31 = new QLabel(groupBox_8);
        label_31->setObjectName(QString::fromUtf8("label_31"));

        horizontalLayout_11->addWidget(label_31);

        vidrgb32RadioButton = new QRadioButton(groupBox_8);
        vidrgb32RadioButton->setObjectName(QString::fromUtf8("vidrgb32RadioButton"));

        horizontalLayout_11->addWidget(vidrgb32RadioButton);

        vidi420RadioButton = new QRadioButton(groupBox_8);
        vidi420RadioButton->setObjectName(QString::fromUtf8("vidi420RadioButton"));

        horizontalLayout_11->addWidget(vidi420RadioButton);

        vidyuy2RadioButton = new QRadioButton(groupBox_8);
        vidyuy2RadioButton->setObjectName(QString::fromUtf8("vidyuy2RadioButton"));

        horizontalLayout_11->addWidget(vidyuy2RadioButton);


        formLayout_2->setLayout(2, QFormLayout::SpanningRole, horizontalLayout_11);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_10);

        vidtestButton = new QPushButton(groupBox_8);
        vidtestButton->setObjectName(QString::fromUtf8("vidtestButton"));

        horizontalLayout_8->addWidget(vidtestButton);

        horizontalSpacer_11 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_11);


        formLayout_2->setLayout(3, QFormLayout::SpanningRole, horizontalLayout_8);

        vidgroupBox = new QGroupBox(groupBox_8);
        vidgroupBox->setObjectName(QString::fromUtf8("vidgroupBox"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(vidgroupBox->sizePolicy().hasHeightForWidth());
        vidgroupBox->setSizePolicy(sizePolicy3);
        verticalLayout_14 = new QVBoxLayout(vidgroupBox);
        verticalLayout_14->setObjectName(QString::fromUtf8("verticalLayout_14"));
        horizontalLayout_19 = new QHBoxLayout();
        horizontalLayout_19->setObjectName(QString::fromUtf8("horizontalLayout_19"));
        label_33 = new QLabel(vidgroupBox);
        label_33->setObjectName(QString::fromUtf8("label_33"));

        horizontalLayout_19->addWidget(label_33);

        vidcodecBox = new QComboBox(vidgroupBox);
        vidcodecBox->setObjectName(QString::fromUtf8("vidcodecBox"));
        QSizePolicy sizePolicy4(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(vidcodecBox->sizePolicy().hasHeightForWidth());
        vidcodecBox->setSizePolicy(sizePolicy4);

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
        sizePolicy4.setHeightForWidth(vp8bitrateSpinBox->sizePolicy().hasHeightForWidth());
        vp8bitrateSpinBox->setSizePolicy(sizePolicy4);
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

        verticalSpacer_9 = new QSpacerItem(20, 73, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_15->addItem(verticalSpacer_9);

        vidcodecStackedWidget->addWidget(vp8_page);

        verticalLayout_14->addWidget(vidcodecStackedWidget);


        formLayout_2->setWidget(4, QFormLayout::SpanningRole, vidgroupBox);

        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        horizontalSpacer_14 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_12->addItem(horizontalSpacer_14);

        viddefaultButton = new QPushButton(groupBox_8);
        viddefaultButton->setObjectName(QString::fromUtf8("viddefaultButton"));

        horizontalLayout_12->addWidget(viddefaultButton);


        formLayout_2->setLayout(5, QFormLayout::SpanningRole, horizontalLayout_12);

        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        formLayout_2->setItem(6, QFormLayout::FieldRole, verticalSpacer_5);


        verticalLayout_22->addWidget(groupBox_8);

        tabWidget->addTab(videoTab, QString());

        gridLayout_6->addWidget(tabWidget, 0, 0, 1, 1);

#if QT_CONFIG(shortcut)
        label->setBuddy(nicknameEdit);
        label_7->setBuddy(maleRadioButton);
        label_35->setBuddy(bearwareidEdit);
        label_2->setBuddy(awaySpinBox);
        label_4->setBuddy(keycompEdit);
        label_5->setBuddy(languageBox);
        label_39->setBuddy(maxtextSpinBox);
        label_41->setBuddy(subdeskinputBtn);
        label_8->setBuddy(tcpportSpinBox);
        label_9->setBuddy(udpportSpinBox);
        label_12->setBuddy(inputdevBox);
        label_14->setBuddy(outputdevBox);
        label_38->setBuddy(questionmodeEdit);
        label_18->setBuddy(srvlostEdit);
        label_19->setBuddy(usermsgEdit);
        label_21->setBuddy(hotkeyEdit);
        label_23->setBuddy(fileupdEdit);
        label_37->setBuddy(desktopsessionEdit);
        label_32->setBuddy(videosessionEdit);
        label_320->setBuddy(voiceactonEdit);
        label_321->setBuddy(voiceactoffEdit);
        label_40->setBuddy(desktopaccessEdit);
        label_24->setBuddy(transferdoneEdit);
        label_16->setBuddy(newuserEdit);
        label_20->setBuddy(chanmsgEdit);
        label_17->setBuddy(rmuserEdit);
        label_22->setBuddy(chansilentEdit);
        label_25->setBuddy(voiceactEdit);
        label_26->setBuddy(volumeincEdit);
        label_27->setBuddy(volumedecEdit);
        label_28->setBuddy(muteallEdit);
        label_29->setBuddy(voicegainincEdit);
        label_30->setBuddy(voicegaindecEdit);
        label_6->setBuddy(videotxEdit);
        label_13->setBuddy(vidcapdevicesBox);
        label_15->setBuddy(captureformatsBox);
        label_33->setBuddy(vidcodecBox);
        label_42->setBuddy(vp8bitrateSpinBox);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(tabWidget, nicknameEdit);
        QWidget::setTabOrder(nicknameEdit, maleRadioButton);
        QWidget::setTabOrder(maleRadioButton, femaleRadioButton);
        QWidget::setTabOrder(femaleRadioButton, bearwareidEdit);
        QWidget::setTabOrder(bearwareidEdit, setupBearWareLoginButton);
        QWidget::setTabOrder(setupBearWareLoginButton, awaySpinBox);
        QWidget::setTabOrder(awaySpinBox, pttChkBox);
        QWidget::setTabOrder(pttChkBox, setupkeysButton);
        QWidget::setTabOrder(setupkeysButton, voiceactChkBox);
        QWidget::setTabOrder(voiceactChkBox, keycompEdit);
        QWidget::setTabOrder(keycompEdit, languageBox);
        QWidget::setTabOrder(languageBox, startminimizedChkBox);
        QWidget::setTabOrder(startminimizedChkBox, trayChkBox);
        QWidget::setTabOrder(trayChkBox, alwaysontopChkBox);
        QWidget::setTabOrder(alwaysontopChkBox, vumeterChkBox);
        QWidget::setTabOrder(vumeterChkBox, usercountChkBox);
        QWidget::setTabOrder(usercountChkBox, showusernameChkBox);
        QWidget::setTabOrder(showusernameChkBox, lasttalkChkBox);
        QWidget::setTabOrder(lasttalkChkBox, msgpopupChkBox);
        QWidget::setTabOrder(msgpopupChkBox, videodlgChkBox);
        QWidget::setTabOrder(videodlgChkBox, returnvidChkBox);
        QWidget::setTabOrder(returnvidChkBox, desktopdlgChkBox);
        QWidget::setTabOrder(desktopdlgChkBox, msgtimestampChkBox);
        QWidget::setTabOrder(msgtimestampChkBox, logstatusbarChkBox);
        QWidget::setTabOrder(logstatusbarChkBox, updatesChkBox);
        QWidget::setTabOrder(updatesChkBox, vidtextChkBox);
        QWidget::setTabOrder(vidtextChkBox, vidtextsrcToolBtn);
        QWidget::setTabOrder(vidtextsrcToolBtn, maxtextSpinBox);
        QWidget::setTabOrder(maxtextSpinBox, autoconnectChkBox);
        QWidget::setTabOrder(autoconnectChkBox, reconnectChkBox);
        QWidget::setTabOrder(reconnectChkBox, autojoinChkBox);
        QWidget::setTabOrder(autojoinChkBox, maxpayloadChkBox);
        QWidget::setTabOrder(maxpayloadChkBox, winfwChkBox);
        QWidget::setTabOrder(winfwChkBox, subusermsgChkBox);
        QWidget::setTabOrder(subusermsgChkBox, subchanmsgChkBox);
        QWidget::setTabOrder(subchanmsgChkBox, subbcastmsgChkBox);
        QWidget::setTabOrder(subbcastmsgChkBox, submediafileChkBox);
        QWidget::setTabOrder(submediafileChkBox, subvoiceChkBox);
        QWidget::setTabOrder(subvoiceChkBox, subvidcapChkBox);
        QWidget::setTabOrder(subvidcapChkBox, subdesktopChkBox);
        QWidget::setTabOrder(subdesktopChkBox, subdeskinputBtn);
        QWidget::setTabOrder(subdeskinputBtn, tcpportSpinBox);
        QWidget::setTabOrder(tcpportSpinBox, udpportSpinBox);
        QWidget::setTabOrder(udpportSpinBox, wasapiButton);
        QWidget::setTabOrder(wasapiButton, dsoundButton);
        QWidget::setTabOrder(dsoundButton, winmmButton);
        QWidget::setTabOrder(winmmButton, alsaButton);
        QWidget::setTabOrder(alsaButton, coreaudioButton);
        QWidget::setTabOrder(coreaudioButton, inputdevBox);
        QWidget::setTabOrder(inputdevBox, refreshinputButton);
        QWidget::setTabOrder(refreshinputButton, outputdevBox);
        QWidget::setTabOrder(outputdevBox, refreshoutputButton);
        QWidget::setTabOrder(refreshoutputButton, sndtestButton);
        QWidget::setTabOrder(sndtestButton, sndduplexBox);
        QWidget::setTabOrder(sndduplexBox, echocancelBox);
        QWidget::setTabOrder(echocancelBox, agcBox);
        QWidget::setTabOrder(agcBox, denoisingBox);
        QWidget::setTabOrder(denoisingBox, mediavsvoiceSlider);
        QWidget::setTabOrder(mediavsvoiceSlider, snddefaultButton);
        QWidget::setTabOrder(snddefaultButton, newuserEdit);
        QWidget::setTabOrder(newuserEdit, newuserButton);
        QWidget::setTabOrder(newuserButton, rmuserEdit);
        QWidget::setTabOrder(rmuserEdit, rmuserButton);
        QWidget::setTabOrder(rmuserButton, srvlostEdit);
        QWidget::setTabOrder(srvlostEdit, srvlostButton);
        QWidget::setTabOrder(srvlostButton, usermsgEdit);
        QWidget::setTabOrder(usermsgEdit, usermsgButton);
        QWidget::setTabOrder(usermsgButton, chanmsgEdit);
        QWidget::setTabOrder(chanmsgEdit, chanmsgButton);
        QWidget::setTabOrder(chanmsgButton, bcastmsgEdit);
        QWidget::setTabOrder(bcastmsgEdit, bcastmsgButton);
        QWidget::setTabOrder(bcastmsgButton, hotkeyEdit);
        QWidget::setTabOrder(hotkeyEdit, hotkeyButton);
        QWidget::setTabOrder(hotkeyButton, chansilentEdit);
        QWidget::setTabOrder(chansilentEdit, chansilentButton);
        QWidget::setTabOrder(chansilentButton, fileupdEdit);
        QWidget::setTabOrder(fileupdEdit, fileupdButton);
        QWidget::setTabOrder(fileupdButton, transferdoneEdit);
        QWidget::setTabOrder(transferdoneEdit, transferdoneButton);
        QWidget::setTabOrder(transferdoneButton, videosessionEdit);
        QWidget::setTabOrder(videosessionEdit, videosessionButton);
        QWidget::setTabOrder(videosessionButton, desktopsessionEdit);
        QWidget::setTabOrder(desktopsessionEdit, desktopsessionButton);
        QWidget::setTabOrder(desktopsessionButton, questionmodeEdit);
        QWidget::setTabOrder(questionmodeEdit, questionmodeBtn);
        QWidget::setTabOrder(questionmodeBtn, desktopaccessEdit);
        QWidget::setTabOrder(desktopaccessEdit, desktopaccessBtn);
        QWidget::setTabOrder(desktopaccessBtn, voiceactonEdit);
        QWidget::setTabOrder(voiceactonEdit, voiceactonButton);
        QWidget::setTabOrder(voiceactonButton, voiceactoffEdit);
        QWidget::setTabOrder(voiceactoffEdit, voiceactoffButton);
        QWidget::setTabOrder(voiceactoffButton, voiceactEdit);
        QWidget::setTabOrder(voiceactEdit, voiceactButton);
        QWidget::setTabOrder(voiceactButton, volumeincEdit);
        QWidget::setTabOrder(volumeincEdit, volumeincButton);
        QWidget::setTabOrder(volumeincButton, volumedecEdit);
        QWidget::setTabOrder(volumedecEdit, volumedecButton);
        QWidget::setTabOrder(volumedecButton, muteallEdit);
        QWidget::setTabOrder(muteallEdit, muteallButton);
        QWidget::setTabOrder(muteallButton, voicegainincEdit);
        QWidget::setTabOrder(voicegainincEdit, voicegainincButton);
        QWidget::setTabOrder(voicegainincButton, voicegaindecEdit);
        QWidget::setTabOrder(voicegaindecEdit, voicegaindecButton);
        QWidget::setTabOrder(voicegaindecButton, videotxEdit);
        QWidget::setTabOrder(videotxEdit, videotxButton);
        QWidget::setTabOrder(videotxButton, vidcapdevicesBox);
        QWidget::setTabOrder(vidcapdevicesBox, captureformatsBox);
        QWidget::setTabOrder(captureformatsBox, vidfmtToolButton);
        QWidget::setTabOrder(vidfmtToolButton, vidrgb32RadioButton);
        QWidget::setTabOrder(vidrgb32RadioButton, vidi420RadioButton);
        QWidget::setTabOrder(vidi420RadioButton, vidyuy2RadioButton);
        QWidget::setTabOrder(vidyuy2RadioButton, vidtestButton);
        QWidget::setTabOrder(vidtestButton, vidcodecBox);
        QWidget::setTabOrder(vidcodecBox, vp8bitrateSpinBox);
        QWidget::setTabOrder(vp8bitrateSpinBox, viddefaultButton);

        retranslateUi(PreferencesDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), PreferencesDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), PreferencesDlg, SLOT(reject()));
        QObject::connect(sndtestButton, SIGNAL(toggled(bool)), refreshinputButton, SLOT(setDisabled(bool)));
        QObject::connect(sndtestButton, SIGNAL(toggled(bool)), refreshoutputButton, SLOT(setDisabled(bool)));

        tabWidget->setCurrentIndex(0);
        vidcodecStackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(PreferencesDlg);
    } // setupUi

    void retranslateUi(QDialog *PreferencesDlg)
    {
        PreferencesDlg->setWindowTitle(QCoreApplication::translate("PreferencesDlg", "Preferences", nullptr));
        groupBox->setTitle(QCoreApplication::translate("PreferencesDlg", "User Settings", nullptr));
        label->setText(QCoreApplication::translate("PreferencesDlg", "Nickname", nullptr));
        label_7->setText(QCoreApplication::translate("PreferencesDlg", "Gender", nullptr));
        maleRadioButton->setText(QCoreApplication::translate("PreferencesDlg", "Male", nullptr));
        femaleRadioButton->setText(QCoreApplication::translate("PreferencesDlg", "Female", nullptr));
        label_35->setText(QCoreApplication::translate("PreferencesDlg", "BearWare.dk Web Login ID", nullptr));
        setupBearWareLoginButton->setText(QCoreApplication::translate("PreferencesDlg", "&Activate", nullptr));
        label_2->setText(QCoreApplication::translate("PreferencesDlg", "Set away status after", nullptr));
        label_3->setText(QCoreApplication::translate("PreferencesDlg", "seconds of inactivity (0 means disabled)", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("PreferencesDlg", "Voice Transmission Mode", nullptr));
        pttChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Push To Talk", nullptr));
        setupkeysButton->setText(QCoreApplication::translate("PreferencesDlg", "&Setup Keys", nullptr));
        label_4->setText(QCoreApplication::translate("PreferencesDlg", "Key Combination", nullptr));
        voiceactChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Voice activated", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(generalTab), QCoreApplication::translate("PreferencesDlg", "General", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("PreferencesDlg", "User Interface Settings", nullptr));
        label_5->setText(QCoreApplication::translate("PreferencesDlg", "User interface language", nullptr));
        startminimizedChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Start minimized", nullptr));
        trayChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Minimize to tray icon", nullptr));
        alwaysontopChkBox->setText(QCoreApplication::translate("PreferencesDlg", "&Always on top", nullptr));
        vumeterChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Enable VU-meter updates", nullptr));
        usercountChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Show number of users in channel", nullptr));
        showusernameChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Show username instead of nickname", nullptr));
        lasttalkChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Show last to talk in yellow", nullptr));
        msgpopupChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Popup dialog when receiving text message", nullptr));
        videodlgChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Start video in popup dialog", nullptr));
        returnvidChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Closed video dialog should return to video-tab", nullptr));
        desktopdlgChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Start desktops in popup dialog", nullptr));
        msgtimestampChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Timestamp text messages", nullptr));
        logstatusbarChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Show statusbar events in chat-window", nullptr));
        updatesChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Check for program updates at startup", nullptr));
        vidtextChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Show source in corner of video window", nullptr));
        vidtextsrcToolBtn->setText(QCoreApplication::translate("PreferencesDlg", "...", nullptr));
        label_39->setText(QCoreApplication::translate("PreferencesDlg", "Maximum text length in channel list", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(displayTab), QCoreApplication::translate("PreferencesDlg", "Display", nullptr));
        groupBox_9->setTitle(QCoreApplication::translate("PreferencesDlg", "Client Connection", nullptr));
        autoconnectChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Connect to latest host on startup", nullptr));
        reconnectChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Reconnect on connection dropped", nullptr));
        autojoinChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Join root channel upon connection", nullptr));
        maxpayloadChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Query server's maximum payload upon connection", nullptr));
        winfwChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Add application to Windows Firewall exceptions list", nullptr));
#if QT_CONFIG(accessibility)
        groupBox_21->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Default Subscriptions upon Connection", nullptr));
#endif // QT_CONFIG(accessibility)
        groupBox_21->setTitle(QCoreApplication::translate("PreferencesDlg", "Default Subscriptions upon Connection", nullptr));
        subusermsgChkBox->setText(QCoreApplication::translate("PreferencesDlg", "User Messages", nullptr));
        subchanmsgChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Channel Messages", nullptr));
        subbcastmsgChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Broadcast Messages", nullptr));
        subvoiceChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Voice", nullptr));
        subvidcapChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Video Capture", nullptr));
        subdesktopChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Desktop", nullptr));
        label_41->setText(QCoreApplication::translate("PreferencesDlg", "Desktop Access", nullptr));
        subdeskinputBtn->setText(QCoreApplication::translate("PreferencesDlg", "...", nullptr));
        submediafileChkBox->setText(QCoreApplication::translate("PreferencesDlg", "Media Files", nullptr));
#if QT_CONFIG(accessibility)
        groupBox_4->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Local Socket Settings", nullptr));
#endif // QT_CONFIG(accessibility)
        groupBox_4->setTitle(QCoreApplication::translate("PreferencesDlg", "Local Socket Settings", nullptr));
        label_8->setText(QCoreApplication::translate("PreferencesDlg", "TCP port", nullptr));
        label_10->setText(QCoreApplication::translate("PreferencesDlg", "Default: 0", nullptr));
        label_9->setText(QCoreApplication::translate("PreferencesDlg", "UDP port", nullptr));
        label_11->setText(QCoreApplication::translate("PreferencesDlg", "Default: 0", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(connectionTab), QCoreApplication::translate("PreferencesDlg", "Connection", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("PreferencesDlg", "Sound System Settings", nullptr));
        wasapiButton->setText(QCoreApplication::translate("PreferencesDlg", "Windows Audio Session", nullptr));
        dsoundButton->setText(QCoreApplication::translate("PreferencesDlg", "DirectSound", nullptr));
        winmmButton->setText(QCoreApplication::translate("PreferencesDlg", "Windows Standard", nullptr));
        alsaButton->setText(QCoreApplication::translate("PreferencesDlg", "ALSA", nullptr));
        coreaudioButton->setText(QCoreApplication::translate("PreferencesDlg", "CoreAudio", nullptr));
        label_12->setText(QCoreApplication::translate("PreferencesDlg", "Input device", nullptr));
        label_14->setText(QCoreApplication::translate("PreferencesDlg", "Output device", nullptr));
        refreshoutputButton->setText(QCoreApplication::translate("PreferencesDlg", "Refresh devices", nullptr));
        refreshinputButton->setText(QCoreApplication::translate("PreferencesDlg", "Refresh devices", nullptr));
        sndtestButton->setText(QCoreApplication::translate("PreferencesDlg", "&Test Selected", nullptr));
        sndduplexBox->setText(QCoreApplication::translate("PreferencesDlg", "Enable duplex mode (required for echo cancellation)", nullptr));
        echocancelBox->setText(QCoreApplication::translate("PreferencesDlg", "Enable echo cancellation (remove echo from speakers)", nullptr));
        agcBox->setText(QCoreApplication::translate("PreferencesDlg", "Enable automatic gain control (microphone level adjusted automatically)", nullptr));
        denoisingBox->setText(QCoreApplication::translate("PreferencesDlg", "Enable denoising (suppress noise from microphone)", nullptr));
        label_34->setText(QCoreApplication::translate("PreferencesDlg", "Media file vs. voice volume", nullptr));
        snddefaultButton->setText(QCoreApplication::translate("PreferencesDlg", "&Default", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(soundTab), QCoreApplication::translate("PreferencesDlg", "Sound System", nullptr));
        groupBox_6->setTitle(QCoreApplication::translate("PreferencesDlg", "Sound Events", nullptr));
#if QT_CONFIG(accessibility)
        rmuserButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        newuserButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
        label_38->setText(QCoreApplication::translate("PreferencesDlg", "User entered question-mode", nullptr));
        label_36->setText(QCoreApplication::translate("PreferencesDlg", "New broadcast message", nullptr));
        label_18->setText(QCoreApplication::translate("PreferencesDlg", "Server lost", nullptr));
        label_19->setText(QCoreApplication::translate("PreferencesDlg", "New user message", nullptr));
#if QT_CONFIG(accessibility)
        hotkeyButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        desktopaccessBtn->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        questionmodeBtn->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        videosessionButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
        label_21->setText(QCoreApplication::translate("PreferencesDlg", "Hotkey pressed", nullptr));
        label_23->setText(QCoreApplication::translate("PreferencesDlg", "Files updated", nullptr));
        label_37->setText(QCoreApplication::translate("PreferencesDlg", "New desktop session", nullptr));
        label_32->setText(QCoreApplication::translate("PreferencesDlg", "New video session", nullptr));
        label_320->setText(QCoreApplication::translate("PreferencesDlg", "Voice activation enabled", nullptr));
#if QT_CONFIG(accessibility)
        voiceactonButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
        label_321->setText(QCoreApplication::translate("PreferencesDlg", "Voice activation disabled", nullptr));
#if QT_CONFIG(accessibility)
        voiceactoffButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        transferdoneButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        srvlostButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        desktopsessionButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
        label_40->setText(QCoreApplication::translate("PreferencesDlg", "Desktop access request", nullptr));
        label_24->setText(QCoreApplication::translate("PreferencesDlg", "File transfer complete", nullptr));
        label_16->setText(QCoreApplication::translate("PreferencesDlg", "New user", nullptr));
#if QT_CONFIG(accessibility)
        usermsgButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
        label_20->setText(QCoreApplication::translate("PreferencesDlg", "New channel message", nullptr));
        label_17->setText(QCoreApplication::translate("PreferencesDlg", "User removed", nullptr));
#if QT_CONFIG(accessibility)
        chansilentButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        fileupdButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        chanmsgButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
        label_22->setText(QCoreApplication::translate("PreferencesDlg", "Channel silent", nullptr));
#if QT_CONFIG(accessibility)
        bcastmsgButton->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Browse", nullptr));
#endif // QT_CONFIG(accessibility)
        tabWidget->setTabText(tabWidget->indexOf(eventsTab), QCoreApplication::translate("PreferencesDlg", "Sound Events", nullptr));
        groupBox_7->setTitle(QCoreApplication::translate("PreferencesDlg", "Keyboard Shortcuts", nullptr));
        label_25->setText(QCoreApplication::translate("PreferencesDlg", "Enable/disable voice activation", nullptr));
        voiceactButton->setText(QCoreApplication::translate("PreferencesDlg", "&Setup Keys", nullptr));
        label_26->setText(QCoreApplication::translate("PreferencesDlg", "Increase volume", nullptr));
        volumeincButton->setText(QCoreApplication::translate("PreferencesDlg", "&Setup Keys", nullptr));
        label_27->setText(QCoreApplication::translate("PreferencesDlg", "Lower volume", nullptr));
        volumedecButton->setText(QCoreApplication::translate("PreferencesDlg", "&Setup Keys", nullptr));
        label_28->setText(QCoreApplication::translate("PreferencesDlg", "Enable/disable mute all", nullptr));
        muteallButton->setText(QCoreApplication::translate("PreferencesDlg", "&Setup Keys", nullptr));
        label_29->setText(QCoreApplication::translate("PreferencesDlg", "Increase microphone gain", nullptr));
        voicegainincButton->setText(QCoreApplication::translate("PreferencesDlg", "&Setup Keys", nullptr));
        label_30->setText(QCoreApplication::translate("PreferencesDlg", "Lower microphone gain", nullptr));
        voicegaindecButton->setText(QCoreApplication::translate("PreferencesDlg", "&Setup Keys", nullptr));
        label_6->setText(QCoreApplication::translate("PreferencesDlg", "Enable/disable video transmission", nullptr));
        videotxButton->setText(QCoreApplication::translate("PreferencesDlg", "Setup Keys", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(shortcutsTab), QCoreApplication::translate("PreferencesDlg", "Shortcuts", nullptr));
        groupBox_8->setTitle(QCoreApplication::translate("PreferencesDlg", "Video Capture Settings", nullptr));
        label_13->setText(QCoreApplication::translate("PreferencesDlg", "Video Capture Device", nullptr));
        label_15->setText(QCoreApplication::translate("PreferencesDlg", "Video Resolution", nullptr));
        vidfmtToolButton->setText(QCoreApplication::translate("PreferencesDlg", "...", nullptr));
        label_31->setText(QCoreApplication::translate("PreferencesDlg", "Image Format", nullptr));
        vidrgb32RadioButton->setText(QCoreApplication::translate("PreferencesDlg", "RGB32", nullptr));
        vidi420RadioButton->setText(QCoreApplication::translate("PreferencesDlg", "I420", nullptr));
        vidyuy2RadioButton->setText(QCoreApplication::translate("PreferencesDlg", "YUY2", nullptr));
        vidtestButton->setText(QCoreApplication::translate("PreferencesDlg", "Test Selected", nullptr));
#if QT_CONFIG(accessibility)
        vidgroupBox->setAccessibleName(QCoreApplication::translate("PreferencesDlg", "Video Codec Settings", nullptr));
#endif // QT_CONFIG(accessibility)
        vidgroupBox->setTitle(QCoreApplication::translate("PreferencesDlg", "Video Codec Settings", nullptr));
        label_33->setText(QCoreApplication::translate("PreferencesDlg", "Codec", nullptr));
        label_42->setText(QCoreApplication::translate("PreferencesDlg", "Bitrate", nullptr));
        viddefaultButton->setText(QCoreApplication::translate("PreferencesDlg", "&Default", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(videoTab), QCoreApplication::translate("PreferencesDlg", "Video Capture", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PreferencesDlg: public Ui_PreferencesDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PREFERENCES_H
