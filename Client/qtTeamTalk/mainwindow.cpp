/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#include "mainwindow.h"
#include "serverlistdlg.h"
#include "serverpropertiesdlg.h"
#include "preferencesdlg.h"
#include "settings.h"
#include "channeldlg.h"
#include "userinfodlg.h"
#include "bannedusersdlg.h"
#include "useraccountsdlg.h"
#include "keycompdlg.h"
#include "textmessagedlg.h"
#include "filetransferdlg.h"
#include "uservolumedlg.h"
#include "changestatusdlg.h"
#include "aboutdlg.h"
#include "mediastoragedlg.h"
#include "channelstree.h"
#include "desktopsharedlg.h"
#include "streammediafiledlg.h"
#include "onlineusersdlg.h"
#include "serverstatsdlg.h"
#include "filesmodel.h"
#include "uservideodlg.h"
#include "userdesktopdlg.h"
#include "appinfo.h"
#include "weblogindlg.h"
#include "bearwarelogindlg.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QTimer>
#include <QSettings>
#include <QDesktopWidget>
#include <QFile>
#include <QTranslator>
#include <QUrl>
#include <QTabWidget>
#include <QHeaderView>
#include <QRegExp>
#include <QDesktopServices>
#include <QProcess>
#include <QNetworkRequest>
#include <QNetworkReply>

#ifdef Q_OS_LINUX //For hotkeys on X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#include <functional>
#include <algorithm>

#ifdef _MSC_VER
#pragma warning(disable:4800)
#endif

extern TTInstance* ttInst;

QSettings* ttSettings = nullptr;
QTranslator* ttTranslator = nullptr;

//strip ampersand from menutext
#define MENUTEXT(text) text.replace("&", "")

MainWindow::MainWindow(const QString& cfgfile)
: m_sysicon(nullptr)
, m_sysmenu(nullptr)
, m_current_cmdid(0)
, m_audiostorage_mode(AUDIOSTORAGE_NONE)
, m_idled_out(false)
, m_statusmode(STATUSMODE_AVAILABLE)
, m_myuseraccount()
, m_clientstats()
, m_last_channel()
, m_srvprop()
, m_mychannel()
, m_onlineusersdlg(nullptr)
, m_useraccountsdlg(nullptr)
, m_bannedusersdlg(nullptr)
, m_serverstatsdlg(nullptr)
, m_desktopsession_id(0)
, m_prev_desktopsession_id(0)
, m_desktopsession_total(0)
, m_desktopsession_remain(0)
, m_desktopsend_on_completion(false)
#if defined(Q_OS_WIN32)
, m_hShareWnd(nullptr)
#elif defined(Q_OS_DARWIN)
, m_nCGShareWnd(kCGNullWindowID)
#elif defined(Q_OS_LINUX)
, m_display(nullptr)
, m_nWindowShareWnd(0)
#endif
{
    //Ensure the correct version of the DLL is loaded
    if(QString(TEAMTALK_VERSION) != _Q(TT_GetVersion()))
        QMessageBox::warning(nullptr, ("DLL load error"),
                             QString("This %3 executable is built for DLL "
                                      "version %1 but the loaded DLL reports "
                                      "it's version %2. Loading an incorrent "
                                      "DLL for %3 may cause problems and crash "
                                      "the application. Please reinstall to "
                                      "solve this problem.")
                                     .arg(TEAMTALK_VERSION).
                                     arg(_Q(TT_GetVersion())).
                                     arg(APPNAME_SHORT));

    QApplication::setOrganizationName(COMPANYNAME);
    QApplication::setApplicationName(APPNAME_SHORT);
    
    //figure out where to load settings from
    QString inipath = QApplication::applicationDirPath();
    inipath += "/" + QString(APPINIFILE);
    //test if cfg is provided by user
    if(cfgfile.size())
    {
        inipath = QDir::fromNativeSeparators(cfgfile);
        if(!QFile::exists(inipath))
            QFile(inipath).open(QIODevice::WriteOnly);
    }

    if(QFile::exists(inipath)) //first try same dir as executable
        ttSettings = new QSettings(inipath, QSettings::IniFormat, this);
    else
    {
        //load from system default user settings
        ttSettings = new QSettings(QSettings::IniFormat, 
                                   QSettings::UserScope,
                                   QApplication::organizationName(),
                                   QApplication::applicationName(), this);

        if (!QFile::exists(ttSettings->fileName()))
        {
            //copy settings from defaults file
            QString defpath = QString(APPDEFAULTINIFILE);
            QSettings defaultSettings(defpath, QSettings::IniFormat, this);
            QStringList keys = defaultSettings.allKeys();
            foreach(QString key, keys)
            {
                ttSettings->setValue(key, defaultSettings.value(key));
            }
        }
    }

    ui.setupUi(this);

    setWindowIcon(QIcon(APPICON));
    updateWindowTitle();

    m_filesmodel = new FilesModel(this);
    ui.filesView->setModel(m_filesmodel);
    QItemSelectionModel* selmodel = ui.filesView->selectionModel();

    ui.volumeSlider->setRange(0, 100);
    ui.micSlider->setRange(0, 100);
    ui.voiceactBar->setRange(SOUND_VU_MIN, DEFAULT_SOUND_VU_MAX /*SOUND_VU_MAX*/);
    ui.voiceactSlider->setRange(SOUND_VU_MIN, DEFAULT_SOUND_VU_MAX /*SOUND_VU_MAX*/);

    m_pinglabel = new QLabel(ui.statusbar);
    m_pinglabel->setAlignment(Qt::AlignHCenter);
    m_pttlabel = new QLabel(ui.statusbar);
    m_pttlabel->setAlignment(Qt::AlignHCenter);
    m_dtxlabel = new QLabel(ui.statusbar);
    QPixmap dtxImg(QString::fromUtf8(":/images/images/desktoptx.png"));
    m_dtxlabel->setPixmap(dtxImg.scaled(16, 16));
    m_dtxprogress = new QProgressBar(ui.statusbar);
    m_dtxprogress->setMaximumWidth(50);
    m_dtxprogress->setMaximumHeight(16);
    m_dtxprogress->setTextVisible(false);
    m_dtxlabel->hide();
    m_dtxprogress->hide();
    ui.statusbar->addPermanentWidget(m_dtxlabel);
    ui.statusbar->addPermanentWidget(m_dtxprogress);
    ui.statusbar->addPermanentWidget(m_pinglabel);
    ui.statusbar->addPermanentWidget(m_pttlabel);

    connect(ui.sendButton, SIGNAL(clicked()),
            SLOT(slotSendChannelMessage()));
    connect(ui.msgEdit, SIGNAL(returnPressed()),
            SLOT(slotSendChannelMessage()));
    connect(ui.videosendButton, SIGNAL(clicked()),
            SLOT(slotSendChannelMessage()));
    connect(ui.desktopsendButton, SIGNAL(clicked()),
            SLOT(slotSendChannelMessage()));
    connect(ui.videomsgEdit, SIGNAL(returnPressed()),
            SLOT(slotSendChannelMessage()));
    connect(ui.desktopmsgEdit, SIGNAL(returnPressed()),
            SLOT(slotSendChannelMessage()));
    connect(ui.micSlider, SIGNAL(valueChanged(int)), 
            SLOT(slotMicrophoneGainChanged(int)));
    connect(ui.volumeSlider, SIGNAL(valueChanged(int)),
            SLOT(slotMasterVolumeChanged(int)));
    connect(ui.voiceactSlider, SIGNAL(valueChanged(int)),
            SLOT(slotVoiceActivationLevelChanged(int)));

    /* ui.channelsWidget */
    connect(ui.channelsWidget, SIGNAL(itemSelectionChanged()),
            SLOT(slotTreeSelectionChanged()));
    connect(ui.channelsWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
            SLOT(slotTreeContextMenu(const QPoint&)));
    connect(ui.channelsWidget, SIGNAL(userDoubleClicked(int)), 
            SLOT(slotUserDoubleClicked(int)));
    connect(ui.channelsWidget, SIGNAL(channelDoubleClicked(int)),
            SLOT(slotChannelDoubleClicked(int)));
    connect(ui.channelsWidget, SIGNAL(fileDropped(const QString&)),
            SLOT(slotLoadTTFile(const QString&)));
    connect(ui.channelsWidget,
            SIGNAL(transmitusersChanged(int, const QMap<int,StreamTypes>&)),
            SLOT(slotTransmitUsersChanged(int, const QMap<int,StreamTypes>&)));
    /* Video-tab (video-grid) */
    connect(this, SIGNAL(newVideoCaptureFrame(int,int)), ui.videogridWidget, 
            SLOT(slotNewVideoFrame(int,int)));
    connect(this, SIGNAL(newMediaVideoFrame(int,int)), ui.videogridWidget, 
            SLOT(slotNewVideoFrame(int,int)));
    connect(ui.videogridWidget, SIGNAL(userVideoEnded(int)),
            SLOT(slotRemoveUserVideo(int)));
    connect(ui.videogridWidget, SIGNAL(videoCountChanged(int)),
            SLOT(slotUpdateVideoCount(int)));
    connect(ui.videogridWidget, SIGNAL(videoCountChanged(int)),
            SLOT(slotUpdateVideoTabUI()));
    connect(ui.videogridWidget, SIGNAL(userVideoSelected(bool)),
            SLOT(slotUpdateVideoTabUI()));            
    /* Desktop-tab (desktop-grid) */
    connect(this, SIGNAL(newDesktopWindow(int,int)), ui.desktopgridWidget,
            SIGNAL(userDesktopWindowUpdate(int,int)));
    connect(this, SIGNAL(userDesktopCursor(int,const DesktopInput&)),
            ui.desktopgridWidget, SIGNAL(userDesktopCursorUpdate(int,const DesktopInput&)));
    connect(this, SIGNAL(userUpdate(const User&)), ui.desktopgridWidget,
            SIGNAL(userUpdated(const User&)));
    connect(this, SIGNAL(userLeft(int,const User&)), ui.desktopgridWidget,
            SLOT(slotRemoveUser(int,const User&)));
    connect(ui.desktopgridWidget, SIGNAL(userDesktopWindowEnded(int)),
            SLOT(slotRemoveUserDesktop(int)));
    connect(ui.desktopgridWidget, SIGNAL(desktopCountChanged(int)),
            SLOT(slotUpdateDesktopCount(int)));
    connect(ui.desktopgridWidget, SIGNAL(desktopCountChanged(int)),
            SLOT(slotUpdateDesktopTabUI()));
    connect(ui.desktopgridWidget, SIGNAL(userDesktopSelected(bool)),
            SLOT(slotUpdateDesktopTabUI()));            
    /* Files-tab */
    connect(ui.uploadButton, SIGNAL(clicked()), SLOT(slotChannelsUploadFile()));
    connect(ui.downloadButton, SIGNAL(clicked()), SLOT(slotChannelsDownloadFile()));
    connect(ui.deleteButton, SIGNAL(clicked()), SLOT(slotChannelsDeleteFile()));
    connect(selmodel, SIGNAL(selectionChanged(const QItemSelection&, 
                                              const QItemSelection&)),
            ui.filesView, SLOT(slotNewSelection(const QItemSelection&)));
    connect(ui.filesView, SIGNAL(filesSelected(bool)), ui.actionDeleteFile, 
            SLOT(setEnabled(bool)));
    connect(ui.filesView, SIGNAL(filesSelected(bool)), ui.deleteButton, 
            SLOT(setEnabled(bool)));
    connect(ui.filesView, SIGNAL(uploadFiles(const QStringList&)),
            SLOT(slotUploadFiles(const QStringList&)));
    /* Video-tab buttons */
    connect(ui.initVideoButton, SIGNAL(clicked(bool)), 
            ui.actionEnableVideoTransmission, SIGNAL(triggered(bool)));
    connect(ui.addVideoButton, SIGNAL(clicked()), SLOT(slotAddUserVideo()));
    connect(ui.removeVideoButton, SIGNAL(clicked()), SLOT(slotRemoveUserVideo()));
    connect(ui.detachVideoButton, SIGNAL(clicked()), SLOT(slotDetachUserVideo()));
    connect(ui.videogridWidget, SIGNAL(userVideoSelected(bool)),
            ui.detachVideoButton, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(preferencesModified()),
            ui.videogridWidget, SIGNAL(preferencesModified()));
    /* Desktop-tab buttons */
    connect(ui.detachDesktopButton, SIGNAL(clicked()),
            SLOT(slotDetachUserDesktop()));
    connect(ui.addDesktopButton, SIGNAL(clicked()),
            SLOT(slotAddUserDesktop()));
    connect(ui.removeDesktopButton, SIGNAL(clicked()),
            SLOT(slotRemoveUserDesktop()));
    connect(ui.desktopaccessButton, SIGNAL(clicked(bool)),
            SLOT(slotAccessUserDesktop(bool)));
    connect(ui.desktopgridWidget, SIGNAL(userDesktopSelected(bool)),
            ui.detachDesktopButton, SLOT(setEnabled(bool)));
    connect(ui.desktopgridWidget, SIGNAL(userDesktopSelected(bool)),
            ui.desktopaccessButton, SLOT(setEnabled(bool)));

    /* Begin - File menu */
    connect(ui.actionNewClient, SIGNAL(triggered(bool)),
            SLOT(slotClientNewInstance(bool)));
    connect(ui.actionConnect, SIGNAL(triggered(bool)),
            SLOT(slotClientConnect(bool)));
    connect(ui.actionPreferences, SIGNAL(triggered(bool)),
            SLOT(slotClientPreferences(bool)));
    connect(ui.actionExit, SIGNAL(triggered(bool)),
            SLOT(slotClientExit(bool)));
    /* End - File menu */

    /* Begin - Me menu */
    connect(ui.actionChangeNickname, SIGNAL(triggered(bool)),
            SLOT(slotMeChangeNickname(bool)));
    connect(ui.actionChangeStatus, SIGNAL(triggered(bool)),
            SLOT(slotMeChangeStatus(bool)));
    connect(ui.actionEnablePushToTalk, SIGNAL(triggered(bool)),
            SLOT(slotMeEnablePushToTalk(bool)));
    connect(ui.actionEnableVoiceActivation, SIGNAL(triggered(bool)),
            SLOT(slotMeEnableVoiceActivation(bool)));
    connect(ui.actionEnableVideoTransmission, SIGNAL(triggered(bool)),
            SLOT(slotMeEnableVideoTransmission(bool)));
    connect(ui.actionEnableDesktopSharing, SIGNAL(triggered(bool)),
            SLOT(slotMeEnableDesktopSharing(bool)));
    /* End - Me menu */

    /* Begin - Users menu */
    connect(ui.actionViewUserInformation, SIGNAL(triggered(bool)),
            SLOT(slotUsersViewUserInformation(bool)));
    connect(ui.actionMessages, SIGNAL(triggered(bool)),
            SLOT(slotUsersMessages(bool)));
    connect(ui.actionMuteVoice, SIGNAL(triggered(bool)),
            SLOT(slotUsersMuteVoice(bool)));
    connect(ui.actionMuteMediaFile, SIGNAL(triggered(bool)),
            SLOT(slotUsersMuteMediaFile(bool)));
    connect(ui.actionVolume, SIGNAL(triggered(bool)),
            SLOT(slotUsersVolume(bool)));
    connect(ui.actionOp, SIGNAL(triggered(bool)),
            SLOT(slotUsersOp(bool)));
    connect(ui.actionKickFromChannel, SIGNAL(triggered(bool)),
            SLOT(slotUsersKickFromChannel(bool)));
    connect(ui.actionKickAndBanFromChannel, SIGNAL(triggered(bool)),
        SLOT(slotUsersKickBanFromChannel(bool)));
    connect(ui.actionKickFromServer, SIGNAL(triggered(bool)),
            SLOT(slotUsersKickFromServer(bool)));
    connect(ui.actionKickBan, SIGNAL(triggered(bool)),
            SLOT(slotUsersKickBanFromServer(bool)));
    connect(ui.actionMuteAll, SIGNAL(triggered(bool)),
            SLOT(slotUsersMuteVoiceAll(bool)));
    connect(ui.actionMediaStorage, SIGNAL(triggered(bool)),
            SLOT(slotUsersStoreAudioToDisk(bool)));
    //Desktop access
    connect(ui.actionDesktopAccessAllow, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsDesktopInput(bool)));
    //subscriptions
    connect(ui.actionUserMessages, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsUserMsg(bool)));
    connect(ui.actionChannelMessages, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsChannelMsg(bool)));
    connect(ui.actionBroadcastMessages, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsBCastMsg(bool)));
    connect(ui.actionVoice, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsVoice(bool)));
    connect(ui.actionVideo, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsVideo(bool)));
    connect(ui.actionDesktop, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsDesktop(bool)));
    connect(ui.actionDesktopInput, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsDesktopInput(bool)));
    connect(ui.actionMediaFile, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsMediaFile(bool)));
    connect(ui.actionInterceptUserMessages, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsInterceptUserMsg(bool)));
    connect(ui.actionInterceptChannelMessages, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsInterceptChannelMsg(bool)));
    connect(ui.actionInterceptVoice, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsInterceptVoice(bool)));
    connect(ui.actionInterceptVideo, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsInterceptVideo(bool)));
    connect(ui.actionInterceptDesktop, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsInterceptDesktop(bool)));
    connect(ui.actionInterceptMediaFile, SIGNAL(triggered(bool)),
            SLOT(slotUsersSubscriptionsInterceptMediaFile(bool)));

    //advanced
    connect(ui.actionIncreaseVoiceVolume, SIGNAL(triggered()),
            SLOT(slotUsersAdvancedIncVolumeVoice()));
    connect(ui.actionLowerVoiceVolume, SIGNAL(triggered()),
            SLOT(slotUsersAdvancedDecVolumeVoice()));
    connect(ui.actionIncreaseMediaFileVolume, SIGNAL(triggered()),
            SLOT(slotUsersAdvancedIncVolumeMediaFile()));
    connect(ui.actionLowerMediaFileVolume, SIGNAL(triggered()),
            SLOT(slotUsersAdvancedDecVolumeMediaFile()));
    connect(ui.actionStoreForMove, SIGNAL(triggered()),
            SLOT(slotUsersAdvancedStoreForMove()));
    connect(ui.actionMoveUser, SIGNAL(triggered()),
            SLOT(slotUsersAdvancedMoveUsers()));
    connect(ui.actionAllowVoiceTransmission, SIGNAL(triggered(bool)),
            SLOT(slotUsersAdvancedVoiceAllowed(bool)));
    connect(ui.actionAllowVideoTransmission, SIGNAL(triggered(bool)),
            SLOT(slotUsersAdvancedVideoAllowed(bool)));
    connect(ui.actionAllowDesktopTransmission, SIGNAL(triggered(bool)),
            SLOT(slotUsersAdvancedDesktopAllowed(bool)));
    connect(ui.actionAllowMediaFileTransmission, SIGNAL(triggered(bool)),
            SLOT(slotUsersAdvancedMediaFileAllowed(bool)));
    /* End - Users menu */

    /* Begin - Channels menu */
    connect(ui.actionCreateChannel, SIGNAL(triggered(bool)),
            SLOT(slotChannelsCreateChannel(bool)));
    connect(ui.actionUpdateChannel, SIGNAL(triggered(bool)),
            SLOT(slotChannelsUpdateChannel(bool)));
    connect(ui.actionDeleteChannel, SIGNAL(triggered(bool)),
            SLOT(slotChannelsDeleteChannel(bool)));
    connect(ui.actionJoinChannel, SIGNAL(triggered(bool)),
            SLOT(slotChannelsJoinChannel(bool)));
    connect(ui.actionViewChannelInfo, SIGNAL(triggered(bool)),
            SLOT(slotChannelsViewChannelInfo(bool)));
    connect(ui.actionBannedUsersInChannel, SIGNAL(triggered(bool)),
            SLOT(slotChannelsListBans(bool)));

    connect(ui.actionStreamMediaFileToChannel, SIGNAL(triggered(bool)),
            SLOT(slotChannelsStreamMediaFile(bool)));
    connect(ui.actionUploadFile, SIGNAL(triggered(bool)),
            SLOT(slotChannelsUploadFile(bool)));
    connect(ui.actionDownloadFile, SIGNAL(triggered(bool)),
            SLOT(slotChannelsDownloadFile(bool)));
    connect(ui.actionDeleteFile, SIGNAL(triggered(bool)),
            SLOT(slotChannelsDeleteFile(bool)));
    /* End - Channels menu */

    /* Begin - Server menu */
    connect(ui.actionUserAccounts, SIGNAL(triggered(bool)),
            SLOT(slotServerUserAccounts(bool)));
    connect(ui.actionBannedUsers, SIGNAL(triggered(bool)),
            SLOT(slotServerBannedUsers(bool)));
    connect(ui.actionOnlineUsers, SIGNAL(triggered(bool)),
            SLOT(slotServerOnlineUsers(bool)));
    connect(ui.actionBroadcastMessage, SIGNAL(triggered(bool)),
            SLOT(slotServerBroadcastMessage(bool)));
    connect(ui.actionServerProperties, SIGNAL(triggered(bool)),
            SLOT(slotServerServerProperties(bool)));
    connect(ui.actionSaveConfiguration, SIGNAL(triggered(bool)),
            SLOT(slotServerSaveConfiguration(bool)));
    connect(ui.actionServerStatistics, SIGNAL(triggered(bool)),
            SLOT(slotServerServerStatistics(bool)));
    /* End - Server menu */

    /* Begin - Help menu */
    connect(ui.actionManual, SIGNAL(triggered(bool)),
            SLOT(slotHelpManual(bool)));
    connect(ui.actionResetPreferencesToDefault, SIGNAL(triggered(bool)),
            SLOT(slotHelpResetPreferences(bool)));
    connect(ui.actionVisitBearWare, SIGNAL(triggered(bool)),
            SLOT(slotHelpVisitBearWare(bool)));
    connect(ui.actionAbout, SIGNAL(triggered(bool)),
            SLOT(slotHelpAbout(bool)));
    /* End - Help menu */

    /* Begin - Extra toolbar buttons */
    connect(ui.actionEnableQuestionMode, SIGNAL(triggered(bool)),
            SLOT(slotEnableQuestionMode(bool)));
    /* End - Extra toolbar buttons */

    /* Begin - CLIENTEVENT_* messages */
    connect(this, SIGNAL(serverUpdate(const ServerProperties&)), ui.channelsWidget,
            SLOT(slotServerUpdate(const ServerProperties&)));
    connect(this, SIGNAL(newChannel(const Channel&)),
            ui.channelsWidget, SLOT(slotAddChannel(const Channel&)));
    connect(this, SIGNAL(updateChannel(const Channel&)),
            SLOT(slotChannelUpdate(const Channel&)));
    connect(this, SIGNAL(updateChannel(const Channel&)),
            ui.channelsWidget, SLOT(slotUpdateChannel(const Channel&)));
    connect(this, SIGNAL(removeChannel(const Channel&)),
            ui.channelsWidget, SLOT(slotRemoveChannel(const Channel&)));
    connect(this, SIGNAL(userLogin(const User&)), ui.channelsWidget,
            SLOT(slotUserLoggedIn(const User&)));
    connect(this, SIGNAL(userLogout(const User&)), ui.channelsWidget,
            SLOT(slotUserLoggedOut(const User&)));
    /* 'this' must be connected first since it needs to extract the old
     * user from channelswidget */
    connect(this, SIGNAL(userUpdate(const User&)), this, 
            SLOT(slotUserUpdate(const User&)));
    connect(this, SIGNAL(userUpdate(const User&)), ui.channelsWidget, 
            SLOT(slotUserUpdate(const User&)));
    connect(this, SIGNAL(userJoined(int,const User&)), this,
            SLOT(slotUserJoin(int,const User&)));
    connect(this, SIGNAL(userJoined(int,const User&)), ui.channelsWidget, 
            SLOT(slotUserJoin(int,const User&)));
    connect(this, SIGNAL(userLeft(int,const User&)), this, 
            SLOT(slotUserLeft(int,const User&)));
    connect(this, SIGNAL(userLeft(int,const User&)), ui.channelsWidget, 
            SLOT(slotUserLeft(int,const User&)));
    connect(this, SIGNAL(userLeft(int,const User&)), ui.videogridWidget, 
            SLOT(slotRemoveUser(int,const User&)));
    connect(this, SIGNAL(userStateChange(const User&)), ui.channelsWidget, 
            SLOT(slotUserStateChange(const User&)));
    connect(this, SIGNAL(updateMyself()), ui.channelsWidget, 
            SLOT(slotUpdateMyself()));
    connect(this, SIGNAL(newVideoCaptureFrame(int,int)), ui.channelsWidget, 
            SLOT(slotUserVideoFrame(int,int)));
    connect(this, SIGNAL(newTextMessage(const TextMessage&)),
            SLOT(slotNewTextMessage(const TextMessage&)));
    /* End - CLIENTEVENT_* messages */

    m_timers.insert(startTimer(1000), TIMER_ONE_SECOND);

#if !defined(Q_OS_WIN32) || USE_POLL
    //Windows uses its HWND for message handling, other platforms must
    //pull using a timer
    m_timers.insert(startTimer(20), TIMER_PROCESS_TTEVENT);
#endif
}

MainWindow::~MainWindow()
{
#ifdef Q_OS_LINUX
    if(m_display)
        XCloseDisplay(m_display);
#endif
}

