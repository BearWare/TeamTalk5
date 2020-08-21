/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "channelstree.h"
#include "chatlineedit.h"
#include "chattextedit.h"
#include "desktopgridwidget.h"
#include "filesview.h"
#include "videogridwidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionServerList;
    QAction *actionConnect;
    QAction *actionPreferences;
    QAction *actionExit;
    QAction *actionEnablePushToTalk;
    QAction *actionAbout;
    QAction *actionManual;
    QAction *actionServerProperties;
    QAction *actionCreateChannel;
    QAction *actionUpdateChannel;
    QAction *actionDeleteChannel;
    QAction *actionJoinChannel;
    QAction *actionEnableVoiceActivation;
    QAction *actionChangeNickname;
    QAction *actionChangeStatus;
    QAction *actionViewUserInformation;
    QAction *actionMuteAll;
    QAction *actionUploadFile;
    QAction *actionDownloadFile;
    QAction *actionDeleteFile;
    QAction *actionBannedUsers;
    QAction *actionUserAccounts;
    QAction *actionSaveConfiguration;
    QAction *actionViewChannelInfo;
    QAction *actionMessages;
    QAction *actionKickBan;
    QAction *actionOp;
    QAction *actionVolume;
    QAction *actionVoice;
    QAction *actionUserMessages;
    QAction *actionChannelMessages;
    QAction *actionVideo;
    QAction *actionBroadcastMessages;
    QAction *actionEnableVideoTransmission;
    QAction *actionStreamAudioFileToChannel;
    QAction *actionStreamAudioFileToUser;
    QAction *actionMediaStorage;
    QAction *actionInterceptUserMessages;
    QAction *actionInterceptChannelMessages;
    QAction *actionInterceptVoice;
    QAction *actionInterceptVideo;
    QAction *actionBroadcastMessage;
    QAction *actionIncreaseVoiceVolume;
    QAction *actionLowerVoiceVolume;
    QAction *actionStoreForMove;
    QAction *actionMoveUser;
    QAction *actionVisitBearWare;
    QAction *actionVisitChangeLog;
    QAction *actionAllowVoiceTransmission;
    QAction *actionAllowVideoTransmission;
    QAction *actionServerStatistics;
    QAction *actionNewClient;
    QAction *actionOnlineUsers;
    QAction *actionEnableQuestionMode;
    QAction *actionEnableDesktopSharing;
    QAction *actionAllowDesktopTransmission;
    QAction *actionDesktop;
    QAction *actionInterceptDesktop;
    QAction *actionStreamMediaFileToChannel;
    QAction *actionDesktopInput;
    QAction *actionDesktopAccessAllow;
    QAction *actionDesktopAccessDeny;
    QAction *actionMediaFile;
    QAction *actionInterceptMediaFile;
    QAction *actionAllowMediaFileTransmission;
    QAction *actionMuteVoice;
    QAction *actionMuteMediaFile;
    QAction *actionKickFromChannel;
    QAction *actionKickFromServer;
    QAction *actionIncreaseMediaFileVolume;
    QAction *actionLowerMediaFileVolume;
    QAction *actionResetPreferencesToDefault;
    QAction *actionKickAndBanFromChannel;
    QAction *actionBannedUsersInChannel;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_2;
    QSplitter *splitter;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    ChannelsTree *channelsWidget;
    QGridLayout *gridLayout;
    QLabel *label;
    QSlider *volumeSlider;
    QLabel *label_3;
    QProgressBar *voiceactBar;
    QLabel *label_2;
    QSlider *micSlider;
    QLabel *label_4;
    QSlider *voiceactSlider;
    QTabWidget *tabWidget;
    QWidget *chatTab;
    QVBoxLayout *verticalLayout_3;
    ChatTextEdit *chatEdit;
    QHBoxLayout *horizontalLayout_2;
    ChatLineEdit *msgEdit;
    QToolButton *sendButton;
    QWidget *videoTab;
    QVBoxLayout *verticalLayout_10;
    QSplitter *videosplitter;
    QWidget *layoutWidget_2;
    QHBoxLayout *horizontalLayout_3;
    VideoGridWidget *videogridWidget;
    QVBoxLayout *verticalLayout_5;
    QSpacerItem *verticalSpacer_6;
    QToolButton *detachVideoButton;
    QSpacerItem *verticalSpacer_8;
    QToolButton *addVideoButton;
    QSpacerItem *verticalSpacer_9;
    QToolButton *removeVideoButton;
    QSpacerItem *verticalSpacer_10;
    QToolButton *initVideoButton;
    QSpacerItem *verticalSpacer_7;
    QStackedWidget *videostackedWidget;
    QWidget *page_3;
    QVBoxLayout *verticalLayout_9;
    ChatTextEdit *videochatEdit;
    QHBoxLayout *horizontalLayout_6;
    ChatLineEdit *videomsgEdit;
    QToolButton *videosendButton;
    QWidget *page_4;
    QWidget *desktopTab;
    QVBoxLayout *verticalLayout_7;
    QSplitter *desktopsplitter;
    QWidget *layoutWidget_1;
    QHBoxLayout *horizontalLayout_4;
    DesktopGridWidget *desktopgridWidget;
    QVBoxLayout *verticalLayout_6;
    QSpacerItem *verticalSpacer_3;
    QToolButton *detachDesktopButton;
    QSpacerItem *verticalSpacer;
    QToolButton *addDesktopButton;
    QSpacerItem *verticalSpacer_2;
    QToolButton *removeDesktopButton;
    QSpacerItem *verticalSpacer_4;
    QToolButton *desktopaccessButton;
    QSpacerItem *verticalSpacer_5;
    QStackedWidget *desktopstackedWidget;
    QWidget *page;
    QVBoxLayout *verticalLayout_8;
    ChatTextEdit *desktopchatEdit;
    QHBoxLayout *horizontalLayout_5;
    ChatLineEdit *desktopmsgEdit;
    QToolButton *desktopsendButton;
    QWidget *page_2;
    QWidget *filesTab;
    QVBoxLayout *verticalLayout_4;
    QLabel *channelLabel;
    FilesView *filesView;
    QHBoxLayout *horizontalLayout;
    QToolButton *uploadButton;
    QToolButton *downloadButton;
    QToolButton *deleteButton;
    QMenuBar *menubar;
    QMenu *menuClient;
    QMenu *menuMe;
    QMenu *menuUsers;
    QMenu *menuSubscriptions;
    QMenu *menu_Advanced;
    QMenu *menuMute;
    QMenu *menu_Kick;
    QMenu *menuChannels;
    QMenu *menuServer;
    QMenu *menuHelp;
    QStatusBar *statusbar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(637, 545);
        MainWindow->setFocusPolicy(Qt::NoFocus);
        MainWindow->setWindowTitle(QString::fromUtf8(""));
        MainWindow->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        MainWindow->setAnimated(false);
        actionServerList = new QAction(MainWindow);
        actionServerList->setObjectName(QString::fromUtf8("actionServerList"));
        actionConnect = new QAction(MainWindow);
        actionConnect->setObjectName(QString::fromUtf8("actionConnect"));
        actionConnect->setCheckable(true);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/images/connect.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionConnect->setIcon(icon);
        actionPreferences = new QAction(MainWindow);
        actionPreferences->setObjectName(QString::fromUtf8("actionPreferences"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionEnablePushToTalk = new QAction(MainWindow);
        actionEnablePushToTalk->setObjectName(QString::fromUtf8("actionEnablePushToTalk"));
        actionEnablePushToTalk->setCheckable(true);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/images/hotkey.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionEnablePushToTalk->setIcon(icon1);
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionManual = new QAction(MainWindow);
        actionManual->setObjectName(QString::fromUtf8("actionManual"));
        actionManual->setEnabled(true);
        actionServerProperties = new QAction(MainWindow);
        actionServerProperties->setObjectName(QString::fromUtf8("actionServerProperties"));
        actionCreateChannel = new QAction(MainWindow);
        actionCreateChannel->setObjectName(QString::fromUtf8("actionCreateChannel"));
        actionUpdateChannel = new QAction(MainWindow);
        actionUpdateChannel->setObjectName(QString::fromUtf8("actionUpdateChannel"));
        actionDeleteChannel = new QAction(MainWindow);
        actionDeleteChannel->setObjectName(QString::fromUtf8("actionDeleteChannel"));
        actionJoinChannel = new QAction(MainWindow);
        actionJoinChannel->setObjectName(QString::fromUtf8("actionJoinChannel"));
        actionEnableVoiceActivation = new QAction(MainWindow);
        actionEnableVoiceActivation->setObjectName(QString::fromUtf8("actionEnableVoiceActivation"));
        actionEnableVoiceActivation->setCheckable(true);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/images/voiceact.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionEnableVoiceActivation->setIcon(icon2);
        actionChangeNickname = new QAction(MainWindow);
        actionChangeNickname->setObjectName(QString::fromUtf8("actionChangeNickname"));
        actionChangeStatus = new QAction(MainWindow);
        actionChangeStatus->setObjectName(QString::fromUtf8("actionChangeStatus"));
        actionViewUserInformation = new QAction(MainWindow);
        actionViewUserInformation->setObjectName(QString::fromUtf8("actionViewUserInformation"));
        actionMuteAll = new QAction(MainWindow);
        actionMuteAll->setObjectName(QString::fromUtf8("actionMuteAll"));
        actionMuteAll->setCheckable(true);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/images/muteall.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionMuteAll->setIcon(icon3);
        actionUploadFile = new QAction(MainWindow);
        actionUploadFile->setObjectName(QString::fromUtf8("actionUploadFile"));
        actionDownloadFile = new QAction(MainWindow);
        actionDownloadFile->setObjectName(QString::fromUtf8("actionDownloadFile"));
        actionDeleteFile = new QAction(MainWindow);
        actionDeleteFile->setObjectName(QString::fromUtf8("actionDeleteFile"));
        actionBannedUsers = new QAction(MainWindow);
        actionBannedUsers->setObjectName(QString::fromUtf8("actionBannedUsers"));
        actionUserAccounts = new QAction(MainWindow);
        actionUserAccounts->setObjectName(QString::fromUtf8("actionUserAccounts"));
        actionSaveConfiguration = new QAction(MainWindow);
        actionSaveConfiguration->setObjectName(QString::fromUtf8("actionSaveConfiguration"));
        actionViewChannelInfo = new QAction(MainWindow);
        actionViewChannelInfo->setObjectName(QString::fromUtf8("actionViewChannelInfo"));
        actionMessages = new QAction(MainWindow);
        actionMessages->setObjectName(QString::fromUtf8("actionMessages"));
        actionKickBan = new QAction(MainWindow);
        actionKickBan->setObjectName(QString::fromUtf8("actionKickBan"));
        actionOp = new QAction(MainWindow);
        actionOp->setObjectName(QString::fromUtf8("actionOp"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/images/images/op.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOp->setIcon(icon4);
        actionVolume = new QAction(MainWindow);
        actionVolume->setObjectName(QString::fromUtf8("actionVolume"));
        actionVoice = new QAction(MainWindow);
        actionVoice->setObjectName(QString::fromUtf8("actionVoice"));
        actionVoice->setCheckable(true);
        actionVoice->setIcon(icon2);
        actionUserMessages = new QAction(MainWindow);
        actionUserMessages->setObjectName(QString::fromUtf8("actionUserMessages"));
        actionUserMessages->setCheckable(true);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/images/images/msg.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionUserMessages->setIcon(icon5);
        actionChannelMessages = new QAction(MainWindow);
        actionChannelMessages->setObjectName(QString::fromUtf8("actionChannelMessages"));
        actionChannelMessages->setCheckable(true);
        actionVideo = new QAction(MainWindow);
        actionVideo->setObjectName(QString::fromUtf8("actionVideo"));
        actionVideo->setCheckable(true);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/images/images/webcam.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionVideo->setIcon(icon6);
        actionBroadcastMessages = new QAction(MainWindow);
        actionBroadcastMessages->setObjectName(QString::fromUtf8("actionBroadcastMessages"));
        actionBroadcastMessages->setCheckable(true);
        actionEnableVideoTransmission = new QAction(MainWindow);
        actionEnableVideoTransmission->setObjectName(QString::fromUtf8("actionEnableVideoTransmission"));
        actionEnableVideoTransmission->setCheckable(true);
        actionEnableVideoTransmission->setIcon(icon6);
        actionStreamAudioFileToChannel = new QAction(MainWindow);
        actionStreamAudioFileToChannel->setObjectName(QString::fromUtf8("actionStreamAudioFileToChannel"));
        actionStreamAudioFileToChannel->setCheckable(true);
        actionStreamAudioFileToUser = new QAction(MainWindow);
        actionStreamAudioFileToUser->setObjectName(QString::fromUtf8("actionStreamAudioFileToUser"));
        actionMediaStorage = new QAction(MainWindow);
        actionMediaStorage->setObjectName(QString::fromUtf8("actionMediaStorage"));
        actionMediaStorage->setCheckable(true);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/images/images/record.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionMediaStorage->setIcon(icon7);
        actionInterceptUserMessages = new QAction(MainWindow);
        actionInterceptUserMessages->setObjectName(QString::fromUtf8("actionInterceptUserMessages"));
        actionInterceptUserMessages->setCheckable(true);
        actionInterceptUserMessages->setIcon(icon5);
        actionInterceptChannelMessages = new QAction(MainWindow);
        actionInterceptChannelMessages->setObjectName(QString::fromUtf8("actionInterceptChannelMessages"));
        actionInterceptChannelMessages->setCheckable(true);
        actionInterceptVoice = new QAction(MainWindow);
        actionInterceptVoice->setObjectName(QString::fromUtf8("actionInterceptVoice"));
        actionInterceptVoice->setCheckable(true);
        actionInterceptVoice->setIcon(icon2);
        actionInterceptVideo = new QAction(MainWindow);
        actionInterceptVideo->setObjectName(QString::fromUtf8("actionInterceptVideo"));
        actionInterceptVideo->setCheckable(true);
        actionInterceptVideo->setIcon(icon6);
        actionBroadcastMessage = new QAction(MainWindow);
        actionBroadcastMessage->setObjectName(QString::fromUtf8("actionBroadcastMessage"));
        actionIncreaseVoiceVolume = new QAction(MainWindow);
        actionIncreaseVoiceVolume->setObjectName(QString::fromUtf8("actionIncreaseVoiceVolume"));
        actionLowerVoiceVolume = new QAction(MainWindow);
        actionLowerVoiceVolume->setObjectName(QString::fromUtf8("actionLowerVoiceVolume"));
        actionStoreForMove = new QAction(MainWindow);
        actionStoreForMove->setObjectName(QString::fromUtf8("actionStoreForMove"));
        actionMoveUser = new QAction(MainWindow);
        actionMoveUser->setObjectName(QString::fromUtf8("actionMoveUser"));
        actionVisitBearWare = new QAction(MainWindow);
        actionVisitBearWare->setObjectName(QString::fromUtf8("actionVisitBearWare"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/images/images/teamtalk.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionVisitBearWare->setIcon(icon8);
        actionVisitChangeLog = new QAction(MainWindow);
        actionVisitChangeLog->setObjectName(QString::fromUtf8("actionVisitChangeLog"));
        actionAllowVoiceTransmission = new QAction(MainWindow);
        actionAllowVoiceTransmission->setObjectName(QString::fromUtf8("actionAllowVoiceTransmission"));
        actionAllowVoiceTransmission->setCheckable(true);
        actionAllowVideoTransmission = new QAction(MainWindow);
        actionAllowVideoTransmission->setObjectName(QString::fromUtf8("actionAllowVideoTransmission"));
        actionAllowVideoTransmission->setCheckable(true);
        actionServerStatistics = new QAction(MainWindow);
        actionServerStatistics->setObjectName(QString::fromUtf8("actionServerStatistics"));
        actionNewClient = new QAction(MainWindow);
        actionNewClient->setObjectName(QString::fromUtf8("actionNewClient"));
        actionOnlineUsers = new QAction(MainWindow);
        actionOnlineUsers->setObjectName(QString::fromUtf8("actionOnlineUsers"));
        actionOnlineUsers->setCheckable(false);
        actionEnableQuestionMode = new QAction(MainWindow);
        actionEnableQuestionMode->setObjectName(QString::fromUtf8("actionEnableQuestionMode"));
        actionEnableQuestionMode->setCheckable(true);
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/images/images/hand.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionEnableQuestionMode->setIcon(icon9);
        actionEnableDesktopSharing = new QAction(MainWindow);
        actionEnableDesktopSharing->setObjectName(QString::fromUtf8("actionEnableDesktopSharing"));
        actionEnableDesktopSharing->setCheckable(true);
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/images/images/desktoptx.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionEnableDesktopSharing->setIcon(icon10);
        actionAllowDesktopTransmission = new QAction(MainWindow);
        actionAllowDesktopTransmission->setObjectName(QString::fromUtf8("actionAllowDesktopTransmission"));
        actionAllowDesktopTransmission->setCheckable(true);
        actionDesktop = new QAction(MainWindow);
        actionDesktop->setObjectName(QString::fromUtf8("actionDesktop"));
        actionDesktop->setCheckable(true);
        actionDesktop->setIcon(icon10);
        actionInterceptDesktop = new QAction(MainWindow);
        actionInterceptDesktop->setObjectName(QString::fromUtf8("actionInterceptDesktop"));
        actionInterceptDesktop->setCheckable(true);
        actionInterceptDesktop->setIcon(icon10);
        actionStreamMediaFileToChannel = new QAction(MainWindow);
        actionStreamMediaFileToChannel->setObjectName(QString::fromUtf8("actionStreamMediaFileToChannel"));
        actionStreamMediaFileToChannel->setCheckable(true);
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/images/images/streammedia.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionStreamMediaFileToChannel->setIcon(icon11);
        actionDesktopInput = new QAction(MainWindow);
        actionDesktopInput->setObjectName(QString::fromUtf8("actionDesktopInput"));
        actionDesktopInput->setCheckable(true);
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/images/images/chalkstick.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDesktopInput->setIcon(icon12);
        actionDesktopAccessAllow = new QAction(MainWindow);
        actionDesktopAccessAllow->setObjectName(QString::fromUtf8("actionDesktopAccessAllow"));
        actionDesktopAccessAllow->setCheckable(true);
        actionDesktopAccessAllow->setIcon(icon12);
        actionDesktopAccessDeny = new QAction(MainWindow);
        actionDesktopAccessDeny->setObjectName(QString::fromUtf8("actionDesktopAccessDeny"));
        actionDesktopAccessDeny->setCheckable(false);
        actionMediaFile = new QAction(MainWindow);
        actionMediaFile->setObjectName(QString::fromUtf8("actionMediaFile"));
        actionMediaFile->setCheckable(true);
        actionMediaFile->setIcon(icon11);
        actionInterceptMediaFile = new QAction(MainWindow);
        actionInterceptMediaFile->setObjectName(QString::fromUtf8("actionInterceptMediaFile"));
        actionInterceptMediaFile->setCheckable(true);
        actionInterceptMediaFile->setIcon(icon11);
        actionAllowMediaFileTransmission = new QAction(MainWindow);
        actionAllowMediaFileTransmission->setObjectName(QString::fromUtf8("actionAllowMediaFileTransmission"));
        actionMuteVoice = new QAction(MainWindow);
        actionMuteVoice->setObjectName(QString::fromUtf8("actionMuteVoice"));
        actionMuteMediaFile = new QAction(MainWindow);
        actionMuteMediaFile->setObjectName(QString::fromUtf8("actionMuteMediaFile"));
        actionKickFromChannel = new QAction(MainWindow);
        actionKickFromChannel->setObjectName(QString::fromUtf8("actionKickFromChannel"));
        actionKickFromServer = new QAction(MainWindow);
        actionKickFromServer->setObjectName(QString::fromUtf8("actionKickFromServer"));
        actionIncreaseMediaFileVolume = new QAction(MainWindow);
        actionIncreaseMediaFileVolume->setObjectName(QString::fromUtf8("actionIncreaseMediaFileVolume"));
        actionLowerMediaFileVolume = new QAction(MainWindow);
        actionLowerMediaFileVolume->setObjectName(QString::fromUtf8("actionLowerMediaFileVolume"));
        actionResetPreferencesToDefault = new QAction(MainWindow);
        actionResetPreferencesToDefault->setObjectName(QString::fromUtf8("actionResetPreferencesToDefault"));
        actionKickAndBanFromChannel = new QAction(MainWindow);
        actionKickAndBanFromChannel->setObjectName(QString::fromUtf8("actionKickAndBanFromChannel"));
        actionBannedUsersInChannel = new QAction(MainWindow);
        actionBannedUsersInChannel->setObjectName(QString::fromUtf8("actionBannedUsersInChannel"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout_2 = new QVBoxLayout(centralwidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(3, 3, 3, 0);
        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        layoutWidget = new QWidget(splitter);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        channelsWidget = new ChannelsTree(layoutWidget);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(4, QString::fromUtf8("5"));
        __qtreewidgetitem->setText(3, QString::fromUtf8("4"));
        __qtreewidgetitem->setText(2, QString::fromUtf8("3"));
        __qtreewidgetitem->setText(1, QString::fromUtf8("2"));
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        channelsWidget->setHeaderItem(__qtreewidgetitem);
        channelsWidget->setObjectName(QString::fromUtf8("channelsWidget"));
        channelsWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        channelsWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        channelsWidget->setProperty("showDropIndicator", QVariant(false));
        channelsWidget->setAlternatingRowColors(false);
        channelsWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        channelsWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
        channelsWidget->setRootIsDecorated(false);
        channelsWidget->setUniformRowHeights(true);
        channelsWidget->setAnimated(false);
        channelsWidget->setHeaderHidden(true);
        channelsWidget->setColumnCount(5);
        channelsWidget->header()->setVisible(false);
        channelsWidget->header()->setMinimumSectionSize(5);
        channelsWidget->header()->setDefaultSectionSize(50);
        channelsWidget->header()->setStretchLastSection(false);

        verticalLayout->addWidget(channelsWidget);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(layoutWidget);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setPixmap(QPixmap(QString::fromUtf8(":/images/images/speaker.png")));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        volumeSlider = new QSlider(layoutWidget);
        volumeSlider->setObjectName(QString::fromUtf8("volumeSlider"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(volumeSlider->sizePolicy().hasHeightForWidth());
        volumeSlider->setSizePolicy(sizePolicy1);
        volumeSlider->setMaximum(100);
        volumeSlider->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(volumeSlider, 0, 1, 1, 1);

        label_3 = new QLabel(layoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setPixmap(QPixmap(QString::fromUtf8(":/images/images/vumeter.png")));
        label_3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_3, 0, 2, 1, 1);

        voiceactBar = new QProgressBar(layoutWidget);
        voiceactBar->setObjectName(QString::fromUtf8("voiceactBar"));
        QSizePolicy sizePolicy2(QSizePolicy::Ignored, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(voiceactBar->sizePolicy().hasHeightForWidth());
        voiceactBar->setSizePolicy(sizePolicy2);
        voiceactBar->setMaximum(20);
        voiceactBar->setValue(0);
        voiceactBar->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(voiceactBar, 0, 3, 1, 1);

        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setPixmap(QPixmap(QString::fromUtf8(":/images/images/mike.png")));
        label_2->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        micSlider = new QSlider(layoutWidget);
        micSlider->setObjectName(QString::fromUtf8("micSlider"));
        sizePolicy1.setHeightForWidth(micSlider->sizePolicy().hasHeightForWidth());
        micSlider->setSizePolicy(sizePolicy1);
        micSlider->setMaximum(100);
        micSlider->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(micSlider, 1, 1, 1, 1);

        label_4 = new QLabel(layoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setPixmap(QPixmap(QString::fromUtf8(":/images/images/voiceact.png")));
        label_4->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_4, 1, 2, 1, 1);

        voiceactSlider = new QSlider(layoutWidget);
        voiceactSlider->setObjectName(QString::fromUtf8("voiceactSlider"));
        sizePolicy1.setHeightForWidth(voiceactSlider->sizePolicy().hasHeightForWidth());
        voiceactSlider->setSizePolicy(sizePolicy1);
        voiceactSlider->setMaximum(20);
        voiceactSlider->setPageStep(5);
        voiceactSlider->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(voiceactSlider, 1, 3, 1, 1);

        gridLayout->setColumnStretch(1, 1);
        gridLayout->setColumnStretch(3, 1);

        verticalLayout->addLayout(gridLayout);

        splitter->addWidget(layoutWidget);
        tabWidget = new QTabWidget(splitter);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        chatTab = new QWidget();
        chatTab->setObjectName(QString::fromUtf8("chatTab"));
        verticalLayout_3 = new QVBoxLayout(chatTab);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(3, 3, 3, 3);
        chatEdit = new ChatTextEdit(chatTab);
        chatEdit->setObjectName(QString::fromUtf8("chatEdit"));
        chatEdit->setMouseTracking(false);
        chatEdit->setTabChangesFocus(true);
        chatEdit->setUndoRedoEnabled(false);
        chatEdit->setReadOnly(true);
        chatEdit->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse|Qt::TextBrowserInteraction|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout_3->addWidget(chatEdit);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        msgEdit = new ChatLineEdit(chatTab);
        msgEdit->setObjectName(QString::fromUtf8("msgEdit"));
        msgEdit->setVisible(false);

        horizontalLayout_2->addWidget(msgEdit);

        sendButton = new QToolButton(chatTab);
        sendButton->setObjectName(QString::fromUtf8("sendButton"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(sendButton->sizePolicy().hasHeightForWidth());
        sendButton->setSizePolicy(sizePolicy3);
        sendButton->setVisible(false);

        horizontalLayout_2->addWidget(sendButton);


        verticalLayout_3->addLayout(horizontalLayout_2);

        tabWidget->addTab(chatTab, QString());
        videoTab = new QWidget();
        videoTab->setObjectName(QString::fromUtf8("videoTab"));
        verticalLayout_10 = new QVBoxLayout(videoTab);
        verticalLayout_10->setSpacing(3);
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        verticalLayout_10->setContentsMargins(3, 3, 3, 3);
        videosplitter = new QSplitter(videoTab);
        videosplitter->setObjectName(QString::fromUtf8("videosplitter"));
        videosplitter->setOrientation(Qt::Vertical);
        videosplitter->setHandleWidth(5);
        layoutWidget_2 = new QWidget(videosplitter);
        layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));
        horizontalLayout_3 = new QHBoxLayout(layoutWidget_2);
        horizontalLayout_3->setSpacing(3);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        videogridWidget = new VideoGridWidget(layoutWidget_2);
        videogridWidget->setObjectName(QString::fromUtf8("videogridWidget"));

        horizontalLayout_3->addWidget(videogridWidget);

        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        verticalSpacer_6 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer_6);

        detachVideoButton = new QToolButton(layoutWidget_2);
        detachVideoButton->setObjectName(QString::fromUtf8("detachVideoButton"));
        detachVideoButton->setEnabled(false);
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/images/images/detach.png"), QSize(), QIcon::Normal, QIcon::Off);
        detachVideoButton->setIcon(icon13);
        detachVideoButton->setIconSize(QSize(20, 20));

        verticalLayout_5->addWidget(detachVideoButton);

        verticalSpacer_8 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer_8);

        addVideoButton = new QToolButton(layoutWidget_2);
        addVideoButton->setObjectName(QString::fromUtf8("addVideoButton"));
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/images/images/plus.png"), QSize(), QIcon::Normal, QIcon::Off);
        addVideoButton->setIcon(icon14);
        addVideoButton->setIconSize(QSize(20, 20));

        verticalLayout_5->addWidget(addVideoButton);

        verticalSpacer_9 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer_9);

        removeVideoButton = new QToolButton(layoutWidget_2);
        removeVideoButton->setObjectName(QString::fromUtf8("removeVideoButton"));
        removeVideoButton->setEnabled(true);
        QIcon icon15;
        icon15.addFile(QString::fromUtf8(":/images/images/minus.png"), QSize(), QIcon::Normal, QIcon::Off);
        removeVideoButton->setIcon(icon15);
        removeVideoButton->setIconSize(QSize(20, 20));

        verticalLayout_5->addWidget(removeVideoButton);

        verticalSpacer_10 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer_10);

        initVideoButton = new QToolButton(layoutWidget_2);
        initVideoButton->setObjectName(QString::fromUtf8("initVideoButton"));
        initVideoButton->setText(QString::fromUtf8("Start/Stop Webcam"));
        QIcon icon16;
        icon16.addFile(QString::fromUtf8(":/images/images/webcam_start.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon16.addFile(QString::fromUtf8(":/images/images/webcam_stop.png"), QSize(), QIcon::Normal, QIcon::On);
        initVideoButton->setIcon(icon16);
        initVideoButton->setIconSize(QSize(20, 20));
        initVideoButton->setCheckable(true);

        verticalLayout_5->addWidget(initVideoButton);

        verticalSpacer_7 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer_7);


        horizontalLayout_3->addLayout(verticalLayout_5);

        horizontalLayout_3->setStretch(0, 1);
        videosplitter->addWidget(layoutWidget_2);
        videostackedWidget = new QStackedWidget(videosplitter);
        videostackedWidget->setObjectName(QString::fromUtf8("videostackedWidget"));
        videostackedWidget->setMaximumSize(QSize(16777215, 16777215));
        page_3 = new QWidget();
        page_3->setObjectName(QString::fromUtf8("page_3"));
        verticalLayout_9 = new QVBoxLayout(page_3);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        verticalLayout_9->setContentsMargins(0, 0, 0, 0);
        videochatEdit = new ChatTextEdit(page_3);
        videochatEdit->setObjectName(QString::fromUtf8("videochatEdit"));
        videochatEdit->setMouseTracking(false);
        videochatEdit->setTabChangesFocus(true);
        videochatEdit->setUndoRedoEnabled(false);
        videochatEdit->setReadOnly(true);
        videochatEdit->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse|Qt::TextBrowserInteraction|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout_9->addWidget(videochatEdit);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        videomsgEdit = new ChatLineEdit(page_3);
        videomsgEdit->setObjectName(QString::fromUtf8("videomsgEdit"));
        videomsgEdit->setVisible(false);

        horizontalLayout_6->addWidget(videomsgEdit);

        videosendButton = new QToolButton(page_3);
        videosendButton->setObjectName(QString::fromUtf8("videosendButton"));
        videosendButton->setVisible(false);

        horizontalLayout_6->addWidget(videosendButton);


        verticalLayout_9->addLayout(horizontalLayout_6);

        verticalLayout_9->setStretch(0, 1);
        videostackedWidget->addWidget(page_3);
        page_4 = new QWidget();
        page_4->setObjectName(QString::fromUtf8("page_4"));
        videostackedWidget->addWidget(page_4);
        videosplitter->addWidget(videostackedWidget);

        verticalLayout_10->addWidget(videosplitter);

        tabWidget->addTab(videoTab, QString());
        desktopTab = new QWidget();
        desktopTab->setObjectName(QString::fromUtf8("desktopTab"));
        verticalLayout_7 = new QVBoxLayout(desktopTab);
        verticalLayout_7->setSpacing(3);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        verticalLayout_7->setContentsMargins(3, 3, 3, 3);
        desktopsplitter = new QSplitter(desktopTab);
        desktopsplitter->setObjectName(QString::fromUtf8("desktopsplitter"));
        desktopsplitter->setOrientation(Qt::Vertical);
        desktopsplitter->setHandleWidth(5);
        layoutWidget_1 = new QWidget(desktopsplitter);
        layoutWidget_1->setObjectName(QString::fromUtf8("layoutWidget_1"));
        horizontalLayout_4 = new QHBoxLayout(layoutWidget_1);
        horizontalLayout_4->setSpacing(3);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        desktopgridWidget = new DesktopGridWidget(layoutWidget_1);
        desktopgridWidget->setObjectName(QString::fromUtf8("desktopgridWidget"));

        horizontalLayout_4->addWidget(desktopgridWidget);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_6->addItem(verticalSpacer_3);

        detachDesktopButton = new QToolButton(layoutWidget_1);
        detachDesktopButton->setObjectName(QString::fromUtf8("detachDesktopButton"));
        detachDesktopButton->setEnabled(false);
        detachDesktopButton->setIcon(icon13);
        detachDesktopButton->setIconSize(QSize(20, 20));

        verticalLayout_6->addWidget(detachDesktopButton);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_6->addItem(verticalSpacer);

        addDesktopButton = new QToolButton(layoutWidget_1);
        addDesktopButton->setObjectName(QString::fromUtf8("addDesktopButton"));
        addDesktopButton->setIcon(icon14);
        addDesktopButton->setIconSize(QSize(20, 20));

        verticalLayout_6->addWidget(addDesktopButton);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_6->addItem(verticalSpacer_2);

        removeDesktopButton = new QToolButton(layoutWidget_1);
        removeDesktopButton->setObjectName(QString::fromUtf8("removeDesktopButton"));
        removeDesktopButton->setIcon(icon15);
        removeDesktopButton->setIconSize(QSize(20, 20));

        verticalLayout_6->addWidget(removeDesktopButton);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_6->addItem(verticalSpacer_4);

        desktopaccessButton = new QToolButton(layoutWidget_1);
        desktopaccessButton->setObjectName(QString::fromUtf8("desktopaccessButton"));
        desktopaccessButton->setEnabled(false);
        desktopaccessButton->setIcon(icon12);
        desktopaccessButton->setIconSize(QSize(20, 20));
        desktopaccessButton->setCheckable(true);

        verticalLayout_6->addWidget(desktopaccessButton);

        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_6->addItem(verticalSpacer_5);


        horizontalLayout_4->addLayout(verticalLayout_6);

        horizontalLayout_4->setStretch(0, 1);
        desktopsplitter->addWidget(layoutWidget_1);
        desktopstackedWidget = new QStackedWidget(desktopsplitter);
        desktopstackedWidget->setObjectName(QString::fromUtf8("desktopstackedWidget"));
        desktopstackedWidget->setMaximumSize(QSize(16777215, 16777215));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        verticalLayout_8 = new QVBoxLayout(page);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        verticalLayout_8->setContentsMargins(0, 0, 0, 0);
        desktopchatEdit = new ChatTextEdit(page);
        desktopchatEdit->setObjectName(QString::fromUtf8("desktopchatEdit"));
        desktopchatEdit->setMouseTracking(false);
        desktopchatEdit->setTabChangesFocus(true);
        desktopchatEdit->setUndoRedoEnabled(false);
        desktopchatEdit->setReadOnly(true);
        desktopchatEdit->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse|Qt::TextBrowserInteraction|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout_8->addWidget(desktopchatEdit);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        desktopmsgEdit = new ChatLineEdit(page);
        desktopmsgEdit->setObjectName(QString::fromUtf8("desktopmsgEdit"));
        desktopmsgEdit->setVisible(false);

        horizontalLayout_5->addWidget(desktopmsgEdit);

        desktopsendButton = new QToolButton(page);
        desktopsendButton->setObjectName(QString::fromUtf8("desktopsendButton"));
        desktopsendButton->setVisible(false);

        horizontalLayout_5->addWidget(desktopsendButton);


        verticalLayout_8->addLayout(horizontalLayout_5);

        verticalLayout_8->setStretch(0, 1);
        desktopstackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        desktopstackedWidget->addWidget(page_2);
        desktopsplitter->addWidget(desktopstackedWidget);

        verticalLayout_7->addWidget(desktopsplitter);

        tabWidget->addTab(desktopTab, QString());
        filesTab = new QWidget();
        filesTab->setObjectName(QString::fromUtf8("filesTab"));
        verticalLayout_4 = new QVBoxLayout(filesTab);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(3, 3, 3, 3);
        channelLabel = new QLabel(filesTab);
        channelLabel->setObjectName(QString::fromUtf8("channelLabel"));

        verticalLayout_4->addWidget(channelLabel);

        filesView = new FilesView(filesTab);
        filesView->setObjectName(QString::fromUtf8("filesView"));
        filesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        filesView->setDragEnabled(false);
        filesView->setDragDropMode(QAbstractItemView::DropOnly);
        filesView->setAlternatingRowColors(true);
        filesView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        filesView->setSelectionBehavior(QAbstractItemView::SelectRows);
        filesView->setIndentation(0);
        filesView->setRootIsDecorated(false);
        filesView->setItemsExpandable(false);
        filesView->setExpandsOnDoubleClick(false);

        verticalLayout_4->addWidget(filesView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        uploadButton = new QToolButton(filesTab);
        uploadButton->setObjectName(QString::fromUtf8("uploadButton"));
        uploadButton->setEnabled(false);
        QIcon icon17;
        icon17.addFile(QString::fromUtf8(":/images/images/upload.png"), QSize(), QIcon::Normal, QIcon::Off);
        uploadButton->setIcon(icon17);
        uploadButton->setIconSize(QSize(20, 20));

        horizontalLayout->addWidget(uploadButton);

        downloadButton = new QToolButton(filesTab);
        downloadButton->setObjectName(QString::fromUtf8("downloadButton"));
        downloadButton->setEnabled(false);
        QIcon icon18;
        icon18.addFile(QString::fromUtf8(":/images/images/download.png"), QSize(), QIcon::Normal, QIcon::Off);
        downloadButton->setIcon(icon18);
        downloadButton->setIconSize(QSize(20, 20));

        horizontalLayout->addWidget(downloadButton);

        deleteButton = new QToolButton(filesTab);
        deleteButton->setObjectName(QString::fromUtf8("deleteButton"));
        deleteButton->setEnabled(false);
        QIcon icon19;
        icon19.addFile(QString::fromUtf8(":/images/images/delete.png"), QSize(), QIcon::Normal, QIcon::Off);
        deleteButton->setIcon(icon19);
        deleteButton->setIconSize(QSize(20, 20));

        horizontalLayout->addWidget(deleteButton);


        verticalLayout_4->addLayout(horizontalLayout);

        tabWidget->addTab(filesTab, QString());
        splitter->addWidget(tabWidget);

        verticalLayout_2->addWidget(splitter);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 637, 22));
        menuClient = new QMenu(menubar);
        menuClient->setObjectName(QString::fromUtf8("menuClient"));
        menuMe = new QMenu(menubar);
        menuMe->setObjectName(QString::fromUtf8("menuMe"));
        menuUsers = new QMenu(menubar);
        menuUsers->setObjectName(QString::fromUtf8("menuUsers"));
        menuSubscriptions = new QMenu(menuUsers);
        menuSubscriptions->setObjectName(QString::fromUtf8("menuSubscriptions"));
        menu_Advanced = new QMenu(menuUsers);
        menu_Advanced->setObjectName(QString::fromUtf8("menu_Advanced"));
        menuMute = new QMenu(menuUsers);
        menuMute->setObjectName(QString::fromUtf8("menuMute"));
        menu_Kick = new QMenu(menuUsers);
        menu_Kick->setObjectName(QString::fromUtf8("menu_Kick"));
        menuChannels = new QMenu(menubar);
        menuChannels->setObjectName(QString::fromUtf8("menuChannels"));
        menuServer = new QMenu(menubar);
        menuServer->setObjectName(QString::fromUtf8("menuServer"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        statusbar->setSizeGripEnabled(true);
        MainWindow->setStatusBar(statusbar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        sizePolicy.setHeightForWidth(toolBar->sizePolicy().hasHeightForWidth());
        toolBar->setSizePolicy(sizePolicy);
        toolBar->setWindowTitle(QString::fromUtf8("toolBar"));
        toolBar->setMovable(false);
        toolBar->setIconSize(QSize(20, 20));
        toolBar->setFloatable(false);
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);
#if QT_CONFIG(shortcut)
        label->setBuddy(volumeSlider);
        label_2->setBuddy(micSlider);
        label_4->setBuddy(voiceactSlider);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(channelsWidget, volumeSlider);
        QWidget::setTabOrder(volumeSlider, micSlider);
        QWidget::setTabOrder(micSlider, voiceactSlider);
        QWidget::setTabOrder(voiceactSlider, tabWidget);
        QWidget::setTabOrder(tabWidget, chatEdit);
        QWidget::setTabOrder(chatEdit, msgEdit);
        QWidget::setTabOrder(msgEdit, sendButton);
        QWidget::setTabOrder(sendButton, detachVideoButton);
        QWidget::setTabOrder(detachVideoButton, addVideoButton);
        QWidget::setTabOrder(addVideoButton, removeVideoButton);
        QWidget::setTabOrder(removeVideoButton, initVideoButton);
        QWidget::setTabOrder(initVideoButton, detachDesktopButton);
        QWidget::setTabOrder(detachDesktopButton, addDesktopButton);
        QWidget::setTabOrder(addDesktopButton, removeDesktopButton);
        QWidget::setTabOrder(removeDesktopButton, desktopaccessButton);
        QWidget::setTabOrder(desktopaccessButton, filesView);
        QWidget::setTabOrder(filesView, uploadButton);
        QWidget::setTabOrder(uploadButton, downloadButton);
        QWidget::setTabOrder(downloadButton, deleteButton);
        QWidget::setTabOrder(deleteButton, desktopchatEdit);
        QWidget::setTabOrder(desktopchatEdit, desktopmsgEdit);
        QWidget::setTabOrder(desktopmsgEdit, desktopsendButton);

        menubar->addAction(menuClient->menuAction());
        menubar->addAction(menuMe->menuAction());
        menubar->addAction(menuUsers->menuAction());
        menubar->addAction(menuChannels->menuAction());
        menubar->addAction(menuServer->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuClient->addAction(actionNewClient);
        menuClient->addSeparator();
        menuClient->addAction(actionConnect);
        menuClient->addSeparator();
        menuClient->addAction(actionPreferences);
        menuClient->addAction(actionMediaStorage);
        menuClient->addSeparator();
        menuClient->addAction(actionExit);
        menuMe->addAction(actionChangeNickname);
        menuMe->addAction(actionChangeStatus);
        menuMe->addSeparator();
        menuMe->addAction(actionEnablePushToTalk);
        menuMe->addAction(actionEnableVoiceActivation);
        menuMe->addAction(actionEnableVideoTransmission);
        menuMe->addAction(actionEnableDesktopSharing);
        menuUsers->addAction(actionViewUserInformation);
        menuUsers->addAction(actionMessages);
        menuUsers->addAction(actionDesktopAccessAllow);
        menuUsers->addAction(actionOp);
        menuUsers->addAction(actionVolume);
        menuUsers->addAction(menuMute->menuAction());
        menuUsers->addAction(menu_Kick->menuAction());
        menuUsers->addAction(menuSubscriptions->menuAction());
        menuUsers->addAction(menu_Advanced->menuAction());
        menuUsers->addSeparator();
        menuUsers->addAction(actionMuteAll);
        menuSubscriptions->addAction(actionUserMessages);
        menuSubscriptions->addAction(actionChannelMessages);
        menuSubscriptions->addAction(actionBroadcastMessages);
        menuSubscriptions->addAction(actionVoice);
        menuSubscriptions->addAction(actionVideo);
        menuSubscriptions->addAction(actionDesktop);
        menuSubscriptions->addAction(actionDesktopInput);
        menuSubscriptions->addAction(actionMediaFile);
        menuSubscriptions->addSeparator();
        menuSubscriptions->addAction(actionInterceptUserMessages);
        menuSubscriptions->addAction(actionInterceptChannelMessages);
        menuSubscriptions->addAction(actionInterceptVoice);
        menuSubscriptions->addAction(actionInterceptVideo);
        menuSubscriptions->addAction(actionInterceptDesktop);
        menuSubscriptions->addAction(actionInterceptMediaFile);
        menu_Advanced->addAction(actionIncreaseVoiceVolume);
        menu_Advanced->addAction(actionLowerVoiceVolume);
        menu_Advanced->addAction(actionIncreaseMediaFileVolume);
        menu_Advanced->addAction(actionLowerMediaFileVolume);
        menu_Advanced->addSeparator();
        menu_Advanced->addAction(actionStoreForMove);
        menu_Advanced->addAction(actionMoveUser);
        menu_Advanced->addSeparator();
        menu_Advanced->addAction(actionAllowVoiceTransmission);
        menu_Advanced->addAction(actionAllowVideoTransmission);
        menu_Advanced->addAction(actionAllowDesktopTransmission);
        menu_Advanced->addAction(actionAllowMediaFileTransmission);
        menuMute->addAction(actionMuteVoice);
        menuMute->addAction(actionMuteMediaFile);
        menu_Kick->addAction(actionKickFromChannel);
        menu_Kick->addAction(actionKickAndBanFromChannel);
        menu_Kick->addSeparator();
        menu_Kick->addAction(actionKickFromServer);
        menu_Kick->addAction(actionKickBan);
        menuChannels->addAction(actionJoinChannel);
        menuChannels->addAction(actionViewChannelInfo);
        menuChannels->addSeparator();
        menuChannels->addAction(actionCreateChannel);
        menuChannels->addAction(actionUpdateChannel);
        menuChannels->addAction(actionDeleteChannel);
        menuChannels->addSeparator();
        menuChannels->addAction(actionBannedUsersInChannel);
        menuChannels->addSeparator();
        menuChannels->addAction(actionStreamMediaFileToChannel);
        menuChannels->addSeparator();
        menuChannels->addAction(actionUploadFile);
        menuChannels->addAction(actionDownloadFile);
        menuChannels->addAction(actionDeleteFile);
        menuServer->addAction(actionUserAccounts);
        menuServer->addAction(actionBannedUsers);
        menuServer->addAction(actionOnlineUsers);
        menuServer->addSeparator();
        menuServer->addAction(actionBroadcastMessage);
        menuServer->addSeparator();
        menuServer->addAction(actionServerProperties);
        menuServer->addAction(actionSaveConfiguration);
        menuServer->addSeparator();
        menuServer->addAction(actionServerStatistics);
        menuHelp->addAction(actionManual);
        menuHelp->addAction(actionResetPreferencesToDefault);
        menuHelp->addAction(actionVisitBearWare);
        menuHelp->addAction(actionVisitChangeLog);
        menuHelp->addSeparator();
        menuHelp->addAction(actionAbout);
        toolBar->addAction(actionConnect);
        toolBar->addSeparator();
        toolBar->addAction(actionEnablePushToTalk);
        toolBar->addAction(actionEnableVoiceActivation);
        toolBar->addAction(actionEnableVideoTransmission);
        toolBar->addAction(actionEnableDesktopSharing);
        toolBar->addAction(actionStreamMediaFileToChannel);
        toolBar->addSeparator();
        toolBar->addAction(actionMuteAll);
        toolBar->addAction(actionMediaStorage);
        toolBar->addSeparator();
        toolBar->addAction(actionEnableQuestionMode);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);
        videostackedWidget->setCurrentIndex(0);
        desktopstackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        actionServerList->setText(QCoreApplication::translate("MainWindow", "&Server List", nullptr));
#if QT_CONFIG(shortcut)
        actionServerList->setShortcut(QCoreApplication::translate("MainWindow", "F3", nullptr));
#endif // QT_CONFIG(shortcut)
        actionConnect->setText(QCoreApplication::translate("MainWindow", "&Connect", nullptr));
#if QT_CONFIG(shortcut)
        actionConnect->setShortcut(QCoreApplication::translate("MainWindow", "F2", nullptr));
#endif // QT_CONFIG(shortcut)
        actionPreferences->setText(QCoreApplication::translate("MainWindow", "&Preferences", nullptr));
#if QT_CONFIG(shortcut)
        actionPreferences->setShortcut(QCoreApplication::translate("MainWindow", "F4", nullptr));
#endif // QT_CONFIG(shortcut)
        actionExit->setText(QCoreApplication::translate("MainWindow", "&Exit", nullptr));
#if QT_CONFIG(shortcut)
        actionExit->setShortcut(QCoreApplication::translate("MainWindow", "Alt+F4", nullptr));
#endif // QT_CONFIG(shortcut)
        actionEnablePushToTalk->setText(QCoreApplication::translate("MainWindow", "Enable &Push To Talk", nullptr));
#if QT_CONFIG(shortcut)
        actionEnablePushToTalk->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+T", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAbout->setText(QCoreApplication::translate("MainWindow", "&About", nullptr));
        actionManual->setText(QCoreApplication::translate("MainWindow", "&Manual", nullptr));
#if QT_CONFIG(shortcut)
        actionManual->setShortcut(QCoreApplication::translate("MainWindow", "F1", nullptr));
#endif // QT_CONFIG(shortcut)
        actionServerProperties->setText(QCoreApplication::translate("MainWindow", "Server &Properties", nullptr));
#if QT_CONFIG(shortcut)
        actionServerProperties->setShortcut(QCoreApplication::translate("MainWindow", "F9", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCreateChannel->setText(QCoreApplication::translate("MainWindow", "&Create Channel", nullptr));
#if QT_CONFIG(shortcut)
        actionCreateChannel->setShortcut(QCoreApplication::translate("MainWindow", "F7", nullptr));
#endif // QT_CONFIG(shortcut)
        actionUpdateChannel->setText(QCoreApplication::translate("MainWindow", "&Update Channel", nullptr));
#if QT_CONFIG(shortcut)
        actionUpdateChannel->setShortcut(QCoreApplication::translate("MainWindow", "Shift+F7", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDeleteChannel->setText(QCoreApplication::translate("MainWindow", "&Delete Channel", nullptr));
#if QT_CONFIG(shortcut)
        actionDeleteChannel->setShortcut(QCoreApplication::translate("MainWindow", "F8", nullptr));
#endif // QT_CONFIG(shortcut)
        actionJoinChannel->setText(QCoreApplication::translate("MainWindow", "&Join Channel", nullptr));
#if QT_CONFIG(shortcut)
        actionJoinChannel->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+J", nullptr));
#endif // QT_CONFIG(shortcut)
        actionEnableVoiceActivation->setText(QCoreApplication::translate("MainWindow", "Enable Voice &Activation", nullptr));
#if QT_CONFIG(shortcut)
        actionEnableVoiceActivation->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+A", nullptr));
#endif // QT_CONFIG(shortcut)
        actionChangeNickname->setText(QCoreApplication::translate("MainWindow", "Change &Nickname", nullptr));
#if QT_CONFIG(shortcut)
        actionChangeNickname->setShortcut(QCoreApplication::translate("MainWindow", "F5", nullptr));
#endif // QT_CONFIG(shortcut)
        actionChangeStatus->setText(QCoreApplication::translate("MainWindow", "Change &Status", nullptr));
#if QT_CONFIG(shortcut)
        actionChangeStatus->setShortcut(QCoreApplication::translate("MainWindow", "F6", nullptr));
#endif // QT_CONFIG(shortcut)
        actionViewUserInformation->setText(QCoreApplication::translate("MainWindow", "&View User Information", nullptr));
#if QT_CONFIG(shortcut)
        actionViewUserInformation->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+I", nullptr));
#endif // QT_CONFIG(shortcut)
        actionMuteAll->setText(QCoreApplication::translate("MainWindow", "Mute &All", nullptr));
#if QT_CONFIG(shortcut)
        actionMuteAll->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+M", nullptr));
#endif // QT_CONFIG(shortcut)
        actionUploadFile->setText(QCoreApplication::translate("MainWindow", "Up&load File", nullptr));
#if QT_CONFIG(shortcut)
        actionUploadFile->setShortcut(QCoreApplication::translate("MainWindow", "Shift+F5", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDownloadFile->setText(QCoreApplication::translate("MainWindow", "D&ownload File", nullptr));
#if QT_CONFIG(shortcut)
        actionDownloadFile->setShortcut(QCoreApplication::translate("MainWindow", "Shift+F6", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDeleteFile->setText(QCoreApplication::translate("MainWindow", "Dele&te File", nullptr));
#if QT_CONFIG(shortcut)
        actionDeleteFile->setShortcut(QCoreApplication::translate("MainWindow", "Shift+Del", nullptr));
#endif // QT_CONFIG(shortcut)
        actionBannedUsers->setText(QCoreApplication::translate("MainWindow", "&Banned Users", nullptr));
#if QT_CONFIG(shortcut)
        actionBannedUsers->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+B", nullptr));
#endif // QT_CONFIG(shortcut)
        actionUserAccounts->setText(QCoreApplication::translate("MainWindow", "&User Accounts", nullptr));
#if QT_CONFIG(shortcut)
        actionUserAccounts->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+L", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSaveConfiguration->setText(QCoreApplication::translate("MainWindow", "&Save Configuration", nullptr));
#if QT_CONFIG(shortcut)
        actionSaveConfiguration->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionViewChannelInfo->setText(QCoreApplication::translate("MainWindow", "&View Channel Info", nullptr));
#if QT_CONFIG(shortcut)
        actionViewChannelInfo->setShortcut(QCoreApplication::translate("MainWindow", "F3", nullptr));
#endif // QT_CONFIG(shortcut)
        actionMessages->setText(QCoreApplication::translate("MainWindow", "M&essages", nullptr));
#if QT_CONFIG(shortcut)
        actionMessages->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+E", nullptr));
#endif // QT_CONFIG(shortcut)
        actionKickBan->setText(QCoreApplication::translate("MainWindow", "Kick and &Ban From Server", nullptr));
#if QT_CONFIG(shortcut)
        actionKickBan->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+K", nullptr));
#endif // QT_CONFIG(shortcut)
        actionOp->setText(QCoreApplication::translate("MainWindow", "&Op", nullptr));
#if QT_CONFIG(shortcut)
        actionOp->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionVolume->setText(QCoreApplication::translate("MainWindow", "&Volume", nullptr));
#if QT_CONFIG(shortcut)
        actionVolume->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+U", nullptr));
#endif // QT_CONFIG(shortcut)
        actionVoice->setText(QCoreApplication::translate("MainWindow", "V&oice", nullptr));
#if QT_CONFIG(shortcut)
        actionVoice->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+4", nullptr));
#endif // QT_CONFIG(shortcut)
        actionUserMessages->setText(QCoreApplication::translate("MainWindow", "&User Messages", nullptr));
#if QT_CONFIG(shortcut)
        actionUserMessages->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+1", nullptr));
#endif // QT_CONFIG(shortcut)
        actionChannelMessages->setText(QCoreApplication::translate("MainWindow", "&Channel Messages", nullptr));
#if QT_CONFIG(shortcut)
        actionChannelMessages->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+2", nullptr));
#endif // QT_CONFIG(shortcut)
        actionVideo->setText(QCoreApplication::translate("MainWindow", "&Video", nullptr));
#if QT_CONFIG(shortcut)
        actionVideo->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+5", nullptr));
#endif // QT_CONFIG(shortcut)
        actionBroadcastMessages->setText(QCoreApplication::translate("MainWindow", "&Broadcast Messages", nullptr));
#if QT_CONFIG(shortcut)
        actionBroadcastMessages->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+3", nullptr));
#endif // QT_CONFIG(shortcut)
        actionEnableVideoTransmission->setText(QCoreApplication::translate("MainWindow", "Enable &Video Transmission", nullptr));
#if QT_CONFIG(shortcut)
        actionEnableVideoTransmission->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+V", nullptr));
#endif // QT_CONFIG(shortcut)
        actionStreamAudioFileToChannel->setText(QCoreApplication::translate("MainWindow", "&Stream Audio File to Channel", nullptr));
#if QT_CONFIG(shortcut)
        actionStreamAudioFileToChannel->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+W", nullptr));
#endif // QT_CONFIG(shortcut)
        actionStreamAudioFileToUser->setText(QCoreApplication::translate("MainWindow", "Stream Audio &File to User", nullptr));
#if QT_CONFIG(shortcut)
        actionStreamAudioFileToUser->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+W", nullptr));
#endif // QT_CONFIG(shortcut)
        actionMediaStorage->setText(QCoreApplication::translate("MainWindow", "Record Conversations to &Disk", nullptr));
#if QT_CONFIG(tooltip)
        actionMediaStorage->setToolTip(QCoreApplication::translate("MainWindow", "Specify a folder where audio from users will be stored", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionMediaStorage->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+A", nullptr));
#endif // QT_CONFIG(shortcut)
        actionInterceptUserMessages->setText(QCoreApplication::translate("MainWindow", "Intercept User Messages", nullptr));
#if QT_CONFIG(shortcut)
        actionInterceptUserMessages->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+1", nullptr));
#endif // QT_CONFIG(shortcut)
        actionInterceptChannelMessages->setText(QCoreApplication::translate("MainWindow", "Intercept Channel Messages", nullptr));
#if QT_CONFIG(shortcut)
        actionInterceptChannelMessages->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+2", nullptr));
#endif // QT_CONFIG(shortcut)
        actionInterceptVoice->setText(QCoreApplication::translate("MainWindow", "Intercept Audio", nullptr));
#if QT_CONFIG(shortcut)
        actionInterceptVoice->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+4", nullptr));
#endif // QT_CONFIG(shortcut)
        actionInterceptVideo->setText(QCoreApplication::translate("MainWindow", "Intercept Video", nullptr));
#if QT_CONFIG(shortcut)
        actionInterceptVideo->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+5", nullptr));
#endif // QT_CONFIG(shortcut)
        actionBroadcastMessage->setText(QCoreApplication::translate("MainWindow", "&Broadcast Message", nullptr));
#if QT_CONFIG(shortcut)
        actionBroadcastMessage->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+B", nullptr));
#endif // QT_CONFIG(shortcut)
        actionIncreaseVoiceVolume->setText(QCoreApplication::translate("MainWindow", "&Increase Voice Volume", nullptr));
#if QT_CONFIG(shortcut)
        actionIncreaseVoiceVolume->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Right", nullptr));
#endif // QT_CONFIG(shortcut)
        actionLowerVoiceVolume->setText(QCoreApplication::translate("MainWindow", "&Lower Voice Volume", nullptr));
#if QT_CONFIG(shortcut)
        actionLowerVoiceVolume->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+F7", nullptr));
#endif // QT_CONFIG(shortcut)
        actionStoreForMove->setText(QCoreApplication::translate("MainWindow", "&Store User(s) for Move", nullptr));
#if QT_CONFIG(shortcut)
        actionStoreForMove->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+X", nullptr));
#endif // QT_CONFIG(shortcut)
        actionMoveUser->setText(QCoreApplication::translate("MainWindow", "&Move User(s)", nullptr));
#if QT_CONFIG(shortcut)
        actionMoveUser->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+V", nullptr));
#endif // QT_CONFIG(shortcut)
        actionVisitBearWare->setText(QCoreApplication::translate("MainWindow", "&BearWare.dk Website", nullptr));
        actionVisitChangeLog->setText(QCoreApplication::translate("MainWindow", "&Change Log", nullptr));
        actionAllowVoiceTransmission->setText(QCoreApplication::translate("MainWindow", "Allow V&oice Transmission", nullptr));
#if QT_CONFIG(shortcut)
        actionAllowVoiceTransmission->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAllowVideoTransmission->setText(QCoreApplication::translate("MainWindow", "Allow Video Transmission", nullptr));
#if QT_CONFIG(shortcut)
        actionAllowVideoTransmission->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+W", nullptr));
#endif // QT_CONFIG(shortcut)
        actionServerStatistics->setText(QCoreApplication::translate("MainWindow", "Server S&tatistics", nullptr));
#if QT_CONFIG(shortcut)
        actionServerStatistics->setShortcut(QCoreApplication::translate("MainWindow", "Shift+F9", nullptr));
#endif // QT_CONFIG(shortcut)
        actionNewClient->setText(QCoreApplication::translate("MainWindow", "&New Client Instance", nullptr));
#if QT_CONFIG(shortcut)
        actionNewClient->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionOnlineUsers->setText(QCoreApplication::translate("MainWindow", "&Online Users", nullptr));
#if QT_CONFIG(shortcut)
        actionOnlineUsers->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+U", nullptr));
#endif // QT_CONFIG(shortcut)
        actionEnableQuestionMode->setText(QCoreApplication::translate("MainWindow", "Enable Question Mode", nullptr));
#if QT_CONFIG(tooltip)
        actionEnableQuestionMode->setToolTip(QCoreApplication::translate("MainWindow", "Show blinking icon", nullptr));
#endif // QT_CONFIG(tooltip)
        actionEnableDesktopSharing->setText(QCoreApplication::translate("MainWindow", "Enable Desktop Sharing", nullptr));
#if QT_CONFIG(shortcut)
        actionEnableDesktopSharing->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+D", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAllowDesktopTransmission->setText(QCoreApplication::translate("MainWindow", "Allow Desktop Transmission", nullptr));
#if QT_CONFIG(shortcut)
        actionAllowDesktopTransmission->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+E", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDesktop->setText(QCoreApplication::translate("MainWindow", "&Desktop", nullptr));
#if QT_CONFIG(shortcut)
        actionDesktop->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+6", nullptr));
#endif // QT_CONFIG(shortcut)
        actionInterceptDesktop->setText(QCoreApplication::translate("MainWindow", "Intercept Desktop", nullptr));
#if QT_CONFIG(shortcut)
        actionInterceptDesktop->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+6", nullptr));
#endif // QT_CONFIG(shortcut)
        actionStreamMediaFileToChannel->setText(QCoreApplication::translate("MainWindow", "Stream &Media File to Channel", nullptr));
#if QT_CONFIG(shortcut)
        actionStreamMediaFileToChannel->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDesktopInput->setText(QCoreApplication::translate("MainWindow", "Desktop Acce&ss", nullptr));
        actionDesktopAccessAllow->setText(QCoreApplication::translate("MainWindow", "Allow Desktop Access", nullptr));
#if QT_CONFIG(shortcut)
        actionDesktopAccessAllow->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+7", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDesktopAccessDeny->setText(QCoreApplication::translate("MainWindow", "&Deny", nullptr));
        actionMediaFile->setText(QCoreApplication::translate("MainWindow", "Media File Stream", nullptr));
#if QT_CONFIG(shortcut)
        actionMediaFile->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+8", nullptr));
#endif // QT_CONFIG(shortcut)
        actionInterceptMediaFile->setText(QCoreApplication::translate("MainWindow", "Intercept Media File Stream", nullptr));
#if QT_CONFIG(shortcut)
        actionInterceptMediaFile->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+8", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAllowMediaFileTransmission->setText(QCoreApplication::translate("MainWindow", "Allow Media File Transmission", nullptr));
#if QT_CONFIG(shortcut)
        actionAllowMediaFileTransmission->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+R", nullptr));
#endif // QT_CONFIG(shortcut)
        actionMuteVoice->setText(QCoreApplication::translate("MainWindow", "Mu&te Voice", nullptr));
#if QT_CONFIG(shortcut)
        actionMuteVoice->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+M", nullptr));
#endif // QT_CONFIG(shortcut)
        actionMuteMediaFile->setText(QCoreApplication::translate("MainWindow", "Mute Media File", nullptr));
#if QT_CONFIG(shortcut)
        actionMuteMediaFile->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+\316\234", nullptr));
#endif // QT_CONFIG(shortcut)
        actionKickFromChannel->setText(QCoreApplication::translate("MainWindow", "Kick From Channel", nullptr));
#if QT_CONFIG(shortcut)
        actionKickFromChannel->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+K", nullptr));
#endif // QT_CONFIG(shortcut)
        actionKickFromServer->setText(QCoreApplication::translate("MainWindow", "Kick From Server", nullptr));
#if QT_CONFIG(shortcut)
        actionKickFromServer->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+K", nullptr));
#endif // QT_CONFIG(shortcut)
        actionIncreaseMediaFileVolume->setText(QCoreApplication::translate("MainWindow", "Increase Media File Volume", nullptr));
#if QT_CONFIG(shortcut)
        actionIncreaseMediaFileVolume->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+F6", nullptr));
#endif // QT_CONFIG(shortcut)
        actionLowerMediaFileVolume->setText(QCoreApplication::translate("MainWindow", "Lower Media File Volume", nullptr));
#if QT_CONFIG(shortcut)
        actionLowerMediaFileVolume->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+F5", nullptr));
#endif // QT_CONFIG(shortcut)
        actionResetPreferencesToDefault->setText(QCoreApplication::translate("MainWindow", "R&eset Preferences to Default", nullptr));
        actionKickAndBanFromChannel->setText(QCoreApplication::translate("MainWindow", "Kick and Ban From &Channel", nullptr));
#if QT_CONFIG(shortcut)
        actionKickAndBanFromChannel->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+B", nullptr));
#endif // QT_CONFIG(shortcut)
        actionBannedUsersInChannel->setText(QCoreApplication::translate("MainWindow", "Banned Users From Channel", nullptr));
#if QT_CONFIG(shortcut)
        actionBannedUsersInChannel->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Alt+Shift+B", nullptr));
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(accessibility)
        channelsWidget->setAccessibleName(QCoreApplication::translate("MainWindow", "Channel list", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(tooltip)
        label->setToolTip(QCoreApplication::translate("MainWindow", "Master volume", nullptr));
#endif // QT_CONFIG(tooltip)
        label->setText(QString());
#if QT_CONFIG(tooltip)
        volumeSlider->setToolTip(QCoreApplication::translate("MainWindow", "Master volume", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(accessibility)
        volumeSlider->setAccessibleName(QCoreApplication::translate("MainWindow", "Master volume", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(tooltip)
        label_3->setToolTip(QCoreApplication::translate("MainWindow", "Voice level", nullptr));
#endif // QT_CONFIG(tooltip)
        label_3->setText(QString());
#if QT_CONFIG(tooltip)
        voiceactBar->setToolTip(QCoreApplication::translate("MainWindow", "Voice level", nullptr));
#endif // QT_CONFIG(tooltip)
        voiceactBar->setFormat(QString());
#if QT_CONFIG(tooltip)
        label_2->setToolTip(QCoreApplication::translate("MainWindow", "Microphone gain", nullptr));
#endif // QT_CONFIG(tooltip)
        label_2->setText(QString());
#if QT_CONFIG(tooltip)
        micSlider->setToolTip(QCoreApplication::translate("MainWindow", "Microphone gain", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(accessibility)
        micSlider->setAccessibleName(QCoreApplication::translate("MainWindow", "Microphone gain", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(tooltip)
        label_4->setToolTip(QCoreApplication::translate("MainWindow", "Voice activation level", nullptr));
#endif // QT_CONFIG(tooltip)
        label_4->setText(QString());
#if QT_CONFIG(tooltip)
        voiceactSlider->setToolTip(QCoreApplication::translate("MainWindow", "Voice activation level", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(accessibility)
        voiceactSlider->setAccessibleName(QCoreApplication::translate("MainWindow", "Voice activation level", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(tooltip)
        tabWidget->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        chatTab->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(accessibility)
        chatTab->setAccessibleName(QString());
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        chatTab->setAccessibleDescription(QString());
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        chatEdit->setAccessibleName(QCoreApplication::translate("MainWindow", "History", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        msgEdit->setAccessibleName(QCoreApplication::translate("MainWindow", "Message", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(tooltip)
        sendButton->setToolTip(QCoreApplication::translate("MainWindow", "Send text message", nullptr));
#endif // QT_CONFIG(tooltip)
        sendButton->setText(QCoreApplication::translate("MainWindow", "Sen&d", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(chatTab), QCoreApplication::translate("MainWindow", "Ch&at", nullptr));
#if QT_CONFIG(accessibility)
        videoTab->setAccessibleName(QString());
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        videoTab->setAccessibleDescription(QString());
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(tooltip)
        detachVideoButton->setToolTip(QCoreApplication::translate("MainWindow", "Detach User Video", nullptr));
#endif // QT_CONFIG(tooltip)
        detachVideoButton->setText(QCoreApplication::translate("MainWindow", "Detach User Video", nullptr));
#if QT_CONFIG(tooltip)
        addVideoButton->setToolTip(QCoreApplication::translate("MainWindow", "Add User to Video Grid", nullptr));
#endif // QT_CONFIG(tooltip)
        addVideoButton->setText(QCoreApplication::translate("MainWindow", "Add User to Video Grid", nullptr));
#if QT_CONFIG(tooltip)
        removeVideoButton->setToolTip(QCoreApplication::translate("MainWindow", "Remove User From Video Grid", nullptr));
#endif // QT_CONFIG(tooltip)
        removeVideoButton->setText(QCoreApplication::translate("MainWindow", "Remove User From Video Grid", nullptr));
#if QT_CONFIG(tooltip)
        initVideoButton->setToolTip(QCoreApplication::translate("MainWindow", "Start/Stop Webcam", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(accessibility)
        videochatEdit->setAccessibleName(QCoreApplication::translate("MainWindow", "History", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        videomsgEdit->setAccessibleName(QCoreApplication::translate("MainWindow", "Message", nullptr));
#endif // QT_CONFIG(accessibility)
        videosendButton->setText(QCoreApplication::translate("MainWindow", "Sen&d", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(videoTab), QCoreApplication::translate("MainWindow", "&Video", nullptr));
#if QT_CONFIG(tooltip)
        detachDesktopButton->setToolTip(QCoreApplication::translate("MainWindow", "Detach selected window", nullptr));
#endif // QT_CONFIG(tooltip)
        detachDesktopButton->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        addDesktopButton->setToolTip(QCoreApplication::translate("MainWindow", "Put back removed window", nullptr));
#endif // QT_CONFIG(tooltip)
        addDesktopButton->setText(QString());
#if QT_CONFIG(tooltip)
        removeDesktopButton->setToolTip(QCoreApplication::translate("MainWindow", "Remove selected window", nullptr));
#endif // QT_CONFIG(tooltip)
        removeDesktopButton->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        desktopaccessButton->setToolTip(QCoreApplication::translate("MainWindow", "Request desktop access", nullptr));
#endif // QT_CONFIG(tooltip)
        desktopaccessButton->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(accessibility)
        desktopchatEdit->setAccessibleName(QCoreApplication::translate("MainWindow", "History", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(accessibility)
        desktopmsgEdit->setAccessibleName(QCoreApplication::translate("MainWindow", "Message", nullptr));
#endif // QT_CONFIG(accessibility)
        desktopsendButton->setText(QCoreApplication::translate("MainWindow", "Sen&d", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(desktopTab), QCoreApplication::translate("MainWindow", "&Desktops", nullptr));
        channelLabel->setText(QString());
#if QT_CONFIG(accessibility)
        filesView->setAccessibleName(QCoreApplication::translate("MainWindow", "Files list", nullptr));
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(tooltip)
        uploadButton->setToolTip(QCoreApplication::translate("MainWindow", "Upload", nullptr));
#endif // QT_CONFIG(tooltip)
        uploadButton->setText(QCoreApplication::translate("MainWindow", "Upload", nullptr));
#if QT_CONFIG(tooltip)
        downloadButton->setToolTip(QCoreApplication::translate("MainWindow", "Download", nullptr));
#endif // QT_CONFIG(tooltip)
        downloadButton->setText(QCoreApplication::translate("MainWindow", "Download", nullptr));
#if QT_CONFIG(tooltip)
        deleteButton->setToolTip(QCoreApplication::translate("MainWindow", "Delete", nullptr));
#endif // QT_CONFIG(tooltip)
        deleteButton->setText(QCoreApplication::translate("MainWindow", "Delete", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(filesTab), QCoreApplication::translate("MainWindow", "&Files", nullptr));
        menuClient->setTitle(QCoreApplication::translate("MainWindow", "C&lient", nullptr));
        menuMe->setTitle(QCoreApplication::translate("MainWindow", "&Me", nullptr));
        menuUsers->setTitle(QCoreApplication::translate("MainWindow", "&Users", nullptr));
        menuSubscriptions->setTitle(QCoreApplication::translate("MainWindow", "&Subscriptions", nullptr));
        menu_Advanced->setTitle(QCoreApplication::translate("MainWindow", "&Advanced", nullptr));
        menuMute->setTitle(QCoreApplication::translate("MainWindow", "&Mute", nullptr));
        menu_Kick->setTitle(QCoreApplication::translate("MainWindow", "&Kick", nullptr));
        menuChannels->setTitle(QCoreApplication::translate("MainWindow", "&Channels", nullptr));
        menuServer->setTitle(QCoreApplication::translate("MainWindow", "&Server", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "&Help", nullptr));
        (void)MainWindow;
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