void MainWindow::loadSettings()
{

    QString iniversion = ttSettings->value(SETTINGS_GENERAL_VERSION,
                                           SETTINGS_GENERAL_VERSION_DEFAULT).toString();
    if(!versionSameOrLater(iniversion, SETTINGS_VERSION))
    {
        // Volume defaults changed in 5.1 format
        ttSettings->remove(SETTINGS_SOUND_MASTERVOLUME);
        ttSettings->remove(SETTINGS_SOUND_MICROPHONEGAIN);
        ttSettings->setValue(SETTINGS_GENERAL_VERSION, SETTINGS_VERSION);
    }

    QString lang = ttSettings->value(SETTINGS_DISPLAY_LANGUAGE, "").toString();
    if(!lang.isEmpty())
    {
        ttTranslator = new QTranslator(this);
        if(!ttTranslator->load(lang, TRANSLATE_FOLDER))
        {
            QMessageBox::information(this, "Translate", 
                QString("Failed to load language file %1").arg(lang));
            delete ttTranslator;
            ttTranslator = nullptr;
        }
        else
        {
            QApplication::installTranslator(ttTranslator);
            this->ui.retranslateUi(this);
        }
    }

    //load settings
    bool ptt = ttSettings->value(SETTINGS_GENERAL_PUSHTOTALK).toBool();
    slotMeEnablePushToTalk(ptt);
    bool vox = ttSettings->value(SETTINGS_GENERAL_VOICEACTIVATED,
                                 SETTINGS_GENERAL_VOICEACTIVATED_DEFAULT).toBool();
    slotMeEnableVoiceActivation(vox);

    //load shortcuts
    loadHotKeys();

    int value = ttSettings->value(SETTINGS_SOUND_MASTERVOLUME, 
                                  SETTINGS_SOUND_MASTERVOLUME_DEFAULT).toInt();
    ui.volumeSlider->setValue(value);
    slotMasterVolumeChanged(value);  //force update on equal
    value = ttSettings->value(SETTINGS_SOUND_VOICEACTIVATIONLEVEL,
                              SETTINGS_SOUND_VOICEACTIVATIONLEVEL_DEFAULT).toInt();
    ui.voiceactSlider->setValue(value);
    slotVoiceActivationLevelChanged(value); //force update on equal

    // setup VU-meter updates
    if(ttSettings->value(SETTINGS_DISPLAY_VU_METER_UPDATES,
                         SETTINGS_DISPLAY_VU_METER_UPDATES_DEFAULT).toBool())
    {
        m_timers.insert(startTimer(50), TIMER_VUMETER_UPDATE);
        ui.voiceactBar->setVisible(true);
    }
    else
    {
        ui.voiceactBar->setVisible(false);
    }

    //default voice gain level depends on whether AGC or normal gain
    //is enabled
    bool agc = ttSettings->value(SETTINGS_SOUND_AGC,
                                 SETTINGS_SOUND_AGC_DEFAULT).toBool();
    value = ttSettings->value(SETTINGS_SOUND_MICROPHONEGAIN, agc?
                              SETTINGS_SOUND_MICROPHONEGAIN_AGC_DEFAULT :
                              SETTINGS_SOUND_MICROPHONEGAIN_GAIN_DEFAULT).toInt();
    ui.micSlider->setValue(value);
    slotMicrophoneGainChanged(value); //force update on equal

    if(ttSettings->value(SETTINGS_DISPLAY_ALWAYSONTOP, false).toBool())
    {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
#ifdef Q_OS_WIN32
        //Qt creates a new HWND to support "always on top" so no more messages
        //are posted to our HWND. This is actually unsafe since the previous HWND
        //might have unhandled messages.
        TT_SwapTeamTalkHWND(ttInst, reinterpret_cast<HWND>(winId()));
#endif
    }

#if defined(Q_OS_WIN32)
    //ask once to install firewall
    if(ttSettings->value(SETTINGS_FIREWALL_ADD, true).toBool())
    {
        firewallInstall();
        ttSettings->setValue(SETTINGS_FIREWALL_ADD, false);
    }
#endif

    if(ttSettings->value(SETTINGS_VIDCAP_ENABLE, SETTINGS_VIDCAP_ENABLE_DEFAULT).toBool())
        slotMeEnableVideoTransmission();

    //show number of users
    ui.channelsWidget->setShowUserCount(ttSettings->value(SETTINGS_DISPLAY_USERSCOUNT,
                                                          SETTINGS_DISPLAY_USERSCOUNT_DEFAULT).toBool());
    ui.channelsWidget->setShowUsername();
    ui.channelsWidget->setShowLastToTalk(ttSettings->value(SETTINGS_DISPLAY_LASTTALK,
                                                           SETTINGS_DISPLAY_LASTTALK_DEFAULT).toBool());
    ui.channelsWidget->updateItemTextLength(ttSettings->value(SETTINGS_DISPLAY_MAX_STRING,
                                            SETTINGS_DISPLAY_MAX_STRING_DEFAULT).toInt());
    slotUpdateUI();

    //move window to last position
    QVariantList windowpos = ttSettings->value(SETTINGS_DISPLAY_WINDOWPOS).toList();
    if(windowpos.size() == 4)
    {
        int x = windowpos[0].toInt();
        int y = windowpos[1].toInt();
        int w = windowpos[2].toInt();
        int h = windowpos[3].toInt();
        int desktopW = QApplication::desktop()->width();
        int desktopH = QApplication::desktop()->height();
        if(x <= desktopW && y <= desktopH)
            setGeometry(x, y, w, h);
        ui.splitter->restoreState(ttSettings->value(SETTINGS_DISPLAY_SPLITTER).toByteArray());
        ui.videosplitter->restoreState(ttSettings->value(SETTINGS_DISPLAY_VIDEOSPLITTER).toByteArray());
        ui.desktopsplitter->restoreState(ttSettings->value(SETTINGS_DISPLAY_DESKTOPSPLITTER).toByteArray());
    }
    //set files header to last position
    ui.filesView->header()->restoreState(ttSettings->value(SETTINGS_DISPLAY_FILESHEADER).toByteArray());

    // http query for app updates
    checkAppUpdate();
    m_timers.insert(startTimer(24 * 60 * 60 * 1000), TIMER_APP_UPDATE);

    if(ttSettings->value(SETTINGS_DISPLAY_STARTMINIMIZED, false).toBool())
        QTimer::singleShot(0, this, SLOT(showMinimized()));

    //started from .tt file?
    bool connect_ok = parseArgs(QApplication::arguments());

    if(connect_ok)
        QTimer::singleShot(0, this, SLOT(slotConnectToLatest()));

     //TT_EnableAudioBlockEvent(ttInst, true);
}

bool MainWindow::parseArgs(const QStringList& args)
{
    for(int i=1;i<args.size();i++)
    {
        if(args[i].right(QString(TTFILE_EXT).size()).toLower() == TTFILE_EXT)
        {
            slotLoadTTFile(args[i]);
            return false;
        }
        else if(args[i].left(QString(TTLINK_PREFIX).size()).toLower() == TTLINK_PREFIX)
        {
            //Slash is removed by Qt 5.3.1 by mistake, therefore /+ for slashes
            //Bug: https://bugreports.qt.io/browse/QTBUG-39972
            QRegExp rx(QString("%1/+([^\\??!/]*)/?\\??").arg(TTLINK_PREFIX));
            if(rx.indexIn(args[i])>=0)
            {
                HostEntry entry;
                entry.ipaddr = rx.cap(1);
                entry.tcpport = DEFAULT_TCPPORT;
                entry.udpport = DEFAULT_UDPPORT;
                QString prop = args[i].mid(rx.matchedLength());
                //&?tcpport=(\d+)&?
                rx.setPattern("&?tcpport=(\\d+)&?");
                if(rx.indexIn(prop)>=0)
                    entry.tcpport = rx.cap(1).toInt();
                //&?udpport=(\d+)&?
                rx.setPattern("&?udpport=(\\d+)&?");
                if(rx.indexIn(prop)>=0)
                    entry.udpport = rx.cap(1).toInt();
                //&?username=([^&]*)&?
                rx.setPattern("&?username=([^&]*)&?");
                if(rx.indexIn(prop)>=0)
                    entry.username = rx.cap(1);
                //&?password=([^&]*)&?
                rx.setPattern("&?password=([^&]*)&?");
                if(rx.indexIn(prop)>=0)
                    entry.password = rx.cap(1);
                //&?channel=([^&]*)&?
                rx.setPattern("&?channel=([^&]*)&?");
                if(rx.indexIn(prop)>=0)
                    entry.channel = rx.cap(1);
                //&?chanpasswd=([^&]*)&?
                rx.setPattern("&?chanpasswd=([^&]*)&?");
                if(rx.indexIn(prop)>=0)
                    entry.chanpasswd = rx.cap(1);

                addLatestHost(entry);
                m_host = entry;
                Disconnect();
                Connect();
                return false;
            }
        }
        else if(args[i] == "-noconnect")
            return false;
#if defined(Q_OS_WIN32)
        else if(args[i] == "-fwadd")
        {
            firewallInstall();
        }
        else if(args[i] == "-fwremove")
        {
            //remove firewall exception
            QString appPath = QApplication::applicationFilePath();
            appPath = QDir::toNativeSeparators(appPath);
            if(TT_Firewall_AppExceptionExists(_W(appPath)))
            {
                if(!TT_Firewall_RemoveAppException(_W(appPath)))
                {
                    QMessageBox::critical(this, tr("Firewall exception"),
                        tr("Failed to remove %1 from Windows Firewall exceptions.")
                        .arg(appPath));
                }
            }
        }
#endif
        else if(args[i] == "-tone")
        {
            TT_DBG_SetSoundInputTone(ttInst, STREAMTYPE_VOICE, 440);
        }
        else if(args[i] == "-cfg")
        {
            i++; // skip filename
        }
        else
        {
            QMessageBox::information(this, tr("Startup arguments"), 
                tr("Program argument \"%1\" is unrecognized.").arg(args[i]));
        }
    }

    return true;
}


void MainWindow::processTTMessage(const TTMessage& msg)
{
    bool update_ui = false;

    switch(msg.nClientEvent)
    {
    case CLIENTEVENT_CON_SUCCESS :
    {
        //disable reconnect timer
        killLocalTimer(TIMER_RECONNECT);

        //switch to connected icon in tray
        if(m_sysicon)
            m_sysicon->setIcon(QIcon(APPTRAYICON_CON));

        //reset stats
        ZERO_STRUCT(m_clientstats);
        // retrieve initial welcome message and access token
        TT_GetServerProperties(ttInst, &m_srvprop);

        if (m_host.username.compare(WEBLOGIN_BEARWARE_USERNAME, Qt::CaseInsensitive) == 0 ||
            m_host.username.endsWith(WEBLOGIN_BEARWARE_USERNAMEPOSTFIX, Qt::CaseInsensitive))
        {
            QString username = ttSettings->value(SETTINGS_GENERAL_BEARWARE_USERNAME).toString();
            QString token = ttSettings->value(SETTINGS_GENERAL_BEARWARE_TOKEN).toString();
            QString accesstoken = _Q(m_srvprop.szAccessToken);

            username = QUrl::toPercentEncoding(username);
            token = QUrl::toPercentEncoding(token);
            accesstoken = QUrl::toPercentEncoding(accesstoken);

            QString urlReq = WEBLOGIN_BEARWARE_URLTOKEN(username, token, accesstoken);

            QUrl url(urlReq);

            auto networkMgr = new QNetworkAccessManager(this);
            connect(networkMgr, SIGNAL(finished(QNetworkReply*)),
                SLOT(slotBearWareAuthReply(QNetworkReply*)));

            QNetworkRequest request(url);
            networkMgr->get(request);
        }
        else if (m_host.username.compare(WEBLOGIN_FACEBOOK_USERNAME, Qt::CaseInsensitive) == 0 ||
            m_host.username.endsWith(WEBLOGIN_FACEBOOK_USERNAMEPOSTFIX, Qt::CaseInsensitive))
        {
            WebLoginDlg dlg(this);
            if(dlg.exec() != QDialog::Accepted)
                return;
            m_host.password = dlg.m_password;

            login();
        }
        else
        {
            login();
        }

        update_ui = true;
    }
    break;
    case CLIENTEVENT_CON_FAILED :
    {
        Disconnect();

        addStatusMsg(tr("Failed to connect to %1 TCP port %2 UDP port %3")
                     .arg(m_host.ipaddr).arg(m_host.tcpport).arg(m_host.udpport));

        update_ui = true;
    }
    break;
    case CLIENTEVENT_CON_LOST :
    {
        Disconnect();
        if(ttSettings->value(SETTINGS_CONNECTION_RECONNECT, true).toBool())
            m_timers[startTimer(5000)] = TIMER_RECONNECT;

        addStatusMsg(tr("Connection lost to %1 TCP port %2 UDP port %3")
                     .arg(m_host.ipaddr).arg(m_host.tcpport).arg(m_host.udpport));

        playSoundEvent(SOUNDEVENT_SERVERLOST);

        update_ui = true;
    }
    break;
    case CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED :
        qDebug() << "User #" << msg.nSource << "max payload is" << msg.nPayloadSize;
    break;
    case CLIENTEVENT_CMD_PROCESSING :
        commandProcessing(msg.nSource, !msg.bActive);
        break;
    case CLIENTEVENT_CMD_ERROR :
    {
        //store current command type
        CommandComplete cmd_type = m_commands[m_current_cmdid];

        //if we're waiting for a result from this command then clear it
        //since the command failed (this must be done before showing 
        //and error message since the login will otherwise complete).
        m_commands.remove(m_current_cmdid);

        emit(cmdError(msg.clienterrormsg.nErrorNo, msg.nSource));

        showTTErrorMessage(msg.clienterrormsg, cmd_type);

        update_ui = true;
    }
    break;
    case CLIENTEVENT_CMD_SUCCESS :
        emit(cmdSuccess(msg.nSource));

        update_ui = true;
        break;
    case CLIENTEVENT_CMD_MYSELF_LOGGEDIN :
        //ui.chatEdit->updateServer();
        addStatusMsg(tr("Logged in"));
        //store user account settings
        m_myuseraccount = msg.useraccount;
        update_ui = true;
        break;
    case CLIENTEVENT_CMD_MYSELF_LOGGEDOUT :
        addStatusMsg(tr("Logged out"));
        Disconnect();
        update_ui = true;

        break;
    case CLIENTEVENT_CMD_MYSELF_KICKED :
    {
        Q_ASSERT(msg.ttType == __USER || msg.ttType == __NONE);

        if(msg.ttType == __USER)
            addStatusMsg(tr("Kicked by %1")
                         .arg(getDisplayName(msg.user)));
        else
            addStatusMsg(tr("Kicked by unknown user"));

        update_ui = true;
    }
    break;
    case CLIENTEVENT_CMD_SERVER_UPDATE :
    {
        Q_ASSERT(msg.ttType == __SERVERPROPERTIES);

        ui.chatEdit->updateServer(msg.serverproperties);

        emit(serverUpdate(msg.serverproperties));

        m_srvprop = msg.serverproperties;

        update_ui = true;
    }
    break;
    case CLIENTEVENT_CMD_SERVERSTATISTICS :
    {
        Q_ASSERT(msg.ttType == __SERVERSTATISTICS);
        emit(serverStatistics(msg.serverstatistics));
    }
    break;
    case CLIENTEVENT_CMD_CHANNEL_NEW :
        Q_ASSERT(msg.ttType == __CHANNEL);
        emit(newChannel(msg.channel));
        break;
    case CLIENTEVENT_CMD_CHANNEL_UPDATE :
    {
        Q_ASSERT(msg.ttType == __CHANNEL);
        
        if(m_mychannel.nChannelID == (int)msg.channel.nChannelID)
        {
            m_mychannel = msg.channel;
            //update AGC, denoise, etc. if changed
            updateAudioConfig();

            update_ui = true;
        }
        emit(updateChannel(msg.channel));
    }
    break;
    case CLIENTEVENT_CMD_CHANNEL_REMOVE :
        Q_ASSERT(msg.ttType == __CHANNEL);
        emit(removeChannel(msg.channel));
        break;
    case CLIENTEVENT_CMD_USER_LOGGEDIN :
    {
        Q_ASSERT(msg.ttType == __USER);
        emit(userLogin(msg.user));
        QString audiofolder = ttSettings->value(SETTINGS_MEDIASTORAGE_AUDIOFOLDER).toString();
        AudioFileFormat aff = (AudioFileFormat)ttSettings->value(SETTINGS_MEDIASTORAGE_FILEFORMAT, AFF_WAVE_FORMAT).toInt();
        if(m_audiostorage_mode & AUDIOSTORAGE_SEPARATEFILES)
            TT_SetUserMediaStorageDir(ttInst, msg.user.nUserID, _W(audiofolder), nullptr, aff);

        updateUserSubscription(msg.user.nUserID);
    }
    break;
    case CLIENTEVENT_CMD_USER_LOGGEDOUT :
        Q_ASSERT(msg.ttType == __USER);
        emit(userLogout(msg.user));
        //remove text-message history from this user
        m_usermessages.remove(msg.user.nUserID);
        break;
    case CLIENTEVENT_CMD_USER_JOINED :
        Q_ASSERT(msg.ttType == __USER);
        if(msg.user.nUserID == TT_GetMyUserID(ttInst))
            processMyselfJoined(msg.user.nChannelID);
        emit(userJoined(msg.user.nChannelID, msg.user));
        update_ui = true;
        break;
    case CLIENTEVENT_CMD_USER_LEFT :
        Q_ASSERT(msg.ttType == __USER);
        if(msg.user.nUserID == TT_GetMyUserID(ttInst))
            processMyselfLeft(msg.nSource);
        emit(userLeft(msg.nSource, msg.user));
        update_ui = true;
        break;
    case CLIENTEVENT_CMD_USER_UPDATE :
    {
        Q_ASSERT(msg.ttType == __USER);

        User prev_user;
        ZERO_STRUCT(prev_user);
        ui.channelsWidget->getUser(msg.user.nUserID, prev_user);
        Q_ASSERT(prev_user.nUserID);

        emit(userUpdate(msg.user));

        if(msg.user.nUserID != TT_GetMyUserID(ttInst) &&
           msg.user.nChannelID == m_mychannel.nChannelID)
        {
            if((prev_user.nStatusMode & STATUSMODE_QUESTION) == 0 &&
               (msg.user.nStatusMode & STATUSMODE_QUESTION))
               playSoundEvent(SOUNDEVENT_QUESTIONMODE);
        }

        //update desktop access button
        if((prev_user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT) !=
           (msg.user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT))
           slotUpdateDesktopTabUI();
    }
    break;
    case CLIENTEVENT_CMD_USER_TEXTMSG :
        Q_ASSERT(msg.ttType == __TEXTMESSAGE);
        processTextMessage(msg.textmessage);
        break;
    case CLIENTEVENT_CMD_FILE_NEW :
    case CLIENTEVENT_CMD_FILE_REMOVE :
    {
        Q_ASSERT(msg.ttType == __REMOTEFILE);
        const RemoteFile& file = msg.remotefile;

        //only update files list if we're not currently logging in or 
        //joining a channel
        cmdreply_t::iterator ite = m_commands.find(m_current_cmdid);
        if(m_filesmodel->getChannelID() == file.nChannelID &&
           (ite == m_commands.end() || (*ite != CMD_COMPLETE_LOGIN && 
                                        *ite != CMD_COMPLETE_JOINCHANNEL)) )
        {
            updateChannelFiles(file.nChannelID);
            playSoundEvent(SOUNDEVENT_FILESUPD);
        }

        update_ui = true;
    }
    break;
    case CLIENTEVENT_CMD_USERACCOUNT :
        Q_ASSERT(msg.ttType == __USERACCOUNT);
        m_useraccounts.push_back(msg.useraccount);
        break;
    case CLIENTEVENT_CMD_BANNEDUSER :
        Q_ASSERT(msg.ttType == __BANNEDUSER);
        m_bannedusers.push_back(msg.banneduser);
        break;
    case CLIENTEVENT_FILETRANSFER :
        Q_ASSERT(msg.ttType == __FILETRANSFER);

        emit(filetransferUpdate(msg.filetransfer));

        if(msg.filetransfer.nStatus == FILETRANSFER_ACTIVE &&
           msg.filetransfer.nTransferred == 0)
        {
            FileTransferDlg* dlg = new FileTransferDlg(msg.filetransfer, nullptr);
            connect(this, SIGNAL(filetransferUpdate(const FileTransfer&)), dlg, 
                    SLOT(slotTransferUpdate(const FileTransfer&)));
            dlg->setAttribute(Qt::WA_DeleteOnClose);
            dlg->show();
        }
        else if(msg.filetransfer.nStatus == FILETRANSFER_ERROR &&
                msg.filetransfer.nTransferred == 0)
        {
            if(msg.filetransfer.bInbound)
                QMessageBox::critical(this, MENUTEXT(ui.actionDownloadFile->text()),
                                        tr("Failed to download file %1")
                                        .arg(_Q(msg.filetransfer.szRemoteFileName)));
            else
                QMessageBox::critical(this, MENUTEXT(ui.actionUploadFile->text()),
                                        tr("Failed to upload file %1")
                                        .arg(_Q(msg.filetransfer.szLocalFilePath)));
        }

        update_ui = true;
        break;
    case CLIENTEVENT_INTERNAL_ERROR :
    {
        Q_ASSERT(msg.ttType == __CLIENTERRORMSG);
        bool critical = true;
        QString textmsg;
        switch(msg.clienterrormsg.nErrorNo)
        {
            /***** Internal TeamTalk errors not related to commands ********/
        case INTERR_SNDINPUT_FAILURE :
            textmsg = tr("Failed to initialize sound input device"); break;
        case INTERR_SNDOUTPUT_FAILURE :
            textmsg = tr("Failed to initialize sound output device"); break;
        case INTERR_AUDIOCODEC_INIT_FAILED :
            textmsg = tr("Failed to initialize audio codec"); break;
        case INTERR_SPEEXDSP_INIT_FAILED :
            critical = false;
            textmsg = tr("Failed to initialize audio configuration"); break;
        case INTERR_TTMESSAGE_QUEUE_OVERFLOW :
            critical = false;
            textmsg = tr("Internal message queue overloaded"); break;
        default :
            textmsg = _Q(msg.clienterrormsg.szErrorMsg);
            break;
        }
        if(critical)
            QMessageBox::critical(this, tr("Internal Error"), textmsg);
        addStatusMsg(textmsg);

        update_ui = true;
    }
    break;
    case CLIENTEVENT_USER_STATECHANGE :
    {
        Q_ASSERT(msg.ttType == __USER);
        const User& user = msg.user;
        emit(userStateChange(user));
        if(user.uUserState & USERSTATE_VOICE)
        {
            m_talking.insert(user.nUserID);

            if(m_sysicon && m_sysicon->isVisible() &&
               m_talking.size() == 1)
                m_sysicon->setIcon(QIcon(APPTRAYICON_ACTIVE));
        }
        else
        {
            m_talking.remove(user.nUserID);

            if(m_sysicon && m_sysicon->isVisible() &&
               m_talking.size() == 0)
                m_sysicon->setIcon(QIcon(APPTRAYICON_CON));
        }
        
        if(m_talking.empty())
            playSoundEvent(SOUNDEVENT_SILENCE);
    }
    break;
    case CLIENTEVENT_VOICE_ACTIVATION :
        Q_ASSERT(msg.ttType == __TTBOOL);
        emit(updateMyself());
        break;
    case CLIENTEVENT_STREAM_MEDIAFILE :
    {
        Q_ASSERT(msg.ttType == __MEDIAFILEINFO);
        switch(msg.mediafileinfo.nStatus)
        {
        case MFS_ERROR :
            addStatusMsg(tr("Error streaming media file to channel"));
            stopStreamMediaFile();
            break;
        case MFS_STARTED :
            addStatusMsg(tr("Started streaming media file to channel"));
            break;
        case MFS_FINISHED :
            addStatusMsg(tr("Finished streaming media file to channel"));
            stopStreamMediaFile();
            break;
        case MFS_ABORTED :
            addStatusMsg(tr("Aborted streaming media file to channel"));
            stopStreamMediaFile();
            break;
        case MFS_CLOSED :
            break;
        }

        if(msg.mediafileinfo.nStatus == MFS_FINISHED &&
           ttSettings->value(SETTINGS_STREAMMEDIA_LOOP, false).toBool())
        {
            startStreamMediaFile();
        }

        emit(mediaStreamUpdate(msg.mediafileinfo));

        //update if still talking
        emit(updateMyself());
        update_ui = true;
    }
    break;
    case CLIENTEVENT_LOCAL_MEDIAFILE:
        emit(mediaPlaybackUpdate(msg.nSource, msg.mediafileinfo));
        break;
    case CLIENTEVENT_USER_VIDEOCAPTURE :
    {
        Q_ASSERT(msg.ttType == __INT32);

        //local video is userID = 0 (msg.nSource)
        int userid = msg.nSource | VIDEOTYPE_CAPTURE;

        //if local video is disabled then don't process pending video
        //frames
        if(msg.nSource == 0 && 
           (TT_GetFlags(ttInst) & CLIENT_VIDEOCAPTURE_READY) == 0)
            break;
    
        //pass new video frame (if it's not being ignored)
        if(m_vid_exclude.find(userid) != m_vid_exclude.end())
            break;

        if(m_user_video.find(userid) == m_user_video.end() &&
           !ui.videogridWidget->userExists(userid))
        {
            //it's a new video session

            if(ttSettings->value(SETTINGS_DISPLAY_VIDEOPOPUP, false).toBool())
                slotNewUserVideoDlg(userid, QSize());
            else
                ui.videogridWidget->slotAddUser(userid);

            playSoundEvent(SOUNDEVENT_NEWVIDEO);

            User user;
            if(TT_GetUser(ttInst, userid & VIDEOTYPE_USERMASK, &user))
                addStatusMsg(tr("New video session from %1")
                             .arg(getDisplayName(user)));

            update_ui = true;
        }
        emit(newVideoCaptureFrame(userid, msg.nStreamID));
    }
    break;
    case CLIENTEVENT_USER_MEDIAFILE_VIDEO :
    {
        Q_ASSERT(msg.ttType == __INT32);

        int userid = msg.nSource | VIDEOTYPE_MEDIAFILE;

       //pass new video frame (if it's not being ignored)
        if(m_vid_exclude.find(userid) != m_vid_exclude.end())
            break;

        if(m_user_video.find(userid) == m_user_video.end() &&
           !ui.videogridWidget->userExists(userid))
        {
            //it's a new video session

            if(ttSettings->value(SETTINGS_DISPLAY_VIDEOPOPUP, false).toBool())
                slotNewUserVideoDlg(userid, QSize());
            else
                ui.videogridWidget->slotAddUser(userid);

            playSoundEvent(SOUNDEVENT_NEWVIDEO);

            User user;
            if(TT_GetUser(ttInst, userid & VIDEOTYPE_USERMASK, &user))
                addStatusMsg(tr("New video session from %1")
                .arg(getDisplayName(user)));

            update_ui = true;
        }
        emit(newMediaVideoFrame(userid, msg.nStreamID));
    }
    break;
    case CLIENTEVENT_USER_DESKTOPWINDOW :
        Q_ASSERT(msg.ttType == __INT32);
        if(m_userdesktop.find(msg.nSource) == m_userdesktop.end() &&
           !ui.desktopgridWidget->userExists(msg.nSource))
        {
            //it's a new desktop session

            //sesion id == 0 means it has ended
            if(msg.nStreamID)
            {
                if(ttSettings->value(SETTINGS_DISPLAY_DESKTOPPOPUP, false).toBool())
                    slotDetachUserDesktop(msg.nSource, QSize());
                else
                    ui.desktopgridWidget->slotAddUser(msg.nSource);

                playSoundEvent(SOUNDEVENT_NEWDESKTOP);

                User user;
                if(ui.channelsWidget->getUser(msg.nSource, user))
                    addStatusMsg(tr("New desktop session from %1")
                    .arg(getDisplayName(user)));
            }

            update_ui = true;
        }
        emit(newDesktopWindow(msg.nSource, msg.nStreamID));
        break;
    case CLIENTEVENT_DESKTOPWINDOW_TRANSFER :
        Q_ASSERT(msg.ttType == __INT32);
        if(m_desktopsession_id != msg.nSource) //new session
            m_desktopsession_remain = 0;
        m_desktopsession_id = msg.nSource;

        if(m_desktopsession_remain == 0)
            m_desktopsession_total = msg.nBytesRemain;
        m_desktopsession_remain = msg.nBytesRemain;

        if(msg.nBytesRemain == 0 && m_desktopsend_on_completion)
        {
            if(sendDesktopWindow())
            {
                restartSendDesktopWindowTimer();
                m_desktopsend_on_completion = false;
            }
        }

        if(msg.nSource == 0)
            addStatusMsg(tr("Your desktop session was cancelled"));
        break;
    case CLIENTEVENT_USER_DESKTOPCURSOR :
        Q_ASSERT(msg.ttType == __DESKTOPINPUT);
        emit(userDesktopCursor(msg.nSource, msg.desktopinput));
        break;
    case CLIENTEVENT_USER_DESKTOPINPUT :
        Q_ASSERT(msg.ttType == __DESKTOPINPUT);
        processDesktopInput(msg.nSource, msg.desktopinput);
        break;
    case CLIENTEVENT_USER_RECORD_MEDIAFILE :
    {
        Q_ASSERT(msg.ttType == __MEDIAFILEINFO);
        User user;
        ZERO_STRUCT(user);
        ui.channelsWidget->getUser(msg.nSource, user);

        switch(msg.mediafileinfo.nStatus)
        {
        case MFS_STARTED :
            addStatusMsg(tr("Writing audio file %1 for %2")
                         .arg(_Q(msg.mediafileinfo.szFileName))
                         .arg(getDisplayName(user)));
            break;
        case MFS_ERROR :
            addStatusMsg(tr("Failed to write audio file %1 for %2")
                         .arg(_Q(msg.mediafileinfo.szFileName))
                         .arg(getDisplayName(user)));
            break;
        case MFS_FINISHED :
            addStatusMsg(tr("Finished audio file %1")
                         .arg(_Q(msg.mediafileinfo.szFileName)));
            break;
        case MFS_ABORTED :
            addStatusMsg(tr("Aborted audio file %1")
                         .arg(_Q(msg.mediafileinfo.szFileName)));
            break;
        case MFS_CLOSED :
            break;
        }
    }
    break;
    case CLIENTEVENT_USER_AUDIOBLOCK :
    {
        AudioBlock* block = TT_AcquireUserAudioBlock(ttInst, msg.nStreamType, msg.nSource);
        if(block)
        {
            int mean = 0;
            int size = block->nSamples * block->nChannels;
            short* ptr = (short*)block->lpRawAudio;
            for(int i=0;i<size;i++)
                mean += ptr[i];
            mean /= size;
            qDebug() << "AudioBlock from #" << msg.nSource 
                << "stream id" << block->nStreamID 
                << "index" << block->uSampleIndex 
                << "samples" << block->nSamples
                << "avg" << mean;
            TT_ReleaseUserAudioBlock(ttInst, block);
        }
    }
    break;
    case CLIENTEVENT_HOTKEY :
        Q_ASSERT(msg.ttType == __TTBOOL);
        hotkeyToggle((HotKeyID)msg.nSource, (bool)msg.bActive);
        break;
    default :
        qDebug() << "Unknown message type" << msg.nClientEvent;
    }

    //update menus, button, etc.
    if(update_ui)
        slotUpdateUI();
}


void MainWindow::commandProcessing(int cmdid, bool complete)
{
    //command reply starting -> store command reply ID
    if(!complete)
        m_current_cmdid = cmdid;

    //check if a dialog should be opened to show server reply
    cmdreply_t::iterator ite;
    if(complete && /* Command has completed */
       (ite = m_commands.find(cmdid)) != m_commands.end()) //Command we're interested in knowing the reply of
    {
        switch(*ite)
        {
        case CMD_COMPLETE_LOGIN :
            cmdLoggedIn(TT_GetMyUserID(ttInst));
            break;
        case CMD_COMPLETE_JOINCHANNEL :
            //unsubscribe
            cmdJoinedChannel(TT_GetMyChannelID(ttInst));
            break;
        case CMD_COMPLETE_LIST_CHANNELBANS :
        case CMD_COMPLETE_LIST_SERVERBANS :
        {
            if(!m_bannedusersdlg)
            {
                QString chanpath;
                if (*ite == CMD_COMPLETE_LIST_CHANNELBANS)
                {
                    int chanid = ui.channelsWidget->selectedChannel(true);
                    TTCHAR path[TT_STRLEN] = {};
                    TT_GetChannelPath(ttInst, chanid, path);
                    chanpath = _Q(path);
                }
                m_bannedusersdlg = new BannedUsersDlg(m_bannedusers, chanpath);
                if (chanpath.size())
                    m_bannedusersdlg->setWindowTitle(tr("Banned Users in Channel %1").arg(chanpath));
                connect(m_bannedusersdlg, SIGNAL(finished(int)),
                        SLOT(slotClosedBannedUsersDlg(int)));
                m_bannedusersdlg->setAttribute(Qt::WA_DeleteOnClose);
                m_bannedusersdlg->show();
                m_bannedusers.clear();
            }
            else
                m_bannedusersdlg->activateWindow();
        }
        break;
        case CMD_COMPLETE_LISTACCOUNTS :
        {
            if(!m_useraccountsdlg)
            {
                m_useraccountsdlg = new UserAccountsDlg(m_useraccounts, UAD_READWRITE);
                connect(this, SIGNAL(cmdSuccess(int)), m_useraccountsdlg, 
                        SLOT(slotCmdSuccess(int)));
                connect(this, SIGNAL(cmdError(int, int)), m_useraccountsdlg, 
                        SLOT(slotCmdError(int,int)));
                connect(m_useraccountsdlg, SIGNAL(finished(int)),
                        SLOT(slotClosedUserAccountsDlg(int)));
                m_useraccountsdlg->setAttribute(Qt::WA_DeleteOnClose);
                m_useraccountsdlg->show();
                m_useraccounts.clear();
            }
            else
                m_useraccountsdlg->activateWindow();
        }
        break;
        default :
            break;
        }
    }

    //command reply completed -> clear command reply ID
    if(complete)
    {
        m_commands.remove(cmdid);
        m_current_cmdid = 0;
    }
}

void MainWindow::cmdLoggedIn(int myuserid)
{
    Q_UNUSED(myuserid);

    //login command completed

    QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();

    if(!ttSettings->value(SETTINGS_GENERAL_GENDER,
                         SETTINGS_GENERAL_GENDER_DEFAULT).toBool())
        m_statusmode |= STATUSMODE_FEMALE;

    //set status mode flags
    if(m_statusmode || statusmsg.size())
        TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));

    UserAccount account;
    ZERO_STRUCT(account);
    TT_GetMyUserAccount(ttInst, &account);

    //join channel (if specified)
    Channel tmpchan;
    int channelid = TT_GetChannelIDFromPath(ttInst, _W(m_host.channel));
    int parentid = 0;
    int account_channelid = TT_GetChannelIDFromPath(ttInst, 
                                                    account.szInitChannel);

    //see if parent channel exists (otherwise we cannot create it)
    QStringList subchannels = m_host.channel.split('/');
    if(subchannels.size())
    {
        QStringList parent = subchannels;
        parent.erase(parent.end()-1);
        QString chanpath = parent.join("/");
        parentid = TT_GetChannelIDFromPath(ttInst, _W(chanpath));
    }

    if(m_last_channel.nChannelID && //join using last channel
       TT_GetChannel(ttInst, m_last_channel.nParentID, 
                     &tmpchan)>0)
    {
        int cmdid = TT_DoJoinChannel(ttInst, &m_last_channel);
        if(cmdid>0)
            m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
    }
    else if(_Q(account.szInitChannel).size() && account_channelid > 0)
    {
        int cmdid = TT_DoJoinChannelByID(ttInst, account_channelid, 
                                         _W(QString()));
        if(cmdid>0)
            m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
    }
    else if(m_host.channel.size() && channelid > 0) //join if channel exists
    {
        int cmdid = TT_DoJoinChannelByID(ttInst, channelid, _W(m_host.chanpasswd));
        if(cmdid>0)
            m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
    }
    else if(m_host.channel.size() && parentid>0) //make a new channel if parent exists
    {
        QString name;
        if(subchannels.size())
            name = subchannels.last();

        Channel chan;
        ZERO_STRUCT(chan);
        chan.nParentID = parentid;
        chan.nMaxUsers = m_srvprop.nMaxUsers;
        initDefaultAudioCodec(chan.audiocodec);

        chan.audiocfg.bEnableAGC = DEFAULT_CHANNEL_AUDIOCONFIG;
        chan.audiocfg.nGainLevel = DEFAULT_AGC_GAINLEVEL;

        COPY_TTSTR(chan.szName, name);
        COPY_TTSTR(chan.szPassword, m_host.chanpasswd);
        int cmdid = TT_DoJoinChannel(ttInst, &chan);
        if(cmdid>0)
            m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
    }
    else if(ttSettings->value(SETTINGS_CONNECTION_AUTOJOIN, true).toBool()) //just join root
    {
        if(m_host.channel.size())
            addStatusMsg(tr("Cannot join channel %1").arg(m_host.channel));

        //auto join root channel
        int cmdid = TT_DoJoinChannelByID(ttInst, 
                                         TT_GetRootChannelID(ttInst), 
                                         _W(QString("")));
        if(cmdid>0)
            m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
    }
}

void MainWindow::cmdJoinedChannel(int channelid)
{
    //update list of files in current channel
    updateChannelFiles(TT_GetMyChannelID(ttInst));
    //show channel information in chat window
    ui.chatEdit->joinedChannel(channelid);
}

void MainWindow::addStatusMsg(const QString& msg)
{
    if(ttSettings->value(SETTINGS_DISPLAY_LOGSTATUSBAR, true).toBool())
    {
        ui.chatEdit->addLogMessage(msg);
        ui.videochatEdit->addLogMessage(msg);
        ui.desktopchatEdit->addLogMessage(msg);
    }
    m_statusmsg.enqueue(msg);

    //just erase status messages if there's too many
    while(m_statusmsg.size()>10)
        m_statusmsg.dequeue();

    if(!timerExists(TIMER_STATUSMSG))
        m_timers[startTimer(1000)] = TIMER_STATUSMSG;
}

void MainWindow::Connect()
{
    Q_ASSERT((TT_GetFlags(ttInst) & CLIENT_CONNECTION) == 0);

    QStringList errors = initSelectedSoundDevices();
    for (auto s : errors)
        addStatusMsg(s);

    //choose default sound devices if configuration failed
    if (errors.size())
    {
        errors = initDefaultSoundDevices();
        for (auto s : errors)
            addStatusMsg(s);
    }

    int localtcpport = ttSettings->value(SETTINGS_CONNECTION_TCPPORT, 0).toInt();
    int localudpport = ttSettings->value(SETTINGS_CONNECTION_UDPPORT, 0).toInt();

    addStatusMsg(tr("Connecting to %1 TCP port %2 UDP port %3")
                 .arg(m_host.ipaddr).arg(m_host.tcpport).arg(m_host.udpport));

    m_desktopaccess_entries.clear();
    getDesktopAccessList(m_desktopaccess_entries, m_host.ipaddr, m_host.tcpport);

    if(!TT_Connect(ttInst, _W(m_host.ipaddr), m_host.tcpport,
                   m_host.udpport, localtcpport, localudpport, m_host.encrypted))
        addStatusMsg(tr("Failed to connect to %1 TCP port %2 UDP port %3")
                     .arg(m_host.ipaddr).arg(m_host.tcpport).arg(m_host.udpport));
}

void MainWindow::Disconnect()
{
    TT_Disconnect(ttInst);

    if(TT_GetFlags(ttInst) & CLIENT_SNDINOUTPUT_DUPLEX)
        TT_CloseSoundDuplexDevices(ttInst);
    else
    {
        TT_CloseSoundInputDevice(ttInst);
        TT_CloseSoundOutputDevice(ttInst);
    }

    ui.channelsWidget->reset();
    ui.videogridWidget->ResetGrid();
    ui.desktopgridWidget->ResetGrid();

    m_vid_exclude.clear();

    m_desktopaccess_entries.clear();

    updateWindowTitle();

    ZERO_STRUCT(m_srvprop);
    ZERO_STRUCT(m_mychannel);

    m_useraccounts.clear();
    m_bannedusers.clear();

    if(m_sysicon)
        m_sysicon->setIcon(QIcon(APPTRAYICON));

    updateWindowTitle();
}

void MainWindow::login()
{
    QString nick = ttSettings->value(SETTINGS_GENERAL_NICKNAME, tr(SETTINGS_GENERAL_NICKNAME_DEFAULT)).toString();

    int cmdid = TT_DoLoginEx(ttInst, _W(nick), _W(m_host.username),
                             _W(m_host.password), _W(QString(APPNAME_SHORT)));
    if (cmdid>0)
        m_commands.insert(cmdid, CMD_COMPLETE_LOGIN);

    addStatusMsg(tr("Connected to %1 TCP port %2 UDP port %3")
        .arg(m_host.ipaddr).arg(m_host.tcpport).arg(m_host.udpport));

    //query server's max payload
    if(ttSettings->value(SETTINGS_CONNECTION_QUERYMAXPAYLOAD, false).toBool())
        TT_QueryMaxPayload(ttInst, 0);
}

void MainWindow::showTTErrorMessage(const ClientErrorMsg& msg, CommandComplete cmd_type)
{
    QString title = tr("Error");
    QString textmsg;
    switch(msg.nErrorNo)
    {
        // command errors
    case CMDERR_SYNTAX_ERROR :
    case CMDERR_UNKNOWN_COMMAND :
    case CMDERR_INCOMPATIBLE_PROTOCOLS :
    case CMDERR_MISSING_PARAMETER :
        textmsg = tr("This client is not compatible with the server, "
                    "so the action cannot be performed."); break;
    case CMDERR_INVALID_USERNAME :
        textmsg = tr("The username is invalid"); break;

        // command errors due to rights
    case CMDERR_INVALID_ACCOUNT :
        {
            bool ok = false;
            m_host.username = QInputDialog::getText(this, tr("Login error"), 
                tr("Invalid user account. Type username:"), 
                QLineEdit::Normal, m_host.username, &ok);
            if(!ok)
                return;
            m_host.password = QInputDialog::getText(this, tr("Login error"), 
                tr("Invalid user account. Type password:"), 
                QLineEdit::Password, m_host.password, &ok);
            if(!ok)
                return;
            
            addLatestHost(m_host);
            QString nickname = ttSettings->value(SETTINGS_GENERAL_NICKNAME, tr(SETTINGS_GENERAL_NICKNAME_DEFAULT)).toString();
            int cmdid = TT_DoLoginEx(ttInst, _W(nickname), 
                                     _W(m_host.username), _W(m_host.password), 
                                     _W(QString(APPNAME_SHORT)));
            if(cmdid>0)
                m_commands.insert(cmdid, CMD_COMPLETE_LOGIN);            
            return;
        }
    case CMDERR_INCORRECT_CHANNEL_PASSWORD :
        {
            bool ok = false;
            QString passwd = QInputDialog::getText(this, tr("Join channel error"), 
                tr("Incorrect channel password. Try again:"), 
                QLineEdit::Password, _Q(m_last_channel.szPassword), &ok);
            if(!ok)
                return;
            m_channel_passwd[m_last_channel.nChannelID] = passwd;
            COPY_TTSTR(m_last_channel.szPassword, passwd);
            int cmdid = TT_DoJoinChannel(ttInst, &m_last_channel);
            if(cmdid>0)
                m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);            
            return;
        }
    case CMDERR_SERVER_BANNED :
        title = tr("Login error");
        textmsg = tr("Banned from server"); break;
    case CMDERR_NOT_AUTHORIZED :
        textmsg = tr("Command not authorized");    break;
    case CMDERR_MAX_SERVER_USERS_EXCEEDED :
        title = tr("Login error");
        textmsg = tr("Maximum number of users on server exceeded"); break;
    case CMDERR_MAX_DISKUSAGE_EXCEEDED :
        textmsg = tr("Maximum disk usage exceeded"); break;
    case CMDERR_MAX_CHANNEL_USERS_EXCEEDED :
        textmsg = tr("Maximum number of users in channel exceeded"); break;
    case CMDERR_INCORRECT_OP_PASSWORD :
        textmsg = tr("Incorrect channel operator password"); break;

        // state errors
    case CMDERR_ALREADY_LOGGEDIN :
        textmsg = tr("Already logged in");break;
    case CMDERR_NOT_LOGGEDIN :
        textmsg = tr("Cannot perform action because client is currently not logged in");break;
    case CMDERR_ALREADY_IN_CHANNEL :
        title = MENUTEXT(ui.actionJoinChannel->text());        
        textmsg = tr("Cannot join the same channel twice");break;
    case CMDERR_CHANNEL_ALREADY_EXISTS :
        textmsg = tr("Channel already exists");break;
    case CMDERR_USER_NOT_FOUND :
        //unsubscribe is also used to close streams, so just ignore if
        //user doesn't exist.
        if(cmd_type != CMD_COMPLETE_SUBSCRIBE &&
           cmd_type != CMD_COMPLETE_UNSUBSCRIBE)
            textmsg = tr("User not found");
        break;
    case CMDERR_CHANNEL_NOT_FOUND :
        textmsg = tr("Channel not found");break;
    case CMDERR_BAN_NOT_FOUND :
        textmsg = tr("Banned user not found"); break;
    case CMDERR_FILETRANSFER_NOT_FOUND :
        textmsg = tr("File transfer not found"); break;
    case CMDERR_ACCOUNT_NOT_FOUND :
        textmsg = tr("User account not found"); break;
    case CMDERR_FILE_NOT_FOUND :
        textmsg = tr("File not found"); break;
    case CMDERR_FILE_ALREADY_EXISTS :
        textmsg = tr("File already exists"); break;
    case CMDERR_FILESHARING_DISABLED :
        textmsg = tr("File sharing is disabled"); break;
    case CMDERR_CHANNEL_HAS_USERS :
        textmsg = tr("Channel has active users"); break;
    default :
        {
            textmsg = _Q(msg.szErrorMsg);
            if(textmsg.isEmpty())
                textmsg = tr("Unknown error occured");
            break;
        }
    }

    if(textmsg.size() && msg.nErrorNo < 10000)
    {
        textmsg = tr("The server reported an error:") + "\r\n\r\n" + textmsg;
        QMessageBox::critical(this, title, textmsg);
    }
}

#ifdef Q_OS_LINUX
void MainWindow::keysActive(quint32 keycode, quint32 mods, bool active)
{
    keycomp_t comp;
    if(mods & ControlMask)
        comp.insert(ControlMask);
    if(mods & Mod1Mask)
        comp.insert(Mod1Mask);
    if(mods & ShiftMask)
        comp.insert(ShiftMask);
    comp.insert(keycode);

    reghotkeys_t::const_iterator ite = m_hotkeys.begin();
    while(ite != m_hotkeys.end())
    {
        const keycomp_t& curcomp = ite.value();
        
        if(keycomp_t(comp).intersect(curcomp) == curcomp)
            hotkeyToggle(ite.key(), active);
        ite++;
    }
}
#endif

void MainWindow::hotkeyToggle(HotKeyID id, bool active)
{
    switch(id)
    {
    case HOTKEY_PUSHTOTALK :
#if defined(Q_OS_LINUX) && QT_VERSION >= 0x050000
        if(active)
        {
            qDebug() << "Hotkeys are using PTT lock in Qt5 for now";
            bool tx = (TT_GetFlags(ttInst) & CLIENT_TX_VOICE) != CLIENT_CLOSED;
            TT_EnableVoiceTransmission(ttInst, !tx);
            emit(updateMyself());
            playSoundEvent(SOUNDEVENT_HOTKEY);
        }
#else
        TT_EnableVoiceTransmission(ttInst, active);
        emit(updateMyself());
        playSoundEvent(SOUNDEVENT_HOTKEY);
#endif
        break;
    case HOTKEY_VOICEACTIVATION :
        if(active)
            TT_EnableVoiceActivation(ttInst, 
                     !(TT_GetFlags(ttInst) & CLIENT_SNDINPUT_VOICEACTIVATED));
        break;
    case HOTKEY_INCVOLUME :
        if(active)
            ui.volumeSlider->setValue(ui.volumeSlider->value()+1);
        break;
    case HOTKEY_DECVOLUME :
        if(active)
            ui.volumeSlider->setValue(ui.volumeSlider->value()-1);
        break;
    case HOTKEY_MUTEALL :
        if(active)
            TT_SetSoundOutputMute(ttInst, 
                                  !(TT_GetFlags(ttInst) & CLIENT_SNDOUTPUT_MUTE));
        break;
    case HOTKEY_MICROPHONEGAIN_INC :
        if(active)
            ui.micSlider->setValue(ui.micSlider->value() + 1);
        break;
    case HOTKEY_MICROPHONEGAIN_DEC :
        if(active)
            ui.micSlider->setValue(ui.micSlider->value() - 1);
        break;
    case HOTKEY_VIDEOTX :
        if(active)
            slotMeEnableVideoTransmission((TT_GetFlags(ttInst) & CLIENT_TX_VIDEOCAPTURE) == 0);
        break;
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    QMainWindow::timerEvent(event);

    timers_t::iterator ite = m_timers.find(event->timerId());
    Q_ASSERT(ite != m_timers.end());
    switch(*ite)
    {
    case TIMER_PROCESS_TTEVENT :
    {
        TTMessage msg;
        int wait_ms = 0;
        while(TT_GetMessage(ttInst, &msg, &wait_ms))
            processTTMessage(msg);
    }
    break;
    case TIMER_ONE_SECOND :
        if(TT_GetFlags(ttInst) & CLIENT_CONNECTED)
        {
            ClientStatistics stats;
            TT_GetClientStatistics(ttInst, &stats);
            float rx = (float)(stats.nUdpBytesRecv - m_clientstats.nUdpBytesRecv);
            float tx = (float)(stats.nUdpBytesSent - m_clientstats.nUdpBytesSent);
            int ping = stats.nUdpPingTimeMs;
            m_clientstats = stats;

            QString status;
            status.sprintf("RX: %.2fKB TX: %.2fKB", rx/1024.0f, tx/1024.0f);

            if(ping != -1)
                m_pinglabel->setText(QString("PING: %1").arg(ping));
            else
                m_pinglabel->setText("");

            //don't show in status if there's status messages
            if(!timerExists(TIMER_STATUSMSG))
                ui.statusbar->showMessage(status);
        }

        //check if desktop transfer has been closed
        if((TT_GetFlags(ttInst) & CLIENT_DESKTOP_ACTIVE) == 0)
        {
            m_desktopsession_id = 0;
            m_prev_desktopsession_id = 0;
            m_desktopsession_total = 0;
            m_desktopsession_remain = 0;
        }

        //only show desktop transfer it's continuesly active
        if(m_prev_desktopsession_id)
        {
            float progress = 100.f;
            if(m_prev_desktopsession_id == m_desktopsession_id &&
               m_desktopsession_total)
                progress = 100.0f * (m_desktopsession_total - m_desktopsession_remain) 
                           / (float)m_desktopsession_total;

            m_dtxprogress->setValue((int)progress);

            if(m_dtxprogress->isHidden())
            {
                m_dtxlabel->show();
                m_dtxprogress->show();
            }
            if(m_prev_desktopsession_id != m_desktopsession_id ||
               m_desktopsession_remain == 0)
                m_prev_desktopsession_id = 0;
        }
        else
        {
            if(m_dtxprogress->isVisible())
            {
                m_dtxlabel->hide();
                m_dtxprogress->hide();
            }
            else if(m_desktopsession_remain>0)
                m_prev_desktopsession_id = m_desktopsession_id;
        }

        if(TT_GetFlags(ttInst) & CLIENT_AUTHORIZED)
        {
            //change to away status if idle-time has been exceeded
            int idle_time = ttSettings->value(SETTINGS_GENERAL_AUTOAWAY).toInt();
            if(idle_time != 0)
            {
                QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();
                if(isComputerIdle(idle_time) && (m_statusmode & STATUSMODE_AWAY) == 0)
                {
                    m_statusmode |= STATUSMODE_AWAY;
                    TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));
                    m_idled_out = true;
                }
                else if(m_idled_out && !isComputerIdle(idle_time))
                {
                    m_statusmode &= ~STATUSMODE_AWAY;
                    TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));
                    m_idled_out = false;
                }
            }
        }
        break;
    case TIMER_VUMETER_UPDATE :
    {
        if(TT_GetFlags(ttInst) & CLIENT_SNDINPUT_READY)
        {
            int voicelevel = TT_GetSoundInputLevel(ttInst);
            if(ui.voiceactBar->value() != voicelevel)
            {
                ui.voiceactBar->setValue(voicelevel);
            }
        }
        break;
    }
    case TIMER_SEND_DESKTOPCURSOR :
    {
        if((TT_GetFlags(ttInst) & CLIENT_DESKTOP_ACTIVE) &&
           ttSettings->value(SETTINGS_DESKTOPSHARE_CURSOR, false).toBool())
        {
            sendDesktopCursor();
        }
        break;
    }
    case TIMER_RECONNECT :
        Q_ASSERT( (TT_GetFlags(ttInst) & CLIENT_CONNECTED) == 0);
        Disconnect();
        Connect();
        break;
    case TIMER_STATUSMSG :
        if(m_statusmsg.size())
        {
            QString status = m_statusmsg.dequeue();
            ui.statusbar->showMessage(status);
            if(m_statusmsg.empty())
            {
                killTimer(event->timerId());
                m_timers.erase(ite);
                m_timers[startTimer(2000)] = TIMER_STATUSMSG;
            }
        }
        else
        {
            killTimer(event->timerId());
            m_timers.erase(ite);
        }
        break;
    case TIMER_SEND_DESKTOPWINDOW :
        if((TT_GetFlags(ttInst) & CLIENT_TX_DESKTOP) == 0)
        {
            //only update desktop if there's users in the channel
            //(save bandwidth)

            users_t users = ui.channelsWidget->getUsers(m_mychannel.nChannelID);
            auto sub = std::find_if(users.begin(), users.end(), [] (const User& user) { return user.uPeerSubscriptions & SUBSCRIBE_DESKTOP;});
            if(sub != users.end())
                sendDesktopWindow();
        }
        else
            m_desktopsend_on_completion = true;
        break;
    case TIMER_APP_UPDATE :
        checkAppUpdate();
        break;
    default :
        Q_ASSERT(0);
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    ttSettings->setValue(SETTINGS_SOUND_MASTERVOLUME, ui.volumeSlider->value());
    ttSettings->setValue(SETTINGS_SOUND_MICROPHONEGAIN, ui.micSlider->value());
    ttSettings->setValue(SETTINGS_SOUND_VOICEACTIVATIONLEVEL, ui.voiceactSlider->value());

    ttSettings->setValue(SETTINGS_GENERAL_PUSHTOTALK, ui.actionEnablePushToTalk->isChecked());
    ttSettings->setValue(SETTINGS_GENERAL_VOICEACTIVATED, ui.actionEnableVoiceActivation->isChecked());

    if(windowState() == Qt::WindowNoState)
    {
        QRect r = geometry();
        QVariantList windowpos;
        windowpos.push_back(r.x());
        windowpos.push_back(r.y());
        windowpos.push_back(r.width());
        windowpos.push_back(r.height());
        ttSettings->setValue(SETTINGS_DISPLAY_WINDOWPOS, windowpos);
        ttSettings->setValue(SETTINGS_DISPLAY_SPLITTER, ui.splitter->saveState());
        ttSettings->setValue(SETTINGS_DISPLAY_VIDEOSPLITTER, ui.videosplitter->saveState());
        ttSettings->setValue(SETTINGS_DISPLAY_DESKTOPSPLITTER, ui.desktopsplitter->saveState());
    }

    ttSettings->setValue(SETTINGS_DISPLAY_FILESHEADER, ui.filesView->header()->saveState());

    return QMainWindow::closeEvent(event);
}

void MainWindow::changeEvent(QEvent* event )
{
    if(event->type() == QEvent::WindowStateChange)
    {
        if(ttSettings->value(SETTINGS_DISPLAY_TRAYMINIMIZE, false).toBool())
        {
            if(isMinimized())
            {
                if(!m_sysicon)
                {
                    m_sysicon = new QSystemTrayIcon(this);
                    connect(m_sysicon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                        SLOT(slotTrayIconChange(QSystemTrayIcon::ActivationReason)));
                    m_sysmenu = new QMenu(this);
                    QAction* restore = new QAction(tr("&Restore"), m_sysmenu);
                    connect(restore, SIGNAL(triggered()), SLOT(showNormal()));
                    QAction* exit = new QAction(tr("&Exit"), m_sysmenu);
                    connect(exit, SIGNAL(triggered(bool)), SLOT(slotClientExit(bool)));
                    m_sysmenu->addAction(restore);
                    m_sysmenu->addSeparator();
                    m_sysmenu->addAction(exit);
                    m_sysicon->setContextMenu(m_sysmenu);
                    m_sysicon->setToolTip(APPTITLE);
                }
                //choose icon to show in sys tray
                QString rc_icon;
                if(m_talking.size())
                    rc_icon = APPTRAYICON_ACTIVE;
                else if(TT_GetFlags(ttInst) & CLIENT_CONNECTED)
                    rc_icon = APPTRAYICON_CON;
                else
                    rc_icon = APPTRAYICON;
                m_sysicon->setIcon(QIcon(rc_icon));

                //ensure application doesn't close when QSystemTrayIcon is active
                //(Qt is weird! Seems it doesn't distinguish 'closed' and 'hidden')
                QApplication::setQuitOnLastWindowClosed(false);
                m_sysicon->show();

                QTimer::singleShot(0, this, SLOT(hide()));
            }
            else if(m_sysicon)
            {
                QApplication::setQuitOnLastWindowClosed(true);
                m_sysicon->hide();
            }
        }
    }
    QMainWindow::changeEvent(event);
}

#if defined(Q_OS_WIN32) && QT_VERSION >= 0x050000

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message,
                             long* result)
{
    MSG* msg = reinterpret_cast<MSG*>(message);
    if(msg->message == WM_TEAMALK_CLIENTEVENT)
    {
        TTMessage ttmsg;
        INT32 wait_ms = 0;
        if(TT_GetMessage(ttInst, &ttmsg, &wait_ms))
            processTTMessage(ttmsg);
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}
#endif

void MainWindow::updateWindowTitle()
{
    QString profilename, title;
    if(ttSettings)
        profilename = ttSettings->value(SETTINGS_GENERAL_PROFILENAME).toString();

    if(m_mychannel.nChannelID > 0 &&
       m_mychannel.nChannelID != TT_GetRootChannelID(ttInst))
        title = QString("%1 - %2").arg(_Q(m_mychannel.szName)).arg(APPTITLE);
    else
        title = APPTITLE;
    if(profilename.size())
        title = QString("%1 - %2").arg(title).arg(profilename);
    setWindowTitle(title);
}

#if defined(Q_OS_WIN32)
void MainWindow::firewallInstall()
{
    //add firewall exception
    QString appPath = QApplication::applicationFilePath();
    appPath = QDir::toNativeSeparators(appPath);
    if(!TT_Firewall_AppExceptionExists(_W(appPath)))
    {
        QMessageBox::StandardButton answer = QMessageBox::question(this, 
            APPTITLE, 
            tr("Do you wish to add %1 to the Windows Firewall exception list?")
            .arg(APPTITLE), 
            QMessageBox::Yes | QMessageBox::No);

        if(answer == QMessageBox::Yes &&
            !TT_Firewall_AddAppException(_W(QString(APPTITLE)), 
                                        _W(appPath)))
        {
            QMessageBox::critical(this, tr("Firewall exception"),
                tr("Failed to add %1 to Windows Firewall exceptions.")
                .arg(appPath));
        }
    }
}
#endif

void MainWindow::subscribeCommon(bool checked, Subscriptions subs, int userid/* = 0*/)
{
    QVector<int> userids;

    if(userid == 0)
        userids = ui.channelsWidget->selectedUsers();
    else
        userids.push_back(userid);

    foreach(userid, userids)
    {
        if(checked)
        {
            int cmdid = TT_DoSubscribe(ttInst, userid, subs);
            if(cmdid>0)
                m_commands[cmdid] = CMD_COMPLETE_SUBSCRIBE;
        }
        else
        {
            int cmdid = TT_DoUnsubscribe(ttInst, userid, subs);
            if(cmdid>0)
                m_commands[cmdid] = CMD_COMPLETE_UNSUBSCRIBE;
        }
    }
}

TextMessageDlg* MainWindow::getTextMessageDlg(int userid)
{
    usermsg_t::iterator ite = m_usermsg.find(userid);
    if(ite != m_usermsg.end())
        return *ite;
    else
    {
        User user;
        if(!ui.channelsWidget->getUser(userid, user))
            return nullptr;

        TextMessageDlg* dlg;
        usermessages_t::iterator ii = m_usermessages.find(userid);
        if(ii != m_usermessages.end())
            dlg = new TextMessageDlg(user, ii.value(), nullptr);
        else
            dlg = new TextMessageDlg(user, nullptr);

        dlg->setAttribute(Qt::WA_DeleteOnClose);
        m_usermsg.insert(userid, dlg);
        connect(dlg, SIGNAL(newMyselfTextMessage(const TextMessage&)),
                SLOT(slotNewMyselfTextMessage(const TextMessage&)));
        connect(dlg, SIGNAL(closedTextMessage(int)), SLOT(slotTextMessageClosed(int)));
        connect(this, SIGNAL(userUpdate(const User&)), dlg, SLOT(slotUpdateUser(const User&)));
        connect(this, SIGNAL(newTextMessage(const TextMessage&)), dlg, 
                SLOT(slotNewMessage(const TextMessage&)));
        connect(this, SIGNAL(userLogout(const User&)), dlg, SLOT(slotUserLogout(const User&)));
        return dlg;
    }
}

void MainWindow::processTextMessage(const TextMessage& textmsg)
{
    switch(textmsg.nMsgType)
    {
    case MSGTYPE_CHANNEL :
    {
        QString line;
        line = ui.chatEdit->addTextMessage(textmsg);
        ui.videochatEdit->addTextMessage(textmsg);
        ui.desktopchatEdit->addTextMessage(textmsg);

        //setup channel text logging
        QString chanlog = ttSettings->value(SETTINGS_MEDIASTORAGE_CHANLOGFOLDER).toString();
        if(chanlog.size())
        {
            if(!m_logChan.isOpen())
                openLogFile(m_logChan, chanlog, _Q(m_mychannel.szName) + ".clog");
            writeLogEntry(m_logChan, line);
        }

        playSoundEvent(SOUNDEVENT_CHANNELMSG);
        break;
    }
    case MSGTYPE_BROADCAST :
        ui.chatEdit->addTextMessage(textmsg);
        ui.videochatEdit->addTextMessage(textmsg);
        ui.desktopchatEdit->addTextMessage(textmsg);
        playSoundEvent(SOUNDEVENT_BROADCASTMSG);
        break;
    case MSGTYPE_USER :
    {
        if(ttSettings->value(SETTINGS_DISPLAY_MESSAGEPOPUP, true).toBool())
        {
            TextMessageDlg* dlg = getTextMessageDlg(textmsg.nFromUserID);
            if(dlg)
            {
                dlg->show();
                dlg->activateWindow();
                dlg->raise();
            }
        }
        ui.channelsWidget->setUserMessaged(textmsg.nFromUserID, true);
        emit(newTextMessage(textmsg));
        playSoundEvent(SOUNDEVENT_USERMSG);
        break;
    }
    case MSGTYPE_CUSTOM :
    {
        emit(newTextMessage(textmsg));

        QStringList cmd = getCustomCommand(textmsg);
        if(cmd.size() < 2)
            break;
        
        if(cmd[0] == TT_INTCMD_DESKTOP_ACCESS)
        {
            User user;
            if(!ui.channelsWidget->getUser(textmsg.nFromUserID, user))
                break;

            ui.channelsWidget->setUserDesktopAccess(textmsg.nFromUserID, cmd[1] == "1");
            if(cmd[1] == "1")
            {
                addStatusMsg(QString(tr("%1 is requesting desktop access")
                             .arg(getDisplayName(user))));
                playSoundEvent(SOUNDEVENT_DESKTOPACCESS);
                if(hasDesktopAccess(m_desktopaccess_entries, user))
                {
                    subscribeCommon(true, SUBSCRIBE_DESKTOPINPUT, user.nUserID);
                    addStatusMsg(QString(tr("%1 granted desktop access")
                                 .arg(getDisplayName(user))));
                }
            }
            else
            {
                addStatusMsg(QString(tr("%1 retracted desktop access")
                             .arg(getDisplayName(user))));
                subscribeCommon(false, SUBSCRIBE_DESKTOPINPUT, user.nUserID);
            }
        }
    }
    break;
    }
}

void MainWindow::processMyselfJoined(int channelid)
{
    TT_GetChannel(ttInst, channelid, &m_mychannel);
    //Enable AGC, denoise etc.
    updateAudioConfig();

    TTCHAR buff[TT_STRLEN] = {};
    TT_GetChannelPath(ttInst, channelid, buff);
    addStatusMsg(tr("Joined channel %1").arg(_Q(buff)));

    //store new muxed audio file if we're changing channel
    if(ui.actionMediaStorage->isChecked() &&
        (m_audiostorage_mode & AUDIOSTORAGE_SINGLEFILE))
    {
        updateAudioStorage(false, AUDIOSTORAGE_SINGLEFILE);
        updateAudioStorage(true, AUDIOSTORAGE_SINGLEFILE);
    }

    updateWindowTitle();
}

void MainWindow::processMyselfLeft(int channelid)
{
    Q_UNUSED(channelid);
    ZERO_STRUCT(m_mychannel);

    m_talking.clear();
    ui.videogridWidget->ResetGrid();
    ui.desktopgridWidget->ResetGrid();

    if(m_logChan.isOpen())
    {
        QString filename = m_logChan.fileName();
        quint64 size = m_logChan.size();
        m_logChan.close();
        if(size == 0)
            QFile::remove(filename);
    }
    updateWindowTitle();
}

void MainWindow::killLocalTimer(TimerEvent e)
{
    timers_t::iterator ite = m_timers.begin();
    while(ite != m_timers.end())
    {
        if(*ite == e)
        {
            killTimer(ite.key());
            m_timers.erase(ite);
            break;
        }
        ite++;
    }
}

bool MainWindow::timerExists(TimerEvent e)
{
    timers_t::iterator ite = m_timers.begin();
    while(ite != m_timers.end())
    {
        if(*ite == e)
            return true;
        ite++;
    }
    return false;
}

void MainWindow::updateChannelFiles(int channelid)
{
    m_filesmodel->slotChannelUpdated(channelid);
    TTCHAR chanpath[TT_STRLEN] = {};
    TT_GetChannelPath(ttInst, channelid, chanpath);
    ui.channelLabel->setText(tr("Files in channel: %1").arg(_Q(chanpath)));

    if(m_filesmodel->rowCount() == 0)
        ui.tabWidget->setTabText(TAB_FILES, tr("Files"));
    else
        ui.tabWidget->setTabText(TAB_FILES, tr("Files (%1)").arg(m_filesmodel->rowCount()));
}

void MainWindow::updateUserSubscription(int userid)
{
    Subscriptions unsub = SUBSCRIBE_NONE;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_USERMSG, true).toBool())
        unsub |= SUBSCRIBE_USER_MSG;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_CHANNELMSG, true).toBool())
        unsub |= SUBSCRIBE_CHANNEL_MSG;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_BROADCASTMSG, true).toBool())
        unsub |= SUBSCRIBE_BROADCAST_MSG;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_VOICE, true).toBool())
        unsub |= SUBSCRIBE_VOICE;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_VIDEOCAPTURE, true).toBool())
        unsub |= SUBSCRIBE_VIDEOCAPTURE;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_DESKTOP, true).toBool())
        unsub |= SUBSCRIBE_DESKTOP;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_MEDIAFILE, true).toBool())
        unsub |= SUBSCRIBE_MEDIAFILE;

    if(unsub)
        subscribeCommon(false, unsub, userid);
}

void MainWindow::updateAudioStorage(bool enable, AudioStorageMode mode)
{
    AudioFileFormat aff = (AudioFileFormat)ttSettings->value(SETTINGS_MEDIASTORAGE_FILEFORMAT, 
                                                             AFF_WAVE_FORMAT).toInt();
    QString audiofolder = ttSettings->value(SETTINGS_MEDIASTORAGE_AUDIOFOLDER).toString();

    if(mode == AUDIOSTORAGE_SINGLEFILE)
    {
        if(enable)
        {
            //don't try to record with no audio codec specified (it will fail)
            if(m_mychannel.audiocodec.nCodec == NO_CODEC)
                return;

            QString filepath = audiofolder;
            filepath += QDir::toNativeSeparators("/") + generateAudioStorageFilename(aff);

            if(!TT_StartRecordingMuxedAudioFile(ttInst, &m_mychannel.audiocodec, 
                                                _W(filepath), aff))
            {
                QMessageBox::critical(this, tr("Error"), tr("Failed to start recording"));
                return;
            }
            else
                addStatusMsg(tr("Recording to file: %1").arg(filepath));
        }
        else
        {
            TT_StopRecordingMuxedAudioFile(ttInst);
        }
    }
    
    if(mode == AUDIOSTORAGE_SEPARATEFILES)
    {
        int userCount = 0;
        QVector<User> users;
        TT_GetServerUsers(ttInst, nullptr, &userCount);
        if(userCount)
        {
            users.resize(userCount);
            TT_GetServerUsers(ttInst, &users[0], &userCount);
        }
        User u; u.nUserID = AUDIOSTORAGE_LOCAL_USERID;
        users.push_back(u); //also store local user's voice stream
        for(int i=0;i<users.size();i++)
        {
            if(enable)
                TT_SetUserMediaStorageDir(ttInst, users[i].nUserID, _W(audiofolder), nullptr, aff);
            else
                TT_SetUserMediaStorageDir(ttInst, users[i].nUserID, _W(QString()), nullptr, aff);
        }
    }
}

void MainWindow::updateAudioConfig()
{
    SpeexDSP spxdsp = {};

    //set default values for audio config
    spxdsp.bEnableAGC = ttSettings->value(SETTINGS_SOUND_AGC, SETTINGS_SOUND_AGC_DEFAULT).toBool();
    spxdsp.nGainLevel = DEFAULT_AGC_GAINLEVEL;
    spxdsp.nMaxIncDBSec = DEFAULT_AGC_INC_MAXDB;
    spxdsp.nMaxDecDBSec = DEFAULT_AGC_DEC_MAXDB;
    spxdsp.nMaxGainDB = DEFAULT_AGC_GAINMAXDB;

    spxdsp.bEnableDenoise = ttSettings->value(SETTINGS_SOUND_DENOISING, 
                                              SETTINGS_SOUND_DENOISING_DEFAULT).toBool();
    spxdsp.nMaxNoiseSuppressDB = DEFAULT_DENOISE_SUPPRESS;

    spxdsp.bEnableEchoCancellation = ttSettings->value(SETTINGS_SOUND_ECHOCANCEL,
                                                       SETTINGS_SOUND_ECHOCANCEL_DEFAULT).toBool();
    spxdsp.nEchoSuppress = DEFAULT_ECHO_SUPPRESS;
    spxdsp.nEchoSuppressActive = DEFAULT_ECHO_SUPPRESSACTIVE;

    //check if channel AGC settings should override default settings
    if(m_mychannel.audiocfg.bEnableAGC)
    {
        spxdsp.bEnableAGC = m_mychannel.audiocfg.bEnableAGC;
        spxdsp.nGainLevel = m_mychannel.audiocfg.nGainLevel;
        //override preset sound gain
        TT_SetSoundInputGainLevel(ttInst, SOUND_GAIN_DEFAULT);
        TT_SetSoundInputPreprocess(ttInst, &spxdsp);
        ui.micSlider->setToolTip(tr("Microphone gain is controlled by channel"));
    }
    else
    {
        SoundDeviceEffects effects = {};
        TT_GetSoundDeviceEffects(ttInst, &effects);
        
        // If sound device provides AGC, AEC and denoise then use these instead
        spxdsp.bEnableAGC &= !effects.bEnableAGC;
        spxdsp.bEnableEchoCancellation &= !effects.bEnableEchoCancellation;
        spxdsp.bEnableDenoise &= !effects.bEnableDenoise;

        TT_SetSoundInputPreprocess(ttInst, &spxdsp);
        slotMicrophoneGainChanged(ui.micSlider->value());
        ui.micSlider->setToolTip(tr("Microphone gain"));
    }
    ui.micSlider->setEnabled(!m_mychannel.audiocfg.bEnableAGC);
}

bool MainWindow::sendDesktopWindow()
{
    switch(ttSettings->value(SETTINGS_DESKTOPSHARE_MODE, 
                             DESKTOPSHARE_NONE).toInt())
    {
    case DESKTOPSHARE_DESKTOP :
#if defined(Q_OS_DARWIN)
        //TODO: macOS, entire desktop
        m_nCGShareWnd = kCGNullWindowID;
#endif
        break;
    case DESKTOPSHARE_ACTIVE_WINDOW :
#if defined(Q_OS_WIN32)
        m_hShareWnd = TT_Windows_GetDesktopActiveHWND();
#elif defined(Q_OS_LINUX)
    {
        if(!m_display)
            m_display = XOpenDisplay(0);

        if(!m_display)
            return false;

        int revert_to_return = 0;
        Window winid = 0;
        XGetInputFocus(m_display, &winid, &revert_to_return);
        m_nWindowShareWnd = winid;
    }
#elif defined(Q_OS_DARWIN)
    //TODO: macOS, active window
        m_nCGShareWnd = kCGNullWindowID;
#endif
    break;
    case DESKTOPSHARE_SPECIFIC_WINDOW :
        break;
    default :
        return false;
    }

    BitmapFormat bmp_mode = 
        (BitmapFormat)ttSettings->value(SETTINGS_DESKTOPSHARE_BMPMODE, 
                                        BMP_RGB16_555).toInt();

    int ret = 0;
#if defined(Q_OS_WIN32)
    if(m_hShareWnd)
        ret = TT_SendDesktopWindowFromHWND(ttInst, m_hShareWnd, bmp_mode, DESKTOPPROTOCOL_ZLIB_1);
/*
    const int W = 100, H = 100;
    QImage img(W, H, QImage::Format_RGB32);
    for(int i=0;i<W;i++)
        img.setPixel(i, 0, 0xF000);
    for(int i=0;i<W;i++)
        img.setPixel(i, H-1, 0xF000);
    for(int i=0;i<H;i++)
        img.setPixel(0, i, 0x00F0);
    for(int i=0;i<H;i++)
        img.setPixel(W-1, i, 0x00F0);

    DesktopWindow wnd;
    ZERO_STRUCT(wnd);
    wnd.bmpFormat = BMP_RGB32;
    wnd.nProtocol = DESKTOPPROTOCOL_ZLIB_1;
    wnd.nWidth = W;
    wnd.nHeight = H;
    ret = TT_SendDesktopWindow(ttInst, img.bits(), img.byteCount(), &wnd, BMP_RGB32);
*/
#elif defined(Q_OS_DARWIN)
    if(m_nCGShareWnd)
        ret = TT_SendDesktopFromWindowID(ttInst, m_nCGShareWnd, bmp_mode, DESKTOPPROTOCOL_ZLIB_1);

#elif defined(Q_OS_LINUX)

    if(!m_display)
        return false;

    XWindowAttributes attr;
    Status s = XGetWindowAttributes(m_display, m_nWindowShareWnd, &attr);
    if(s)
    {
        XImage* img = XGetImage(m_display, m_nWindowShareWnd, 0, 0, attr.width,
                                attr.height, -1, ZPixmap);
        if(img)
        {
            DesktopWindow wnd;
            ZERO_STRUCT(wnd);
            
            wnd.nProtocol = DESKTOPPROTOCOL_ZLIB_1;
            wnd.nWidth = img->width;
            wnd.nHeight = img->height;
            wnd.nBytesPerLine = img->bytes_per_line;
            switch(img->bits_per_pixel)
            {
            case 16 :
                wnd.bmpFormat = BMP_RGB16_555;
                break;
            case 24 :
                wnd.bmpFormat = BMP_RGB24;
                break;
            case 32 :
                wnd.bmpFormat = BMP_RGB32;
                break;
            default :
                return false;
            }

            wnd.nFrameBufferSize = img->bytes_per_line * img->height;
            wnd.frameBuffer = img->data;
            ret = TT_SendDesktopWindow(ttInst, &wnd, bmp_mode);

            XDestroyImage(img);
        }
    }
#endif
    return ret >= 0;
}

void MainWindow::restartSendDesktopWindowTimer()
{
    killLocalTimer(TIMER_SEND_DESKTOPWINDOW);
    int upd_interval = ttSettings->value(SETTINGS_DESKTOPSHARE_INTERVAL).toInt();
    if(upd_interval>0)
    {
        m_timers.insert(startTimer(upd_interval), TIMER_SEND_DESKTOPWINDOW);
        m_timers.insert(startTimer(50), TIMER_SEND_DESKTOPCURSOR);
    }
}

QRect MainWindow::getSharedWindowRect()
{
#if defined(Q_OS_LINUX)
    //TODO: X11, get active window (this only handles entire desktop)

    if(!m_display)
        return QRect();

    XWindowAttributes attr;
    Status s = XGetWindowAttributes(m_display, m_nWindowShareWnd, &attr);
    if(s)
        return QRect(attr.x, attr.y, attr.x + attr.width, attr.y + attr.height);

#elif defined(Q_OS_DARWIN)
    ShareWindow wnd;
    ZERO_STRUCT(wnd);

    if(TT_MacOS_GetWindowFromWindowID(m_nCGShareWnd, &wnd))
        return QRect(wnd.nWindowX, wnd.nWindowY, wnd.nWindowX + wnd.nWidth,
                     wnd.nWindowY + wnd.nHeight);
#elif defined(Q_OS_WIN32)
    RECT r;
    if(::GetWindowRect(m_hShareWnd, &r))
        return QRect(r.left, r.top, r.right-r.left, r.bottom - r.top);
#endif
    return QRect();
}

void MainWindow::sendDesktopCursor()
{   
    QPoint curPos = QCursor::pos();

#if defined(Q_OS_LINUX)
    if(!m_display)
        return;

    XWindowAttributes attr;
    Status s = XGetWindowAttributes(m_display, m_nWindowShareWnd, &attr);

    if(s)
    {
        int x = curPos.x() - attr.x;
        int y = curPos.y() - attr.y;
        x = qMax(-1, x);
        y = qMax(-1, y);
        curPos = QPoint(x, y);
    }
    else
    {
        curPos = QPoint(-1, -1);
    }
#elif defined(Q_OS_DARWIN)
    ShareWindow wnd;
    if(!TT_MacOS_GetWindowFromWindowID(m_nCGShareWnd, &wnd))
        return;

    int x = curPos.x() - wnd.nWindowX;
    int y = curPos.y() - wnd.nWindowY;
    if(x<0 || x>wnd.nWidth || y < 0 || y > wnd.nHeight)
        x = y = -1;
    curPos = QPoint(x, y);
#elif defined(Q_OS_WIN32)
    ShareWindow wnd;
    if(!TT_Windows_GetWindow(m_hShareWnd, &wnd))
        return;

    int x = curPos.x() - wnd.nWndX;
    int y = curPos.y() - wnd.nWndY;
    if(x<0 || x>wnd.nWidth || y < 0 || y > wnd.nHeight)
        x = y = -1;
    curPos = QPoint(x, y);
#endif

    if(m_lastCursorPos != curPos)
    {
        m_lastCursorPos = curPos;
        TT_SendDesktopCursorPosition(ttInst, UINT16(m_lastCursorPos.x()),
                                     UINT16(m_lastCursorPos.y()));
        
    }
}

void MainWindow::processDesktopInput(int userid, const DesktopInput& input)
{
    Q_UNUSED(userid);

    //extract as many DesktopInputs as possible
    QVector<DesktopInput> inputs;
    inputs.push_back(input);

    if(inputs.isEmpty())return;

    //ignore input if we're not sharing a window
    if((TT_GetFlags(ttInst) & CLIENT_DESKTOP_ACTIVE) == 0)
        return;
/*
    //ignore input if incorrect window has focus
    DesktopShareMode mode = (DesktopShareMode)ttSettings->value(SETTINGS_DESKTOPSHARE_MODE, 
                             DESKTOPSHARE_NONE).toInt();
    switch(mode)
    {
    case DESKTOPSHARE_SPECIFIC_WINDOW :
#if defined(Q_OS_WIN32)
        if(m_hShareWnd != TT_Windows_GetDesktopActiveHWND())
        {
            qDebug() << "Ignoring desktop input. Incorrect window has focus.";
            return;
        }
#elif defined(Q_OS_LINUX)
     {
        if(!m_display)
            return;

        int revert_to_return = 0;
        Window winid = 0;
        XGetInputFocus(m_display, &winid, &revert_to_return);
        if(m_nWindowShareWnd != winid)
        {
            qDebug() << "Ignoring desktop input. Incorrect window has focus.";
            return;
        }
     }
#else
        qDebug() << "Ignoring desktop input. Share specific window not supported on macOS";
        return;
#endif
     break;
    case DESKTOPSHARE_ACTIVE_WINDOW :
    case DESKTOPSHARE_DESKTOP :
    default :
        break;
    }
*/
    TTKeyTranslate key_trans = TTKEY_NO_TRANSLATE;
#if ENABLE_KEY_TRANSLATION
#if defined(Q_OS_WIN32)
    key_trans = TTKEY_TTKEYCODE_TO_WINKEYCODE;
#elif defined(Q_OS_DARWIN)
    key_trans = TTKEY_TTKEYCODE_TO_MACKEYCODE;
#elif defined(Q_OS_LINUX)
//TODO: X11, key translate for X11
#endif
#endif /* ENABLE_KEY_TRANSLATION */

    QVector<DesktopInput> executeInputs;

    QRect offset = getSharedWindowRect();
    for(int i=0;i<inputs.size();i++)
    {
        //calculate absolute offset
        if(inputs[i].uMousePosX != TT_DESKTOPINPUT_MOUSEPOS_IGNORE &&
           inputs[i].uMousePosY != TT_DESKTOPINPUT_MOUSEPOS_IGNORE)
        {
            //don't allow mouse input outside shared window
            if(inputs[i].uMousePosX > offset.width() ||
               inputs[i].uMousePosY > offset.height())
                continue;

            inputs[i].uMousePosX += offset.left();
            inputs[i].uMousePosY += offset.top();

        }
        //QDebug(QtDebugMsg) << "DesktopInput " << inputs[i].uMousePosX << "," << inputs[i].uMousePosY
        //                   << inputs[i].uKeyCode << inputs[i].uKeyState;

        //QDebug dbg2(QtDebugMsg);
        //dbg2 << "Mouse input: ";
        //dbg2 << input.mouseInputs[i].nPosX << "," << input.mouseInputs[i].nPosY;
        //dbg2 << "Buttons: "; 
        //for(int j=0;j<TT_DESKTOPINPUT_KEYS_MAX;j++)
        //    dbg2 << input.mouseInputs[i].buttonsDown[j] << ",";

        DesktopInput trans_input;
        ZERO_STRUCT(trans_input);

        if(key_trans == TTKEY_NO_TRANSLATE)
            executeInputs.push_back(inputs[i]);
        else if(TT_DesktopInput_KeyTranslate(key_trans, &inputs[i], &trans_input, 1))
            executeInputs.push_back(trans_input);
        else
            qDebug() << "Failed to translate received desktop input. KeyCode: 0x" 
                     << QString::number(inputs[i].uKeyCode, 16);
    }

    if(executeInputs.size())
    {
#if defined(Q_OS_LINUX)
        for(int i=0;i<executeInputs.size();i++)
            executeDesktopInput(executeInputs[i]);
#else
        TT_DesktopInput_Execute(&executeInputs[0], executeInputs.size());
#endif
        //send desktop update immediately and restart desktop tx timer.
        if(sendDesktopWindow())
            restartSendDesktopWindowTimer();
        else
            m_desktopsend_on_completion = true;
    }
}

void MainWindow::startStreamMediaFile()
{
    QString fileName = ttSettings->value(QString(SETTINGS_STREAMMEDIA_FILENAME).arg(0)).toString();

    VideoCodec vidcodec;
    vidcodec.nCodec = (Codec)ttSettings->value(SETTINGS_STREAMMEDIA_CODEC).toInt();
    switch(vidcodec.nCodec)
    {
    case WEBM_VP8_CODEC :
        vidcodec.webm_vp8.nRcTargetBitrate = ttSettings->value(SETTINGS_STREAMMEDIA_WEBMVP8_BITRATE).toInt();
        vidcodec.webm_vp8.nEncodeDeadline = DEFAULT_WEBMVP8_DEADLINE;
        break;
    default :
        break;
    }

    MediaFilePlayback mfp = {};
    mfp.audioPreprocessor.nPreprocessor = AudioPreprocessorType(ttSettings->value(SETTINGS_STREAMMEDIA_AUDIOPREPROCESSOR,
        SETTINGS_STREAMMEDIA_AUDIOPREPROCESSOR_DEFAULT).toInt());
    loadAudioPreprocessor(mfp.audioPreprocessor);
    mfp.bPaused = false;
    mfp.uOffsetMSec = ttSettings->value(SETTINGS_STREAMMEDIA_OFFSET, SETTINGS_STREAMMEDIA_OFFSET_DEFAULT).toUInt();
    if (!TT_StartStreamingMediaFileToChannelEx(ttInst, _W(fileName), &mfp, &vidcodec))
    {
        QMessageBox::information(this,
                                 MENUTEXT(ui.actionStreamMediaFileToChannel->text()),
                                 QString(tr("Failed to stream media file %1").arg(fileName)));
        stopStreamMediaFile();
    }
    else
    {
        m_statusmode |= STATUSMODE_STREAM_MEDIAFILE;
        ////since streaming video takes over webcam stream we show as 
        ////transmitting video
        //if(tx_mode & TRANSMIT_VIDEO)
        //    m_statusmode |= STATUSMODE_VIDEOTX;

        QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();
        TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));
    }
}

void MainWindow::stopStreamMediaFile()
{
    TT_StopStreamingMediaFileToChannel(ttInst);

    m_statusmode &= ~STATUSMODE_STREAM_MEDIAFILE;
    ////clear video if not transmitting
    //if(TT_IsTransmitting(ttInst, TRANSMIT_VIDEO) == TRANSMIT_NONE)
    //    m_statusmode &= ~STATUSMODE_VIDEOTX;

    QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();
    TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));

    slotUpdateUI();
}

void MainWindow::loadHotKeys()
{
    hotkey_t hotkey;
    if(loadHotKeySettings(HOTKEY_VOICEACTIVATION, hotkey))
        enableHotKey(HOTKEY_VOICEACTIVATION, hotkey);
    hotkey.clear();
    if(loadHotKeySettings(HOTKEY_INCVOLUME, hotkey))
        enableHotKey(HOTKEY_INCVOLUME, hotkey);
    hotkey.clear();
    if(loadHotKeySettings(HOTKEY_DECVOLUME, hotkey))
        enableHotKey(HOTKEY_DECVOLUME, hotkey);
    hotkey.clear();
    if(loadHotKeySettings(HOTKEY_MUTEALL, hotkey))
        enableHotKey(HOTKEY_MUTEALL, hotkey);
    hotkey.clear();
    if(loadHotKeySettings(HOTKEY_MICROPHONEGAIN_INC, hotkey))
        enableHotKey(HOTKEY_MICROPHONEGAIN_INC, hotkey);
    hotkey.clear();
    if(loadHotKeySettings(HOTKEY_MICROPHONEGAIN_DEC, hotkey))
        enableHotKey(HOTKEY_MICROPHONEGAIN_DEC, hotkey);
    hotkey.clear();
    if(loadHotKeySettings(HOTKEY_VIDEOTX, hotkey))
        enableHotKey(HOTKEY_VIDEOTX, hotkey);
}

void MainWindow::enableHotKey(HotKeyID id, const hotkey_t& hk)
{
    Q_ASSERT(hk.size());

    //disable first so we don't double register
    disableHotKey(id);

#ifdef Q_OS_WIN32
    TT_HotKey_Register(ttInst, id, &hk[0], hk.size());

#elif defined(Q_OS_LINUX)

    Display* display = QX11Info::display();
    Window x11window = QX11Info::appRootWindow();

    keycomp_t keycomp;
    quint32 mods = 0, keycode = 0;
    for(int i=0;i<hk.size();i++)
    {
        switch(hk[i])
        {
        case Qt::CTRL :
            mods |= ControlMask;
            keycomp.insert(ControlMask);
            break;
        case Qt::ALT :
            mods |= Mod1Mask;
            keycomp.insert(Mod1Mask);
            break;
        case Qt::SHIFT :
            mods |= ShiftMask;
            keycomp.insert(ShiftMask);
            break;
        default:
            keycode = XKeysymToKeycode(display, XStringToKeysym(QKeySequence(hk[i]).toString().toLatin1().data()));  
            keycomp.insert(keycode);
            break;
        }
    }

    m_hotkeys.insert(id, keycomp);
    Bool owner = True;
    int pointer = GrabModeAsync;
    int keyboard = GrabModeAsync;

    // no way to check for success
    XGrabKey(display, keycode, mods, x11window, owner, pointer, keyboard);
    // allow numlock
    XGrabKey(display, keycode, mods | Mod2Mask, x11window, owner, pointer, keyboard);

#elif defined(Q_OS_DARWIN)

    if(hk.empty() || hk.size() != MAC_HOTKEY_SIZE)
        return;

    quint32 mods = 0, keycode = 0;
    if(hk[0] != (INT32)MAC_NO_KEY)
    {
        if(hk[0] & cmdKey)
            mods |= cmdKey;
        if(hk[0] & optionKey)
            mods |= optionKey;
        if(hk[0] & shiftKey)
            mods |= shiftKey;
        if(hk[0] & controlKey)
            mods |= controlKey;
    }

    keycode = hk[1];

    EventHotKeyID keyID;
    keyID.signature = 'cute';
    keyID.id = id;

    EventHotKeyRef ref = nullptr;
    if(RegisterEventHotKey(keycode, mods, keyID, GetApplicationEventTarget(), 0, &ref) == 0)
        m_hotkeys[id] = ref;
    else
    {
        QMessageBox::warning(this, tr("Enable HotKey"), 
                             tr("Failed to register hotkey. Please try another key combination."));
    }
#endif

    if(id == HOTKEY_PUSHTOTALK)
        m_pttlabel->setText(tr("Push To Talk: ") + getHotKeyText(hk));
}

void MainWindow::disableHotKey(HotKeyID id)
{
#ifdef Q_OS_WIN32

    TT_HotKey_Unregister(ttInst, id);

#elif defined(Q_OS_LINUX)

    Display* display = QX11Info::display();
    Window window = QX11Info::appRootWindow();

    reghotkeys_t::iterator hk_ite = m_hotkeys.find(id);
    if(hk_ite != m_hotkeys.end())
    {
        quint32 mods = 0;
        quint32 keycode = 0;
        const keycomp_t& comp = hk_ite.value();
        keycomp_t::const_iterator ite = comp.begin();
        while(ite != comp.end())
        {
            switch(*ite)
            {
            case ControlMask :
                mods |= ControlMask;
                break;
            case Mod1Mask :
                mods |= Mod1Mask;
                break;
            case ShiftMask :
                mods |= ShiftMask;
                break;
            default:
                keycode = *ite;
                break;
            }
            ite++;
        }
        XUngrabKey(display, keycode, mods, window);
        XUngrabKey(display, keycode, mods | Mod2Mask, window);
    }

    m_hotkeys.remove(id);

#elif defined(Q_OS_DARWIN)

    reghotkeys_t::iterator i = m_hotkeys.find(id);
    if(i != m_hotkeys.end())
    {
        UnregisterEventHotKey(i.value());
        m_hotkeys.erase(i);
    }

#endif

    if(id == HOTKEY_PUSHTOTALK)
        m_pttlabel->setText("");
}

#if defined(Q_OS_LINUX)
void MainWindow::executeDesktopInput(const DesktopInput& input)
{
    //TODO: X11, mouse held down for selection (e.g. text in edit field)
    //TODO: X11, key strokes

    if(input.uMousePosX != TT_DESKTOPINPUT_MOUSEPOS_IGNORE &&
       input.uMousePosY != TT_DESKTOPINPUT_MOUSEPOS_IGNORE)
    {
         XWarpPointer(m_display, None, (Window)m_nWindowShareWnd, 0, 0, 0, 0,
                      input.uMousePosX, input.uMousePosY);

        XEvent event;
        ZERO_STRUCT(event);
        
        switch(input.uKeyState)
        {
        case DESKTOPKEYSTATE_DOWN :
            event.type = ButtonPress;
            event.xbutton.same_screen = True;
            event.xbutton.send_event = True;
            event.xbutton.x = input.uMousePosX;
            event.xbutton.y = input.uMousePosY;
            break;
        case DESKTOPKEYSTATE_UP :
            event.type = ButtonRelease;
            event.xbutton.same_screen = True;
            event.xbutton.send_event = True;
            event.xbutton.x = input.uMousePosX;
            event.xbutton.y = input.uMousePosY;
            break;
        default :
            return;
        }

        switch(input.uKeyCode)
        {
        case TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN :
            event.xbutton.button = Button1;
            break;
        case TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN :
            event.xbutton.button = Button3;
            break;
        case TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN :
            event.xbutton.button = Button2;
            break;
        }

        if(input.uKeyState == DESKTOPKEYSTATE_DOWN || input.uKeyState == DESKTOPKEYSTATE_UP)
        {
            XQueryPointer(m_display, (Window)m_nWindowShareWnd, &event.xbutton.root,
                          &event.xbutton.window, &event.xbutton.x_root,
                          &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y,
                          &event.xbutton.state);

            event.xbutton.subwindow = event.xbutton.window;

            while(event.xbutton.subwindow)
            {
                event.xbutton.window = event.xbutton.subwindow;

                XQueryPointer(m_display, event.xbutton.window, &event.xbutton.root,
                              &event.xbutton.subwindow, &event.xbutton.x_root,
                              &event.xbutton.y_root, &event.xbutton.x,
                              &event.xbutton.y, &event.xbutton.state);
            }
        }

        XSendEvent(m_display, PointerWindow, True, 0, &event);
        XFlush(m_display);
    }
}
#endif

void MainWindow::checkAppUpdate()
{
    // check for software update and get bearware.dk web-login url
    QUrl url(URL_APPUPDATE);

    auto networkMgr = new QNetworkAccessManager(this);
    connect(networkMgr, SIGNAL(finished(QNetworkReply*)),
            SLOT(slotSoftwareUpdateReply(QNetworkReply*)));

    QNetworkRequest request(url);
    networkMgr->get(request);
}

void MainWindow::slotClientNewInstance(bool /*checked=false*/)
{
    QString inipath = ttSettings->fileName();

    // check if we are creating a new profile from a profile
    if(ttSettings->value(SETTINGS_GENERAL_PROFILENAME).toString().size())
    {
        inipath.remove(QRegExp(".\\d{1,2}$"));
    }

    // load existing profiles
    QMap<QString, QString> profiles;
    QStringList profilenames;
    const int MAX_PROFILES = 16;
    int freeno = -1;
    for (int i = 1;i <= MAX_PROFILES;i++)
    {
        QString inifile = QString("%1.%2").arg(inipath).arg(i);
        if(QFile::exists(inifile))
        {
            QSettings settings(inifile, QSettings::IniFormat, this);
            QString name = settings.value(SETTINGS_GENERAL_PROFILENAME).toString();
            profilenames.push_back(name);
            profiles[name] = inifile;
        }
        else if(freeno < 0)
            freeno = i;
    }
    

    const QString newprofile = tr("New Profile"), delprofile = tr("Delete Profile");
    if(profiles.size() < MAX_PROFILES)
        profilenames.push_back(newprofile);
    if(profiles.size() > 0)
        profilenames.push_back(delprofile);

    bool ok = false;
    QString choice = QInputDialog::getItem(this, tr("New Client Instance"), 
        tr("Select profile"), profilenames, 0, false, &ok);

    if(choice == delprofile)
    {
        profilenames.removeAll(newprofile);
        profilenames.removeAll(delprofile);

        QString choice = QInputDialog::getItem(this, tr("New Client Instance"),
            tr("Delete profile"), profilenames, 0, false, &ok);
        if(ok && ttSettings->fileName() != profiles[choice])
            QFile::remove(profiles[choice]);
        return;
    }
    else if(choice == newprofile)
    {
        QString newname = QInputDialog::getText(this,
            tr("New Profile"), tr("Profile name"), QLineEdit::Normal,
            QString("Profile %1").arg(freeno), &ok);
        if(ok && newname.size())
        {
            inipath = QString("%1.%2").arg(inipath).arg(freeno);
            QFile::copy(ttSettings->fileName(), inipath);
            QSettings settings(inipath, QSettings::IniFormat, this);
            settings.setValue(SETTINGS_GENERAL_PROFILENAME, newname);
        }
        else return;
    }
    else 
    {
        inipath = profiles[choice];
    }

    QString path = QApplication::applicationFilePath();
    QStringList args = { "-noconnect" };
    args.push_back(QString("-cfg"));
    args.push_back(inipath);

#if defined(_DEBUG)
    QProcess::startDetached(path, args);
#else
    QProcess::startDetached(path, args, QApplication::applicationDirPath());
#endif
}

void MainWindow::slotClientConnect(bool /*checked =false */)
{
    killLocalTimer(TIMER_RECONNECT);

    //reset last channel, since we're starting a new connection
    ZERO_STRUCT(m_last_channel);

    if(TT_GetFlags(ttInst) & CLIENT_CONNECTION)
        Disconnect();
    else
    {
        ServerListDlg dlg(this);
        if(dlg.exec())
        {
            m_host = HostEntry();
            getLatestHost(0, m_host);
            Connect();
        }
    }

    //update checked state
    slotUpdateUI();
}

void MainWindow::slotClientPreferences(bool /*checked =false */)
{
    PreferencesDlg dlg(this);

    //we need to be able to process local frames (userid 0),
    //so ensure these video frames are not being displayed elsewhere
    int localvideo_userid = (0 | VIDEOTYPE_CAPTURE);
    bool ignore_set = m_vid_exclude.find(localvideo_userid) != m_vid_exclude.end();
    disconnect(this, SIGNAL(newVideoCaptureFrame(int,int)),
               ui.videogridWidget, SLOT(slotNewVideoFrame(int,int)));
    m_vid_exclude.remove(localvideo_userid);

    uservideo_t::iterator local_ite = m_user_video.find(localvideo_userid);
    if(local_ite != m_user_video.end())
        disconnect(this, SIGNAL(newVideoCaptureFrame(int,int)),
                   (*local_ite)->uservideoWidget, 
                   SLOT(slotNewVideoFrame(int,int)));

    connect(this, SIGNAL(newVideoCaptureFrame(int,int)), &dlg, 
            SLOT(slotNewVideoFrame(int,int)));

    //see if we need to retranslate
    QString lang = ttSettings->value(SETTINGS_DISPLAY_LANGUAGE).toString();

    int mediavsvoice = ttSettings->value(SETTINGS_SOUND_MEDIASTREAM_VOLUME,
                                         SETTINGS_SOUND_MEDIASTREAM_VOLUME_DEFAULT).toInt();

    //show dialog
    bool b = dlg.exec();

    disconnect(this, SIGNAL(newVideoCaptureFrame(int,int)), &dlg, 
               SLOT(slotNewVideoFrame(int,int)));

    if(ignore_set)
        m_vid_exclude.insert(localvideo_userid);
    if(local_ite != m_user_video.end())
        connect(this, SIGNAL(newVideoCaptureFrame(int,int)),
                (*local_ite)->uservideoWidget, 
                SLOT(slotNewVideoFrame(int,int)));

    connect(this, SIGNAL(newVideoCaptureFrame(int,int)),
            ui.videogridWidget, SLOT(slotNewVideoFrame(int,int)));

    if(!b)return;

    User myself;
    if((TT_GetFlags(ttInst) & CLIENT_AUTHORIZED) &&
        TT_GetUser(ttInst, TT_GetMyUserID(ttInst), &myself))
    {
        QString nickname = ttSettings->value(SETTINGS_GENERAL_NICKNAME, tr(SETTINGS_GENERAL_NICKNAME_DEFAULT)).toString();
        if(_Q(myself.szNickname) != nickname)
            TT_DoChangeNickname(ttInst, _W(nickname));

        QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();
        //change to female if set
        if(!ttSettings->value(SETTINGS_GENERAL_GENDER,
                              SETTINGS_GENERAL_GENDER_DEFAULT).toBool())
            m_statusmode |= STATUSMODE_FEMALE;
        else
            m_statusmode &= ~STATUSMODE_FEMALE;

        //set status mode flags
        if(m_statusmode != myself.nStatusMode || statusmsg != _Q(myself.szStatusMsg))
            TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));
    }
    slotMeEnableVoiceActivation(ttSettings->value(SETTINGS_GENERAL_VOICEACTIVATED,
                                                  SETTINGS_GENERAL_VOICEACTIVATED_DEFAULT).toBool());
    slotMeEnablePushToTalk(ttSettings->value(SETTINGS_GENERAL_PUSHTOTALK).toBool());

    updateAudioConfig();

    //always on top property
    Qt::WindowFlags wndflags = windowFlags();
    if(ttSettings->value(SETTINGS_DISPLAY_ALWAYSONTOP, false).toBool())
        wndflags |= Qt::WindowStaysOnTopHint;
    else
        wndflags &= ~Qt::WindowStaysOnTopHint;

    if(wndflags != windowFlags())
    {
        setWindowFlags(wndflags);
#ifdef Q_OS_WIN32
        //Qt creates a new HWND to support "always on top" so no more messages
        //are posted to our HWND. This is actually unsafe since the previous HWND
        //might have unhandled messages.
        TT_SwapTeamTalkHWND(ttInst, reinterpret_cast<HWND>(winId()));
#endif
        show(); //for some reason this has to be called, otherwise the window disappears
    }

    // check vu-meter setting
    if(ttSettings->value(SETTINGS_DISPLAY_VU_METER_UPDATES,
                         SETTINGS_DISPLAY_VU_METER_UPDATES_DEFAULT).toBool())
    {
        if(!timerExists(TIMER_VUMETER_UPDATE))
        {
            m_timers.insert(startTimer(50), TIMER_VUMETER_UPDATE);
            ui.voiceactBar->setVisible(true);
        }
    }
    else if(timerExists(TIMER_VUMETER_UPDATE))
    {
        killLocalTimer(TIMER_VUMETER_UPDATE);
        ui.voiceactBar->setVisible(false);
    }

    //show user count property
    ui.channelsWidget->setShowUserCount(ttSettings->value(SETTINGS_DISPLAY_USERSCOUNT,
                                                          SETTINGS_DISPLAY_USERSCOUNT_DEFAULT).toBool());
    ui.channelsWidget->setShowUsername();
    ui.channelsWidget->setShowLastToTalk(ttSettings->value(SETTINGS_DISPLAY_LASTTALK,
                                                           SETTINGS_DISPLAY_LASTTALK_DEFAULT).toBool());
    ui.channelsWidget->updateItemTextLength(ttSettings->value(SETTINGS_DISPLAY_MAX_STRING,
                                            SETTINGS_DISPLAY_MAX_STRING_DEFAULT).toInt());

    if(lang != ttSettings->value(SETTINGS_DISPLAY_LANGUAGE).toString())
        ui.retranslateUi(this);

    double d = ttSettings->value(SETTINGS_SOUND_MEDIASTREAM_VOLUME,
                                 SETTINGS_SOUND_MEDIASTREAM_VOLUME_DEFAULT).toDouble();
    if(d != mediavsvoice)
    {
        d /= 100.;
        QVector<int> userids = ui.channelsWidget->getUsers();
        for(int i=0;i<userids.size();i++)
        {
            TT_SetUserVolume(ttInst, userids[i], STREAMTYPE_MEDIAFILE_AUDIO,
                             (int)(refVolume(SETTINGS_SOUND_MASTERVOLUME_DEFAULT) * d));
            TT_PumpMessage(ttInst, CLIENTEVENT_USER_STATECHANGE, userids[i]);
        }
    }


#if defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
    //hotkeys are not registered in PreferencesDlg as on Windows
    loadHotKeys();
#endif

    emit(preferencesModified());

    m_desktopaccess_entries.clear();

    getDesktopAccessList(m_desktopaccess_entries, m_host.ipaddr, m_host.tcpport);

    slotUpdateUI();
}

void MainWindow::slotClientExit(bool /*checked =false */)
{
    //close using timer, otherwise gets a Qt assertion from the 
    //'setQuitOnLastWindowClosed' call.
    QApplication::setQuitOnLastWindowClosed(true);
    QTimer::singleShot(0, this, SLOT(close()));
}

void MainWindow::slotMeChangeNickname(bool /*checked =false */)
{
    bool ok = false;
    QString s = QInputDialog::getText(this, 
                                      MENUTEXT(ui.actionChangeNickname->text()), 
                                      tr("Specify new nickname"), QLineEdit::Normal, 
                                      ttSettings->value(SETTINGS_GENERAL_NICKNAME,
                                                        tr(SETTINGS_GENERAL_NICKNAME_DEFAULT)).toString(), &ok);
    if(ok)
    {
        ttSettings->setValue(SETTINGS_GENERAL_NICKNAME, s);
        TT_DoChangeNickname(ttInst, _W(s));
    }
}

void MainWindow::slotMeChangeStatus(bool /*checked =false */)
{
    ChangeStatusDlg dlg(this);
    if(dlg.exec())
        m_statusmode = dlg.m_user.nStatusMode;
}

void MainWindow::slotMeEnablePushToTalk(bool checked)
{
    if(checked)
    {
        hotkey_t hotkey;
        if(!loadHotKeySettings(HOTKEY_PUSHTOTALK, hotkey))
        {
            KeyCompDlg dlg(this);
            if(!dlg.exec())
                return;
            saveHotKeySettings(HOTKEY_PUSHTOTALK, dlg.m_hotkey);
            hotkey = dlg.m_hotkey;
        }

        if(hotkey.size())
            enableHotKey(HOTKEY_PUSHTOTALK, hotkey);
    }
    else
    {
        disableHotKey(HOTKEY_PUSHTOTALK);
    }

    ttSettings->setValue(SETTINGS_GENERAL_PUSHTOTALK, checked);

    slotUpdateUI();
}

void MainWindow::slotMeEnableVoiceActivation(bool checked)
{
    TT_EnableVoiceActivation(ttInst, checked);
    ui.voiceactSlider->setVisible(checked);
    ttSettings->setValue(SETTINGS_GENERAL_VOICEACTIVATED, checked);
    if(TT_GetFlags(ttInst) & CLIENT_CONNECTED)
        emit(updateMyself());
    slotUpdateUI();
}

void MainWindow::slotMeEnableVideoTransmission(bool /*checked*/)
{
    ClientFlags flags = TT_GetFlags(ttInst);
    if((flags & CLIENT_VIDEOCAPTURE_READY) == 0)
    {
        VideoCodec vidcodec;
        if(!getVideoCaptureCodec(vidcodec) || !initVideoCaptureFromSettings())
        {
            ui.actionEnableVideoTransmission->setChecked(false);
            ttSettings->setValue(SETTINGS_VIDCAP_ENABLE, false);
            QMessageBox::warning(this,
            MENUTEXT(ui.actionEnableVideoTransmission->text()), 
            tr("Video device hasn't been configured property. Check settings in 'Preferences'"));
        }
        else 
        {
            if(!TT_StartVideoCaptureTransmission(ttInst, &vidcodec))
            {
                ui.actionEnableVideoTransmission->setChecked(false);
                TT_CloseVideoCaptureDevice(ttInst);
                ttSettings->setValue(SETTINGS_VIDCAP_ENABLE, false);
                QMessageBox::warning(this,
                                 MENUTEXT(ui.actionEnableVideoTransmission->text()), 
                             tr("Failed to configure video codec. Check settings in 'Preferences'"));
                return;
            }

            m_statusmode |= STATUSMODE_VIDEOTX;
            if(flags & CLIENT_AUTHORIZED)
            {
                TT_DoChangeStatus(ttInst, m_statusmode, 
                _W(ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString()));
            }
            ttSettings->setValue(SETTINGS_VIDCAP_ENABLE, true);
        }
    }
    else
    {
        TT_StopVideoCaptureTransmission(ttInst);
        TT_CloseVideoCaptureDevice(ttInst);
        m_statusmode &= ~STATUSMODE_VIDEOTX;
        if(flags & CLIENT_AUTHORIZED)
        {
            TT_DoChangeStatus(ttInst, m_statusmode, 
            _W(ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString()));
        }

        //remove local from video grid
        if(ui.videogridWidget->userExists(0))
            ui.videogridWidget->removeUser(0 /* local video*/);

        ttSettings->setValue(SETTINGS_VIDCAP_ENABLE, false);
    }

    slotUpdateUI();
    slotUpdateVideoTabUI();
}

void MainWindow::slotMeEnableDesktopSharing(bool checked/*=false*/)
{
    if(checked)
    {
#if defined(Q_OS_LINUX)
        if(!m_display)
            m_display = XOpenDisplay(0);

        if(!m_display)
            QMessageBox::critical(this, MENUTEXT(ui.actionEnableDesktopSharing->text()),
                                  tr("Failed to open X11 display."));

        DesktopShareDlg dlg(m_display, this);
#else
        DesktopShareDlg dlg(this);
#endif
        if(dlg.exec())
        {
#if defined(Q_OS_WIN32)
            m_hShareWnd = dlg.m_hShareWnd;
#elif defined(Q_OS_DARWIN)
            m_nCGShareWnd = dlg.m_nCGShareWnd;
#elif defined(Q_OS_LINUX)
            m_nWindowShareWnd = dlg.m_nWindowShareWnd;
#endif
            if(!sendDesktopWindow())
            {
                ui.actionEnableDesktopSharing->setChecked(false);
                QMessageBox::information(this, MENUTEXT(ui.actionEnableDesktopSharing->text()),
                    tr("Failed to start desktop sharing"));
                return;
            }

            restartSendDesktopWindowTimer();

            m_statusmode |= STATUSMODE_DESKTOP;
            QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();
            if(TT_GetFlags(ttInst) & CLIENT_AUTHORIZED)
                TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));
        }
        else
            ui.actionEnableDesktopSharing->setChecked(false);
    }
    else
    {
        killLocalTimer(TIMER_SEND_DESKTOPWINDOW);
        killLocalTimer(TIMER_SEND_DESKTOPCURSOR);
        TT_CloseDesktopWindow(ttInst);

#if defined(Q_OS_LINUX)
        if(m_display)
            XCloseDisplay(m_display);
        m_display = nullptr;
#endif
            m_statusmode &= ~STATUSMODE_DESKTOP;
            QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();
            if(TT_GetFlags(ttInst) & CLIENT_AUTHORIZED)
                TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));
    }
}

void MainWindow::slotUsersViewUserInformation(bool /*checked =false */)
{
    slotUsersViewUserInformation(ui.channelsWidget->selectedUser());
}

void MainWindow::slotUsersMessages(bool /*checked =false */)
{
    int userid = ui.channelsWidget->selectedUser();
    slotUsersMessages(userid);
}

void MainWindow::slotUsersMuteVoice(bool checked /*=false */)
{
    foreach(int userid, ui.channelsWidget->selectedUsers())
        slotUsersMuteVoice(userid, checked);
}

void MainWindow::slotUsersMuteMediaFile(bool checked /*=false */)
{
    foreach(int userid, ui.channelsWidget->selectedUsers())
        slotUsersMuteMediaFile(userid, checked);
}

void MainWindow::slotUsersVolume(bool /*checked =false */)
{
    int userid = ui.channelsWidget->selectedUser();
    slotUsersVolume(userid);
}

void MainWindow::slotUsersMuteVoiceAll(bool checked /*=false */)
{
    TT_SetSoundOutputMute(ttInst, checked);
}

void MainWindow::slotUsersOp(bool /*checked =false */)
{
    foreach(User u, ui.channelsWidget->getSelectedUsers())
        slotUsersOp(u.nUserID, u.nChannelID);
}

void MainWindow::slotUsersKickFromChannel(bool /*checked =false */)
{
    foreach(User u, ui.channelsWidget->getSelectedUsers())
        slotUsersKick(u.nUserID, u.nChannelID);
}

void MainWindow::slotUsersKickBanFromChannel(bool /*checked =false */)
{
    foreach(User u, ui.channelsWidget->getSelectedUsers())
        slotUsersKickBan(u.nUserID, u.nChannelID);
}

void MainWindow::slotUsersKickFromServer(bool /*checked =false */)
{
    foreach(User u, ui.channelsWidget->getSelectedUsers())
        slotUsersKick(u.nUserID, 0);
}

void MainWindow::slotUsersKickBanFromServer(bool /*checked =false */)
{
    foreach(User u, ui.channelsWidget->getSelectedUsers())    
        slotUsersKickBan(u.nUserID, 0);
}

void MainWindow::slotUsersSubscriptionsUserMsg(bool checked /*=false */)
{
    subscribeCommon(checked, SUBSCRIBE_USER_MSG);
}

void MainWindow::slotUsersSubscriptionsChannelMsg(bool checked /*=false */)
{
    subscribeCommon(checked, SUBSCRIBE_CHANNEL_MSG);
}

void MainWindow::slotUsersSubscriptionsBCastMsg(bool checked /*=false */)
{
    subscribeCommon(checked, SUBSCRIBE_BROADCAST_MSG);
}

void MainWindow::slotUsersSubscriptionsVoice(bool checked /*=false */)
{
    subscribeCommon(checked, SUBSCRIBE_VOICE);
}

void MainWindow::slotUsersSubscriptionsVideo(bool checked /*=false */)
{
    subscribeCommon(checked, SUBSCRIBE_VIDEOCAPTURE);
    if(checked)
    {
        //remove from ignore if user wants video again
        int userid = ui.channelsWidget->selectedUser();
        if(userid>0)
            m_vid_exclude.remove(userid);
    }
}

void MainWindow::slotUsersSubscriptionsDesktop(bool checked /*=false */)
{
    subscribeCommon(checked, SUBSCRIBE_DESKTOP);
}

void MainWindow::slotUsersSubscriptionsDesktopInput(bool checked /*=false */)
{
    subscribeCommon(checked, SUBSCRIBE_DESKTOPINPUT);
    foreach(User user, ui.channelsWidget->getSelectedUsers())
        addStatusMsg(QString(tr("%1 granted desktop access")
                        .arg(getDisplayName(user))));
}

void MainWindow::slotUsersSubscriptionsMediaFile(bool checked /*=false*/)
{
    subscribeCommon(checked, SUBSCRIBE_MEDIAFILE);
}

void MainWindow::slotUsersSubscriptionsInterceptUserMsg(bool checked/*=false*/)
{
    subscribeCommon(checked, SUBSCRIBE_INTERCEPT_USER_MSG);
}

void MainWindow::slotUsersSubscriptionsInterceptChannelMsg(bool checked/*=false*/)
{
    subscribeCommon(checked, SUBSCRIBE_INTERCEPT_CHANNEL_MSG);
}

void MainWindow::slotUsersSubscriptionsInterceptVoice(bool checked/*=false*/)
{
    subscribeCommon(checked, SUBSCRIBE_INTERCEPT_VOICE);
}

void MainWindow::slotUsersSubscriptionsInterceptVideo(bool checked/*=false*/)
{
    subscribeCommon(checked, SUBSCRIBE_INTERCEPT_VIDEOCAPTURE);
    if(checked)
    {
        //remove from ignore if user wants video again
        int userid = ui.channelsWidget->selectedUser();
        if(userid>0)
            m_vid_exclude.remove(userid);
    }
}

void MainWindow::slotUsersSubscriptionsInterceptDesktop(bool checked/*=false*/)
{
    subscribeCommon(checked, SUBSCRIBE_INTERCEPT_DESKTOP);
}

void MainWindow::slotUsersSubscriptionsInterceptMediaFile(bool checked /*=false*/)
{
    subscribeCommon(checked, SUBSCRIBE_INTERCEPT_MEDIAFILE);
}

void MainWindow::slotUsersAdvancedIncVolumeVoice()
{
    userids_t users = ui.channelsWidget->selectedUsers();
    std::for_each(users.begin(), users.end(),
                  std::bind2nd(std::ptr_fun(&incVolume),
                               STREAMTYPE_VOICE));

    slotUpdateUI();
}

void MainWindow::slotUsersAdvancedDecVolumeVoice()
{
    userids_t users = ui.channelsWidget->selectedUsers();
    std::for_each(users.begin(), users.end(),
                  std::bind2nd(std::ptr_fun(&decVolume),
                               STREAMTYPE_VOICE));
    
    slotUpdateUI();
}

void MainWindow::slotUsersAdvancedIncVolumeMediaFile()
{
    userids_t users = ui.channelsWidget->selectedUsers();
    std::for_each(users.begin(), users.end(), 
                  std::bind2nd(std::ptr_fun(&incVolume),
                               STREAMTYPE_MEDIAFILE_AUDIO));
    slotUpdateUI();
}

void MainWindow::slotUsersAdvancedDecVolumeMediaFile()
{
    userids_t users = ui.channelsWidget->selectedUsers();
    std::for_each(users.begin(), users.end(), 
                  std::bind2nd(std::ptr_fun(&decVolume),
                               STREAMTYPE_MEDIAFILE_AUDIO));
    slotUpdateUI();
}

void MainWindow::slotUsersAdvancedStoreForMove()
{
    m_moveusers.clear();
    m_moveusers = ui.channelsWidget->selectedUsers();
    slotUpdateUI();
}

void MainWindow::slotUsersAdvancedMoveUsers()
{
    int chanid = ui.channelsWidget->selectedChannel(true);
    if(chanid>0)
    {
        for(int i=0;i<m_moveusers.size();i++)
            TT_DoMoveUser(ttInst, m_moveusers[i], chanid);
    }
    slotUpdateUI();
}

void MainWindow::slotUsersAdvancedVoiceAllowed(bool checked/*=false*/)
{
    int userid = ui.channelsWidget->selectedUser();
    int channelid = ui.channelsWidget->selectedChannel(true);
    if(userid>0 && channelid>0)
    {
        QMap<int,StreamTypes> transmitUsers;
        ui.channelsWidget->getTransmitUsers(channelid, transmitUsers);
        if(checked)
            transmitUsers[userid] |= STREAMTYPE_VOICE;
        else
            transmitUsers[userid] &= ~STREAMTYPE_VOICE;
        slotTransmitUsersChanged(channelid, transmitUsers);
    }
}

void MainWindow::slotUsersAdvancedVideoAllowed(bool checked/*=false*/)
{
    int userid = ui.channelsWidget->selectedUser();
    int channelid = ui.channelsWidget->selectedChannel(true);
    if(userid>0 && channelid>0)
    {
        QMap<int,StreamTypes> transmitUsers;
        ui.channelsWidget->getTransmitUsers(channelid, transmitUsers);
        if(checked)
            transmitUsers[userid] |= STREAMTYPE_VIDEOCAPTURE;
        else
            transmitUsers[userid] &= ~STREAMTYPE_VIDEOCAPTURE;
        slotTransmitUsersChanged(channelid, transmitUsers);
    }
}

void MainWindow::slotUsersAdvancedDesktopAllowed(bool checked/*=false*/)
{
    int userid = ui.channelsWidget->selectedUser();
    int channelid = ui.channelsWidget->selectedChannel(true);
    if(userid>0 && channelid>0)
    {
        QMap<int,StreamTypes> transmitUsers;
        ui.channelsWidget->getTransmitUsers(channelid, transmitUsers);
        if(checked)
            transmitUsers[userid] |= STREAMTYPE_DESKTOP;
        else
            transmitUsers[userid] &= ~STREAMTYPE_DESKTOP;
        slotTransmitUsersChanged(channelid, transmitUsers);
    }
}

void MainWindow::slotUsersAdvancedMediaFileAllowed(bool checked/*=false*/)
{
    int userid = ui.channelsWidget->selectedUser();
    int channelid = ui.channelsWidget->selectedChannel(true);
    if(userid>0 && channelid>0)
    {
        QMap<int,StreamTypes> transmitUsers;
        ui.channelsWidget->getTransmitUsers(channelid, transmitUsers);
        if(checked)
            transmitUsers[userid] |= STREAMTYPE_MEDIAFILE;
        else
            transmitUsers[userid] &= ~STREAMTYPE_MEDIAFILE;
        slotTransmitUsersChanged(channelid, transmitUsers);
    }
}

void MainWindow::slotUsersStoreAudioToDisk(bool/* checked*/)
{
    quint32 old_mode = m_audiostorage_mode;

    if(MediaStorageDlg(this).exec())
    {
        m_audiostorage_mode = ttSettings->value(SETTINGS_MEDIASTORAGE_MODE, 
                                                AUDIOSTORAGE_NONE).toUInt();
        ui.actionMediaStorage->setChecked(true);
        quint32 new_mode = ttSettings->value(SETTINGS_MEDIASTORAGE_MODE, 
                                             AUDIOSTORAGE_NONE).toUInt();
        if(old_mode & AUDIOSTORAGE_SINGLEFILE)
        {
            updateAudioStorage(false, AUDIOSTORAGE_SINGLEFILE);
            m_audiostorage_mode &= ~AUDIOSTORAGE_SINGLEFILE;
        }
        if(old_mode & AUDIOSTORAGE_SEPARATEFILES)
        {
            updateAudioStorage(false, AUDIOSTORAGE_SEPARATEFILES);
            m_audiostorage_mode &= ~AUDIOSTORAGE_SEPARATEFILES;
        }

        if(new_mode & AUDIOSTORAGE_SINGLEFILE)
        {
            updateAudioStorage(true, AUDIOSTORAGE_SINGLEFILE);
            m_audiostorage_mode |= AUDIOSTORAGE_SINGLEFILE;
        }
        if(new_mode & AUDIOSTORAGE_SEPARATEFILES)
        {
            updateAudioStorage(true, AUDIOSTORAGE_SEPARATEFILES);
            m_audiostorage_mode |= AUDIOSTORAGE_SEPARATEFILES;
        }

        if(ttSettings->value(SETTINGS_MEDIASTORAGE_CHANLOGFOLDER).toString().isEmpty())
            m_logChan.close();
    }
    else
    {
        m_audiostorage_mode = AUDIOSTORAGE_NONE;
    }
    slotUpdateUI();
}

void MainWindow::slotChannelsCreateChannel(bool /*checked =false */)
{
    Channel chan;
    ZERO_STRUCT(chan);

    if(ui.channelsWidget->selectedChannel() && (TT_GetMyUserType(ttInst) & USERTYPE_ADMIN))
        chan.nParentID = ui.channelsWidget->selectedChannel();
    else
    {
        chan.nParentID = TT_GetMyChannelID(ttInst);
        if(!chan.nParentID)
            chan.nParentID = TT_GetRootChannelID(ttInst);
    }

    ChannelDlg dlg(ChannelDlg::CHANNEL_CREATE, chan, this);
    if(!dlg.exec())return;
    chan = dlg.GetChannel();

    //only admins can create channels. Users can only create a new channel
    //if they at the same time join it.
    if(TT_GetMyUserType(ttInst) & USERTYPE_ADMIN)
    {
        if(TT_DoMakeChannel(ttInst, &chan)<0)
            QMessageBox::critical(this, MENUTEXT(ui.actionCreateChannel->text()), 
            tr("Failed to issue command to create channel"));
    }
    else
    {
        int cmdid = TT_DoJoinChannel(ttInst, &chan);
        if(cmdid>0)
            m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
        else
            QMessageBox::critical(this, MENUTEXT(ui.actionCreateChannel->text()), 
                                  tr("Failed to issue command to create channel"));
    }
}

void MainWindow::slotChannelsUpdateChannel(bool /*checked =false */)
{
    Channel chan;
    if(ui.channelsWidget->getSelectedChannel(chan))
    {
        ChannelDlg dlg(ChannelDlg::CHANNEL_UPDATE, chan, this);
        if(!dlg.exec())return;
        chan = dlg.GetChannel();
        if(TT_DoUpdateChannel(ttInst, &chan)<0)
            QMessageBox::critical(this, MENUTEXT(ui.actionUpdateChannel->text()),
            tr("Failed to issue command to update channel"));
    }
}

void MainWindow::slotChannelsDeleteChannel(bool /*checked =false */)
{
    int chanid = ui.channelsWidget->selectedChannel();
    if(chanid<=0)
        return;

    TTCHAR buff[TT_STRLEN] = {};
    TT_GetChannelPath(ttInst, chanid, buff);
    if(QMessageBox::information(this, MENUTEXT(ui.actionDeleteChannel->text()),
        tr("Are you sure you want to delete channel \"%1\"?").arg(_Q(buff)), 
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        return;

    if(TT_DoRemoveChannel(ttInst, chanid)<0)
        QMessageBox::critical(this, MENUTEXT(ui.actionDeleteChannel->text()), 
        tr("Failed to issue command to delete channel"));
}

void MainWindow::slotChannelsJoinChannel(bool /*checked=false*/)
{
    Channel chan;
    if(!ui.channelsWidget->getSelectedChannel(chan))
        return;

    if(chan.nChannelID == m_mychannel.nChannelID)
    {
        int cmdid = TT_DoLeaveChannel(ttInst);
        m_commands.insert(cmdid, CMD_COMPLETE_LEAVECHANNEL);
        return;
    }

    QString password = m_channel_passwd[chan.nChannelID];
    if(chan.bPassword)
    {
        bool ok = false;
        password = QInputDialog::getText(this, MENUTEXT(ui.actionJoinChannel->text()), 
            tr("Specify password"), QLineEdit::Password, password, &ok);
        if(!ok)
            return;
    }
    m_channel_passwd[chan.nChannelID] = password;

    int cmdid = TT_DoJoinChannelByID(ttInst, chan.nChannelID, _W(password));
    if(cmdid>0)
    {
        m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
        m_last_channel = chan;
        COPY_TTSTR(m_last_channel.szPassword, password);
    }
    else
        QMessageBox::critical(this, MENUTEXT(ui.actionJoinChannel->text()),
                              tr("Failed to issue command to join channel"));
}

void MainWindow::slotChannelsViewChannelInfo(bool /*checked=false*/)
{
    Channel chan;
    if(ui.channelsWidget->getSelectedChannel(chan))
    {
        ChannelDlg dlg(ChannelDlg::CHANNEL_READONLY, chan, this);
        dlg.exec();
    }
}

void MainWindow::slotChannelsListBans(bool /*checked=false*/)
{
    //don't display dialog box until we get the result
    int chanid = ui.channelsWidget->selectedChannel(true);
    int cmdid = TT_DoListBans(ttInst, chanid, 0, 1000000);
    if(cmdid>0)
        m_commands.insert(cmdid, CMD_COMPLETE_LIST_CHANNELBANS);
}

void MainWindow::slotChannelsStreamMediaFile(bool checked/*=false*/)
{
    if(!checked)
    {
        stopStreamMediaFile();
        return;
    }

    StreamMediaFileDlg dlg(this);
    connect(this, &MainWindow::mediaStreamUpdate, &dlg, &StreamMediaFileDlg::slotMediaStreamProgress);
    connect(this, &MainWindow::mediaPlaybackUpdate, &dlg, &StreamMediaFileDlg::slotMediaPlaybackProgress);

    if(!dlg.exec())
    {
        ui.actionStreamMediaFileToChannel->setChecked(false);
        return;
    }

    startStreamMediaFile();
}

void MainWindow::slotChannelsUploadFile(bool /*checked =false */)
{
    int channelid = m_filesmodel->getChannelID();
    if(channelid>0)
    {
        QString start_dir = ttSettings->value(SETTINGS_LAST_DIRECTORY, QDir::homePath()).toString();
        QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                       start_dir/*, tr("All Files (*.*)")*/);
        if(filename.isEmpty())
            return;
        ttSettings->setValue(SETTINGS_LAST_DIRECTORY, QFileInfo(filename).absolutePath());
        QStringList files;
        files.push_back(filename);
        slotUploadFiles(files);
    }
}

void MainWindow::slotChannelsDownloadFile(bool /*checked =false */)
{
    int fileid = (int)ui.filesView->currentIndex().internalId();
    int channelid = m_filesmodel->getChannelID();
    RemoteFile remotefile;
    if(fileid>0 && channelid>0 && 
       TT_GetChannelFile(ttInst, channelid, fileid, &remotefile))
    {
        QDir lastDir(ttSettings->value(SETTINGS_LAST_DIRECTORY, QDir::homePath()).toString());
        QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
                           lastDir.filePath(_Q(remotefile.szFileName))
                           /*, tr("All Files (*.*)")*/);
        if(filename.isEmpty())
            return;
        filename = QDir::toNativeSeparators(filename);
        ttSettings->setValue(SETTINGS_LAST_DIRECTORY, QFileInfo(filename).absolutePath());
        if(!TT_DoRecvFile(ttInst, channelid, fileid, _W(filename)))
            QMessageBox::critical(this, MENUTEXT(ui.actionDownloadFile->text()),
                                  tr("Failed to download file %1").arg(filename));
    }
}

void MainWindow::slotChannelsDeleteFile(bool /*checked =false */)
{
    int channelid = m_filesmodel->getChannelID();
    if(!channelid)
        return;
    QStringList filenames;
    QList<int> files = ui.filesView->selectedFiles(&filenames);
    bool delete_ok = false;
    if(filenames.size() == 1)
        delete_ok = QMessageBox::information(this,
                                             MENUTEXT(ui.actionDeleteFile->text()),
                                             tr("Are you sure you want to delete \"%1\"?").arg(filenames[0]),
                                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
    else if(filenames.size()>1)
        delete_ok = QMessageBox::information(this,
                                             MENUTEXT(ui.actionDeleteFile->text()),
                                             tr("Are you sure you want to delete %1 file(s)?").arg(filenames.size()),
                                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;

    for(int i=0;i<files.size() && delete_ok;i++)
        TT_DoDeleteFile(ttInst, channelid, files[i]);
}
   
void MainWindow::slotServerUserAccounts(bool /*checked =false */)
{
    if(TT_GetMyUserType(ttInst) & USERTYPE_ADMIN)
    {
        //don't display dialog box until we get the result
        int cmdid = TT_DoListUserAccounts(ttInst, 0, 1000000);
        if(cmdid>0)
            m_commands.insert(cmdid, CMD_COMPLETE_LISTACCOUNTS);
    }
    else
    {
        if(!m_useraccountsdlg)
        {
            useraccounts_t useraccounts(1);
            TT_GetMyUserAccount(ttInst, &useraccounts[0]);

            m_useraccountsdlg = new UserAccountsDlg(useraccounts, UAD_READONLY);
            connect(m_useraccountsdlg, SIGNAL(finished(int)),
                SLOT(slotClosedUserAccountsDlg(int)));
            m_useraccountsdlg->setAttribute(Qt::WA_DeleteOnClose);
            m_useraccountsdlg->show();
            m_useraccounts.clear();
        }
        else
            m_useraccountsdlg->activateWindow();
    }
}

void MainWindow::slotServerBannedUsers(bool /*checked =false */)
{
    //don't display dialog box until we get the result
    int cmdid = TT_DoListBans(ttInst, 0, 0, 1000000);
    if(cmdid>0)
        m_commands.insert(cmdid, CMD_COMPLETE_LIST_SERVERBANS);
}

void MainWindow::slotServerOnlineUsers(bool /*checked=false*/)
{
    if(m_onlineusersdlg)
        m_onlineusersdlg->activateWindow();
    else
    {
        m_onlineusersdlg = new OnlineUsersDlg();
        connect(m_onlineusersdlg, SIGNAL(finished(int)),
                SLOT(slotClosedOnlineUsersDlg(int)));
        m_onlineusersdlg->setAttribute(Qt::WA_DeleteOnClose);
    }

    connect(this, SIGNAL(userLogin(const User&)), m_onlineusersdlg, 
            SLOT(slotUserLoggedIn(const User&)));
    connect(this, SIGNAL(userLogout(const User&)), m_onlineusersdlg, 
            SLOT(slotUserLoggedOut(const User&)));
    connect(this, SIGNAL(userUpdate(const User&)), m_onlineusersdlg, 
            SLOT(slotUserUpdate(const User&)));
    connect(this, SIGNAL(userJoined(int,const User&)), m_onlineusersdlg, 
            SLOT(slotUserJoin(int,const User&)));
    connect(this, SIGNAL(userLeft(int,const User&)), m_onlineusersdlg, 
            SLOT(slotUserLeft(int,const User&)));

    connect(m_onlineusersdlg, SIGNAL(viewUserInformation(int)), 
            SLOT(slotUsersViewUserInformation(int)));
    connect(m_onlineusersdlg, SIGNAL(sendUserMessage(int)), 
            SLOT(slotUsersMessages(int)));
    connect(m_onlineusersdlg, SIGNAL(muteUser(int,bool)), 
            SLOT(slotUsersMuteVoice(int, bool)));
    connect(m_onlineusersdlg, SIGNAL(changeUserVolume(int)), 
            SLOT(slotUsersVolume(int)));
    connect(m_onlineusersdlg, SIGNAL(opUser(int,int)), 
            SLOT(slotUsersOp(int,int)));
    connect(m_onlineusersdlg, SIGNAL(kickUser(int,int)), 
            SLOT(slotUsersKick(int,int)));
    connect(m_onlineusersdlg, SIGNAL(kickbanUser(int,int)), 
            SLOT(slotUsersKickBan(int,int)));
    m_onlineusersdlg->show();
}

void MainWindow::slotServerBroadcastMessage(bool /*checked=false*/)
{
    bool ok = false;
    QString bcast = QInputDialog::getText(this, 
        MENUTEXT(ui.actionBroadcastMessage->text()), 
        tr("Message to broadcast:"), QLineEdit::Normal, "", &ok);
    if(!ok)
        return;
    TextMessage msg;
    msg.nMsgType = MSGTYPE_BROADCAST;
    msg.nFromUserID = TT_GetMyUserID(ttInst);
    COPY_TTSTR(msg.szMessage, bcast);
    TT_DoTextMessage(ttInst, &msg);
}

void MainWindow::slotServerServerProperties(bool /*checked =false */)
{
    ServerPropertiesDlg dlg(this);
    if(!dlg.exec())return;
}

void MainWindow::slotServerSaveConfiguration(bool /*checked =false */)
{
    TT_DoSaveConfig(ttInst);
}

void MainWindow::slotServerServerStatistics(bool /*checked=false*/)
{
    if(m_serverstatsdlg)
        m_serverstatsdlg->activateWindow();
    else
    {
        m_serverstatsdlg = new ServerStatisticsDlg();
        connect(this, SIGNAL(cmdSuccess(int)), m_serverstatsdlg, 
                SLOT(slotCmdSuccess(int)));
        connect(m_serverstatsdlg, SIGNAL(finished(int)),
                SLOT(slotClosedServerStatsDlg(int)));
        connect(this, SIGNAL(serverStatistics(const ServerStatistics&)),
                m_serverstatsdlg, SLOT(slotUpdateStats(const ServerStatistics&)));
        m_serverstatsdlg->setAttribute(Qt::WA_DeleteOnClose);
        m_serverstatsdlg->show();
    }
}

void MainWindow::slotHelpResetPreferences(bool /*checked=false*/)
{
    if (QMessageBox::question(this, MENUTEXT(ui.actionResetPreferencesToDefault->text()),
        tr("Are you sure you want to delete your existing settings?"),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        QString cfgpath = ttSettings->fileName();
        QString defpath = QString(APPDEFAULTINIFILE);

        if(!QFile::exists(defpath))
        {
            QMessageBox::critical(this, MENUTEXT(ui.actionResetPreferencesToDefault->text()),
            tr("Cannot find %1").arg(QDir::toNativeSeparators(defpath)));
            return;
        }

        if(!QFile::remove(cfgpath))
        {
            QMessageBox::critical(this, MENUTEXT(ui.actionResetPreferencesToDefault->text()),
            tr("Cannot remove %1").arg(QDir::toNativeSeparators(cfgpath)));
            return;
        }

        if(!QFile::copy(defpath, cfgpath))
        {
            QMessageBox::critical(this, MENUTEXT(ui.actionResetPreferencesToDefault->text()),
            tr("Failed to copy %1 to %2").arg(QDir::toNativeSeparators(defpath)).arg(QDir::toNativeSeparators(cfgpath)));
        }
        else
        {
            slotClientNewInstance();
            slotClientExit();
        }
    }
}

void MainWindow::slotHelpManual(bool /*checked =false */)
{
    QString file = "file:///" + APPMANUAL;
    QDesktopServices::openUrl(file);
}
    
void MainWindow::slotHelpVisitBearWare(bool /*checked=false*/)
{
   QDesktopServices::openUrl(QUrl(APPWEBSITE));
}

void MainWindow::slotHelpAbout(bool /*checked =false */)
{
    AboutDlg(this).exec();
}

void MainWindow::slotConnectToLatest()
{
    HostEntry lasthost;

    //auto connect to latest host
    if(ttSettings->value(SETTINGS_CONNECTION_AUTOCONNECT, false).toBool() &&
        getLatestHost(0, lasthost))
    {
        m_host = lasthost;
        Connect();
    }
}

void MainWindow::slotUsersViewUserInformation(int userid)
{
    UserInfoDlg dlg(userid, this);
    dlg.exec();
}

void MainWindow::slotUsersMessages(int userid)
{
    TextMessageDlg* dlg = getTextMessageDlg(userid);
    if(dlg)
    {
        dlg->show();
        dlg->activateWindow();
        dlg->raise();
    }
}

void MainWindow::slotUsersMuteVoice(int userid, bool mute)
{
    TT_SetUserMute(ttInst, userid, STREAMTYPE_VOICE, mute);
    TT_PumpMessage(ttInst, CLIENTEVENT_USER_STATECHANGE, userid);
}

void MainWindow::slotUsersMuteMediaFile(int userid, bool mute)
{
    TT_SetUserMute(ttInst, userid, STREAMTYPE_MEDIAFILE_AUDIO, mute);
    TT_PumpMessage(ttInst, CLIENTEVENT_USER_STATECHANGE, userid);
}

void MainWindow::slotUsersVolume(int userid)
{
    UserVolumeDlg dlg(userid, this);
    dlg.exec();
    slotUpdateUI();
}

void MainWindow::slotUsersOp(int userid, int chanid)
{
    bool op = (bool)TT_IsChannelOperator(ttInst, userid, chanid);

    bool me_opadmin = (bool)TT_IsChannelOperator(ttInst, TT_GetMyUserID(ttInst), chanid);
    me_opadmin |= (TT_GetMyUserType(ttInst) & USERTYPE_ADMIN) != 0;
    if(me_opadmin) //don't need password in this case
        TT_DoChannelOp(ttInst, userid, chanid, !op);
    else
    {
        bool ok = false;
        QString oppasswd = QInputDialog::getText(this, MENUTEXT(ui.actionOp->text()), 
            tr("Specify password"), QLineEdit::Password, "", &ok);
        if(ok)
            TT_DoChannelOpEx(ttInst, userid, chanid, _W(oppasswd), !op);
    }
}

void MainWindow::slotUsersKick(int userid, int chanid)
{
    TT_DoKickUser(ttInst, userid, chanid);
}

void MainWindow::slotUsersKickBan(int userid, int chanid)
{
    QStringList items = { tr("IP-address"), tr("Username") };
    bool ok = false;
    QString choice = QInputDialog::getItem(this, tr("Ban User From Channel"), tr("Ban user's"), items, 0, false, &ok);
    if (ok)
    {
        //ban first since the user will otherwise have disappeared
        if (choice == items[0])
            TT_DoBanUserEx(ttInst, userid, chanid != 0 ? BANTYPE_CHANNEL | BANTYPE_IPADDR : BANTYPE_IPADDR);
        else
            TT_DoBanUserEx(ttInst, userid, chanid != 0 ? BANTYPE_CHANNEL | BANTYPE_USERNAME : BANTYPE_USERNAME);
        TT_DoKickUser(ttInst, userid, chanid);
    }
}

void MainWindow::slotTreeSelectionChanged()
{
    slotUpdateUI();
    int channelid = ui.channelsWidget->selectedChannel(true);
    if(m_filesmodel->getChannelID() != channelid &&
       (TT_GetMyUserType(ttInst) & USERTYPE_ADMIN))
    {
        //if admin changed selection change files view to new channel.
        //if not admin then keep joined channel as file view.
        updateChannelFiles(channelid);
    }
}

void MainWindow::slotTreeContextMenu(const QPoint &/* pos*/)
{
    int userid = ui.channelsWidget->selectedUser();
    if(userid>0)
        ui.menuUsers->exec(QCursor::pos());

    int chanid = ui.channelsWidget->selectedChannel();
    if(chanid>0)
        ui.menuChannels->exec(QCursor::pos());
}

void MainWindow::slotUpdateUI()
{
    int userid = ui.channelsWidget->selectedUser();
    int chanid = ui.channelsWidget->selectedChannel();
    int user_chanid = chanid;
    if(!chanid)
        user_chanid = ui.channelsWidget->selectedChannel(true);
    int mychannel = TT_GetMyChannelID(ttInst);
    int filescount = ui.filesView->selectedFiles().size();
    ClientFlags statemask = TT_GetFlags(ttInst);
    UserRights userrights = TT_GetMyUserRights(ttInst);
    bool auth = (statemask & CLIENT_AUTHORIZED);
    bool me_admin = (TT_GetMyUserType(ttInst) & USERTYPE_ADMIN);
    bool me_op = TT_IsChannelOperator(ttInst, TT_GetMyUserID(ttInst), user_chanid);

    ui.actionConnect->setChecked( (statemask & CLIENT_CONNECTING) || (statemask & CLIENT_CONNECTED));
    ui.actionChangeNickname->setEnabled(auth);
    ui.actionChangeStatus->setEnabled(auth);
#ifdef Q_OS_WIN32
    ui.actionEnablePushToTalk->setChecked(TT_HotKey_IsActive(ttInst, HOTKEY_PUSHTOTALK) >= 0);
#elif defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
    ui.actionEnablePushToTalk->setChecked(m_hotkeys.find(HOTKEY_PUSHTOTALK) != m_hotkeys.end());
#endif
    ui.actionEnableVoiceActivation->setChecked(statemask & CLIENT_SNDINPUT_VOICEACTIVATED);
    //don't allow web cam to stream when video streaming is active
    ui.actionEnableVideoTransmission->setChecked((CLIENT_VIDEOCAPTURE_READY & statemask) && 
                                                 (CLIENT_TX_VIDEOCAPTURE & statemask));
    ui.actionEnableDesktopSharing->setEnabled(mychannel>0);
    ui.actionEnableDesktopSharing->setChecked(statemask & CLIENT_DESKTOP_ACTIVE);

    User user;
    ZERO_STRUCT(user);
    if(TT_GetUser(ttInst, userid, &user))
    {
        ui.actionMuteVoice->setChecked(user.uUserState & USERSTATE_MUTE_VOICE);
        ui.actionMuteMediaFile->setChecked(user.uUserState & USERSTATE_MEDIAFILE_AUDIO);
        ui.actionDesktopAccessAllow->setChecked(user.uLocalSubscriptions & SUBSCRIBE_DESKTOPINPUT);

        ui.actionUserMessages->setChecked(user.uLocalSubscriptions & SUBSCRIBE_USER_MSG);
        ui.actionChannelMessages->setChecked(user.uLocalSubscriptions & SUBSCRIBE_CHANNEL_MSG);
        ui.actionBroadcastMessages->setChecked(user.uLocalSubscriptions & SUBSCRIBE_BROADCAST_MSG);
        ui.actionVoice->setChecked(user.uLocalSubscriptions & SUBSCRIBE_VOICE);
        ui.actionVideo->setChecked(user.uLocalSubscriptions & SUBSCRIBE_VIDEOCAPTURE);
        ui.actionDesktop->setChecked(user.uLocalSubscriptions & SUBSCRIBE_DESKTOP);
        ui.actionDesktopInput->setChecked(user.uLocalSubscriptions & SUBSCRIBE_DESKTOPINPUT);
        ui.actionMediaFile->setChecked(user.uLocalSubscriptions & SUBSCRIBE_MEDIAFILE);

        ui.actionInterceptUserMessages->setChecked(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_USER_MSG);
        ui.actionInterceptChannelMessages->setChecked(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG);
        ui.actionInterceptVoice->setChecked(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_VOICE);
        ui.actionInterceptVideo->setChecked(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE);
        ui.actionInterceptDesktop->setChecked(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_DESKTOP);
        ui.actionInterceptMediaFile->setChecked(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_MEDIAFILE);
    }

    ui.actionViewUserInformation->setEnabled(userid>0);
    ui.actionMessages->setEnabled(userid>0);
    ui.actionMuteVoice->setEnabled(userid>0);
    ui.actionMuteMediaFile->setEnabled(userid>0);
    ui.actionVolume->setEnabled(userid>0);
    ui.actionOp->setEnabled(userid>0);
    ui.actionKickFromChannel->setEnabled(userid>0);
    ui.actionKickFromServer->setEnabled(userid>0 && (userrights & USERRIGHT_KICK_USERS));
    ui.actionKickBan->setEnabled(userid>0 && (userrights & USERRIGHT_BAN_USERS));
    ui.actionDesktopAccessAllow->setEnabled(userid>0);

    ui.actionUserMessages->setEnabled(userid>0);
    ui.actionChannelMessages->setEnabled(userid>0);
    ui.actionBroadcastMessages->setEnabled(userid>0);
    ui.actionVoice->setEnabled(userid>0);
    ui.actionVideo->setEnabled(userid>0);
    ui.actionDesktop->setEnabled(userid>0);
    ui.actionDesktopInput->setEnabled(userid>0);
    ui.actionMediaFile->setEnabled(userid>0);
    //intercept only works for admins
    ui.actionInterceptUserMessages->setEnabled(userid>0);
    ui.actionInterceptChannelMessages->setEnabled(userid>0);
    ui.actionInterceptVoice->setEnabled(userid>0);
    ui.actionInterceptVideo->setEnabled(userid>0);
    ui.actionInterceptDesktop->setEnabled(userid>0);
    ui.actionInterceptMediaFile->setEnabled(userid>0);

    ui.actionIncreaseVoiceVolume->setEnabled(userid>0 && user.nVolumeVoice < SOUND_VOLUME_MAX);
    ui.actionLowerVoiceVolume->setEnabled(userid>0 && user.nVolumeVoice > SOUND_VOLUME_MIN);
    ui.actionIncreaseMediaFileVolume->setEnabled(userid>0 && user.nVolumeMediaFile < SOUND_VOLUME_MAX);
    ui.actionLowerMediaFileVolume->setEnabled(userid>0 && user.nVolumeMediaFile > SOUND_VOLUME_MIN);
    ui.actionStoreForMove->setEnabled(userid>0 && (userrights & USERRIGHT_MOVE_USERS));
    ui.actionMoveUser->setEnabled(m_moveusers.size() && (userrights & USERRIGHT_MOVE_USERS));

    //ui.actionMuteAll->setEnabled(statemask & CLIENT_SOUND_READY);
    ui.actionMuteAll->setChecked(statemask & CLIENT_SNDOUTPUT_MUTE);
    ui.actionMediaStorage->setChecked(m_audiostorage_mode != AUDIOSTORAGE_NONE);

    //Channel-menu items
    Channel chan;
    ZERO_STRUCT(chan);
    if(TT_GetChannel(ttInst, user_chanid, &chan))
    {
    }

    if(user_chanid == mychannel)
        ui.actionJoinChannel->setText(tr("&Leave Channel"));
    else
        ui.actionJoinChannel->setText(tr("&Join Channel"));

    ui.actionJoinChannel->setEnabled(chanid>0);
    ui.actionViewChannelInfo->setEnabled(chanid>0);
    ui.actionBannedUsersInChannel->setEnabled(chanid>0);
    ui.actionCreateChannel->setEnabled(chanid>0 || mychannel>0);
    ui.actionUpdateChannel->setEnabled(chanid>0);
    ui.actionDeleteChannel->setEnabled(chanid>0);
    ui.actionStreamMediaFileToChannel->setChecked(statemask & 
                                                  (CLIENT_STREAM_AUDIO | CLIENT_STREAM_VIDEO));
    ui.actionUploadFile->setEnabled(mychannel>0);
    ui.actionDownloadFile->setEnabled(mychannel>0);
    ui.actionDeleteFile->setEnabled(filescount>0);

    //Users-menu items dependent on Channel
    ui.actionAllowVoiceTransmission->setChecked(userCanVoiceTx(userid, chan));
    ui.actionAllowVoiceTransmission->setEnabled(userid>0 && (me_op || (userrights & USERRIGHT_MODIFY_CHANNELS)));
    ui.actionAllowVideoTransmission->setChecked(userCanVideoTx(userid, chan));
    ui.actionAllowVideoTransmission->setEnabled(userid>0 && (me_op || (userrights & USERRIGHT_MODIFY_CHANNELS)));
    ui.actionAllowDesktopTransmission->setChecked(userCanDesktopTx(userid, chan));
    ui.actionAllowDesktopTransmission->setEnabled(userid>0 && (me_op || (userrights & USERRIGHT_MODIFY_CHANNELS)));
    ui.actionAllowMediaFileTransmission->setChecked(userCanMediaFileTx(userid, chan));
    ui.actionAllowMediaFileTransmission->setEnabled(userid>0 && (me_op || (userrights & USERRIGHT_MODIFY_CHANNELS)));

    //Server-menu items
    ui.actionUserAccounts->setEnabled(auth);
    ui.actionBannedUsers->setEnabled(me_op || (userrights & USERRIGHT_BAN_USERS));
    ui.actionOnlineUsers->setEnabled(auth);
    ui.actionBroadcastMessage->setEnabled(auth && (userrights & USERRIGHT_TEXTMESSAGE_BROADCAST));
    ui.actionServerProperties->setEnabled(auth);
    ui.actionSaveConfiguration->setEnabled(auth && me_admin);
    ui.actionServerStatistics->setEnabled(auth && me_admin);

    ui.uploadButton->setEnabled(mychannel>0);
    ui.downloadButton->setEnabled(mychannel>0);
}

void MainWindow::slotUpdateVideoTabUI()
{
    ClientFlags statemask = TT_GetFlags(ttInst);
    int userid = ui.videogridWidget->selectedUser();

    ui.detachVideoButton->setEnabled(userid>0);
    ui.removeVideoButton->setEnabled(ui.videogridWidget->getActiveUsersCount());
    ui.initVideoButton->setChecked(statemask & CLIENT_VIDEOCAPTURE_READY);
}

void MainWindow::slotUpdateDesktopTabUI()
{
    int userid = ui.desktopgridWidget->selectedUser();
    User user;
    ZERO_STRUCT(user);
    ui.channelsWidget->getUser(userid, user);
    
    ui.detachDesktopButton->setEnabled(userid>0);
    ui.removeDesktopButton->setEnabled(ui.desktopgridWidget->getActiveUsersCount());
    ui.desktopaccessButton->setEnabled(userid>0);
    ui.desktopaccessButton->setChecked(user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT);
}

void MainWindow::slotUploadFiles(const QStringList& files)
{
    int channelid = m_filesmodel->getChannelID();
    Q_ASSERT(channelid>0);
    for(int i=0;i<files.size();i++)
    {
        QString filename = QDir::toNativeSeparators(files[i]);
        if(!TT_DoSendFile(ttInst, channelid, _W(filename)))
            QMessageBox::critical(this, MENUTEXT(ui.actionUploadFile->text()),
            tr("Failed to upload file %1").arg(filename));
    }
}

void MainWindow::slotSendChannelMessage()
{
    int mychanid = TT_GetMyChannelID(ttInst);
    if(mychanid<=0)
        return;

    QString txtmsg;
    switch(ui.tabWidget->currentIndex())
    {
    case TAB_CHAT :
        txtmsg = ui.msgEdit->text();
        ui.msgEdit->clear();
        break;
    case TAB_VIDEO :
        txtmsg = ui.videomsgEdit->text();
        ui.videomsgEdit->clear();
        break;
    case TAB_DESKTOP :
        txtmsg = ui.desktopmsgEdit->text();
        ui.desktopmsgEdit->clear();
        break;
    default :
        break;
    }

    if(txtmsg.isEmpty())
        return;

    TextMessage msg;
    msg.nFromUserID = TT_GetMyUserID(ttInst);
    msg.nChannelID = mychanid;
    msg.nMsgType = MSGTYPE_CHANNEL;
    COPY_TTSTR(msg.szMessage, txtmsg);
    TT_DoTextMessage(ttInst, &msg);
}

void MainWindow::slotUserDoubleClicked(int)
{
    if(QApplication::keyboardModifiers() & Qt::ControlModifier)
    {
        User user;
        if(!ui.channelsWidget->getSelectedUser(user))
            return;

        //first look through video capture
        int userid = user.nUserID | VIDEOTYPE_CAPTURE;

        //ensure we don't open two dialogs
        if(m_user_video.find(userid) != m_user_video.end())
            m_user_video.find(userid).value()->raise();
        else
        {
            //resubscribe in case disabled
            Subscriptions subs = SUBSCRIBE_NONE;
            if((user.uLocalSubscriptions & SUBSCRIBE_VIDEOCAPTURE) == 0)
                subs |= SUBSCRIBE_VIDEOCAPTURE;
            if(user.nChannelID != m_mychannel.nChannelID)
                subs |= SUBSCRIBE_INTERCEPT_VIDEOCAPTURE;

            subscribeCommon(true, subs, user.nUserID);

            m_vid_exclude.remove(userid);

            slotDetachUserVideo(userid);
        }

        //now look through media files
        userid = user.nUserID | VIDEOTYPE_MEDIAFILE;
        //ensure we don't open two dialogs
        if(m_user_video.find(userid) != m_user_video.end())
            m_user_video.find(userid).value()->raise();
        else
        {
            //resubscribe in case disabled
            Subscriptions subs = SUBSCRIBE_NONE;
            if((user.uLocalSubscriptions & SUBSCRIBE_MEDIAFILE) == 0)
                subs |= SUBSCRIBE_MEDIAFILE;
            if(user.nChannelID != m_mychannel.nChannelID)
                subs |= SUBSCRIBE_INTERCEPT_MEDIAFILE;

            subscribeCommon(true, subs, user.nUserID);

            m_vid_exclude.remove(userid);

            slotDetachUserVideo(userid);
        }
    }
    else
        slotUsersMessages(false);
}

void MainWindow::slotChannelDoubleClicked(int)
{
    slotChannelsJoinChannel(false);
}

void MainWindow::slotNewTextMessage(const TextMessage& textmsg)
{
    if(textmsg.nMsgType != MSGTYPE_USER)
        return;

    usermessages_t::iterator ii = m_usermessages.find(textmsg.nFromUserID);
    if(ii != m_usermessages.end())
        ii.value().push_back(textmsg);
    else
    {
        textmessages_t msgs;
        msgs.push_back(textmsg);
        m_usermessages.insert(textmsg.nFromUserID, msgs);
    }
}

void MainWindow::slotNewMyselfTextMessage(const TextMessage& textmsg)
{
    if(textmsg.nMsgType != MSGTYPE_USER)
        return;

    usermessages_t::iterator ii = m_usermessages.find(textmsg.nToUserID);
    if(ii != m_usermessages.end())
        ii.value().push_back(textmsg);
    else
    {
        textmessages_t msgs;
        msgs.push_back(textmsg);
        m_usermessages.insert(textmsg.nToUserID, msgs);
    }
}

void MainWindow::slotTextMessageClosed(int userid)
{
    usermsg_t::iterator ite = m_usermsg.find(userid);
    Q_ASSERT(ite != m_usermsg.end());
    if(ite != m_usermsg.end())
    {
        //(*ite)->deleteLater();
        m_usermsg.erase(ite);
        ui.channelsWidget->setUserMessaged(userid, false);
    }
}

void MainWindow::slotTransmitUsersChanged(int channelid, 
                                      const QMap<int,StreamTypes>& transmitUsers)
{
    Channel chan;
    if(!TT_GetChannel(ttInst, channelid, &chan))
        return;

    if(transmitUsers.size()>TT_TRANSMITUSERS_MAX)
    {
        QMessageBox::information(this, 
            MENUTEXT(ui.actionAllowVoiceTransmission->text()),
            tr("The maximum number of users who can transmit is %1")
            .arg(TT_TRANSMITUSERS_MAX));
        return;
    }

    int j = 0;
    QMap<int,StreamTypes>::const_iterator i = transmitUsers.begin();
    while(i != transmitUsers.end())
    {
        chan.transmitUsers[j][TT_TRANSMITUSERS_USERID_INDEX] = i.key();
        if (chan.uChannelType & CHANNEL_CLASSROOM)
            chan.transmitUsers[j][TT_TRANSMITUSERS_STREAMTYPE_INDEX] = i.value();
        else
            chan.transmitUsers[j][TT_TRANSMITUSERS_STREAMTYPE_INDEX] = ~i.value();
        i++;j++;
    }
    if(j<TT_TRANSMITUSERS_MAX)
    {
        chan.transmitUsers[j][TT_TRANSMITUSERS_USERID_INDEX] = 0;
        chan.transmitUsers[j][TT_TRANSMITUSERS_STREAMTYPE_INDEX] = STREAMTYPE_NONE;
    }
    TT_DoUpdateChannel(ttInst, &chan);
}

void MainWindow::slotChannelUpdate(const Channel& chan)
{
    Channel oldchan;
    if(!ui.channelsWidget->getChannel(chan.nChannelID, oldchan))
        return;
    
    //specific to classroom channel
    QString msg;
    bool before = false, after = false;
    before = userCanVoiceTx(TT_GetMyUserID(ttInst), oldchan);
    after = userCanVoiceTx(TT_GetMyUserID(ttInst), chan);
    if(before != after)
    {
        if(after)
            msg = tr("You can now transmit audio!");
        else
            msg = tr("You can no longer transmit audio!");
        addStatusMsg(msg);
    }
    before = userCanVideoTx(TT_GetMyUserID(ttInst), oldchan);
    after = userCanVideoTx(TT_GetMyUserID(ttInst), chan);
    if(before != after)
    {
        if(after)
            msg = tr("You can now transmit video!");
        else
            msg = tr("You can no longer transmit video!");
        addStatusMsg(msg);
    }
    before = userCanDesktopTx(TT_GetMyUserID(ttInst), oldchan);
    after = userCanDesktopTx(TT_GetMyUserID(ttInst), chan);
    if(before != after)
    {
        if(after)
            msg = tr("You can now transmit desktop windows!");
        else
            msg = tr("You can no longer transmit desktop windows!");
        addStatusMsg(msg);
    }
}

void MainWindow::slotInitVideo()
{
    if((TT_GetFlags(ttInst) & CLIENT_VIDEOCAPTURE_READY) == 0)
    {
        if(!initVideoCaptureFromSettings())
            QMessageBox::warning(this, tr("Start Webcam"), 
            tr("Video device hasn't been configured property. Check settings in 'Preferences'"));
    }
    else
    {
        TT_CloseVideoCaptureDevice(ttInst);
    }
    slotUpdateUI();
    slotUpdateVideoTabUI();
}

//TODO: remove this
void MainWindow::slotAddUserVideo()
{
    int chanid = TT_GetMyChannelID(ttInst);
    QVector<int> users = ui.channelsWidget->getUsersInChannel(chanid);
    QMenu menu(this);

    //local video frames have userid 0
    if((TT_GetFlags(ttInst) & CLIENT_VIDEOCAPTURE_READY) &&
       m_user_video.find(0 | VIDEOTYPE_CAPTURE) == m_user_video.end() &&
       !ui.videogridWidget->userExists(0 | VIDEOTYPE_CAPTURE))
    {
        QAction* myself = menu.addAction(tr("Myself"));
        myself->setData(0);
        if(users.size())
            menu.addSeparator();
        users.push_back(0);
    }

    //we don't want "myself" in the list
    int index = -1;
    if((index = users.indexOf(TT_GetMyUserID(ttInst))) != -1)
        users.remove(index);

    for(int i=0;i<users.size();i++)
    {
        if(m_user_video.find(users[i]) != m_user_video.end() ||
           ui.videogridWidget->userExists(users[i]))
           continue;

        User user;
        if(TT_GetUser(ttInst, users[i], &user) &&
           (user.uLocalSubscriptions & SUBSCRIBE_VIDEOCAPTURE) == 0)
           menu.addAction(getDisplayName(user))->setData(users[i]);
    }

    if(menu.isEmpty())return;
    
    QAction* action = menu.exec(QCursor::pos());
    if(!action)
        return;

    int userid = action->data().toInt();
    if(userid != 0)
    {
        //this is basically the same as slotUsersSubscriptionsVideo(..)
        subscribeCommon(true, SUBSCRIBE_VIDEOCAPTURE, userid);
    }

    ui.videogridWidget->slotAddUser(userid);

    m_vid_exclude.remove(userid);
}

//TODO: remove this
void MainWindow::slotRemoveUserVideo()
{
    int userid = ui.videogridWidget->selectedUser();
    if(userid<0)
    {
        QVector<int> users = ui.videogridWidget->activeUsers();
        QMenu menu(this);
        if(ui.videogridWidget->userExists(0))
        {
            QAction* myself = menu.addAction(tr("Myself"));
            myself->setData(0);
            if(users.size())
                menu.addSeparator();
        }
        User user;
        for(int i=0;i<users.size();i++)
        {
            if(TT_GetUser(ttInst, users[i], &user))
                menu.addAction(getDisplayName(user))->setData(users[i]);
        }

        if(menu.isEmpty())
            return;

        QAction* action = menu.exec(QCursor::pos());
        if(!action)
            return;
        userid = action->data().toInt();
    }

    if(userid>0)
        subscribeCommon(false, SUBSCRIBE_VIDEOCAPTURE, userid);
    m_vid_exclude.insert(userid);

    slotRemoveUserVideo(userid);
}

void MainWindow::slotRemoveUserVideo(int userid)
{
    ui.videogridWidget->removeUser(userid);
}

void MainWindow::slotDetachUserVideo(int userid)
{
    if(userid == 0)
        userid = ui.videogridWidget->selectedUser();

    if(userid<0)
        return;

    QSize size = ui.videogridWidget->getUserImageSize(userid);
    //if size if (0,0) then set size to invalid
    if(QSize(0,0) == size)
        size = QSize();

    ui.videogridWidget->removeUser(userid);
    Q_ASSERT(m_user_video.find(userid) == m_user_video.end());

    slotNewUserVideoDlg(userid, size);
}

void MainWindow::slotNewUserVideoDlg(int userid, const QSize& size)
{
    User user;
    ZERO_STRUCT(user);
    //user might not exist in channels tree since it can be local video (userid=0)
    ui.channelsWidget->getUser(userid & VIDEOTYPE_USERMASK, user);

    UserVideoDlg* dlg;
    if(size.isValid())
        dlg = new UserVideoDlg(userid, user, size, nullptr);
    else
        dlg = new UserVideoDlg(userid, user, nullptr);

    connect(this, SIGNAL(userUpdate(const User&)), dlg, 
            SLOT(slotUserUpdate(const User&)));
    connect(dlg, SIGNAL(userVideoDlgClosing(int)), 
            SLOT(slotUserVideoDlgClosing(int)));
    connect(this, SIGNAL(preferencesModified()), dlg->uservideoWidget,
            SLOT(slotUpdateVideoTextBox()));
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    switch(userid & VIDEOTYPE_MASK)
    {
    case VIDEOTYPE_CAPTURE :
        connect(this, SIGNAL(newVideoCaptureFrame(int,int)),
                dlg->uservideoWidget, SLOT(slotNewVideoFrame(int,int)));
        connect(this, SIGNAL(newVideoCaptureFrame(int,int)), dlg,
                SLOT(slotNewVideoFrame(int,int)));
        m_user_video[userid] = dlg;
        break;
    case VIDEOTYPE_MEDIAFILE :
        connect(this, SIGNAL(newMediaVideoFrame(int,int)),
                dlg->uservideoWidget, SLOT(slotNewVideoFrame(int,int)));
        connect(this, SIGNAL(newMediaVideoFrame(int,int)), dlg,
                SLOT(slotNewVideoFrame(int,int)));
        m_user_video[userid] = dlg;
        break;
    }
    dlg->show();
}

void MainWindow::slotUserVideoDlgClosing(int userid)
{
    Q_ASSERT(userid & VIDEOTYPE_MASK);
    switch(userid & VIDEOTYPE_MASK)
    {
    case VIDEOTYPE_CAPTURE :
    {
        uservideo_t::iterator ite = m_user_video.find(userid);
        Q_ASSERT(ite != m_user_video.end());
        if(ite == m_user_video.end())
            return;

        m_user_video.erase(ite);

        User user;
        if(!TT_GetUser(ttInst, userid & VIDEOTYPE_USERMASK, &user))
            return;
        //check to see if video session is still active
        if((user.uUserState & USERSTATE_VIDEOCAPTURE) == USERSTATE_NONE)
            return;
        //add back to grid (if enabled)
        if(ttSettings->value(SETTINGS_DISPLAY_VIDEORETURNTOGRID,
                             SETTINGS_DISPLAY_VIDEORETURNTOGRID_DEFAULT).toBool())
            ui.videogridWidget->slotAddUser(userid);
        else
        {
            Subscriptions subs = SUBSCRIBE_VIDEOCAPTURE;
            if(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE)
                subs |= SUBSCRIBE_INTERCEPT_VIDEOCAPTURE;
            subscribeCommon(false, subs, userid & VIDEOTYPE_USERMASK);
            m_vid_exclude.insert(userid);
        }
    }
    break;
    case VIDEOTYPE_MEDIAFILE :
    {
        uservideo_t::iterator ite = m_user_video.find(userid);
        Q_ASSERT(ite != m_user_video.end());
        if(ite == m_user_video.end())
            return;

        m_user_video.erase(ite);

        User user;
        if(!TT_GetUser(ttInst, userid & VIDEOTYPE_USERMASK, &user))
            return;
        //check to see if video session is still active
        if((user.uUserState & USERSTATE_MEDIAFILE_VIDEO) == USERSTATE_NONE)
            return;
        //add back to grid (if enabled)
        if(ttSettings->value(SETTINGS_DISPLAY_VIDEORETURNTOGRID,
                             SETTINGS_DISPLAY_VIDEORETURNTOGRID_DEFAULT).toBool())
            ui.videogridWidget->slotAddUser(userid);
        else
        {
            Subscriptions subs = SUBSCRIBE_MEDIAFILE;
            if(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_MEDIAFILE)
                subs |= SUBSCRIBE_INTERCEPT_MEDIAFILE;
            subscribeCommon(false, subs, userid & VIDEOTYPE_USERMASK);
            m_vid_exclude.insert(userid);
        }
    }
    break;
    }
}

void MainWindow::slotAddUserDesktop()
{
    int chanid = TT_GetMyChannelID(ttInst);
    QVector<int> users = ui.channelsWidget->getUsersInChannel(chanid);
    QMenu menu(this);

    //we don't want "myself" in the list
    int index = -1;
    if((index = users.indexOf(TT_GetMyUserID(ttInst))) != -1)
        users.remove(index);

    for(int i=0;i<users.size();i++)
    {
        if(m_userdesktop.find(users[i]) != m_userdesktop.end() ||
           ui.desktopgridWidget->userExists(users[i]))
           continue;

        User user;
        if(TT_GetUser(ttInst, users[i], &user) &&
           (user.uLocalSubscriptions & SUBSCRIBE_DESKTOP) == 0)
           menu.addAction(getDisplayName(user))->setData(users[i]);
    }

    if(menu.isEmpty())
        return;
    
    QAction* action = menu.exec(QCursor::pos());
    if(!action)
        return;

    int userid = action->data().toInt();
    if(userid != 0)
    {
        //this is basically the same as slotUsersSubscriptionsDesktop(..)
        subscribeCommon(true, SUBSCRIBE_DESKTOP, userid);
    }
}

void MainWindow::slotRemoveUserDesktop()
{
    int userid = ui.desktopgridWidget->selectedUser();
    if(userid<0)
    {
        QVector<int> users = ui.desktopgridWidget->activeUsers();
        QMenu menu(this);
        User user;
        for(int i=0;i<users.size();i++)
        {
            if(TT_GetUser(ttInst, users[i], &user))
                menu.addAction(getDisplayName(user))->setData(users[i]);
        }

        if(menu.isEmpty())
            return;

        QAction* action = menu.exec(QCursor::pos());
        if(!action)
            return;
        userid = action->data().toInt();
    }
    ui.desktopgridWidget->removeUser(userid);
    if(userid>0)
        subscribeCommon(false, SUBSCRIBE_DESKTOP, userid);
}

void MainWindow::slotRemoveUserDesktop(int userid)
{
    //remove from grid
    ui.desktopgridWidget->removeUser(userid);

    //close dialog
    userdesktop_t::iterator ite = m_userdesktop.find(userid);
    if(ite == m_userdesktop.end())
        return;
    (*ite)->close();
}

void MainWindow::slotAccessUserDesktop(bool enable)
{
    int userid = ui.desktopgridWidget->selectedUser();
    if(!userid)
        return;

    TextMessage msg = {};
    msg.nFromUserID = TT_GetMyUserID(ttInst);
    msg.nMsgType = MSGTYPE_CUSTOM;
    msg.nToUserID = userid;
    QString cmd = makeCustomCommand(TT_INTCMD_DESKTOP_ACCESS, 
                                    QString::number(enable));
    COPY_TTSTR(msg.szMessage, cmd);
    TT_DoTextMessage(ttInst, &msg);

    slotUpdateDesktopTabUI();
}

void MainWindow::slotDetachUserDesktop()
{
    int userid = ui.desktopgridWidget->selectedUser();
    if(userid<0)
        return;

    QSize imgsize = ui.desktopgridWidget->getUserImageSize(userid);
    if(!imgsize.isValid())
        return;

    ui.desktopgridWidget->removeUser(userid);
    Q_ASSERT(m_userdesktop.find(userid) == m_userdesktop.end());
    slotDetachUserDesktop(userid, imgsize);
}

void MainWindow::slotDetachUserDesktop(int userid, const QSize& size)
{
    User user;
    if(!ui.channelsWidget->getUser(userid, user))
        return;

    UserDesktopDlg* dlg;
    if(size.isValid())
        dlg = new UserDesktopDlg(user, size, nullptr);
    else
    {
        dlg = new UserDesktopDlg(user, QSize(640, 480), nullptr);
    }

    connect(this, SIGNAL(newDesktopWindow(int,int)),
            dlg, SIGNAL(userDesktopWindowUpdate(int,int)));
    connect(this, SIGNAL(userUpdate(const User&)), dlg, 
            SLOT(slotUserUpdate(const User&)));
    connect(this, SIGNAL(userUpdate(const User&)), dlg, 
            SIGNAL(userUpdated(const User&)));
    connect(this, SIGNAL(userDesktopCursor(int,const DesktopInput&)),
            dlg, SIGNAL(userDesktopCursorUpdate(int,const DesktopInput&)));
    connect(dlg, SIGNAL(userDesktopDlgClosing(int)), 
            SLOT(slotUserDesktopDlgClosing(int)));
    connect(dlg, SIGNAL(userDesktopWindowEnded(int)),
            SLOT(slotRemoveUserDesktop(int)));
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    m_userdesktop[userid] = dlg;
    dlg->show();
}

void MainWindow::slotUserDesktopDlgClosing(int userid)
{
    userdesktop_t::iterator ite = m_userdesktop.find(userid);
    Q_ASSERT(ite != m_userdesktop.end());
    if(ite == m_userdesktop.end())
        return;

    m_userdesktop.erase(ite);

    User user;
    if(!TT_GetUser(ttInst, userid, &user))
        return;
    //check to see if desktop session is still active
    if((user.uUserState & USERSTATE_DESKTOP) == USERSTATE_NONE)
        return;
    //add back to grid   
    ui.desktopgridWidget->slotAddUser(userid);
}

void MainWindow::slotUserJoin(int channelid, const User& user)
{
    //also set here in case VIEW_ALL_USERS is false
    QString audiofolder = ttSettings->value(SETTINGS_MEDIASTORAGE_AUDIOFOLDER).toString();
    AudioFileFormat aff = (AudioFileFormat)ttSettings->value(SETTINGS_MEDIASTORAGE_FILEFORMAT, AFF_WAVE_FORMAT).toInt();
    if(m_audiostorage_mode & AUDIOSTORAGE_SEPARATEFILES)
        TT_SetUserMediaStorageDir(ttInst, user.nUserID, _W(audiofolder), nullptr, aff);

    //only play sound when we're not currently performing an operation
    //like e.g. joining a new channel
    if(m_mychannel.nChannelID == channelid && m_current_cmdid == 0)
    {
        playSoundEvent(SOUNDEVENT_NEWUSER);
        addStatusMsg(tr("%1 joined channel").arg(getDisplayName(user)));
    }

    //set use to mute if enabled
    TT_SetUserStereo(ttInst, user.nUserID, STREAMTYPE_VOICE,
                     !ttSettings->value(SETTINGS_SOUND_SOUNDOUT_MUTE_LEFT,
                     SETTINGS_SOUND_SOUNDOUT_MUTE_LEFT_DEFAULT).toBool(),
                     !ttSettings->value(SETTINGS_SOUND_SOUNDOUT_MUTE_RIGHT,
                     SETTINGS_SOUND_SOUNDOUT_MUTE_RIGHT_DEFAULT).toBool());

    double d = ttSettings->value(SETTINGS_SOUND_MEDIASTREAM_VOLUME,
                                 SETTINGS_SOUND_MEDIASTREAM_VOLUME_DEFAULT).toDouble() / 100;
    TT_SetUserVolume(ttInst, user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO,
                     (int)(refVolume(SETTINGS_SOUND_MASTERVOLUME_DEFAULT) * d));
    TT_PumpMessage(ttInst, CLIENTEVENT_USER_STATECHANGE, user.nUserID);
}

void MainWindow::slotUserLeft(int channelid, const User& user)
{
    if(m_mychannel.nChannelID == channelid && m_current_cmdid == 0)
    {
        playSoundEvent(SOUNDEVENT_REMOVEUSER);
        addStatusMsg(tr("%1 left channel").arg(getDisplayName(user)));
    }

    //we cannot get user from channels-widget since user has left channel
    if(m_mychannel.nChannelID == channelid &&
       (user.uLocalSubscriptions & SUBSCRIBE_DESKTOPINPUT))
       subscribeCommon(false, SUBSCRIBE_DESKTOPINPUT, user.nUserID);
}

void MainWindow::slotUserUpdate(const User& user)
{
    User oldUser;
    if(ui.channelsWidget->getUser(user.nUserID, oldUser) &&
       m_mychannel.nChannelID == user.nChannelID && user.nChannelID)
    {
        QString nickname = getDisplayName(user);
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_USER_MSG) !=
            (user.uPeerSubscriptions & SUBSCRIBE_USER_MSG))
        {
            addStatusMsg(tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionUserMessages->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_USER_MSG?
                     tr("On"):tr("Off")));
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_CHANNEL_MSG) !=
            (user.uPeerSubscriptions & SUBSCRIBE_CHANNEL_MSG))
        {
            addStatusMsg(tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionChannelMessages->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_CHANNEL_MSG?
                     tr("On"):tr("Off")));
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_BROADCAST_MSG) !=
            (user.uPeerSubscriptions & SUBSCRIBE_BROADCAST_MSG))
        {
            addStatusMsg(tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionBroadcastMessages->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_BROADCAST_MSG?
                     tr("On"):tr("Off")));
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_VOICE) !=
            (user.uPeerSubscriptions & SUBSCRIBE_VOICE))
        {
            addStatusMsg(tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionVoice->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_VOICE?
                     tr("On"):tr("Off")));
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_VIDEOCAPTURE) !=
            (user.uPeerSubscriptions & SUBSCRIBE_VIDEOCAPTURE))
        {
            addStatusMsg(tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionVideo->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_VIDEOCAPTURE?
                     tr("On"):tr("Off")));
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_DESKTOP) !=
            (user.uPeerSubscriptions & SUBSCRIBE_DESKTOP))
        {
            addStatusMsg(tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionDesktop->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_DESKTOP?
                     tr("On"):tr("Off")));
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT) !=
            (user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT))
        {
            addStatusMsg(tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionDesktopInput->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT?
                     tr("On"):tr("Off")));
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_USER_MSG) !=
            (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_USER_MSG))
        {
            addStatusMsg(tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionInterceptUserMessages->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_USER_MSG?
                     tr("On"):tr("Off")));
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG) !=
            (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG))
        {
            addStatusMsg(tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionInterceptChannelMessages->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG?
                     tr("On"):tr("Off")));
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VOICE) !=
            (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VOICE))
        {
            addStatusMsg(tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionInterceptVoice->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VOICE?
                     tr("On"):tr("Off")));
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE) !=
            (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE))
        {
            addStatusMsg(tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionInterceptVideo->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE?
                     tr("On"):tr("Off")));
        }
    }
}

void MainWindow::slotEnableQuestionMode(bool checked)
{
    if(checked)
        m_statusmode |= STATUSMODE_QUESTION;
    else
        m_statusmode &= ~STATUSMODE_QUESTION;

    QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();
    if(TT_GetFlags(ttInst) & CLIENT_AUTHORIZED)
        TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));
}

void MainWindow::slotUpdateVideoCount(int count)
{
    if(count == 0)
        ui.tabWidget->setTabText(TAB_VIDEO, tr("Video"));
    else
        ui.tabWidget->setTabText(TAB_VIDEO, tr("Video (%1)").arg(count));
}

void MainWindow::slotUpdateDesktopCount(int count)
{
    if(count == 0)
        ui.tabWidget->setTabText(TAB_DESKTOP, tr("Desktops"));
    else
        ui.tabWidget->setTabText(TAB_DESKTOP, tr("Desktops (%1)").arg(count));
}

void MainWindow::slotMasterVolumeChanged(int value)
{
    int vol = refVolume(value);
//    qDebug() << "Volume is " << vol << " and percent is " << value;
//    qDebug() << "Percent is " << refVolumeToPercent(vol) << endl;
    TT_SetSoundOutputVolume(ttInst, vol);
}

void MainWindow::slotMicrophoneGainChanged(int value)
{
    SpeexDSP spxdsp;
    ZERO_STRUCT(spxdsp);
    if(TT_GetSoundInputPreprocess(ttInst, &spxdsp) && spxdsp.bEnableAGC)
    {
        double percent = value;
        percent /= 100.;
        spxdsp.nGainLevel = (INT32)(SOUND_GAIN_MAX * percent);
        TT_SetSoundInputPreprocess(ttInst, &spxdsp);
        TT_SetSoundInputGainLevel(ttInst, SOUND_GAIN_DEFAULT);
    }
    else
    {
        int gain = refGain(value);
//        qDebug() << "Gain is " << gain << " and percent is " << value;
        TT_SetSoundInputGainLevel(ttInst, gain);
    }
}

void MainWindow::slotVoiceActivationLevelChanged(int value)
{
    TT_SetVoiceActivationLevel(ttInst, value);
}

void MainWindow::slotTrayIconChange(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        showNormal();
        activateWindow(); //if we don't call this the window will end up in 
                          //the background for some reason (on win32)
    }
}

void MainWindow::slotLoadTTFile(const QString& filepath)
{
    QFile ttfile(QDir::fromNativeSeparators(filepath));
    if(!ttfile.open(QFile::ReadOnly))
    {
        QMessageBox::information(this, tr("Load File"), 
            tr("Failed to load file %1").arg(filepath));
        return;
    }

    QByteArray data = ttfile.readAll();
    QDomDocument doc(TTFILE_ROOT);
    if(!doc.setContent(data))
    {
        QMessageBox::information(this, tr("Load File"), 
            tr("Failed to load file %1").arg(filepath));
        return;
    }

    QDomElement rootElement(doc.documentElement());
    QString version = rootElement.attribute("version");
    
    if(!versionSameOrLater(version, TTFILE_VERSION))
    {
        QMessageBox::information(this, tr("Load File"), 
            tr("The file \"%1\" is incompatible with %2")
            .arg(QDir::toNativeSeparators(filepath))
            .arg(APPTITLE));
        return;
    }

    QDomElement element = rootElement.firstChildElement("host");
    HostEntry entry;
    if(!getServerEntry(element, entry))
    {
        QMessageBox::information(this, tr("Load File"), 
            tr("Failed to extract host-information from %1").arg(filepath));
        return;
    }

    addLatestHost(entry);
    m_host = entry;

    if(!element.firstChildElement(CLIENTSETUP_TAG).isNull() &&
        QMessageBox::question(this, tr("Load %1 File").arg(TTFILE_EXT),
        tr("The file %1 contains %2 setup information.\r\nShould these settings be applied?")
        .arg(filepath).arg(APPNAME_SHORT),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        //if no nickname specified use from .tt file
        if(m_host.nickname.size())
            ttSettings->setValue(SETTINGS_GENERAL_NICKNAME, m_host.nickname);

        //if no gender specified use from .tt file
        if(m_host.gender != GENDER_NONE)
            ttSettings->setValue(SETTINGS_GENERAL_GENDER, m_host.gender != GENDER_FEMALE);
        
        //if no PTT-key specified use from .tt file
        hotkey_t hotkey;
        if(m_host.hotkey.size())
        {
            saveHotKeySettings(HOTKEY_PUSHTOTALK, m_host.hotkey);
            enableHotKey(HOTKEY_PUSHTOTALK, m_host.hotkey);
        }

        //voice activation
        if(m_host.voiceact >= 0)
            slotMeEnableVoiceActivation(m_host.voiceact>0);

        //video capture
        if(isValid(m_host.capformat))
        {
            ttSettings->setValue(SETTINGS_VIDCAP_FOURCC, m_host.capformat.picFourCC);
            ttSettings->setValue(SETTINGS_VIDCAP_RESOLUTION, QString("%1x%2")
                                 .arg(m_host.capformat.nWidth)
                                 .arg(m_host.capformat.nHeight));
            ttSettings->setValue(SETTINGS_VIDCAP_FPS, QString("%1/%2")
                                 .arg(m_host.capformat.nFPS_Numerator)
                                 .arg(m_host.capformat.nFPS_Denominator));
            TT_CloseVideoCaptureDevice(ttInst);
        }

        //video codec
        switch(m_host.vidcodec.nCodec)
        {
            case WEBM_VP8_CODEC :
                ttSettings->setValue(SETTINGS_VIDCAP_CODEC,
                                     m_host.vidcodec.nCodec);
                ttSettings->setValue(SETTINGS_VIDCAP_WEBMVP8_BITRATE,
                                     m_host.vidcodec.webm_vp8.nRcTargetBitrate);
                TT_CloseVideoCaptureDevice(ttInst);
            break;
        case SPEEX_CODEC :
        case SPEEX_VBR_CODEC :
        case OPUS_CODEC :
        case NO_CODEC :
            break;
        }
    }

    Disconnect();
    Connect();
}

void MainWindow::slotSoftwareUpdateReply(QNetworkReply* reply)
{
    QByteArray data = reply->readAll();

    QDomDocument doc("foo");
    if(doc.setContent(data))
    {
        if(ttSettings->value(SETTINGS_DISPLAY_APPUPDATE, true).toBool())
        {
            QString version = newVersionAvailable(doc);
            if(version.size())
                addStatusMsg(tr("New version available: %1").arg(version));
        }
        
        BearWareLoginDlg::registerUrl = getBearWareRegistrationUrl(doc);
    }

    reply->manager()->deleteLater();
}

void MainWindow::slotBearWareAuthReply(QNetworkReply* reply)
{
    QByteArray data = reply->readAll();
    QDomDocument doc("foo");
    if(doc.setContent(data))
    {
        auto child = doc.firstChildElement("teamtalk");
        if(!child.isNull())
        {
            child = child.firstChildElement("bearware");
            if(!child.isNull())
            {
                auto id = child.firstChildElement("username");
                if(!id.isNull())
                    m_host.username = id.text();
            }
        }
    }
    reply->manager()->deleteLater();

    // connect even if auth failed. Otherwise user will not see progress
    login();
}

void MainWindow::slotClosedOnlineUsersDlg(int)
{
    m_onlineusersdlg = nullptr;
}

void MainWindow::slotClosedServerStatsDlg(int)
{
    m_serverstatsdlg = nullptr;
}

void MainWindow::slotClosedUserAccountsDlg(int)
{
    m_useraccountsdlg = nullptr;
}

void MainWindow::slotClosedBannedUsersDlg(int)
{
    m_bannedusersdlg = nullptr;
}

