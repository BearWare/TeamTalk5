/*
 * Copyright (C) 2023, Bjørn D. Rasmussen, BearWare.dk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
#include "bearwarelogindlg.h"
#include "utilos.h"
#include "utilvideo.h"
#include "utiltts.h"
#include "utilxml.h"
#include "utilmedia.h"
#include "moveusersdlg.h"
#include "useraccountdlg.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QTimer>
#include <QSettings>
#include <QFile>
#include <QTranslator>
#include <QUrl>
#include <QTabWidget>
#include <QHeaderView>
#include <QRegularExpression>
#include <QDesktopServices>
#include <QProcess>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QClipboard>
#include <QSysInfo>

#if defined(QT_TEXTTOSPEECH_LIB)
#include <QTextToSpeech>
#endif

#ifdef Q_OS_LINUX //For hotkeys and DBus on X11
#include <QtDBus/QtDBus>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#include <functional>
#include <algorithm>
using namespace std::placeholders;

extern TTInstance* ttInst;

QSettings* ttSettings = nullptr;
QTranslator* ttTranslator = nullptr;
PlaySoundEvent* playsoundevent = nullptr;

#if defined(QT_TEXTTOSPEECH_LIB)
QTextToSpeech* ttSpeech = nullptr;
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6,8,0)
QObject* announcerObject = nullptr;
#endif

//strip ampersand from menutext
#define MENUTEXT(text) text.replace("&", "")

#define CHANNELID_TEMPPASSWORD -1

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
        QMessageBox::warning(nullptr, "DLL load error",
                             QString("This %3 executable is built for DLL version %1 but the loaded DLL reports it's version %2. Loading an incorrect DLL for %3 may cause problems and crash the application. Please reinstall to solve this problem.")
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

    playsoundevent = new PlaySoundEvent(this);
    m_filesmodel = new FilesModel(this);
    m_proxyFilesModel = new QSortFilterProxyModel(this);
    m_proxyFilesModel->setSourceModel(m_filesmodel);
    m_proxyFilesModel->setSortRole(Qt::UserRole);
    ui.filesView->setModel(m_proxyFilesModel);
    m_proxyFilesModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyFilesModel->sort(COLUMN_INDEX_NAME, Qt::AscendingOrder);
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


#if defined(Q_OS_WIN32)
    ui.actionExit->setShortcut(QKeySequence(Qt::ALT | Qt::Key_F4));
#else
    ui.actionExit->setShortcut(QKeySequence::Quit);
#endif

    connect(ui.msgEdit, &QLineEdit::textChanged, this, &MainWindow::slotTextChanged);
    connect(ui.sendButton, &QAbstractButton::clicked,
            this, &MainWindow::slotSendChannelMessage);
    connect(ui.msgEdit, &ChatLineEdit::sendTextMessage,
            this, &MainWindow::slotSendChannelMessage);
    connect(ui.videosendButton, &QAbstractButton::clicked,
            this, &MainWindow::slotSendChannelMessage);
    connect(ui.desktopsendButton, &QAbstractButton::clicked,
            this, &MainWindow::slotSendChannelMessage);
    connect(ui.videomsgEdit, &ChatLineEdit::sendTextMessage,
            this, &MainWindow::slotSendChannelMessage);
    connect(ui.videomsgEdit, &QLineEdit::textChanged, this, &MainWindow::slotTextChanged);
    connect(ui.desktopmsgEdit, &QLineEdit::textChanged, this, &MainWindow::slotTextChanged);
    connect(ui.desktopmsgEdit, &ChatLineEdit::sendTextMessage,
            this, &MainWindow::slotSendChannelMessage);
    connect(ui.micSlider, &QAbstractSlider::valueChanged,
            this, &MainWindow::slotMicrophoneGainChanged);
    connect(ui.volumeSlider, &QAbstractSlider::valueChanged,
            this, &MainWindow::slotMasterVolumeChanged);
    connect(ui.voiceactSlider, &QAbstractSlider::valueChanged,
            this, &MainWindow::slotVoiceActivationLevelChanged);

    /* ui.channelsWidget */
    connect(ui.channelsWidget, &QTreeWidget::itemSelectionChanged,
            this, &MainWindow::slotTreeSelectionChanged);
    connect(ui.channelsWidget, &QWidget::customContextMenuRequested,
            this, &MainWindow::slotTreeContextMenu);
    connect(ui.channelsWidget, &ChannelsTree::userDoubleClicked,
            this, &MainWindow::slotUserDoubleClicked);
    connect(ui.channelsWidget, &ChannelsTree::channelDoubleClicked,
            this, &MainWindow::slotChannelDoubleClicked);
    connect(ui.channelsWidget, &ChannelsTree::fileDropped,
            this, &MainWindow::slotLoadTTFile);
    connect(ui.channelsWidget,
            &ChannelsTree::transmitusersChanged,
            this, &MainWindow::slotTransmitUsersChanged);
    /* Video-tab (video-grid) */
    connect(this, &MainWindow::newVideoCaptureFrame, ui.videogridWidget,
            &VideoGridWidget::slotNewVideoFrame);
    connect(this, &MainWindow::newMediaVideoFrame, ui.videogridWidget,
            &VideoGridWidget::slotNewVideoFrame);
    connect(ui.videogridWidget, &VideoGridWidget::userVideoEnded,
            this, &MainWindow::slotRemoveUserVideo);
    connect(ui.videogridWidget, &VideoGridWidget::videoCountChanged,
            this, &MainWindow::slotUpdateVideoCount);
    connect(ui.videogridWidget, &VideoGridWidget::videoCountChanged,
            this, &MainWindow::slotUpdateVideoTabUI);
    connect(ui.videogridWidget, &VideoGridWidget::userVideoSelected,
            this, &MainWindow::slotUpdateVideoTabUI);
    /* Desktop-tab (desktop-grid) */
    connect(this, &MainWindow::newDesktopWindow, ui.desktopgridWidget,
            &DesktopGridWidget::userDesktopWindowUpdate);
    connect(this, &MainWindow::userDesktopCursor,
            ui.desktopgridWidget, &DesktopGridWidget::userDesktopCursorUpdate);
    connect(this, &MainWindow::userUpdate, ui.desktopgridWidget,
            &DesktopGridWidget::userUpdated);
    connect(this, &MainWindow::userLeft, ui.desktopgridWidget,
            &DesktopGridWidget::slotRemoveUser);
    connect(ui.desktopgridWidget, &DesktopGridWidget::userDesktopWindowEnded,
            this, &MainWindow::slotRemoveUserDesktop);
    connect(ui.desktopgridWidget, &DesktopGridWidget::desktopCountChanged,
            this, &MainWindow::slotUpdateDesktopCount);
    connect(ui.desktopgridWidget, &DesktopGridWidget::desktopCountChanged,
            this, &MainWindow::slotUpdateDesktopTabUI);
    connect(ui.desktopgridWidget, &DesktopGridWidget::userDesktopSelected,
            this, &MainWindow::slotUpdateDesktopTabUI);
    /* Files-tab */
    connect(ui.uploadButton, &QAbstractButton::clicked, this, &MainWindow::slotChannelsUploadFile);
    connect(ui.downloadButton, &QAbstractButton::clicked, this, &MainWindow::slotChannelsDownloadFile);
    connect(ui.deleteButton, &QAbstractButton::clicked, this, &MainWindow::slotChannelsDeleteFile);
    connect(selmodel, &QItemSelectionModel::selectionChanged,
            ui.filesView, &FilesView::slotNewSelection);
    connect(ui.filesView, &FilesView::filesSelected, ui.actionDeleteFile,
            &QAction::setEnabled);
    connect(ui.filesView, &FilesView::filesSelected, ui.deleteButton,
            &QWidget::setEnabled);
    connect(ui.filesView, &FilesView::uploadFiles,
            this, &MainWindow::slotUploadFiles);
    connect(ui.filesView, &QWidget::customContextMenuRequested,
            this, &MainWindow::slotFilesContextMenu);
    /* Video-tab buttons */
    connect(ui.initVideoButton, &QAbstractButton::clicked,
            ui.actionEnableVideoTransmission, &QAction::triggered);
    connect(ui.addVideoButton, &QAbstractButton::clicked, this, &MainWindow::slotAddUserVideo);
    connect(ui.removeVideoButton, &QAbstractButton::clicked, this, &MainWindow::slotRemoveUserVideoGrid);
    connect(ui.detachVideoButton, &QAbstractButton::clicked, this, &MainWindow::slotDetachUserVideo);
    connect(ui.videogridWidget, &VideoGridWidget::userVideoSelected,
            ui.detachVideoButton, &QWidget::setEnabled);
    connect(this, &MainWindow::preferencesModified,
            ui.videogridWidget, &VideoGridWidget::preferencesModified);
    /* Desktop-tab buttons */
    connect(ui.detachDesktopButton, &QAbstractButton::clicked,
            this, &MainWindow::slotDetachUserDesktopGrid);
    connect(ui.addDesktopButton, &QAbstractButton::clicked,
            this, &MainWindow::slotAddUserDesktopGrid);
    connect(ui.removeDesktopButton, &QAbstractButton::clicked,
            this, &MainWindow::slotRemoveUserDesktopGrid);
    connect(ui.desktopaccessButton, &QAbstractButton::clicked,
            this, &MainWindow::slotAccessUserDesktop);
    connect(ui.desktopgridWidget, &DesktopGridWidget::userDesktopSelected,
            ui.detachDesktopButton, &QWidget::setEnabled);
    connect(ui.desktopgridWidget, &DesktopGridWidget::userDesktopSelected,
            ui.desktopaccessButton, &QWidget::setEnabled);

    /* Begin - Client menu */
    connect(ui.actionNewClient, &QAction::triggered,
            this, &MainWindow::slotClientNewInstance);
    connect(ui.actionConnect, &QAction::triggered,
            this, &MainWindow::slotClientConnect);
    connect(ui.actionPreferences, &QAction::triggered,
            this, &MainWindow::slotClientPreferences);
    connect(ui.menuSoundConfiguration, &QMenu::aboutToShow,
            this, &MainWindow::slotClientSoundDevices);
    connect(ui.actionRefreshSoundDevices, &QAction::triggered,
        this, &MainWindow::initSound);

    connect(ui.actionEnableEchoCancel, &QAction::triggered,
            this, &MainWindow::slotClientAudioEffect);
    connect(ui.actionEnableAGC, &QAction::triggered,
            this, &MainWindow::slotClientAudioEffect);
    connect(ui.actionEnableDenoising, &QAction::triggered,
            this, &MainWindow::slotClientAudioEffect);
    connect(ui.actionExit, &QAction::triggered,
            this, &MainWindow::slotClientExit);
    connect(ui.actionSpeakClientStats, &QAction::triggered,
            this, &MainWindow::slotSpeakClientStats);
    /* End - Client menu */

    /* Begin - Me menu */
    connect(ui.actionChangeNickname, &QAction::triggered,
            this, &MainWindow::slotMeChangeNickname);
    connect(ui.actionChangeStatus, &QAction::triggered,
            this, &MainWindow::slotMeChangeStatus);
    connect(ui.actionHearMyself, &QAction::triggered,
            this, &MainWindow::slotMeHearMyself);
    connect(ui.actionEnablePushToTalk, &QAction::triggered,
            this, &MainWindow::slotMeEnablePushToTalk);
    connect(ui.actionEnableVoiceActivation, &QAction::triggered,
            this, &MainWindow::slotMeEnableVoiceActivation);
    connect(ui.actionEnableVideoTransmission, &QAction::triggered,
            this, &MainWindow::slotMeEnableVideoTransmission);
    connect(ui.actionEnableDesktopSharing, &QAction::triggered,
            this, &MainWindow::slotMeEnableDesktopSharing);
    connect(ui.actionEnableTTS, &QAction::triggered,
            this, &MainWindow::slotMeEnableTTS);
    connect(ui.actionEnableSounds, &QAction::triggered,
            this, &MainWindow::slotMeEnableSounds);
    /* End - Me menu */

    /* Begin - Users menu */
    connect(ui.actionViewUserInformation, &QAction::triggered,
            this, &MainWindow::slotUsersViewUserInformationGrid);
    connect(ui.actionSpeakUserInformation, &QAction::triggered,
            this, &MainWindow::slotUsersSpeakUserInformationGrid);
    connect(ui.actionMessages, &QAction::triggered,
            this, &MainWindow::slotUsersMessagesGrid);
    connect(ui.actionMuteVoice, &QAction::triggered,
            this, &MainWindow::slotUsersMuteVoiceGrid);
    connect(ui.actionMuteMediaFile, &QAction::triggered,
            this, &MainWindow::slotUsersMuteMediaFileGrid);
    connect(ui.actionVolume, &QAction::triggered,
            this, &MainWindow::slotUsersVolumeGrid);
    connect(ui.actionOp, &QAction::triggered,
            this, &MainWindow::slotUsersOpGrid);
    connect(ui.actionKickFromChannel, &QAction::triggered,
            this, &MainWindow::slotUsersKickFromChannel);
    connect(ui.actionKickAndBanFromChannel, &QAction::triggered,
        this, &MainWindow::slotUsersKickBanFromChannel);
    connect(ui.actionKickFromServer, &QAction::triggered,
            this, &MainWindow::slotUsersKickFromServer);
    connect(ui.actionKickBan, &QAction::triggered,
            this, &MainWindow::slotUsersKickBanFromServer);
    connect(ui.actionMuteAll, &QAction::triggered,
            this, &MainWindow::slotUsersMuteVoiceAll);
    connect(ui.actionMediaStorage, &QAction::triggered,
            this, &MainWindow::slotClientRecordConversations);
    //Desktop access
    connect(ui.actionDesktopAccessAllow, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsDesktopInput);
    //subscriptions
    connect(ui.actionUserMessages, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsUserMsg);
    connect(ui.actionChannelMessages, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsChannelMsg);
    connect(ui.actionBroadcastMessages, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsBCastMsg);
    connect(ui.actionVoice, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsVoice);
    connect(ui.actionVideo, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsVideo);
    connect(ui.actionDesktop, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsDesktop);
    connect(ui.actionDesktopInput, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsDesktopInput);
    connect(ui.actionMediaFile, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsMediaFile);
    connect(ui.actionInterceptUserMessages, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsInterceptUserMsg);
    connect(ui.actionInterceptChannelMessages, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsInterceptChannelMsg);
    connect(ui.actionInterceptVoice, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsInterceptVoice);
    connect(ui.actionInterceptVideo, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsInterceptVideo);
    connect(ui.actionInterceptDesktop, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsInterceptDesktop);
    connect(ui.actionInterceptMediaFile, &QAction::triggered,
            this, &MainWindow::slotUsersSubscriptionsInterceptMediaFile);

    //advanced
    connect(ui.actionIncreaseVoiceVolume, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedIncVolumeVoice);
    connect(ui.actionLowerVoiceVolume, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedDecVolumeVoice);
    connect(ui.actionIncreaseMediaFileVolume, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedIncVolumeMediaFile);
    connect(ui.actionLowerMediaFileVolume, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedDecVolumeMediaFile);
    connect(ui.actionStoreForMove, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedStoreForMove);
    connect(ui.actionMoveUser, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedMoveUsers);
    connect(ui.actionMoveUsersDialog, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedMoveUsersDialog);
    connect(ui.actionAllowChannelTextMessages, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedChanMsgAllowed);
    connect(ui.actionAllowVoiceTransmission, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedVoiceAllowed);
    connect(ui.actionAllowVideoTransmission, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedVideoAllowed);
    connect(ui.actionAllowDesktopTransmission, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedDesktopAllowed);
    connect(ui.actionRelayVoiceStream, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedRelayUserVoice);
    connect(ui.actionRelayMediaFileStream, &QAction::triggered,
        this, &MainWindow::slotUsersAdvancedRelayUserMediaFile);
    connect(ui.actionAllowMediaFileTransmission, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedMediaFileAllowed);
    connect(ui.actionAllowAllChannelTextMessages, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedChanMsgAllowed);
    connect(ui.actionAllowAllVoiceTransmission, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedVoiceAllowed);
    connect(ui.actionAllowAllVideoTransmission, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedVideoAllowed);
    connect(ui.actionAllowAllDesktopTransmission, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedDesktopAllowed);
    connect(ui.actionAllowAllMediaFileTransmission, &QAction::triggered,
            this, &MainWindow::slotUsersAdvancedMediaFileAllowed);
    /* End - Users menu */

    /* Begin - Channels menu */
    connect(ui.actionCreateChannel, &QAction::triggered,
            this, &MainWindow::slotChannelsCreateChannel);
    connect(ui.actionUpdateChannel, &QAction::triggered,
            this, &MainWindow::slotChannelsUpdateChannel);
    connect(ui.actionDeleteChannel, &QAction::triggered,
            this, &MainWindow::slotChannelsDeleteChannel);
    connect(ui.actionJoinChannel, &QAction::triggered,
            this, &MainWindow::slotChannelsJoinChannel);
    connect(ui.actionLeaveChannel, &QAction::triggered,
            this, &MainWindow::slotChannelsLeaveChannel);
    connect(ui.actionViewChannelInfo, &QAction::triggered,
            this, &MainWindow::slotChannelsViewChannelInfo);
    connect(ui.actionSpeakChannelInfo, &QAction::triggered,
            this, &MainWindow::slotChannelsSpeakChannelInformationGrid);
    connect(ui.actionSpeakChannelStat, &QAction::triggered,
            this, &MainWindow::slotChannelsSpeakChannelStatusGrid);
    connect(ui.actionBannedUsersInChannel, &QAction::triggered,
            this, &MainWindow::slotChannelsListBans);

    connect(ui.actionStreamMediaFileToChannel, &QAction::triggered,
            this, &MainWindow::slotChannelsStreamMediaFile);
    connect(ui.actionPauseResumeStream, &QAction::triggered,
            this, &MainWindow::slotPauseResumeStream);
    connect(ui.actionUploadFile, &QAction::triggered,
            this, &MainWindow::slotChannelsUploadFile);
    connect(ui.actionDownloadFile, &QAction::triggered,
            this, &MainWindow::slotChannelsDownloadFile);
    connect(ui.actionDeleteFile, &QAction::triggered,
            this, &MainWindow::slotChannelsDeleteFile);
    connect(ui.actionGenerateTTURL, &QAction::triggered,
            this, &MainWindow::slotChannelsGenerateTTUrl);
    /* End - Channels menu */

    /* Begin - Server menu */
    connect(ui.actionUserAccounts, &QAction::triggered,
            this, &MainWindow::slotServerUserAccounts);
    connect(ui.actionBannedUsers, &QAction::triggered,
            this, &MainWindow::slotServerBannedUsers);
    connect(ui.actionOnlineUsers, &QAction::triggered,
            this, &MainWindow::slotServerOnlineUsers);
    connect(ui.actionBroadcastMessage, &QAction::triggered,
            this, &MainWindow::slotServerBroadcastMessage);
    connect(ui.actionServerProperties, &QAction::triggered,
            this, &MainWindow::slotServerServerProperties);
    connect(ui.actionSaveConfiguration, &QAction::triggered,
            this, &MainWindow::slotServerSaveConfiguration);
    connect(ui.actionServerStatistics, &QAction::triggered,
            this, &MainWindow::slotServerServerStatistics);
    /* End - Server menu */

    /* Begin - Help menu */
    connect(ui.actionManual, &QAction::triggered,
            this, &MainWindow::slotHelpManual);
    connect(ui.actionResetPreferencesToDefault, &QAction::triggered,
            this, &MainWindow::slotHelpResetPreferences);
    connect(ui.actionVisitBearWare, &QAction::triggered,
            this, &MainWindow::slotHelpVisitBearWare);
    connect(ui.actionCheckUpdate, &QAction::triggered,
            this, [this]() {
                checkAppUpdate(true);
            });
    connect(ui.actionAbout, &QAction::triggered,
            this, &MainWindow::slotHelpAbout);
    /* End - Help menu */

    /* Begin - Extra toolbar buttons */
    connect(ui.actionToggleQuestionMode, &QAction::triggered,
            this, &MainWindow::slotToggleQuestionMode);
    /* End - Extra toolbar buttons */

    /* Begin - CLIENTEVENT_* messages */
    connect(this, &MainWindow::serverUpdate, ui.channelsWidget,
            &ChannelsTree::slotServerUpdate);
    connect(this, &MainWindow::newChannel,
            ui.channelsWidget, &ChannelsTree::slotAddChannel);
    connect(this, &MainWindow::updateChannel,
            this, &MainWindow::slotChannelUpdate);
    connect(this, &MainWindow::updateChannel,
            ui.channelsWidget, &ChannelsTree::slotUpdateChannel);
    connect(this, &MainWindow::removeChannel,
            ui.channelsWidget, &ChannelsTree::slotRemoveChannel);
    connect(this, &MainWindow::userLogin, ui.channelsWidget,
            &ChannelsTree::slotUserLoggedIn);
    connect(this, &MainWindow::userLogout, ui.channelsWidget,
            &ChannelsTree::slotUserLoggedOut);
    /* 'this' must be connected first since it needs to extract the old
     * user from channelswidget */
    connect(this, &MainWindow::userUpdate, this,
            &MainWindow::slotUserUpdate);
    connect(this, &MainWindow::userUpdate, ui.channelsWidget,
            &ChannelsTree::slotUserUpdate);
    connect(this, &MainWindow::userJoined, this,
            &MainWindow::slotUserJoin);
    connect(this, &MainWindow::userJoined, ui.channelsWidget,
            &ChannelsTree::slotUserJoin);
    connect(this, &MainWindow::userLeft, this,
            &MainWindow::slotUserLeft);
    connect(this, &MainWindow::userLeft, ui.channelsWidget,
            &ChannelsTree::slotUserLeft);
    connect(this, &MainWindow::userLeft, ui.videogridWidget,
            &VideoGridWidget::slotRemoveUser);
    connect(this, &MainWindow::userStateChange, ui.channelsWidget,
            &ChannelsTree::slotUserStateChange);
    connect(this, &MainWindow::updateMyself, ui.channelsWidget,
            &ChannelsTree::slotUpdateMyself);
    connect(this, &MainWindow::newVideoCaptureFrame, ui.channelsWidget,
            &ChannelsTree::slotUserVideoFrame);
    connect(this, &MainWindow::cmdSuccess, this, &MainWindow::slotCmdSuccess);
    connect(this, &MainWindow::mediaPlaybackUpdate, playsoundevent, &PlaySoundEvent::playbackUpdate);
    /* End - CLIENTEVENT_* messages */

    m_timers.insert(startTimer(1000), TIMER_ONE_SECOND);

#if !defined(Q_OS_WIN32)
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
    ttSettings->setValue(SETTINGS_SOUND_MASTERVOLUME, ui.volumeSlider->value());
    ttSettings->setValue(SETTINGS_SOUND_MICROPHONEGAIN, ui.micSlider->value());
    ttSettings->setValue(SETTINGS_SOUND_VOICEACTIVATIONLEVEL, ui.voiceactSlider->value());

    auto activekeys = ttSettings->value(SETTINGS_SHORTCUTS_ACTIVEHKS, SETTINGS_SHORTCUTS_ACTIVEHKS_DEFAULT).toULongLong();
    ttSettings->setValue(SETTINGS_SHORTCUTS_ACTIVEHKS, (ui.actionEnablePushToTalk->isChecked() ? activekeys | HOTKEY_PUSHTOTALK : activekeys & ~HOTKEY_PUSHTOTALK));
    ttSettings->setValue(SETTINGS_GENERAL_VOICEACTIVATED, ui.actionEnableVoiceActivation->isChecked());

    if(windowState() == Qt::WindowNoState)
    {
        saveWindowPosition(SETTINGS_DISPLAY_MAINWINDOWPOS, this);
        ttSettings->setValue(SETTINGS_DISPLAY_SPLITTER, ui.splitter->saveState());
        ttSettings->setValue(SETTINGS_DISPLAY_VIDEOSPLITTER, ui.videosplitter->saveState());
        ttSettings->setValue(SETTINGS_DISPLAY_DESKTOPSPLITTER, ui.desktopsplitter->saveState());
    }

    ttSettings->setValue(SETTINGS_DISPLAY_FILESHEADER, ui.filesView->horizontalHeader()->saveState());

    ttSettings->setValue(SETTINGS_DISPLAY_WINDOW_MAXIMIZE, this->isMaximized());

    delete ttSettings;
}

void MainWindow::loadSettings()
{
    migrateSettings();

    // Ask to set language at first start
    if (!ttSettings->contains(SETTINGS_DISPLAY_LANGUAGE))
    {
        QLocale locale = QLocale::system();
        QString languageCode = locale.name();
        if (switchLanguage(languageCode))
            this->ui.retranslateUi(this);
        QMessageBox answer;
        answer.setText(tr("%1 has detected your system language to be %2. Continue in %2?").arg(APPNAME_SHORT).arg(getLanguageDisplayName(languageCode)));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle(tr("Language configuration"));
        answer.exec();
        if(answer.clickedButton() == YesButton)
        {
            ttSettings->setValue(SETTINGS_DISPLAY_LANGUAGE, languageCode);
        }
        else if(answer.clickedButton() == NoButton)
        {
            QStringList languages = extractLanguages();
            languages.insert(0, SETTINGS_DISPLAY_LANGUAGE_DEFAULT); //default language is none
            QMap<QString, QString> languageMap;
            for (const QString &code : languages)
            {
                QString displayName = (code == "none") ? "" : getLanguageDisplayName(code);
                languageMap[displayName] = code;
            }
            QStringList displayNames = languageMap.keys();
            bool ok = false;
            QInputDialog inputDialog;
            inputDialog.setOkButtonText(tr("&OK"));
            inputDialog.setCancelButtonText(tr("&Cancel"));
            inputDialog.setComboBoxItems(displayNames);
            inputDialog.setComboBoxEditable(false);
            inputDialog.setWindowTitle(tr("Choose language"));
            inputDialog.setLabelText(tr("Select the language will be use by %1").arg(APPNAME_SHORT));
            ok = inputDialog.exec();
            QString choice = inputDialog.textValue();
            if (ok)
            {
                QString lc_code = languageMap.value(choice, "");
                ttSettings->setValue(SETTINGS_DISPLAY_LANGUAGE, lc_code);
            }
        }
    }

    QString lang = ttSettings->value(SETTINGS_DISPLAY_LANGUAGE, "").toString();
    if (!lang.isEmpty())
    {
        if (switchLanguage(lang))
            this->ui.retranslateUi(this);
        else
        {
            QMessageBox::information(this, tr("Translate"),
                QString("Failed to load language file %1").arg(lang));
        }
    }

    initSound();

    //load settings
    bool ptt = ttSettings->value(SETTINGS_SHORTCUTS_ACTIVEHKS, SETTINGS_SHORTCUTS_ACTIVEHKS_DEFAULT).toULongLong() & HOTKEY_PUSHTOTALK;
    slotMeEnablePushToTalk(ptt);
    bool vox = ttSettings->value(SETTINGS_GENERAL_VOICEACTIVATED,
                                 SETTINGS_GENERAL_VOICEACTIVATED_DEFAULT).toBool();
    enableVoiceActivation(vox, SOUNDEVENT_VOICEACTON, SOUNDEVENT_VOICEACTOFF);

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

    //default voice gain level depends on whether AGC or normal gain
    //is enabled
    value = ttSettings->value(SETTINGS_SOUND_MICROPHONEGAIN,
                              SETTINGS_SOUND_MICROPHONEGAIN_GAIN_DEFAULT).toInt();
    ui.micSlider->setValue(value);
    slotMicrophoneGainChanged(value); //force update on equal

    m_audiostorage_mode = ttSettings->value(SETTINGS_MEDIASTORAGE_MODE,
                                            AUDIOSTORAGE_NONE).toUInt();


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

    //move window to last position
    if (restoreWindowPosition(SETTINGS_DISPLAY_MAINWINDOWPOS, this))
    {
        ui.splitter->restoreState(ttSettings->value(SETTINGS_DISPLAY_SPLITTER).toByteArray());
        ui.videosplitter->restoreState(ttSettings->value(SETTINGS_DISPLAY_VIDEOSPLITTER).toByteArray());
        ui.desktopsplitter->restoreState(ttSettings->value(SETTINGS_DISPLAY_DESKTOPSPLITTER).toByteArray());
    }
    //set files header to last position
    ui.filesView->horizontalHeader()->restoreState(ttSettings->value(SETTINGS_DISPLAY_FILESHEADER).toByteArray());
    ui.filesView->horizontalHeader()->setSectionsMovable(false);
    // Maximize window if necessary
    if (ttSettings->value(SETTINGS_DISPLAY_WINDOW_MAXIMIZE).toBool() == true)
        this->showMaximized();
    // http query for app updates
    checkAppUpdate();
    m_timers.insert(startTimer(24 * 60 * 60 * 1000), TIMER_APP_UPDATE);

    if(ttSettings->value(SETTINGS_DISPLAY_STARTMINIMIZED, false).toBool())
        QTimer::singleShot(0, this, &MainWindow::showMinimized);

    //started from .tt file?
    bool connect_ok = parseArgs(QApplication::arguments());

    if(connect_ok)
        QTimer::singleShot(0, this, &MainWindow::slotConnectToLatest);

    initialScreenReaderSetup();

    // Hide/Show widgets
    if (ttSettings->value(SETTINGS_DISPLAY_VU_METER_UPDATES,
        SETTINGS_DISPLAY_VU_METER_UPDATES_DEFAULT).toBool())
    {
        m_timers.insert(startTimer(50), TIMER_VUMETER_UPDATE);
        ui.vumeterLabel->setVisible(true);
        ui.voiceactBar->setVisible(true);
    }
    else
    {
        ui.vumeterLabel->setVisible(false);
        ui.voiceactBar->setVisible(false);
    }
    ui.voiceactLabel->setVisible(ui.actionEnableVoiceActivation->isChecked() && ttSettings->value(SETTINGS_DISPLAY_VOICE_ACT_SLIDER, SETTINGS_DISPLAY_VOICE_ACT_SLIDER_DEFAULT).toBool());
    ui.voiceactSlider->setVisible(ui.actionEnableVoiceActivation->isChecked() && ttSettings->value(SETTINGS_DISPLAY_VOICE_ACT_SLIDER, SETTINGS_DISPLAY_VOICE_ACT_SLIDER_DEFAULT).toBool());

    // Sounds pack checks
    QString packset = ttSettings->value(SETTINGS_SOUNDS_PACK).toString();
    QString packname = QString("%1/%2").arg(SOUNDSPATH).arg(packset);
    QDir packdir(packname);
    if((!packdir.exists()) && packset != tr("Default"))
    {
        QMessageBox answer;
        answer.setText(tr("The sound pack %1 does not exist. Would you like to use the default sound pack?").arg(packset));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        Q_UNUSED(NoButton);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle(APPNAME_SHORT);
        answer.exec();
        if(answer.clickedButton() == YesButton)
        {
            resetDefaultSoundsPack();
        }
    }
    else if(packset.isEmpty())
    {
        resetDefaultSoundsPack();
    }

    startTTS();

    if ((ttSettings->value(SETTINGS_DISPLAY_START_SERVERLIST, SETTINGS_DISPLAY_START_SERVERLIST_DEFAULT).toBool() == true && ttSettings->value(SETTINGS_CONNECTION_AUTOCONNECT, SETTINGS_CONNECTION_AUTOCONNECT_DEFAULT).toBool() == false) && ((TT_GetFlags(ttInst) & CLIENT_CONNECTION) == CLIENT_CLOSED))
        slotClientConnect();
    slotUpdateUI();
}

void MainWindow::initialScreenReaderSetup()
{
#if defined(ENABLE_TOLK) || defined(Q_OS_LINUX)
    if (ttSettings->value(SETTINGS_GENERAL_FIRSTSTART, SETTINGS_GENERAL_FIRSTSTART_DEFAULT).toBool())
    {
        bool SRActive = false;
#if defined(ENABLE_TOLK)
        bool tolkLoaded = Tolk_IsLoaded();
        if (!tolkLoaded)
            Tolk_Load();

        SRActive = Tolk_DetectScreenReader() != nullptr;

        if (!tolkLoaded)
            Tolk_Unload();
#elif defined(Q_OS_LINUX)
        QDBusInterface interface("org.a11y.Bus", "/org/a11y/bus", "org.a11y.Status", QDBusConnection::sessionBus());
        if (interface.isValid())
        {
            SRActive = interface.property("ScreenReaderEnabled").toBool();
        }
#endif
        if (SRActive)
        {
            QMessageBox answer;
            answer.setText(tr("%1 has detected usage of a screenreader on your computer. Do you wish to enable accessibility options offered by %1 with recommended settings?").arg(APPNAME_SHORT));
            QAbstractButton* YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
            QAbstractButton* NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
            Q_UNUSED(NoButton);
            answer.setIcon(QMessageBox::Question);
            answer.setWindowTitle(APPNAME_SHORT);
            answer.exec();

            if (answer.clickedButton() == YesButton)
            {
#if defined(ENABLE_TOLK)
                ttSettings->setValue(SETTINGS_TTS_ENGINE, TTSENGINE_TOLK);
#elif defined(Q_OS_LINUX)
                ttSettings->setValue(SETTINGS_TTS_ENGINE, QFile::exists(TTSENGINE_NOTIFY_PATH) ? TTSENGINE_NOTIFY : TTSENGINE_QT);
#endif
                ttSettings->setValue(SETTINGS_DISPLAY_VU_METER_UPDATES, false);
            }
        }
        ttSettings->setValue(SETTINGS_GENERAL_FIRSTSTART, false);
    }
#endif
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
            QRegularExpression rx(QString("%1/+([^\\?\?!/]*)/?\\??").arg(TTLINK_PREFIX));
            QRegularExpressionMatch m = rx.match(args[i]);
            if (m.hasMatch())
            {
                HostEntry entry;
                entry.ipaddr = m.captured(1);
                entry.tcpport = DEFAULT_TCPPORT;
                entry.udpport = DEFAULT_UDPPORT;
                QString prop = args[i].mid(m.capturedLength());
                //&?tcpport=(\d+)&?
                rx.setPattern("&?tcpport=(\\d+)&?");
                m = rx.match(prop);
                if (m.hasMatch())
                    entry.tcpport = m.captured(1).toInt();
                //&?udpport=(\d+)&?
                rx.setPattern("&?udpport=(\\d+)&?");
                m = rx.match(prop);
                if (m.hasMatch())
                    entry.udpport = m.captured(1).toInt();
                //&?username=([^&]*)&?
                rx.setPattern("&?username=([^&]*)&?");
                m = rx.match(prop);
                if (m.hasMatch())
                    entry.username = QUrl::fromPercentEncoding(m.captured(1).toUtf8());
                //&?password=([^&]*)&?
                rx.setPattern("&?password=([^&]*)&?");
                m = rx.match(prop);
                if (m.hasMatch())
                    entry.password = QUrl::fromPercentEncoding(m.captured(1).toUtf8());
                //&?channel=([^&]*)&?
                rx.setPattern("&?channel=([^&]*)&?");
                m = rx.match(prop);
                if (m.hasMatch())
                    entry.channel = QUrl::fromPercentEncoding(m.captured(1).toUtf8());
                //&?chanpasswd=([^&]*)&?
                rx.setPattern("&?chanpasswd=([^&]*)&?");
                m = rx.match(prop);
                if (m.hasMatch())
                    entry.chanpasswd = QUrl::fromPercentEncoding(m.captured(1).toUtf8());
                //&?encrypted=([^&]*)&?
                rx.setPattern("&?encrypted=([^&]*)&?");
                m = rx.match(prop);
                if (m.hasMatch())
                    entry.encrypted = (m.captured(1).compare("true", Qt::CaseInsensitive) == 0 || m.captured(1) == "1");

                addLatestHost(entry);
                m_host = entry;
                disconnectFromServer();
                connectToServer();
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


void MainWindow::clienteventConSuccess()
{
    //disable reconnect timer
    killLocalTimer(TIMER_RECONNECT);

    //switch to connected icon in tray
    if(m_sysicon)
        m_sysicon->setIcon(QIcon(APPTRAYICON_CON));

    //reset stats
    m_clientstats = {};

    // retrieve initial welcome message and access token
    TT_GetServerProperties(ttInst, &m_srvprop);

    if (isWebLogin(m_host.username, true))
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
        connect(networkMgr, &QNetworkAccessManager::finished,
            this, &MainWindow::slotBearWareAuthReply);

        QNetworkRequest request(url);
        networkMgr->get(request);
    }
    else
    {
        login();
    }
    updateWindowTitle();
}

void MainWindow::clienteventConFailed()
{
    disconnectFromServer();

    killLocalTimer(TIMER_RECONNECT);
    if (ttSettings->value(SETTINGS_CONNECTION_RECONNECT, SETTINGS_CONNECTION_RECONNECT_DEFAULT).toBool())
    {
        m_timers[startTimer(5000)] = TIMER_RECONNECT;
    }

    addStatusMsg(STATUSBAR_BYPASS, tr("Failed to connect to %1 TCP port %2 UDP port %3")
                 .arg(m_host.ipaddr).arg(m_host.tcpport).arg(m_host.udpport));
}

void MainWindow::clienteventConCryptError(const TTMessage& msg)
{
    addStatusMsg(STATUSBAR_BYPASS, tr("Secure connection failed due to error 0x%1: %2.").arg(msg.nSource, 0, 16).arg(_Q(msg.clienterrormsg.szErrorMsg)));
    addTextToSpeechMessage(TTS_SERVER_CONNECTIVITY, tr("Secure connection failed due to error 0x%1: %2.").arg(msg.nSource, 0, 16).arg(_Q(msg.clienterrormsg.szErrorMsg)));
}

void MainWindow::clienteventConLost()
{
    disconnectFromServer();
    if(ttSettings->value(SETTINGS_CONNECTION_RECONNECT, SETTINGS_CONNECTION_RECONNECT_DEFAULT).toBool())
        m_timers[startTimer(5000)] = TIMER_RECONNECT;

    addStatusMsg(STATUSBAR_BYPASS, tr("Connection lost to %1 TCP port %2 UDP port %3")
                 .arg(m_host.ipaddr).arg(m_host.tcpport).arg(m_host.udpport));

    playSoundEvent(SOUNDEVENT_SERVERLOST);
    addTextToSpeechMessage(TTS_SERVER_CONNECTIVITY, tr("Connection lost to %1 TCP port %2 UDP port %3")
                 .arg(m_host.ipaddr).arg(m_host.tcpport).arg(m_host.udpport));
}

void MainWindow::clienteventMyselfKicked(const TTMessage& msg)
{
    Q_ASSERT(msg.ttType == __USER || msg.ttType == __NONE);
    if (msg.nSource == 0)
    {
        playSoundEvent(SOUNDEVENT_SERVERLOST);
        if(ttSettings->value(SETTINGS_DISPLAY_CHANEXCLUDE_DLG, SETTINGS_DISPLAY_CHANEXCLUDE_DLG_DEFAULT).toBool() == false)
        {
            if (msg.ttType == __USER)
                addStatusMsg(STATUSBAR_BYPASS, tr("Kicked from server by %1")
                    .arg(getDisplayName(msg.user)));
            else
                addStatusMsg(STATUSBAR_BYPASS, tr("Kicked from server by unknown user"));
        }
        else
        {
            if (msg.ttType == __USER)
                QMessageBox::information(this, tr("Kicked from server"),
                    QString(tr("You have been kicked from server by %1").arg(getDisplayName(msg.user))));
            else
                QMessageBox::information(this, tr("Kicked from server"),
                    tr("You have been kicked from server by unknown user"));
        }
    }
    else
    {
        if(ttSettings->value(SETTINGS_DISPLAY_CHANEXCLUDE_DLG, SETTINGS_DISPLAY_CHANEXCLUDE_DLG_DEFAULT).toBool() == false)
        {
            if (msg.ttType == __USER)
                addStatusMsg(STATUSBAR_BYPASS, tr("Kicked from channel by %1")
                    .arg(getDisplayName(msg.user)));
            else
                addStatusMsg(STATUSBAR_BYPASS, tr("Kicked from channel by unknown user"));
        }
        else
        {
            if (msg.ttType == __USER)
                QMessageBox::information(this, tr("Kicked from channel"),
                    QString(tr("You have been kicked from channel by %1").arg(getDisplayName(msg.user))));
            else
                QMessageBox::information(this, tr("Kicked from channel"),
                    tr("You have been kicked from channel by unknown user"));
        }
    }
}

void MainWindow::clienteventCmdProcessing(int cmdid, bool complete)
{
    //command reply starting -> store command reply ID
    if (!complete)
        m_current_cmdid = cmdid;

    //check if a dialog should be opened to show server reply
    cmdreply_t::iterator ite;
    if (complete && /* Command has completed */
        (ite = m_commands.find(cmdid)) != m_commands.end()) //Command we're interested in knowing the reply of
    {
        switch (*ite)
        {
        case CMD_COMPLETE_LOGIN:
            cmdCompleteLoggedIn(TT_GetMyUserID(ttInst));
            break;
        case CMD_COMPLETE_JOINCHANNEL:
            break;
        case CMD_COMPLETE_LIST_CHANNELBANS:
        case CMD_COMPLETE_LIST_SERVERBANS:
            cmdCompleteListServers(*ite);
        break;
        case CMD_COMPLETE_LISTACCOUNTS:
            cmdCompleteListUserAccounts();
        break;
        default:
            break;
        }
    }

    //command reply completed -> clear command reply ID
    if (complete)
    {
        m_commands.remove(cmdid);
        m_current_cmdid = 0;
    }
}

void MainWindow::clienteventCmdChannelUpdate(const Channel& channel)
{
    if(m_mychannel.nChannelID == (int)channel.nChannelID)
    {
        m_mychannel = channel;
        //update AGC, denoise, etc. if changed
        updateAudioConfig();
        updateWindowTitle();
    }
    emit(updateChannel(channel));
}

void MainWindow::clienteventCmdUserLoggedIn(const User& user)
{
    emit(userLogin(user));
    QString audiofolder = ttSettings->value(SETTINGS_MEDIASTORAGE_AUDIOFOLDER).toString();
    AudioFileFormat aff = (AudioFileFormat)ttSettings->value(SETTINGS_MEDIASTORAGE_FILEFORMAT, AFF_WAVE_FORMAT).toInt();
    if(m_audiostorage_mode & AUDIOSTORAGE_SEPARATEFILES)
        TT_SetUserMediaStorageDir(ttInst, user.nUserID, _W(audiofolder), nullptr, aff);

    updateUserSubscription(user.nUserID);
    if(m_commands[m_current_cmdid] != CMD_COMPLETE_LOGIN)
    {
        addStatusMsg(STATUSBAR_USER_LOGGEDIN, UtilUI::getStatusBarMessage(SETTINGS_STATUSBARMSG_USER_LOGGEDIN, {{"{user}", getDisplayName(user)}, {"{server}", limitText(_Q(m_srvprop.szServerName))}}));
        playSoundEvent(SOUNDEVENT_USERLOGGEDIN);
        addTextToSpeechMessage(TTS_USER_LOGGEDIN, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_USER_LOGGEDIN, {{"{user}", getDisplayName(user)}, {"{server}", limitText(_Q(m_srvprop.szServerName))}}));
    }

    // sync user settings from cache
    QString cacheid = userCacheID(user);
    if (m_usercache.find(cacheid) != m_usercache.end())
        m_usercache[cacheid].sync(ttInst, user);
}

void MainWindow::clienteventCmdUserLoggedOut(const User& user)
{
    emit(userLogout(user));
    //remove text-message history from this user
    m_textmessages.clearUserTextMessages(user.nUserID);
    if (user.nUserID != TT_GetMyUserID(ttInst))
    {
        addStatusMsg(STATUSBAR_USER_LOGGEDOUT, UtilUI::getStatusBarMessage(SETTINGS_STATUSBARMSG_USER_LOGGEDOUT, {{"{user}", getDisplayName(user)}, {"{server}", limitText(_Q(m_srvprop.szServerName))}}));
        playSoundEvent(SOUNDEVENT_USERLOGGEDOUT);
        addTextToSpeechMessage(TTS_USER_LOGGEDOUT, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_USER_LOGGEDOUT, {{"{user}", getDisplayName(user)}, {"{server}", limitText(_Q(m_srvprop.szServerName))}}));
    }

    // sync user settings to cache
    QString cacheid = userCacheID(user);
    if (!cacheid.isEmpty())
        m_usercache[cacheid] = UserCached(user);
}

void MainWindow::clienteventCmdUserJoined(const User& user)
{
    if (user.nUserID == TT_GetMyUserID(ttInst))
        processMyselfJoined(user.nChannelID);

    emit (userJoined(user.nChannelID, user));

    if (m_commands[m_current_cmdid] != CMD_COMPLETE_LOGIN &&
        user.nUserID != TT_GetMyUserID(ttInst))
    {
        Channel chan = {};
        ui.channelsWidget->getChannel(user.nChannelID, chan);
        QString userjoinchanStatus = UtilUI::getStatusBarMessage(SETTINGS_STATUSBARMSG_USER_JOINED_SAME, {{"{user}", getDisplayName(user)}});
        QString userjoinchanTTS = UtilTTS::getTTSMessage(SETTINGS_TTSMSG_USER_JOINED_SAME, {{"{user}", getDisplayName(user)}});
        TextToSpeechEvent ttsType = TTS_USER_JOINED_SAME;
        StatusBarEvent statusType = STATUSBAR_USER_JOINED_SAME;
        if ((chan.nParentID == 0 && user.nChannelID != m_mychannel.nChannelID) || (user.nChannelID != m_mychannel.nChannelID))
        {
            userjoinchanStatus = UtilUI::getStatusBarMessage(SETTINGS_STATUSBARMSG_USER_JOINED, {{"{user}", getDisplayName(user)}, {"{channel}", (chan.nParentID == 0 && user.nChannelID != m_mychannel.nChannelID)?tr("root"):_Q(chan.szName)}});
            userjoinchanTTS = UtilTTS::getTTSMessage(SETTINGS_TTSMSG_USER_JOINED, {{"{user}", getDisplayName(user)}, {"{channel}", (chan.nParentID == 0 && user.nChannelID != m_mychannel.nChannelID)?tr("root"):_Q(chan.szName)}});
            ttsType = TTS_USER_JOINED;
            statusType = STATUSBAR_USER_JOINED;
        }
        addStatusMsg(statusType, userjoinchanStatus);
        addTextToSpeechMessage(ttsType, userjoinchanTTS);
    }

    // sync user settings from cache
    if ((m_myuseraccount.uUserRights & USERRIGHT_VIEW_ALL_USERS) == USERRIGHT_NONE)
    {
        QString cacheid = userCacheID(user);
        if (m_usercache.find(cacheid) != m_usercache.end())
            m_usercache[cacheid].sync(ttInst, user);
    }
}

void MainWindow::clienteventCmdUserLeft(int prevchannelid, const User& user)
{
    if (user.nUserID == TT_GetMyUserID(ttInst))
        processMyselfLeft(prevchannelid);

    emit (userLeft(prevchannelid, user));

    if (user.nUserID == m_relayvoice_userid || user.nUserID == m_relayvoice_userid)
        relayAudioStream(user.nUserID, STREAMTYPE_NONE, false);

    if (m_commands[m_current_cmdid] != CMD_COMPLETE_JOINCHANNEL &&
        user.nUserID != TT_GetMyUserID(ttInst))
    {
        Channel chan = {};
        ui.channelsWidget->getChannel(prevchannelid, chan);
        QString userleftchanStatus = UtilUI::getStatusBarMessage(SETTINGS_STATUSBARMSG_USER_LEFT_SAME, {{"{user}", getDisplayName(user)}});
        QString userleftchanTTS = UtilTTS::getTTSMessage(SETTINGS_TTSMSG_USER_LEFT_SAME, {{"{user}", getDisplayName(user)}});
        TextToSpeechEvent ttsType = TTS_USER_LEFT_SAME;
        StatusBarEvent statusType = STATUSBAR_USER_LEFT_SAME;
        if ((chan.nParentID == 0 && prevchannelid != m_mychannel.nChannelID) || (prevchannelid != m_mychannel.nChannelID))
        {
            userleftchanStatus = UtilUI::getStatusBarMessage(SETTINGS_STATUSBARMSG_USER_LEFT, {{"{user}", getDisplayName(user)}, {"{channel}", (chan.nParentID == 0 && prevchannelid != m_mychannel.nChannelID)?tr("root"):_Q(chan.szName)}});
            userleftchanTTS = UtilTTS::getTTSMessage(SETTINGS_TTSMSG_USER_LEFT, {{"{user}", getDisplayName(user)}, {"{channel}", (chan.nParentID == 0 && prevchannelid != m_mychannel.nChannelID)?tr("root"):_Q(chan.szName)}});
            ttsType = TTS_USER_LEFT;
            statusType = STATUSBAR_USER_LEFT;
        }
        addStatusMsg(statusType, userleftchanStatus);
        addTextToSpeechMessage(ttsType, userleftchanTTS);
    }

    if ((m_myuseraccount.uUserRights & USERRIGHT_VIEW_ALL_USERS) == USERRIGHT_NONE)
    {
        // sync user settings to cache
        QString cacheid = userCacheID(user);
        if (!cacheid.isEmpty())
            m_usercache[cacheid] = UserCached(user);
    }
}

void MainWindow::clienteventCmdUserUpdate(const User& user)
{
    User prev_user = {};
    ui.channelsWidget->getUser(user.nUserID, prev_user);
    Q_ASSERT(prev_user.nUserID);

    emit(userUpdate(user));

    if (user.nUserID != TT_GetMyUserID(ttInst) && user.nChannelID == m_mychannel.nChannelID)
    {
        if ((prev_user.nStatusMode & STATUSMODE_QUESTION) == 0 && (user.nStatusMode & STATUSMODE_QUESTION))
        {
           playSoundEvent(SOUNDEVENT_QUESTIONMODE);
           addTextToSpeechMessage(TTS_USER_QUESTIONMODE, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_QUESTIONMODE, {{"{user}", getDisplayName(user)}, {"{server}", limitText(_Q(m_srvprop.szServerName))}}));
        }
    }

    //update desktop access button
    if ((prev_user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT) != (user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT))
       slotUpdateDesktopTabUI();
}

void MainWindow::clienteventCmdFileNew(const RemoteFile& file)
{
    //only update files list if we're not currently logging in or
    //joining a channel
    cmdreply_t::iterator ite = m_commands.find(m_current_cmdid);
    if(m_filesmodel->getChannelID() == file.nChannelID &&
       (ite == m_commands.end() || (*ite != CMD_COMPLETE_LOGIN &&
                                    *ite != CMD_COMPLETE_JOINCHANNEL)) )
    {
        updateChannelFiles(file.nChannelID);
        playSoundEvent(SOUNDEVENT_FILESUPD);
        User user;
        TT_GetUserByUsername(ttInst, file.szUsername, &user);
        QString name = m_host.username != _Q(file.szUsername)?getDisplayName(user):tr("You");
        addStatusMsg(STATUSBAR_FILE_ADD, UtilUI::getStatusBarMessage(SETTINGS_STATUSBARMSG_FILE_ADDED, {{"{filename}", _Q(file.szFileName)}, {"{user}", name}, {"{filesize}", getFormattedFileSize(file.nFileSize)}}));
        addTextToSpeechMessage(TTS_FILE_ADD, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_FILE_ADDED, {{"{filename}", _Q(file.szFileName)}, {"{user}", name}, {"{filesize}", getFormattedFileSize(file.nFileSize)}}));
    }
}

void MainWindow::clienteventCmdFileRemove(const RemoteFile& file)
{
    User user;
    //only update files list if we're not currently logging in or
    //joining a channel
    cmdreply_t::iterator ite = m_commands.find(m_current_cmdid);
    if(m_filesmodel->getChannelID() == file.nChannelID &&
       (ite == m_commands.end() || (*ite != CMD_COMPLETE_LOGIN &&
                                    *ite != CMD_COMPLETE_JOINCHANNEL)) )
    {
        updateChannelFiles(file.nChannelID);
        playSoundEvent(SOUNDEVENT_FILESUPD);
        TT_GetUserByUsername(ttInst, file.szUsername, &user);
        QString name = m_host.username != _Q(file.szUsername)?getDisplayName(user):tr("You");
        addStatusMsg(STATUSBAR_FILE_REMOVE, UtilUI::getStatusBarMessage(SETTINGS_STATUSBARMSG_FILE_REMOVED, {{"{file}", _Q(file.szFileName)}, {"{user}", name}}));
        addTextToSpeechMessage(TTS_FILE_REMOVE, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_FILE_REMOVED, {{"{file}", _Q(file.szFileName)}, {"{user}", name}}));
    }
}

void MainWindow::clienteventFileTransfer(const FileTransfer& filetransfer)
{
    emit(filetransferUpdate(filetransfer));

    if (filetransfer.nStatus == FILETRANSFER_ACTIVE && filetransfer.nTransferred == 0)
    {
        FileTransferDlg* dlg = new FileTransferDlg(filetransfer, nullptr);
        connect(this, &MainWindow::filetransferUpdate, dlg, &FileTransferDlg::slotTransferUpdate);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();
    }
    else if (filetransfer.nStatus == FILETRANSFER_ERROR && filetransfer.nTransferred == 0)
    {
        if (filetransfer.bInbound)
            QMessageBox::critical(this, MENUTEXT(ui.actionDownloadFile->text()),
                                  tr("Failed to download file %1")
                                  .arg(_Q(filetransfer.szRemoteFileName)));
        else
            QMessageBox::critical(this, MENUTEXT(ui.actionUploadFile->text()),
                                  tr("Failed to upload file %1")
                                  .arg(_Q(filetransfer.szLocalFilePath)));
    }
}

void MainWindow::clienteventInternalError(const ClientErrorMsg& clienterrormsg)
{
    bool critical = true;
    QString textmsg;
    switch (clienterrormsg.nErrorNo)
    {
        /***** Internal TeamTalk errors not related to commands ********/
    case INTERR_SNDINPUT_FAILURE :
        textmsg = tr("Failed to initialize sound input device"); break;
    case INTERR_SNDOUTPUT_FAILURE :
        textmsg = tr("Failed to initialize sound output device"); break;
    case INTERR_AUDIOCODEC_INIT_FAILED :
        textmsg = tr("Failed to initialize audio codec"); break;
    case INTERR_AUDIOPREPROCESSOR_INIT_FAILED :
        textmsg = tr("Audio preprocessor failed to initialize"); break;
    case INTERR_SNDEFFECT_FAILURE :
        textmsg = tr("An audio effect could not be applied on the sound device"); break;
    case INTERR_TTMESSAGE_QUEUE_OVERFLOW :
        critical = false;
        textmsg = tr("Internal message queue overloaded"); break;
    default :
        textmsg = _Q(clienterrormsg.szErrorMsg);
        break;
    }
    if (critical)
        QMessageBox::critical(this, tr("Internal Error"), textmsg);
    addStatusMsg(STATUSBAR_BYPASS, textmsg);
}

void MainWindow::clienteventUserStateChange(const User& user)
{
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

    if(user.uUserState & USERSTATE_MEDIAFILE_AUDIO && user.nUserID != TT_GetMyUserID(ttInst))
    {
        User nameuser;
        TT_GetUser(ttInst, user.nUserID, &nameuser);
        addStatusMsg(STATUSBAR_BYPASS, tr("Streaming from %1 started") .arg(getDisplayName(nameuser)));
    }

    if(m_talking.empty() && TT_GetMyChannelID(ttInst) > 0)
        playSoundEvent(SOUNDEVENT_SILENCE);
}

void MainWindow::clienteventVoiceActivation(bool active)
{
    playSoundEvent(active? SOUNDEVENT_VOICEACTTRIG :  SOUNDEVENT_VOICEACTSTOP);
    emit(updateMyself());
    if (active)
    {
        transmitOn(STREAMTYPE_VOICE);
    }
}

void MainWindow::clienteventStreamMediaFile(const MediaFileInfo& mediafileinfo)
{
    switch (mediafileinfo.nStatus)
    {
    case MFS_ERROR :
        addStatusMsg(STATUSBAR_BYPASS, tr("Error streaming media file to channel"));
        stopStreamMediaFile();
        break;
    case MFS_STARTED :
        addStatusMsg(STATUSBAR_BYPASS, tr("Started streaming media file to channel"));
        break;
    case MFS_FINISHED :
        addStatusMsg(STATUSBAR_BYPASS, tr("Finished streaming media file to channel"));
        stopStreamMediaFile();
        break;
    case MFS_ABORTED :
        addStatusMsg(STATUSBAR_BYPASS, tr("Aborted streaming media file to channel"));
        stopStreamMediaFile();
        break;
    case MFS_CLOSED :
    case MFS_PAUSED :
    case MFS_PLAYING :
        break;
    }

    if (mediafileinfo.nStatus == MFS_FINISHED &&
       ttSettings->value(SETTINGS_STREAMMEDIA_LOOP, false).toBool())
    {
        startStreamMediaFile();
    }

    emit(mediaStreamUpdate(mediafileinfo));

    //update if still talking
    emit(updateMyself());

    //update media tab
    updateMediaFileProgress(mediafileinfo);
}

void MainWindow::clienteventUserVideoCapture(int source, int streamid)
{
    int userid = source | VIDEOTYPE_CAPTURE;

    //if local video is disabled then don't process pending video
    //frames
    if (source == 0 && (TT_GetFlags(ttInst) & CLIENT_VIDEOCAPTURE_READY) == 0)
        return;

    //pass new video frame (if it's not being ignored)
    if(m_vid_exclude.find(userid) != m_vid_exclude.end())
        return;

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
            addStatusMsg(STATUSBAR_BYPASS, tr("New video session from %1")
                         .arg(getDisplayName(user)));
    }
    emit(newVideoCaptureFrame(userid, streamid));
}

void MainWindow::clienteventUserMediaFileVideo(int source, int streamid)
{
    int userid = source | VIDEOTYPE_MEDIAFILE;

   //pass new video frame (if it's not being ignored)
    if(m_vid_exclude.find(userid) != m_vid_exclude.end())
        return;

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
            addStatusMsg(STATUSBAR_BYPASS, tr("New video session from %1")
            .arg(getDisplayName(user)));
    }
    emit(newMediaVideoFrame(userid, streamid));
}

void MainWindow::clienteventUserDesktopWindow(int source, int streamid)
{
    if (m_userdesktop.find(source) == m_userdesktop.end() &&
       !ui.desktopgridWidget->userExists(source))
    {
        //it's a new desktop session

        //sesion id == 0 means it has ended
        if (streamid)
        {
            if(ttSettings->value(SETTINGS_DISPLAY_DESKTOPPOPUP, false).toBool())
                slotDetachUserDesktop(source, QSize());
            else
                ui.desktopgridWidget->slotAddUser(source);

            playSoundEvent(SOUNDEVENT_NEWDESKTOP);

            User user;
            if(ui.channelsWidget->getUser(source, user))
                addStatusMsg(STATUSBAR_BYPASS, tr("New desktop session from %1")
                .arg(getDisplayName(user)));
        }
    }
    emit(newDesktopWindow(source, streamid));
}

void MainWindow::clienteventDesktopWindowTransfer(int source, int bytesremain)
{
    if (m_desktopsession_id != source) //new session
        m_desktopsession_remain = 0;
    m_desktopsession_id = source;

    if(m_desktopsession_remain == 0)
        m_desktopsession_total = bytesremain;
    m_desktopsession_remain = bytesremain;

    if (bytesremain == 0 && m_desktopsend_on_completion)
    {
        if(sendDesktopWindow())
        {
            restartSendDesktopWindowTimer();
            m_desktopsend_on_completion = false;
        }
    }

    if (source == 0)
        addStatusMsg(STATUSBAR_BYPASS, tr("Your desktop session was cancelled"));
}

void MainWindow::clienteventUserRecordMediaFile(int source, const MediaFileInfo& mediafileinfo)
{
    User user = {};
    ui.channelsWidget->getUser(source, user);

    switch (mediafileinfo.nStatus)
    {
    case MFS_STARTED :
        addStatusMsg(STATUSBAR_BYPASS, tr("Writing audio file %1 for %2")
                     .arg(_Q(mediafileinfo.szFileName))
                     .arg(getDisplayName(user)));
        break;
    case MFS_ERROR :
        addStatusMsg(STATUSBAR_BYPASS, tr("Failed to write audio file %1 for %2")
                     .arg(_Q(mediafileinfo.szFileName))
                     .arg(getDisplayName(user)));
        break;
    case MFS_FINISHED :
        addStatusMsg(STATUSBAR_BYPASS, tr("Finished writing to audio file %1")
                     .arg(_Q(mediafileinfo.szFileName)));
        break;
    case MFS_ABORTED :
        addStatusMsg(STATUSBAR_BYPASS, tr("Aborted audio file %1")
                     .arg(_Q(mediafileinfo.szFileName)));
        break;
    case MFS_CLOSED :
    case MFS_PLAYING :
    case MFS_PAUSED :
        break;
    }
}

void MainWindow::clienteventUserAudioBlock(int source, StreamTypes streamtypes)
{
    AudioBlock* block = TT_AcquireUserAudioBlock(ttInst, streamtypes, source);
    if (block)
    {
        if (m_relayvoice_userid == source || m_relaymediafile_userid == source)
        {
            if (!TT_InsertAudioBlock(ttInst, block))
            {
                qDebug() << "Failed to insert audioblock";
            }
        }
        TT_ReleaseUserAudioBlock(ttInst, block);
    }
}

void MainWindow::clienteventSoundDeviceAdded(const SoundDevice& snddev)
{
    addStatusMsg(STATUSBAR_BYPASS, tr("New sound device available: %1. Refresh sound devices to discover new device.").arg(_Q(snddev.szDeviceName)));
}

void MainWindow::clienteventSoundDeviceRemoved(const SoundDevice& snddev)
{
    addStatusMsg(STATUSBAR_BYPASS, tr("Sound device removed: %1.").arg(_Q(snddev.szDeviceName)));

    auto devid = _Q(snddev.szDeviceID);
    if (devid.size() && (devid == _Q(m_devin.szDeviceID) || devid == _Q(m_devout.szDeviceID)))
    {
        initSound();
    }
}

void MainWindow::processTTMessage(const TTMessage& msg)
{
    switch (msg.nClientEvent)
    {
    case CLIENTEVENT_CON_SUCCESS :
        clienteventConSuccess();
    break;
    case CLIENTEVENT_CON_FAILED :
        clienteventConFailed();
    break;
    case CLIENTEVENT_CON_CRYPT_ERROR :
        clienteventConCryptError(msg);
    break;
    case CLIENTEVENT_CON_LOST :
        clienteventConLost();
    break;
    case CLIENTEVENT_CON_MAX_PAYLOAD_UPDATED :
        qDebug() << "User #" << msg.nSource << "max payload is" << msg.nPayloadSize;
    break;
    case CLIENTEVENT_CMD_PROCESSING :
        emit cmdProcessing(msg.nSource, msg.bActive);
        clienteventCmdProcessing(msg.nSource, !msg.bActive);
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
    }
    break;
    case CLIENTEVENT_CMD_SUCCESS :
        emit(cmdSuccess(msg.nSource));
    break;
    case CLIENTEVENT_CMD_MYSELF_LOGGEDIN :
        //ui.chatEdit->updateServer();
        //store user account settings
        m_myuseraccount = msg.useraccount;
        break;
    case CLIENTEVENT_CMD_MYSELF_LOGGEDOUT :
        disconnectFromServer();
        break;
    case CLIENTEVENT_CMD_MYSELF_KICKED :
        clienteventMyselfKicked(msg);
    break;
    case CLIENTEVENT_CMD_SERVER_UPDATE :
        Q_ASSERT(msg.ttType == __SERVERPROPERTIES);
        ui.chatEdit->updateServer(msg.serverproperties);
        emit(serverUpdate(msg.serverproperties));
        m_srvprop = msg.serverproperties;
        updateWindowTitle();
    break;
    case CLIENTEVENT_CMD_SERVERSTATISTICS :
        Q_ASSERT(msg.ttType == __SERVERSTATISTICS);
        emit(serverStatistics(msg.serverstatistics));
    break;
    case CLIENTEVENT_CMD_CHANNEL_NEW :
        Q_ASSERT(msg.ttType == __CHANNEL);
        emit(newChannel(msg.channel));
        break;
    case CLIENTEVENT_CMD_CHANNEL_UPDATE :
        Q_ASSERT(msg.ttType == __CHANNEL);
        clienteventCmdChannelUpdate(msg.channel);
    break;
    case CLIENTEVENT_CMD_CHANNEL_REMOVE :
        Q_ASSERT(msg.ttType == __CHANNEL);
        emit(removeChannel(msg.channel));
        break;
    case CLIENTEVENT_CMD_USER_LOGGEDIN :
        Q_ASSERT(msg.ttType == __USER);
        clienteventCmdUserLoggedIn(msg.user);
    break;
    case CLIENTEVENT_CMD_USER_LOGGEDOUT :
        Q_ASSERT(msg.ttType == __USER);
        clienteventCmdUserLoggedOut(msg.user);
    break;
    case CLIENTEVENT_CMD_USER_JOINED :
        Q_ASSERT(msg.ttType == __USER);
        clienteventCmdUserJoined(msg.user);
        break;
    case CLIENTEVENT_CMD_USER_LEFT :
        Q_ASSERT(msg.ttType == __USER);
        clienteventCmdUserLeft(msg.nSource, msg.user);
        break;
    case CLIENTEVENT_CMD_USER_UPDATE :
        Q_ASSERT(msg.ttType == __USER);
        clienteventCmdUserUpdate(msg.user);
    break;
    case CLIENTEVENT_CMD_USER_TEXTMSG :
    {
        Q_ASSERT(msg.ttType == __TEXTMESSAGE);
        MyTextMessage mymsg = m_textmessages.addTextMessage(msg.textmessage);
        if (mymsg.nMsgType != MSGTYPE_NONE)
            processTextMessage(mymsg);
        break;
    }
    case CLIENTEVENT_CMD_FILE_NEW :
        Q_ASSERT(msg.ttType == __REMOTEFILE);
        clienteventCmdFileNew(msg.remotefile);
    break;
    case CLIENTEVENT_CMD_FILE_REMOVE :
        Q_ASSERT(msg.ttType == __REMOTEFILE);
        clienteventCmdFileRemove(msg.remotefile);
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
        clienteventFileTransfer(msg.filetransfer);
        break;
    case CLIENTEVENT_INTERNAL_ERROR :
        Q_ASSERT(msg.ttType == __CLIENTERRORMSG);
        clienteventInternalError(msg.clienterrormsg);
    break;
    case CLIENTEVENT_USER_STATECHANGE :
        Q_ASSERT(msg.ttType == __USER);
        clienteventUserStateChange(msg.user);
    break;
    case CLIENTEVENT_VOICE_ACTIVATION :
        Q_ASSERT(msg.ttType == __TTBOOL);
        clienteventVoiceActivation(msg.bActive);
        break;
    case CLIENTEVENT_STREAM_MEDIAFILE :
        Q_ASSERT(msg.ttType == __MEDIAFILEINFO);
        clienteventStreamMediaFile(msg.mediafileinfo);
    break;
    case CLIENTEVENT_LOCAL_MEDIAFILE:
        emit(mediaPlaybackUpdate(msg.nSource, msg.mediafileinfo));
        break;
    case CLIENTEVENT_USER_VIDEOCAPTURE :
        Q_ASSERT(msg.ttType == __INT32);
        //local video is userID = 0 (msg.nSource)
        clienteventUserVideoCapture(msg.nSource, msg.nStreamID);
    break;
    case CLIENTEVENT_USER_MEDIAFILE_VIDEO :
        Q_ASSERT(msg.ttType == __INT32);
        clienteventUserMediaFileVideo(msg.nSource, msg.nStreamID);
    break;
    case CLIENTEVENT_USER_DESKTOPWINDOW :
        Q_ASSERT(msg.ttType == __INT32);
        clienteventUserDesktopWindow(msg.nSource, msg.nStreamID);
        break;
    case CLIENTEVENT_DESKTOPWINDOW_TRANSFER :
        Q_ASSERT(msg.ttType == __INT32);
        clienteventDesktopWindowTransfer(msg.nSource, msg.nBytesRemain);
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
        Q_ASSERT(msg.ttType == __MEDIAFILEINFO);
        clienteventUserRecordMediaFile(msg.nSource, msg.mediafileinfo);
    break;
    case CLIENTEVENT_USER_AUDIOBLOCK :
        clienteventUserAudioBlock(msg.nSource, msg.nStreamType);
    break;
    case CLIENTEVENT_AUDIOINPUT :
        qDebug() << "Active audio input #" << msg.nSource << " " << msg.audioinputprogress.uElapsedMSec << " msec. Queue: " << msg.audioinputprogress.uQueueMSec;
        break;
    case CLIENTEVENT_HOTKEY :
        Q_ASSERT(msg.ttType == __TTBOOL);
        hotkeyToggle((HotKeyID)msg.nSource, (bool)msg.bActive);
        break;
    case CLIENTEVENT_SOUNDDEVICE_ADDED :
        Q_ASSERT(msg.ttType == __SOUNDDEVICE);
        clienteventSoundDeviceAdded(msg.sounddevice);
        break;
    case CLIENTEVENT_SOUNDDEVICE_REMOVED :
        Q_ASSERT(msg.ttType == __SOUNDDEVICE);
        clienteventSoundDeviceRemoved(msg.sounddevice);
        break;
    case CLIENTEVENT_SOUNDDEVICE_UNPLUGGED:
        qDebug() << "Unplugged sound device: " << _Q(msg.sounddevice.szDeviceName);
        break;
    case CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_INPUT:
        qDebug() << "New default sound input device: " << _Q(msg.sounddevice.szDeviceName);
        break;
    case CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_OUTPUT:
        qDebug() << "New default sound output device: " << _Q(msg.sounddevice.szDeviceName);
        break;
    case CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_INPUT_COMDEVICE:
        qDebug() << "New default communication input sound device: " << _Q(msg.sounddevice.szDeviceName);
        break;
    case CLIENTEVENT_SOUNDDEVICE_NEW_DEFAULT_OUTPUT_COMDEVICE:
        qDebug() << "New default communication output sound device: " << _Q(msg.sounddevice.szDeviceName);
        break;
    default :
        qDebug() << "Unknown message type" << msg.nClientEvent;
    }

    // don't update UI when we're in the process of completing a
    // command. Otherwise the UI will get a bunch of updates. E.g. on
    // initial login. See #CLIENTEVENT_CMD_PROCESSING.
    if (m_current_cmdid == 0)
        slotUpdateUI();
}

void MainWindow::cmdCompleteLoggedIn(int myuserid)
{
    Q_UNUSED(myuserid);

    //login command completed

    addStatusMsg(STATUSBAR_BYPASS, tr("Connected to %1").arg(limitText(_Q(m_srvprop.szServerName))));
    addTextToSpeechMessage(TTS_SERVER_CONNECTIVITY, tr("Connected to %1").arg(limitText(_Q(m_srvprop.szServerName))));

    QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();
    m_statusmode &= ~STATUSMODE_GENDER_MASK;
    switch (Gender(ttSettings->value(SETTINGS_GENERAL_GENDER, SETTINGS_GENERAL_GENDER_DEFAULT).toInt()))
    {
    case GENDER_MALE :
        m_statusmode |= STATUSMODE_MALE;
        break;
    case GENDER_FEMALE :
        m_statusmode |= STATUSMODE_FEMALE;
        break;
    case GENDER_NEUTRAL :
    default:
        m_statusmode |= STATUSMODE_NEUTRAL;
        break;
    }

    //set status mode flags
    if(m_statusmode || statusmsg.size())
        TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));

    UserAccount account = {};
    TT_GetMyUserAccount(ttInst, &account);

    //join channel (if specified)
    Channel tmpchan;
    int channelid = 0;
    QString channelpath = _Q(account.szInitChannel).isEmpty() ? m_host.channel : _Q(account.szInitChannel);
    if (channelpath.size())
        channelid = TT_GetChannelIDFromPath(ttInst, _W(channelpath));

    //see if parent channel exists (otherwise we cannot create it)
    int parentid = 0;
    QStringList subchannels = channelpath.split('/');
    subchannels.removeAll(""); //remove blanks caused by beginning '/' and ending '/'
    if (subchannels.size())
    {
        QStringList parent = subchannels;
        parent.erase(parent.end()-1);
        QString parentpath = parent.join("/");
        parentid = TT_GetChannelIDFromPath(ttInst, _W(parentpath));
    }

    if (m_last_channel.nChannelID && //join using last channel
        (ui.channelsWidget->getChannel(m_last_channel.nChannelID, tmpchan) ||
         ui.channelsWidget->getChannel(m_last_channel.nParentID, tmpchan)))
    {
        COPY_TTSTR(m_last_channel.szPassword, m_channel_passwd[m_last_channel.nChannelID]);
        int cmdid = TT_DoJoinChannel(ttInst, &m_last_channel);
        if(cmdid>0)
            m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
    }
    else if (channelpath.size() && channelid > 0) //join if channel exists
    {
        int cmdid = TT_DoJoinChannelByID(ttInst, channelid, _W(m_host.chanpasswd));
        if(cmdid>0)
            m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
    }
    else if (channelpath.size() && parentid > 0) //make a new channel if parent exists
    {
        QString name;
        if(subchannels.size())
            name = subchannels.last();

        Channel chan = {};
        chan.nParentID = parentid;
        chan.nMaxUsers = m_srvprop.nMaxUsers;
        initDefaultAudioCodec(chan.audiocodec);

        chan.audiocfg.bEnableAGC = DEFAULT_CHANNEL_AUDIOCONFIG_ENABLE;
        chan.audiocfg.nGainLevel = DEFAULT_CHANNEL_AUDIOCONFIG_LEVEL;

        COPY_TTSTR(chan.szName, name);
        COPY_TTSTR(chan.szPassword, m_host.chanpasswd);
        int cmdid = TT_DoJoinChannel(ttInst, &chan);
        if(cmdid>0)
            m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
    }
    else if(ttSettings->value(SETTINGS_CONNECTION_AUTOJOIN, SETTINGS_CONNECTION_AUTOJOIN_DEFAULT).toBool()) //just join root
    {
        if (channelpath.size())
            addStatusMsg(STATUSBAR_BYPASS, tr("Cannot join channel %1").arg(channelpath));

        //auto join root channel
        int cmdid = TT_DoJoinChannelByID(ttInst, TT_GetRootChannelID(ttInst), _W(QString("")));
        if (cmdid > 0)
            m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
    }
}

void MainWindow::cmdCompleteListServers(CommandComplete complete)
{
    if (!m_bannedusersdlg)
    {
        QString chanpath;
        if (complete == CMD_COMPLETE_LIST_CHANNELBANS)
        {
            int chanid = ui.channelsWidget->selectedChannel(true);
            TTCHAR path[TT_STRLEN] = {};
            TT_GetChannelPath(ttInst, chanid, path);
            chanpath = _Q(path);
        }
        m_bannedusersdlg = new BannedUsersDlg(m_bannedusers, chanpath);
        connect(this, &MainWindow::cmdProcessing, m_bannedusersdlg, &BannedUsersDlg::cmdProcessing);
        if (chanpath.size())
            m_bannedusersdlg->setWindowTitle(tr("Banned Users in Channel %1").arg(chanpath));
        connect(m_bannedusersdlg, &QDialog::finished,
            this, &MainWindow::slotClosedBannedUsersDlg);
        m_bannedusersdlg->setAttribute(Qt::WA_DeleteOnClose);
        m_bannedusersdlg->show();
        m_bannedusers.clear();
    }
    else
        m_bannedusersdlg->activateWindow();
}

void MainWindow::cmdCompleteListUserAccounts()
{
    if (!m_useraccountsdlg)
    {
        m_useraccountsdlg = new UserAccountsDlg(m_useraccounts);
        connect(this, &MainWindow::cmdSuccess, m_useraccountsdlg,
            &UserAccountsDlg::slotCmdSuccess);
        connect(this, &MainWindow::cmdError, m_useraccountsdlg,
            &UserAccountsDlg::slotCmdError);
        connect(m_useraccountsdlg, &QDialog::finished,
            this, &MainWindow::slotClosedUserAccountsDlg);
        m_useraccountsdlg->setAttribute(Qt::WA_DeleteOnClose);
        m_useraccountsdlg->show();
        m_useraccounts.clear();
    }
    else
        m_useraccountsdlg->activateWindow();
}

void MainWindow::addStatusMsg(StatusBarEvent event, const QString& msg)
{
    if (ttSettings->value(SETTINGS_DISPLAY_LOGSTATUSBAR, SETTINGS_DISPLAY_LOGSTATUSBAR_DEFAULT).toBool() &&
        ((ttSettings->value(SETTINGS_STATUSBAR_ACTIVEEVENTS, SETTINGS_STATUSBAR_ACTIVEEVENTS_DEFAULT).toULongLong() & event) || event == STATUSBAR_BYPASS))
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

void MainWindow::initSound()
{
    QStringList errors = initSelectedSoundDevices(m_devin, m_devout);
    for (auto s : errors)
        addStatusMsg(STATUSBAR_BYPASS, s);

    //choose default sound devices if configuration failed
    if (errors.size())
    {
        errors = initDefaultSoundDevices(m_devin, m_devout);
        for (auto s : errors)
            addStatusMsg(STATUSBAR_BYPASS, s);
    }
    QString soundev = tr("Using sound input: %1").arg(_Q(m_devin.szDeviceName)) + "\r\n" + tr("Using sound output: %2").arg(_Q(m_devout.szDeviceName));
    addStatusMsg(STATUSBAR_BYPASS, soundev);
}

void MainWindow::connectToServer()
{
    Q_ASSERT((TT_GetFlags(ttInst) & CLIENT_CONNECTION) == 0);

    if (!setupEncryption(m_host))
    {
        addStatusMsg(STATUSBAR_BYPASS, tr("Failed to setup encryption settings"));
        return;
    }

    int localtcpport = ttSettings->value(SETTINGS_CONNECTION_TCPPORT, 0).toInt();
    int localudpport = ttSettings->value(SETTINGS_CONNECTION_UDPPORT, 0).toInt();

    addStatusMsg(STATUSBAR_BYPASS, tr("Connecting to %1 TCP port %2 UDP port %3")
                 .arg(m_host.ipaddr).arg(m_host.tcpport).arg(m_host.udpport));

    m_desktopaccess_entries.clear();
    getDesktopAccessList(m_desktopaccess_entries, m_host.ipaddr, m_host.tcpport);

    if(!TT_Connect(ttInst, _W(m_host.ipaddr), m_host.tcpport,
                   m_host.udpport, localtcpport, localudpport, m_host.encrypted))
        addStatusMsg(STATUSBAR_BYPASS, tr("Failed to connect to %1 TCP port %2 UDP port %3")
                     .arg(m_host.ipaddr).arg(m_host.tcpport).arg(m_host.udpport));
}

void MainWindow::disconnectFromServer()
{
    if (!timerExists(TIMER_RECONNECT))
    {
        addStatusMsg(STATUSBAR_BYPASS, (TT_GetFlags(ttInst) & CLIENT_AUTHORIZED?tr("Disconnected from %1").arg(limitText(_Q(m_srvprop.szServerName))):tr("Disconnected from server")));
        addTextToSpeechMessage(TTS_SERVER_CONNECTIVITY, (TT_GetFlags(ttInst) & CLIENT_AUTHORIZED?tr("Disconnected from %1").arg(limitText(_Q(m_srvprop.szServerName))):tr("Disconnected from server")));
    }
    
    if (m_host.latesthost == false && m_host.lastChan == true)
    {
        if (m_mychannel.nChannelID > 0)
        {
            TTCHAR cpath[TT_STRLEN];
            if (TT_GetChannelPath(ttInst, m_mychannel.nChannelID, cpath))
            {
                m_host.channel = _Q(cpath);
                m_host.chanpasswd = _Q(m_mychannel.szPassword);
            }
        }
        else
        {
            m_host.channel = "";
            m_host.chanpasswd = "";
        }
        deleteServerEntry(m_host);
        addServerEntry(m_host);
    }

    TT_Disconnect(ttInst);

    // sync user settings to cache
    auto users = ui.channelsWidget->getUsers();
    for (int uid : users)
    {
        User u;
        if (ui.channelsWidget->getUser(uid, u) && !userCacheID(u).isEmpty())
            m_usercache[userCacheID(u)] = UserCached(u);
    }

    ui.channelsWidget->resetChannels();
    ui.videogridWidget->resetGrid();
    ui.desktopgridWidget->resetGrid();
    ui.msgEdit->setVisible(false);
    ui.sendButton->setVisible(false);
    ui.videomsgEdit->setVisible(false);
    ui.videosendButton->setVisible(false);
    ui.desktopmsgEdit->setVisible(false);
    ui.desktopsendButton->setVisible(false);
    ui.channelLabel->setText(tr("Files in channel"));
    ui.filesView->setAccessibleName(tr("Files in channel"));

    m_vid_exclude.clear();

    m_desktopaccess_entries.clear();

    m_srvprop = {};
    m_mychannel = {};
    m_myuseraccount = {};
    relayAudioStream(0, STREAMTYPE_NONE, false);

    m_useraccounts.clear();
    m_bannedusers.clear();
    m_textmessages.clear();
    m_commands.clear();
    m_talking.clear();

    if(m_sysicon)
        m_sysicon->setIcon(QIcon(APPTRAYICON));

    updateWindowTitle();
}

void MainWindow::login()
{
    QString nick = ttSettings->value(SETTINGS_GENERAL_NICKNAME, SETTINGS_GENERAL_NICKNAME_DEFAULT).toString();
    if(m_host.nickname.size())
        nick = m_host.nickname;
    QString client = QString("%1 (%2)").arg(APPNAME_SHORT).arg(QSysInfo::productType());
    int cmdid = TT_DoLoginEx(ttInst, _W(nick), _W(m_host.username),
                             _W(m_host.password), _W(client));
    if (cmdid>0)
        m_commands.insert(cmdid, CMD_COMPLETE_LOGIN);

    //query server's max payload
    if(ttSettings->value(SETTINGS_CONNECTION_QUERYMAXPAYLOAD, SETTINGS_CONNECTION_QUERYMAXPAYLOAD_DEFAULT).toBool())
        TT_QueryMaxPayload(ttInst, 0);
}

void MainWindow::slotCmdSuccess(int cmdid)
{
    if (m_commands[cmdid] == CMD_COMPLETE_SAVECONFIG)
    {
        addStatusMsg(STATUSBAR_SAVE_SERVER_CONFIG, tr("Server configuration saved"));
        addTextToSpeechMessage(TTS_MENU_ACTIONS, tr("Server configuration saved"));
    }
}

void MainWindow::showTTErrorMessage(const ClientErrorMsg& msg, CommandComplete cmd_type)
{
    QString title = tr("Error");
    QString textmsg;
    switch(msg.nErrorNo)
    {
        // command errors
    case CMDERR_SYNTAX_ERROR :
        textmsg = tr("Syntax error"); break;
    case CMDERR_UNKNOWN_COMMAND :
        textmsg = tr("Unknown command"); break;
    case CMDERR_INCOMPATIBLE_PROTOCOLS :
        textmsg = tr("The server uses a protocol which is incompatible with the client instance"); break;
    case CMDERR_UNKNOWN_AUDIOCODEC :
        textmsg = tr("Unknown audio codec"); break;
    case CMDERR_MISSING_PARAMETER :
        textmsg = tr("This client is not compatible with the server, "
                    "so the action cannot be performed."); break;
    case CMDERR_INVALID_USERNAME :
        textmsg = tr("The username is invalid"); break;

        // command errors due to rights
    case CMDERR_INVALID_ACCOUNT :
        {
            bool ok = false;
            LoginInfoDialog loginDialog(tr("Login error"), tr("Incorrect username or password. Try again."), m_host.username, m_host.password, this);
            ok = (loginDialog.exec() == QDialog::Accepted);
            if (!ok)
                return;
            m_host.username = loginDialog.getUsername();
            m_host.password = loginDialog.getPassword();
            addLatestHost(m_host);
            QString nickname = ttSettings->value(SETTINGS_GENERAL_NICKNAME, SETTINGS_GENERAL_NICKNAME_DEFAULT).toString();
            if (m_host.nickname.size())
                nickname = m_host.nickname;
            int cmdid = TT_DoLoginEx(ttInst, _W(nickname), 
                                     _W(m_host.username), _W(m_host.password), 
                                     _W(QString(APPNAME_SHORT)));
            if (cmdid > 0)
                m_commands.insert(cmdid, CMD_COMPLETE_LOGIN);
        }
        break;
    case CMDERR_INCORRECT_CHANNEL_PASSWORD :
        {
            bool ok = false;
            PasswordDialog passDialog(tr("Join channel error"), tr("Incorrect channel password. Try again."), _Q(m_last_channel.szPassword), this);
            ok = (passDialog.exec() == QDialog::Accepted);
            if (!ok)
                return;
            QString passwd = passDialog.getPassword();
            m_channel_passwd[m_last_channel.nChannelID] = passwd;
            COPY_TTSTR(m_last_channel.szPassword, passwd);
            int cmdid = TT_DoJoinChannel(ttInst, &m_last_channel);
            if (cmdid > 0)
                m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
        }
        break;
    case CMDERR_SERVER_BANNED :
        title = tr("Login error");
        textmsg = tr("Banned from server"); break;
    case CMDERR_CHANNEL_BANNED :
        textmsg = tr("Banned from channel");    break;
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
    case CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED :
        textmsg = tr("Maximum number of logins per IP-address exceeded"); break;
    case CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED :
        textmsg = tr("Maximum bitrate for audio codec exceeded"); break;
    case CMDERR_MAX_CHANNELS_EXCEEDED :
        textmsg = tr("The maximum number of channels has been exceeded"); break;
    case CMDERR_COMMAND_FLOOD :
        textmsg = tr("Command flooding prevented by server"); break;
    case CMDERR_MAX_FILETRANSFERS_EXCEEDED :
        textmsg = tr("Maximum number of file transfers exceeded"); break;

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
    case CMDERR_OPENFILE_FAILED :
        textmsg = tr("Server failed to open file");break;
    case CMDERR_LOGINSERVICE_UNAVAILABLE :
        textmsg = tr("The login service is currently unavailable");break;
    case CMDERR_CHANNEL_CANNOT_BE_HIDDEN :
        textmsg = tr("This channel cannot be hidden");break;
    case CMDERR_CHANNEL_NOT_FOUND :
        textmsg = tr("Channel not found");break;
    case CMDERR_NOT_IN_CHANNEL :
        textmsg = tr("Cannot leave channel because not in channel.");break;
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
        pttHotKey(active);
        break;
    case HOTKEY_VOICEACTIVATION :
        if(active)
            slotMeEnableVoiceActivation(!(TT_GetFlags(ttInst) & CLIENT_SNDINPUT_VOICEACTIVATED));
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
            slotUsersMuteVoiceAll(!(TT_GetFlags(ttInst) & CLIENT_SNDOUTPUT_MUTE));
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
    case HOTKEY_REINITSOUNDDEVS :
        if (active)
            initSound();
        break;
    case HOTKEY_SHOWHIDE_WINDOW :
        if (active)
        {
            if (isMinimized())
            {
                showNormal();
                activateWindow();
                raise();
             }
            else
                showMinimized();
        }
        break;
    }
}

void MainWindow::pttHotKey(bool active)
{
    bool pttfail = false;
    if (ttSettings->value(SETTINGS_GENERAL_PUSHTOTALKLOCK,
                          SETTINGS_GENERAL_PUSHTOTALKLOCK_DEFAULT).toBool())
    {
        if (active)
        {
            bool tx = (TT_GetFlags(ttInst) & CLIENT_TX_VOICE) != CLIENT_CLOSED;
            pttfail = !TT_EnableVoiceTransmission(ttInst, !tx);
            emit(updateMyself());
            playSoundEvent(SOUNDEVENT_HOTKEY);
            if (!tx)
                transmitOn(STREAMTYPE_VOICE);
        }
    }
    else
    {
        pttfail = !TT_EnableVoiceTransmission(ttInst, active) && active;
        emit(updateMyself());
        playSoundEvent(SOUNDEVENT_HOTKEY);
        if (active)
            transmitOn(STREAMTYPE_VOICE);
    }

    // PTT will e.g. fail during audio stream relay
    if (pttfail)
    {
        addStatusMsg(STATUSBAR_BYPASS, tr("Voice transmission failed"));
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
            float rx = float(stats.nUdpBytesRecv - m_clientstats.nUdpBytesRecv);
            float tx = float(stats.nUdpBytesSent - m_clientstats.nUdpBytesSent);
            int ping = stats.nUdpPingTimeMs;
            m_clientstats = stats;

            QString status = QString("RX: %1KB TX: %2KB").arg(rx / 1024.0, 2, 'f', 2, '0').arg(tx / 1024.0, 2, 'f', 2, '0');

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

        updateIdleTimeout();
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
        disconnectFromServer();
        addTextToSpeechMessage(TTS_SERVER_CONNECTIVITY, tr("Trying to reconnect to %1 port %2").arg(m_host.ipaddr).arg(m_host.tcpport));
        connectToServer();
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

void MainWindow::updateIdleTimeout()
{
    //change to away status if idle-time has been exceeded
    int idle_time = ttSettings->value(SETTINGS_GENERAL_AUTOAWAY, SETTINGS_GENERAL_AUTOAWAY_DEFAULT).toInt();
    if (idle_time != 0)
    {
        QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();
        if (isComputerIdle(idle_time) && (m_statusmode & STATUSMODE_MODE) == STATUSMODE_AVAILABLE)
        {
            m_statusmode |= STATUSMODE_AWAY;
            statusmsg = (ttSettings->value(SETTINGS_GENERAL_AWAY_STATUSMSG).toString().isEmpty()?statusmsg:ttSettings->value(SETTINGS_GENERAL_AWAY_STATUSMSG).toString());
            if (TT_GetFlags(ttInst) & CLIENT_AUTHORIZED)
            {
                TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));
            }
            m_idled_out = true;
            if (ttSettings->value(SETTINGS_GENERAL_INACTIVITY_DISABLE_VOICEACT, SETTINGS_GENERAL_INACTIVITY_DISABLE_VOICEACT_DEFAULT).toBool() &&
                ttSettings->value(SETTINGS_GENERAL_VOICEACTIVATED, SETTINGS_GENERAL_VOICEACTIVATED_DEFAULT).toBool())
            {
                enableVoiceActivation(false);
            }
        }
        else if (m_idled_out && !isComputerIdle(idle_time))
        {
            m_statusmode &= ~STATUSMODE_AWAY;
            if (TT_GetFlags(ttInst) & CLIENT_AUTHORIZED)
            {
                TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));
            }
            m_idled_out = false;
            if (ttSettings->value(SETTINGS_GENERAL_INACTIVITY_DISABLE_VOICEACT, SETTINGS_GENERAL_INACTIVITY_DISABLE_VOICEACT_DEFAULT).toBool() &&
                ttSettings->value(SETTINGS_GENERAL_VOICEACTIVATED, SETTINGS_GENERAL_VOICEACTIVATED_DEFAULT).toBool())
            {
                enableVoiceActivation(true);
            }
        }
    }
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
                    connect(m_sysicon, &QSystemTrayIcon::activated,
                        this, &MainWindow::slotTrayIconChange);
                    m_sysmenu = new QMenu(this);
                    QAction* restore = new QAction(tr("&Restore"), m_sysmenu);
                    connect(restore, &QAction::triggered, this, &QWidget::showNormal);
                    QAction* exit = new QAction(tr("&Exit"), m_sysmenu);
                    connect(exit, &QAction::triggered, this, &MainWindow::slotClientExit);
                    m_sysmenu->addAction(restore);
                    m_sysmenu->addSeparator();
                    m_sysmenu->addAction(exit);
                    m_sysicon->setContextMenu(m_sysmenu);
                    m_sysicon->setToolTip(getTitle());
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

                QTimer::singleShot(0, this, &MainWindow::hide);
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

#if defined(Q_OS_WIN32)

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
bool MainWindow::nativeEvent(const QByteArray& eventType, void* message,
                             long* result)
#else
bool MainWindow::nativeEvent(const QByteArray& eventType, void* message,
                             qintptr* result)
#endif /* QT_VERSION */
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
#endif /* Q_OS_WIN32 */

QString MainWindow::getTitle()
{
    QString profilename, title = APPTITLE;
    if(ttSettings)
        profilename = ttSettings->value(SETTINGS_GENERAL_PROFILENAME).toString();

    bool Servname = ttSettings->value(SETTINGS_DISPLAY_SERVNAME, SETTINGS_DISPLAY_SERVNAME_DEFAULT).toBool();
    if(m_mychannel.nChannelID > 0 &&
       m_mychannel.nChannelID != TT_GetRootChannelID(ttInst))
    {
        if (Servname)
        {
            title = QString("%1/%2 - %3").arg(limitText(_Q(m_srvprop.szServerName))).arg(limitText(_Q(m_mychannel.szName))).arg(APPTITLE);
        }
        else
        {
            title = QString("%1 - %2").arg(limitText(_Q(m_mychannel.szName))).arg(APPTITLE);
        }
    }
    else if (TT_GetFlags(ttInst) & CLIENT_AUTHORIZED)
    {
        title = QString("%1 - %2").arg(limitText(_Q(m_srvprop.szServerName))).arg(APPTITLE);
    }

    if(profilename.size())
        title = QString("%1 - %2").arg(title).arg(profilename);
    return title;
}

void MainWindow::updateWindowTitle()
{
    setWindowTitle(getTitle());
}

#if defined(Q_OS_WIN32)
void MainWindow::firewallInstall()
{
    //add firewall exception
    QString appPath = QApplication::applicationFilePath();
    appPath = QDir::toNativeSeparators(appPath);
    if(!TT_Firewall_AppExceptionExists(_W(appPath)))
    {
        QMessageBox answer;
        answer.setText(tr("Do you wish to add %1 to the Windows Firewall exception list?").arg(APPTITLE));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        Q_UNUSED(NoButton);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle(APPTITLE);
        answer.exec();

        if(answer.clickedButton() == YesButton &&
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
    QString subType;
    TextToSpeechEvent subTypeTTS;
    StatusBarEvent subTypeSB;
    switch (subs)
    {
    case SUBSCRIBE_USER_MSG :
        subType = tr("Private messages");
        subTypeTTS = TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_TEXTMSG_PRIVATE;
        break;
    case SUBSCRIBE_CHANNEL_MSG :
        subType = tr("Channel messages");
        subTypeTTS = TTS_SUBSCRIPTIONS_TEXTMSG_CHANNEL;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_TEXTMSG_CHANNEL;
        break;
    case SUBSCRIBE_BROADCAST_MSG :
        subType = tr("Broadcast messages");
        subTypeTTS = TTS_SUBSCRIPTIONS_TEXTMSG_BROADCAST;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_TEXTMSG_BROADCAST;
        break;
    case SUBSCRIBE_VOICE :
        subType = tr("Voice");
        subTypeTTS = TTS_SUBSCRIPTIONS_VOICE;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_VOICE;
        break;
    case SUBSCRIBE_VIDEOCAPTURE :
        subType = tr("Video");
        subTypeTTS = TTS_SUBSCRIPTIONS_VIDEO;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_VIDEO;
        break;
    case SUBSCRIBE_DESKTOP :
        subType = tr("Desktop");
        subTypeTTS = TTS_SUBSCRIPTIONS_DESKTOP;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_DESKTOP;
        break;
    case SUBSCRIBE_DESKTOPINPUT :
        subType = tr("Desktop input");
        subTypeTTS = TTS_SUBSCRIPTIONS_DESKTOPINPUT;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_DESKTOPINPUT;
        break;
    case SUBSCRIBE_MEDIAFILE :
        subType = tr("Media files");
        subTypeTTS = TTS_SUBSCRIPTIONS_MEDIAFILE;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_MEDIAFILE;
        break;
    case SUBSCRIBE_INTERCEPT_USER_MSG :
        subType = tr("Intercept private messages");
        subTypeTTS = TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE;
        break;
    case SUBSCRIBE_INTERCEPT_CHANNEL_MSG :
        subType = tr("Intercept channel messages");
        subTypeTTS = TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL;
        break;
    case SUBSCRIBE_INTERCEPT_VOICE :
        subType = tr("Intercept voice");
        subTypeTTS = TTS_SUBSCRIPTIONS_INTERCEPT_VOICE;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VOICE;
        break;
    case SUBSCRIBE_INTERCEPT_VIDEOCAPTURE :
        subType = tr("Intercept video capture");
        subTypeTTS = TTS_SUBSCRIPTIONS_INTERCEPT_VIDEO;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VIDEO;
        break;
    case SUBSCRIBE_INTERCEPT_DESKTOP :
        subType = tr("Intercept desktop");
        subTypeTTS = TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOP;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_INTERCEPT_DESKTOP;
        break;
    case SUBSCRIBE_INTERCEPT_MEDIAFILE :
        subType = tr("Intercept media files");
        subTypeTTS = TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE;
        subTypeSB = STATUSBAR_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE;
        break;
    case SUBSCRIBE_NONE :
        break;
    }
    QVector<int> userids;

    if(userid == 0)
        userids = ui.channelsWidget->selectedUsers();
    else
        userids.push_back(userid);

    foreach(userid, userids)
    {
        User user = {};
        ui.channelsWidget->getUser(userid, user);
        QString state;
        if(checked)
        {
            int cmdid = TT_DoSubscribe(ttInst, userid, subs);
            if(cmdid>0)
            {
                m_commands[cmdid] = CMD_COMPLETE_SUBSCRIBE;
                state = tr("Enabled");
            }
        }
        else
        {
            int cmdid = TT_DoUnsubscribe(ttInst, userid, subs);
            if(cmdid>0)
            {
                m_commands[cmdid] = CMD_COMPLETE_UNSUBSCRIBE;
                state = tr("Disabled");
            }
        }
        addStatusMsg(subTypeSB, UtilUI::getStatusBarMessage(SETTINGS_STATUSBARMSG_SUBCHANGE, {{"{user}", getDisplayName(user)}, {"{type}", subType}, {"{state}", state}}));
        addTextToSpeechMessage(subTypeTTS, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", getDisplayName(user)}, {"{type}", subType}, {"{state}", state}}));
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
        auto messages = m_textmessages.getUserTextMessages(userid);
        if (messages)
            dlg = new TextMessageDlg(user, *messages, nullptr);
        else
            dlg = new TextMessageDlg(user, nullptr);

        dlg->setAttribute(Qt::WA_DeleteOnClose);
        m_usermsg.insert(userid, dlg);
        connect(dlg, &TextMessageDlg::newMyselfTextMessage,
                this, &MainWindow::slotNewMyselfTextMessage);
        connect(dlg, &TextMessageDlg::closedTextMessage, this, &MainWindow::slotTextMessageClosed);
        connect(dlg, &TextMessageDlg::clearUserTextMessages, &m_textmessages, &TextMessageContainer::clearUserTextMessages);
        connect(this, &MainWindow::userUpdate, dlg, &TextMessageDlg::slotUpdateUser);
        connect(this, &MainWindow::newTextMessage, dlg,
                &TextMessageDlg::slotNewMessage);
        connect(this, &MainWindow::userLogout, dlg, &TextMessageDlg::slotUserLogout);
        return dlg;
    }
}

void MainWindow::processTextMessage(const MyTextMessage& textmsg)
{
    switch(textmsg.nMsgType)
    {
    case MSGTYPE_CHANNEL :
    {
        QString line = ui.chatEdit->addTextMessage(textmsg);
        ui.videochatEdit->addTextMessage(textmsg);
        ui.desktopchatEdit->addTextMessage(textmsg);

        //setup channel text logging
        QString chanlog = ttSettings->value(SETTINGS_MEDIASTORAGE_CHANLOGFOLDER).toString();
        if(chanlog.size())
        {
            if(!m_logChan.isOpen())
            {
                QString channame;
                if (m_mychannel.nChannelID == TT_GetRootChannelID(ttInst))
                {
                    channame = _Q(m_srvprop.szServerName);
                }
                else
                    channame = _Q(m_mychannel.szName);
                openLogFile(m_logChan, chanlog, channame + ".clog");
            }
            writeLogEntry(m_logChan, line);
        }

        if (textmsg.nFromUserID != TT_GetMyUserID(ttInst))
        {
            User user;
            if (ui.channelsWidget->getUser(textmsg.nFromUserID, user))
                addTextToSpeechMessage(TTS_USER_TEXTMSG_CHANNEL, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_CHANNELMSG, {{"{user}", getDisplayName(user)}, {"{message}", textmsg.moreMessage}, {"{server}", limitText(_Q(m_srvprop.szServerName))}}));
            playSoundEvent(SOUNDEVENT_CHANNELMSG);
        }
        else
        {
            addTextToSpeechMessage(TTS_USER_TEXTMSG_CHANNEL_SEND, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_CHANNELMSGSEND, {{"{message}", textmsg.moreMessage}}));
            playSoundEvent(SOUNDEVENT_CHANNELMSGSENT);
        }

        break;
    }
    case MSGTYPE_BROADCAST :
    {
        ui.chatEdit->addTextMessage(textmsg);
        ui.videochatEdit->addTextMessage(textmsg);
        ui.desktopchatEdit->addTextMessage(textmsg);

        User user;
        if (ui.channelsWidget->getUser(textmsg.nFromUserID, user) && user.nUserID != TT_GetMyUserID(ttInst))
            addTextToSpeechMessage(TTS_USER_TEXTMSG_BROADCAST, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_BROADCASTMSG, {{"{user}", getDisplayName(user)}, {"{message}", textmsg.moreMessage}, {"{server}", limitText(_Q(m_srvprop.szServerName))}}));
        playSoundEvent(SOUNDEVENT_BROADCASTMSG);
        break;
    }
    case MSGTYPE_USER :
    {
        ui.channelsWidget->setUserMessaged(textmsg.nFromUserID, true);
        emit(newTextMessage(textmsg));
        User user;
        if (ui.channelsWidget->getUser(textmsg.nFromUserID, user))
            addTextToSpeechMessage(TTS_USER_TEXTMSG_PRIVATE, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_PRIVATEMSG, {{"{user}", getDisplayName(user)}, {"{message}", textmsg.moreMessage}, {"{server}", limitText(_Q(m_srvprop.szServerName))}}));

        if(ttSettings->value(SETTINGS_DISPLAY_MESSAGEPOPUP, SETTINGS_DISPLAY_MESSAGEPOPUP_DEFAULT).toBool())
        {
            TextMessageDlg* dlg = getTextMessageDlg(textmsg.nFromUserID);
            if(dlg)
            {
                dlg->show();
                dlg->raise();
            }
        }
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
                addStatusMsg(STATUSBAR_BYPASS, QString(tr("%1 is requesting desktop access")
                             .arg(getDisplayName(user))));
                playSoundEvent(SOUNDEVENT_DESKTOPACCESS);
                if(hasDesktopAccess(m_desktopaccess_entries, user))
                {
                    subscribeCommon(true, SUBSCRIBE_DESKTOPINPUT, user.nUserID);
                    addStatusMsg(STATUSBAR_BYPASS, QString(tr("%1 granted desktop access")
                                 .arg(getDisplayName(user))));
                }
            }
            else
            {
                addStatusMsg(STATUSBAR_BYPASS, QString(tr("%1 retracted desktop access")
                             .arg(getDisplayName(user))));
                subscribeCommon(false, SUBSCRIBE_DESKTOPINPUT, user.nUserID);
            }
        }
    }
    break;
    case MSGTYPE_NONE :
        break;
    }
}

void MainWindow::processMyselfJoined(int channelid)
{
    ui.channelsWidget->getChannel(channelid, m_mychannel);

    // store channel for rejoin
    m_last_channel = m_mychannel;

    // store password of newly created channel
    if (m_channel_passwd.contains(CHANNELID_TEMPPASSWORD))
    {
        m_channel_passwd[m_mychannel.nChannelID] = m_channel_passwd[CHANNELID_TEMPPASSWORD];
        m_channel_passwd.remove(CHANNELID_TEMPPASSWORD);
    }

    //Enable AGC, denoise etc.
    updateAudioConfig();

    QString statusjoin;
    if (m_mychannel.nChannelID > 0 && TT_GetRootChannelID(ttInst) != m_mychannel.nChannelID)
    {
        if (m_mychannel.uChannelType & CHANNEL_CLASSROOM)
        {
            statusjoin = tr("Joined classroom channel %1").arg(_Q(m_mychannel.szName));
        }
        else
        {
            statusjoin = tr("Joined channel %1").arg(_Q(m_mychannel.szName));
        }
    }
    else
    {
        QString root = tr("root");
        if (m_mychannel.uChannelType & CHANNEL_CLASSROOM)
        {
            statusjoin = tr("Joined classroom channel %1").arg(root);
        }
        else
        {
            statusjoin = tr("Joined channel %1").arg(root);
        }
    }

    //update list of files in current channel
    updateChannelFiles(channelid);

    addTextToSpeechMessage(TTS_USER_JOINED, statusjoin);

    //show channel information in chat window
    ui.chatEdit->joinedChannel(channelid);

    ui.msgEdit->setVisible(true);
    ui.videomsgEdit->setVisible(true);
    ui.desktopmsgEdit->setVisible(true);
    updateWindowTitle();

    //store new muxed audio file if we're changing channel
    if (ui.actionMediaStorage->isChecked() && (m_audiostorage_mode & AUDIOSTORAGE_SINGLEFILE))
    {
        updateAudioStorage(false, AUDIOSTORAGE_SINGLEFILE);
        updateAudioStorage(true, AUDIOSTORAGE_SINGLEFILE);
    }

    if (ttSettings->value(SETTINGS_CONNECTION_HEAR_MYSELF, SETTINGS_CONNECTION_HEAR_MYSELF_DEFAULT).toBool())
        subscribeCommon(true, SUBSCRIBE_VOICE, TT_GetMyUserID(ttInst));
}

void MainWindow::processMyselfLeft(int /*channelid*/)
{
    Q_ASSERT(m_mychannel.nChannelID > 0);

    QString statusleft;
    if (TT_GetRootChannelID(ttInst) != m_mychannel.nChannelID)
    {
        if(m_mychannel.uChannelType & CHANNEL_CLASSROOM)
        {
            statusleft = tr("Left classroom channel %1").arg(_Q(m_mychannel.szName));
        }
        else
        {
            statusleft = tr("Left channel %1").arg(_Q(m_mychannel.szName));
        }
    }
    else
    {
        QString root = tr("root");
        if (m_mychannel.uChannelType & CHANNEL_CLASSROOM)
        {
            statusleft = tr("Left classroom channel %1").arg(root);
        }
        else
        {
            statusleft = tr("Left channel %1").arg(root);
        }
    }
    addStatusMsg(STATUSBAR_USER_LEFT, statusleft);
    addTextToSpeechMessage(TTS_USER_LEFT, statusleft);

    m_mychannel = {};
    m_last_channel = {};

    m_talking.clear();
    ui.videogridWidget->resetGrid();
    ui.desktopgridWidget->resetGrid();

    if(m_logChan.isOpen())
    {
        QString filename = m_logChan.fileName();
        quint64 size = m_logChan.size();
        m_logChan.close();
        if(size == 0)
            QFile::remove(filename);
    }
    ui.msgEdit->setVisible(false);
    ui.sendButton->setVisible(false);
    ui.videomsgEdit->setVisible(false);
    ui.videosendButton->setVisible(false);
    ui.desktopmsgEdit->setVisible(false);
    ui.desktopsendButton->setVisible(false);
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
    ui.filesView->setAccessibleName(tr("Files in channel: %1").arg(_Q(chanpath)));

    if (m_proxyFilesModel->rowCount() == 0)
    {
        ui.tabWidget->setTabText(TAB_FILES, tr("&Files"));
        ui.deleteButton->setVisible(false);
        ui.downloadButton->setVisible(false);
    }
    else
    {
        ui.tabWidget->setTabText(TAB_FILES, tr("&Files (%1)").arg(m_proxyFilesModel->rowCount()));
        ui.deleteButton->setVisible(true);
        ui.downloadButton->setVisible(true);
    }
}

void MainWindow::updateUserSubscription(int userid)
{
    Subscriptions unsub = SUBSCRIBE_NONE;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_USERMSG, SETTINGS_CONNECTION_SUBSCRIBE_USERMSG_DEFAULT).toBool())
        unsub |= SUBSCRIBE_USER_MSG;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_CHANNELMSG, SETTINGS_CONNECTION_SUBSCRIBE_CHANNELMSG_DEFAULT).toBool())
        unsub |= SUBSCRIBE_CHANNEL_MSG;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_BROADCASTMSG, SETTINGS_CONNECTION_SUBSCRIBE_BROADCASTMSG_DEFAULT).toBool())
        unsub |= SUBSCRIBE_BROADCAST_MSG;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_VOICE, SETTINGS_CONNECTION_SUBSCRIBE_VOICE_DEFAULT).toBool())
        unsub |= SUBSCRIBE_VOICE;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_VIDEOCAPTURE, SETTINGS_CONNECTION_SUBSCRIBE_VIDEOCAPTURE_DEFAULT).toBool())
        unsub |= SUBSCRIBE_VIDEOCAPTURE;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_DESKTOP, SETTINGS_CONNECTION_SUBSCRIBE_DESKTOP_DEFAULT).toBool())
        unsub |= SUBSCRIBE_DESKTOP;
    if(!ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_MEDIAFILE, SETTINGS_CONNECTION_SUBSCRIBE_MEDIAFILE_DEFAULT).toBool())
        unsub |= SUBSCRIBE_MEDIAFILE;

    if(unsub)
        subscribeCommon(false, unsub, userid);
}

void MainWindow::updateAudioStorage(bool enable, AudioStorageMode mode)
{
    AudioFileFormat aff = (AudioFileFormat)ttSettings->value(SETTINGS_MEDIASTORAGE_FILEFORMAT, 
                                                             AFF_WAVE_FORMAT).toInt();
    QString audiofolder = ttSettings->value(SETTINGS_MEDIASTORAGE_AUDIOFOLDER).toString();
    auto sts = ttSettings->value(SETTINGS_MEDIASTORAGE_STREAMTYPES,
                                 SETTINGS_MEDIASTORAGE_STREAMTYPES_DEFAULT).toUInt();

    if (mode & AUDIOSTORAGE_SINGLEFILE)
    {
        if(enable)
        {
            //don't try to record with no audio codec specified (it will fail)
            if(m_mychannel.audiocodec.nCodec == NO_CODEC)
                return;

            QString filepath = audiofolder;
            filepath += QDir::toNativeSeparators("/") + generateAudioStorageFilename(aff);

            if (!TT_StartRecordingMuxedStreams(ttInst, sts, &m_mychannel.audiocodec,
                                               _W(filepath), aff))
            {
                QMessageBox::critical(this, tr("Error"), tr("Failed to start recording"));
                return;
            }
            else
                addStatusMsg(STATUSBAR_START_RECORD, tr("Recording to file: %1").arg(filepath));
        }
        else
        {
            TT_StopRecordingMuxedAudioFile(ttInst);
        }
    }
    
    if (mode & AUDIOSTORAGE_SEPARATEFILES)
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
    bool denoise = ttSettings->value(SETTINGS_SOUND_DENOISING,
                                     SETTINGS_SOUND_DENOISING_DEFAULT).toBool();
    bool echocancel = ttSettings->value(SETTINGS_SOUND_ECHOCANCEL,
                                     SETTINGS_SOUND_ECHOCANCEL_DEFAULT).toBool();
    bool agc = ttSettings->value(SETTINGS_SOUND_AGC, SETTINGS_SOUND_AGC_DEFAULT).toBool();

    bool duplex = getSoundDuplexSampleRate(m_devin, m_devout) > 0;
    //check if channel AGC settings should override default settings
    if (m_mychannel.audiocfg.bEnableAGC)
    {
        AudioPreprocessor preprocessor = initDefaultAudioPreprocessor(WEBRTC_AUDIOPREPROCESSOR);

        preprocessor.webrtc.noisesuppression.bEnable = denoise;
        preprocessor.webrtc.echocanceller.bEnable = echocancel && duplex;

        preprocessor.webrtc.gaincontroller2.bEnable = true;
        float gainlevel = float(m_mychannel.audiocfg.nGainLevel) / CHANNEL_AUDIOCONFIG_MAX;
        preprocessor.webrtc.gaincontroller2.fixeddigital.fGainDB = WEBRTC_GAINCONTROLLER2_FIXEDGAIN_MAX * gainlevel;

        //override preset sound gain
        TT_SetSoundInputGainLevel(ttInst, SOUND_GAIN_DEFAULT);
        TT_SetSoundInputPreprocessEx(ttInst, &preprocessor);
        ui.micSlider->setToolTip(tr("Microphone gain is controlled by channel"));
    }
    else
    {
        AudioPreprocessor preprocessor;
        if (denoise || agc || echocancel)
        {
            preprocessor = initDefaultAudioPreprocessor(WEBRTC_AUDIOPREPROCESSOR);
            preprocessor.webrtc.noisesuppression.bEnable = denoise;
            preprocessor.webrtc.echocanceller.bEnable = echocancel && duplex;
            preprocessor.webrtc.gaincontroller2.bEnable = agc;
        }
        else
        {
            preprocessor = initDefaultAudioPreprocessor(TEAMTALK_AUDIOPREPROCESSOR);
        }
        TT_SetSoundInputPreprocessEx(ttInst, &preprocessor);

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

    DesktopWindow wnd = {};
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
            DesktopWindow wnd = {};
            
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
    ShareWindow wnd = {};

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

        DesktopInput trans_input = {};

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

void MainWindow::loadHotKeys()
{
    Hotkeys activeHotkeys = ttSettings->value(SETTINGS_SHORTCUTS_ACTIVEHKS, SETTINGS_SHORTCUTS_ACTIVEHKS_DEFAULT).toULongLong();
    for (Hotkeys hk = HOTKEY_FIRST; hk < HOTKEY_NEXT_UNUSED; hk <<= 1)
    {
        HotKeyID hki = static_cast<HotKeyID>(hk);
        hotkey_t hotkey;
        if (activeHotkeys & hk)
        {
            if (loadHotKeySettings(hki, hotkey))
            {
                enableHotKey(hki, hotkey);
            }
            else
            {
                disableHotKey(hki);
            }
        }
        else
        {
            disableHotKey(hki);
        }
    }
}

void MainWindow::enableHotKey(HotKeyID id, const hotkey_t& hk)
{
    Q_ASSERT(hk.size());

    //disable first so we don't double register
    disableHotKey(id);

#ifdef Q_OS_WIN32
    TT_HotKey_Register(ttInst, id, &hk[0], INT32(hk.size()));

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

         XEvent event = {};
        
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

void MainWindow::checkAppUpdate(bool manualCheck/* = false*/)
{
    // check for software update and get bearware.dk web-login url
    bool beta = ttSettings->value(SETTINGS_DISPLAY_APPUPDATE_BETA, SETTINGS_DISPLAY_APPUPDATE_BETA_DEFAULT).toBool();
    QUrl url(URL_APPUPDATE(beta));

    auto networkMgr = new QNetworkAccessManager(this);
    connect(networkMgr, &QNetworkAccessManager::finished,
            this, [this, manualCheck](QNetworkReply* reply) {
                slotSoftwareUpdateReply(reply, manualCheck);
            });

    QNetworkRequest request(url);
    networkMgr->get(request);
}

void MainWindow::toggleAllowStreamType(bool checked, int userid, int channelid, StreamType st)
{
    QMap<int,StreamTypes> transmitUsers;
    ui.channelsWidget->getTransmitUsers(channelid, transmitUsers);
    if (checked)
        transmitUsers[userid] |= st;
    else
        transmitUsers[userid] &= ~st;
    slotTransmitUsersChanged(channelid, transmitUsers);
}

void MainWindow::toggleAllowStreamType(bool checked, StreamType st)
{
    int userid = ui.channelsWidget->selectedUser();
    int channelid = ui.channelsWidget->selectedChannel(true);
    if (userid > 0 && channelid > 0)
        toggleAllowStreamType(checked, userid, channelid, st);
}

void MainWindow::transmitOn(StreamType st)
{
    switch (st)
    {
    case STREAMTYPE_CHANNELMSG :
        if (!userCanChanMessage(TT_GetMyUserID(ttInst), m_mychannel, true))
            addStatusMsg(STATUSBAR_TRANSMISSION_BLOCKED, tr("Text messages blocked by channel operator"));
        break;
    case STREAMTYPE_VOICE :
        if (!userCanVoiceTx(TT_GetMyUserID(ttInst), m_mychannel, true))
            addStatusMsg(STATUSBAR_TRANSMISSION_BLOCKED, tr("Voice transmission blocked by channel operator"));
        break;
    case STREAMTYPE_MEDIAFILE :
        if (!userCanMediaFileTx(TT_GetMyUserID(ttInst), m_mychannel, true))
            addStatusMsg(STATUSBAR_TRANSMISSION_BLOCKED, tr("Media file transmission blocked by channel operator"));
        break;
    case STREAMTYPE_VIDEOCAPTURE :
        if (!userCanVideoTx(TT_GetMyUserID(ttInst), m_mychannel, true))
            addStatusMsg(STATUSBAR_TRANSMISSION_BLOCKED, tr("Video transmission blocked by channel operator"));
        break;
    case STREAMTYPE_DESKTOP :
        if (!userCanDesktopTx(TT_GetMyUserID(ttInst), m_mychannel, true))
            addStatusMsg(STATUSBAR_TRANSMISSION_BLOCKED, tr("Desktop transmission blocked by channel operator"));
        break;
    default :
        break;
    }
}

void MainWindow::relayAudioStream(int userid, StreamType st, bool enable)
{
    if (m_relayvoice_userid)
    {
        TT_EnableAudioBlockEvent(ttInst, userid, STREAMTYPE_VOICE, false);
        TT_InsertAudioBlock(ttInst, nullptr);
        m_relayvoice_userid = 0;
    }

    if (m_relaymediafile_userid)
    {
        TT_EnableAudioBlockEvent(ttInst, userid, STREAMTYPE_MEDIAFILE_AUDIO, false);
        TT_InsertAudioBlock(ttInst, nullptr);
        m_relaymediafile_userid = 0;
    }

    if (enable)
    {
        TT_EnableAudioBlockEvent(ttInst, userid, st, true);
        switch (st)
        {
        case STREAMTYPE_VOICE:
            m_relayvoice_userid = userid;
            break;
        case STREAMTYPE_MEDIAFILE_AUDIO :
            m_relaymediafile_userid = userid;
            break;
        default :
            Q_ASSERT(0);
        }
    }
}

void MainWindow::toggleAllowStreamTypeForAll(bool checked, StreamType st)
{
    int channelid = ui.channelsWidget->selectedChannel(true);
    if (channelid > 0)
        toggleAllowStreamType(checked, TT_TRANSMITUSERS_FREEFORALL, channelid, st);
}

void MainWindow::slotClientNewInstance(bool /*checked=false*/)
{
    QString inipath = ttSettings->fileName();

    // check if we are creating a new profile from a profile
    if(ttSettings->value(SETTINGS_GENERAL_PROFILENAME).toString().size())
    {
        inipath.remove(QRegularExpression(".\\d{1,2}$"));
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
    

    const QString newprofile = tr("New Profile"), delprofile = tr("Delete Profile"), curprofile = tr("Current Profile");
    if(profiles.size() < MAX_PROFILES)
        profilenames.push_back(newprofile);
    if(profiles.size() > 0)
        profilenames.push_back(delprofile);
    profilenames.push_back(curprofile);

    bool ok = false;
    QInputDialog inputDialog;
    inputDialog.setOkButtonText(tr("&OK"));
    inputDialog.setCancelButtonText(tr("&Cancel"));
    inputDialog.setComboBoxItems(profilenames);
    inputDialog.setComboBoxEditable(false);
    inputDialog.setWindowTitle(tr("New Client Instance"));
    inputDialog.setLabelText(tr("Select profile"));
    ok = inputDialog.exec();
    QString choice = inputDialog.textValue();

    if (ok)
    {
        QStringList args;

        if(choice == delprofile)
        {
            profilenames.removeAll(newprofile);
            profilenames.removeAll(delprofile);
            QInputDialog inputDialog;
            inputDialog.setOkButtonText(tr("&OK"));
            inputDialog.setCancelButtonText(tr("&Cancel"));
            inputDialog.setComboBoxItems(profilenames);
            inputDialog.setComboBoxEditable(false);
            inputDialog.setWindowTitle(tr("New Client Instance"));
            inputDialog.setLabelText(tr("Delete profile"));
            ok = inputDialog.exec();
            QString choice = inputDialog.textValue();
            if(ok && ttSettings->fileName() != profiles[choice])
                QFile::remove(profiles[choice]);
            return;
        }
        else if(choice == newprofile)
        {
            QInputDialog inputDialog;
            inputDialog.setOkButtonText(tr("&OK"));
            inputDialog.setCancelButtonText(tr("&Cancel"));
            inputDialog.setInputMode(QInputDialog::TextInput);
            inputDialog.setTextValue(QString("Profile %1").arg(freeno));
            inputDialog.setWindowTitle(tr("New Profile"));
            inputDialog.setLabelText(tr("Profile name"));
            ok = inputDialog.exec();
            QString newname = inputDialog.textValue();
            if(ok && newname.size())
            {
                inipath = QString("%1.%2").arg(inipath).arg(freeno);
                QSettings settings(inipath, QSettings::IniFormat, this);
                settings.setValue(SETTINGS_GENERAL_PROFILENAME, newname);
            }
            else return;
        }
        else if (choice == curprofile)
        {
            inipath = ttSettings->fileName();
            args.push_back("-noconnect");
        }
        else 
        {
            inipath = profiles[choice];
        }

        QString path = QApplication::applicationFilePath();
        args.push_back(QString("-cfg"));
        args.push_back(inipath);

#if defined(_DEBUG)
        QProcess::startDetached(path, args);
#else
        QProcess::startDetached(path, args, QApplication::applicationDirPath());
#endif
    }
}

void MainWindow::slotClientConnect(bool /*checked =false */)
{
    killLocalTimer(TIMER_RECONNECT);

    //reset last channel, since we're starting a new connection
    m_last_channel = {};
    m_channel_passwd.clear();

    if(TT_GetFlags(ttInst) & CLIENT_CONNECTION)
    {
        disconnectFromServer();
    }
    else
    {
        ServerListDlg dlg(this);
        if(dlg.exec())
        {
            m_host = dlg.getHostEntry();
            m_channel_passwd[CHANNELID_TEMPPASSWORD] = m_host.chanpasswd;
            connectToServer();
        }
    }

    //update checked state
    slotUpdateUI();
}

void MainWindow::slotClientPreferences(bool /*checked =false */)
{
    PreferencesDlg dlg(m_devin, m_devout, this);

    //we need to be able to process local frames (userid 0),
    //so ensure these video frames are not being displayed elsewhere
    int localvideo_userid = (0 | VIDEOTYPE_CAPTURE);
    bool ignore_set = m_vid_exclude.find(localvideo_userid) != m_vid_exclude.end();
    disconnect(this, &MainWindow::newVideoCaptureFrame,
               ui.videogridWidget, &VideoGridWidget::slotNewVideoFrame);
    m_vid_exclude.remove(localvideo_userid);

    uservideo_t::iterator local_ite = m_user_video.find(localvideo_userid);
    if(local_ite != m_user_video.end())
        disconnect(this, &MainWindow::newVideoCaptureFrame,
                   (*local_ite)->uservideoWidget, 
                   &UserVideoWidget::slotNewVideoFrame);

    connect(this, &MainWindow::newVideoCaptureFrame, &dlg,
            &PreferencesDlg::slotNewVideoFrame);

    //see if we need to retranslate
    QString lang = ttSettings->value(SETTINGS_DISPLAY_LANGUAGE).toString();

    int mediavsvoice = ttSettings->value(SETTINGS_SOUND_MEDIASTREAM_VOLUME,
                                         SETTINGS_SOUND_MEDIASTREAM_VOLUME_DEFAULT).toInt();

    int sndinputid = getSelectedSndInputDevice();
    int sndoutputid = getSelectedSndOutputDevice();

    //show dialog
    bool b = dlg.exec();

    disconnect(this, &MainWindow::newVideoCaptureFrame, &dlg,
               &PreferencesDlg::slotNewVideoFrame);

    if(ignore_set)
        m_vid_exclude.insert(localvideo_userid);
    if(local_ite != m_user_video.end())
        connect(this, &MainWindow::newVideoCaptureFrame,
                (*local_ite)->uservideoWidget, 
                &UserVideoWidget::slotNewVideoFrame);

    connect(this, &MainWindow::newVideoCaptureFrame,
            ui.videogridWidget, &VideoGridWidget::slotNewVideoFrame);

    if(!b)return;

#if defined(QT_TEXTTOSPEECH_LIB)
    if (ttSettings->value(SETTINGS_TTS_ENGINE, SETTINGS_TTS_ENGINE_DEFAULT).toUInt() == TTSENGINE_QT && ttSpeech == nullptr)
        ttSpeech = new QTextToSpeech(this);
    else
    {
        delete ttSpeech;
        ttSpeech = nullptr;
    }
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6,8,0)
    if (ttSettings->value(SETTINGS_TTS_ENGINE, SETTINGS_TTS_ENGINE_DEFAULT).toUInt() == TTSENGINE_QTANNOUNCEMENT && announcerObject == nullptr)
        announcerObject = this;
    else
        announcerObject = nullptr;
#endif

    if (sndinputid != getSelectedSndInputDevice() || sndoutputid != getSelectedSndOutputDevice())
        initSound();

    User myself;
    if((TT_GetFlags(ttInst) & CLIENT_AUTHORIZED) &&
        TT_GetUser(ttInst, TT_GetMyUserID(ttInst), &myself))
    {
        QString nickname = ttSettings->value(SETTINGS_GENERAL_NICKNAME, SETTINGS_GENERAL_NICKNAME_DEFAULT).toString();
        if((_Q(myself.szNickname) != nickname) && m_host.nickname.isEmpty())
            TT_DoChangeNickname(ttInst, _W(nickname));

        QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();
        m_statusmode &= ~STATUSMODE_GENDER_MASK;
        switch (Gender(ttSettings->value(SETTINGS_GENERAL_GENDER, SETTINGS_GENERAL_GENDER).toInt()))
        {
        case GENDER_MALE :
            m_statusmode |= STATUSMODE_MALE;
            break;
        case GENDER_FEMALE :
            m_statusmode |= STATUSMODE_FEMALE;
            break;
        case GENDER_NEUTRAL :
        default :
            m_statusmode |= STATUSMODE_NEUTRAL;
        }

        //set status mode flags
        if(m_statusmode != myself.nStatusMode || statusmsg != _Q(myself.szStatusMsg))
            TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));
    }
    enableVoiceActivation(ttSettings->value(SETTINGS_GENERAL_VOICEACTIVATED,
                                                  SETTINGS_GENERAL_VOICEACTIVATED_DEFAULT).toBool());
    slotMeEnablePushToTalk(ttSettings->value(SETTINGS_SHORTCUTS_ACTIVEHKS, SETTINGS_SHORTCUTS_ACTIVEHKS_DEFAULT).toULongLong() & HOTKEY_PUSHTOTALK);

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

    // Hide/Show widgets
    if(ttSettings->value(SETTINGS_DISPLAY_VU_METER_UPDATES,
                         SETTINGS_DISPLAY_VU_METER_UPDATES_DEFAULT).toBool())
    {
        if(!timerExists(TIMER_VUMETER_UPDATE))
        {
            m_timers.insert(startTimer(50), TIMER_VUMETER_UPDATE);
            ui.vumeterLabel->setVisible(true);
            ui.voiceactBar->setVisible(true);
        }
    }
    else if(timerExists(TIMER_VUMETER_UPDATE))
    {
        killLocalTimer(TIMER_VUMETER_UPDATE);
        ui.vumeterLabel->setVisible(false);
        ui.voiceactBar->setVisible(false);
    }
    ui.voiceactLabel->setVisible(ui.actionEnableVoiceActivation->isChecked() && ttSettings->value(SETTINGS_DISPLAY_VOICE_ACT_SLIDER, SETTINGS_DISPLAY_VOICE_ACT_SLIDER_DEFAULT).toBool());
    ui.voiceactSlider->setVisible(ui.actionEnableVoiceActivation->isChecked() && ttSettings->value(SETTINGS_DISPLAY_VOICE_ACT_SLIDER, SETTINGS_DISPLAY_VOICE_ACT_SLIDER_DEFAULT).toBool());

    ui.channelsWidget->updateAllItems();

    if (lang != ttSettings->value(SETTINGS_DISPLAY_LANGUAGE).toString())
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

    updateWindowTitle();
    slotUpdateUI();
    startTTS();
}

void MainWindow::slotClientSoundDevices()
{
    ui.menuInputDev->clear();
    ui.menuOutputDev->clear();
    
    QMap<SoundSystem, QString> sndsys;
    sndsys[SOUNDSYSTEM_DSOUND] = "DirectSound";
    sndsys[SOUNDSYSTEM_ALSA] = "ALSA";
    sndsys[SOUNDSYSTEM_COREAUDIO] = "CoreAudio";
    sndsys[SOUNDSYSTEM_AUDIOUNIT] = "AudioUnit";
    sndsys[SOUNDSYSTEM_WINMM] = "WinMM";
    sndsys[SOUNDSYSTEM_WASAPI] = "WASAPI";
    sndsys[SOUNDSYSTEM_NONE] = "None";

    auto devs = getSoundDevices();

    // Current sound system is set in Preferences Dialog
    SoundSystem selectedSoundSystem = SoundSystem(ttSettings->value(SETTINGS_SOUND_SOUNDSYSTEM, SOUNDSYSTEM_NONE).toUInt());

    // If no sound system is selected then find the default sound system for the platform
    if (selectedSoundSystem == SOUNDSYSTEM_NONE)
    {
        int nInDev, nOutDev;
        SoundDevice dev;
        if (TT_GetDefaultSoundDevices(&nInDev, &nOutDev) && getSoundDevice(nOutDev, devs, dev))
            selectedSoundSystem = dev.nSoundSystem;
    }

    auto reinitfunc = std::bind(&MainWindow::initSound, this);
    for (auto& dev : devs)
    {
        if (dev.nSoundSystem != SOUNDSYSTEM_NONE && dev.nSoundSystem != selectedSoundSystem)
            continue;

        if (dev.nSoundSystem == SOUNDSYSTEM_NONE)
            COPY_TTSTR(dev.szDeviceName, tr("No Sound Device")); // Same translatable string as in preferencesdlg.cpp

        if (dev.nMaxInputChannels > 0)
        {
            auto newaction = ui.menuInputDev->addAction(_Q(dev.szDeviceName) + " [" + sndsys[dev.nSoundSystem] + "]");
            newaction->setCheckable(true);
            newaction->setChecked(dev.nDeviceID == ttSettings->value(SETTINGS_SOUND_INPUTDEVICE, SOUNDDEVICEID_DEFAULT).toInt());
            connect(newaction, &QAction::triggered, [dev, reinitfunc] {
                ttSettings->setValue(SETTINGS_SOUND_INPUTDEVICE, dev.nDeviceID);
                ttSettings->setValue(SETTINGS_SOUND_INPUTDEVICE_UID, _Q(dev.szDeviceID));
                reinitfunc();
                });
        }
        if (dev.nMaxOutputChannels > 0)
        {
            auto newaction = ui.menuOutputDev->addAction(_Q(dev.szDeviceName) + " [" + sndsys[dev.nSoundSystem] + "]");
            newaction->setCheckable(true);
            newaction->setChecked(dev.nDeviceID == ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE, SOUNDDEVICEID_DEFAULT).toInt());
            connect(newaction, &QAction::triggered, [dev, reinitfunc] {
                ttSettings->setValue(SETTINGS_SOUND_OUTPUTDEVICE, dev.nDeviceID);
                ttSettings->setValue(SETTINGS_SOUND_OUTPUTDEVICE_UID, _Q(dev.szDeviceID));
                reinitfunc();
                });
        }
    }
}

void MainWindow::slotClientAudioEffect()
{
    if (QObject::sender() == ui.actionEnableEchoCancel)
        ttSettings->setValue(SETTINGS_SOUND_ECHOCANCEL, ui.actionEnableEchoCancel->isChecked());
    else if (QObject::sender() == ui.actionEnableAGC)
        ttSettings->setValue(SETTINGS_SOUND_AGC, ui.actionEnableAGC->isChecked());
    else if (QObject::sender() == ui.actionEnableDenoising)
        ttSettings->setValue(SETTINGS_SOUND_DENOISING, ui.actionEnableDenoising->isChecked());
    slotUpdateUI();
    updateAudioConfig();
}

void MainWindow::slotClientRecordConversations(bool/* checked*/)
{
    quint32 old_mode = m_audiostorage_mode;

    if(MediaStorageDlg(this).exec())
    {
        m_audiostorage_mode = ttSettings->value(SETTINGS_MEDIASTORAGE_MODE,
                                                AUDIOSTORAGE_NONE).toUInt();

        quint32 new_mode = ttSettings->value(SETTINGS_MEDIASTORAGE_MODE,
                                             AUDIOSTORAGE_NONE).toUInt();

        if ((old_mode & AUDIOSTORAGE_SINGLEFILE))
        {
            updateAudioStorage(false, AUDIOSTORAGE_SINGLEFILE);
            m_audiostorage_mode &= ~AUDIOSTORAGE_SINGLEFILE;
        }
        if (old_mode & AUDIOSTORAGE_SEPARATEFILES)
        {
            updateAudioStorage(false, AUDIOSTORAGE_SEPARATEFILES);
            m_audiostorage_mode &= ~AUDIOSTORAGE_SEPARATEFILES;
        }

        if (new_mode & AUDIOSTORAGE_SINGLEFILE)
        {
            updateAudioStorage(true, AUDIOSTORAGE_SINGLEFILE);
            m_audiostorage_mode |= AUDIOSTORAGE_SINGLEFILE;
        }
        if (new_mode & AUDIOSTORAGE_SEPARATEFILES)
        {
            updateAudioStorage(true, AUDIOSTORAGE_SEPARATEFILES);
            m_audiostorage_mode |= AUDIOSTORAGE_SEPARATEFILES;
        }

        if(ttSettings->value(SETTINGS_MEDIASTORAGE_CHANLOGFOLDER).toString().isEmpty())
            m_logChan.close();
    }

    slotUpdateUI();
}

void MainWindow::slotClientExit(bool /*checked =false */)
{
    //close using timer, otherwise gets a Qt assertion from the 
    //'setQuitOnLastWindowClosed' call.
#if defined(ENABLE_TOLK)
    if(Tolk_IsLoaded())
        Tolk_Unload();
#endif
    if(TT_GetFlags(ttInst) & CLIENT_CONNECTED)
        disconnectFromServer();
    QApplication::quit();
}

void MainWindow::slotMeChangeNickname(bool /*checked =false */)
{
    QString nick = ttSettings->value(SETTINGS_GENERAL_NICKNAME, SETTINGS_GENERAL_NICKNAME_DEFAULT).toString();
    if ((TT_GetFlags(ttInst) & CLIENT_AUTHORIZED) && m_host.nickname.size())
        nick = m_host.nickname;
    bool ok = false;
    QInputDialog inputDialog;
    inputDialog.setOkButtonText(tr("&OK"));
    inputDialog.setCancelButtonText(tr("&Cancel"));
    inputDialog.setInputMode(QInputDialog::TextInput);
    inputDialog.setTextValue(nick);
    inputDialog.setWindowTitle(MENUTEXT(ui.actionChangeNickname->text()));
    if (TT_GetFlags(ttInst) & CLIENT_AUTHORIZED)
        inputDialog.setLabelText(tr("Specify new nickname for current server"));
    else
        inputDialog.setLabelText(tr("Specify new nickname"));
    ok = inputDialog.exec();
    QString s = inputDialog.textValue();
    if(ok)
    {
        if(TT_GetFlags(ttInst) & CLIENT_AUTHORIZED)
        {
            m_host.nickname = s;
            TT_DoChangeNickname(ttInst, (s.isEmpty() && !ttSettings->value(SETTINGS_GENERAL_NICKNAME).toString().isEmpty())?_W(ttSettings->value(SETTINGS_GENERAL_NICKNAME).toString()):_W(s));
            HostEntry tmp = HostEntry();
            int serv, lasthost, index = 0;
            while (getServerEntry(index, tmp, false))
            {
                if (m_host.sameHost(tmp, false))
                    serv = index;
                index++;
                tmp = HostEntry();
            }
            tmp = HostEntry();
            index = 0;
            while(getServerEntry(index, tmp, true))
            {
                if (m_host.sameHostEntry(tmp))
                    lasthost = index;
                index++;
            }
            if(s != ttSettings->value(SETTINGS_GENERAL_NICKNAME).toString())
            {
                ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_NICKNAME).arg(serv), s);
                ttSettings->setValue(QString(SETTINGS_LATESTHOST_NICKNAME).arg(lasthost), s);
            }
            else if(s == ttSettings->value(SETTINGS_GENERAL_NICKNAME).toString() || s.isEmpty())
            {
                ttSettings->remove(QString(SETTINGS_SERVERENTRIES_NICKNAME).arg(serv));
                ttSettings->remove(QString(SETTINGS_LATESTHOST_NICKNAME).arg(lasthost));
            }
        }
        else
            ttSettings->setValue(SETTINGS_GENERAL_NICKNAME, s);
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
            KeyCompDlg dlg(HOTKEY_PUSHTOTALK, this);
            if(!dlg.exec())
                return;
            saveHotKeySettings(HOTKEY_PUSHTOTALK, dlg.m_hotkey);
            hotkey = dlg.m_hotkey;
        }

        if(hotkey.size())
            enableHotKey(HOTKEY_PUSHTOTALK, hotkey);
        if (QObject::sender() == ui.actionEnablePushToTalk)
            addTextToSpeechMessage(TTS_TOGGLE_VOICETRANSMISSION, tr("Push-To-Talk enabled"));
    }
    else
    {
        disableHotKey(HOTKEY_PUSHTOTALK);
        if (QObject::sender() == ui.actionEnablePushToTalk)
            addTextToSpeechMessage(TTS_TOGGLE_VOICETRANSMISSION, tr("Push-To-Talk disabled"));
    }

    auto activekeys = ttSettings->value(SETTINGS_SHORTCUTS_ACTIVEHKS, SETTINGS_SHORTCUTS_ACTIVEHKS_DEFAULT).toULongLong();
    ttSettings->setValue(SETTINGS_SHORTCUTS_ACTIVEHKS, (checked ? activekeys | HOTKEY_PUSHTOTALK : activekeys & ~HOTKEY_PUSHTOTALK));

    slotUpdateUI();
}

void MainWindow::slotMeHearMyself(bool checked/*=false*/)
{
    subscribeCommon(checked, SUBSCRIBE_VOICE, TT_GetMyUserID(ttInst));
    ttSettings->setValue(SETTINGS_CONNECTION_HEAR_MYSELF, checked);
}

void MainWindow::slotMeEnableVoiceActivation(bool checked)
{
    enableVoiceActivation(checked, SOUNDEVENT_VOICEACTMEON, SOUNDEVENT_VOICEACTMEOFF);
    addTextToSpeechMessage(TTS_TOGGLE_VOICETRANSMISSION, checked == true ? tr("Voice activation enabled") : tr("Voice activation disabled"));
}

void MainWindow::enableVoiceActivation(bool checked, SoundEvent on, SoundEvent off)
{
    if (!TT_EnableVoiceActivation(ttInst, checked) && checked)
    {
        addStatusMsg(STATUSBAR_BYPASS, tr("Failed to enable voice activation"));
    }
    else
    {
        ui.voiceactLabel->setVisible(checked && ttSettings->value(SETTINGS_DISPLAY_VOICE_ACT_SLIDER, SETTINGS_DISPLAY_VOICE_ACT_SLIDER_DEFAULT).toBool());
        ui.voiceactSlider->setVisible(checked && ttSettings->value(SETTINGS_DISPLAY_VOICE_ACT_SLIDER, SETTINGS_DISPLAY_VOICE_ACT_SLIDER_DEFAULT).toBool());
        if (TT_GetFlags(ttInst) & CLIENT_CONNECTED)
            emit(updateMyself());
        playSoundEvent(checked == true ? on : off);
        ttSettings->setValue(SETTINGS_GENERAL_VOICEACTIVATED, checked);
    }
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
            tr("Video device hasn't been configured properly. Check settings in 'Preferences'"));
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
            transmitOn(STREAMTYPE_VIDEOCAPTURE);
            addTextToSpeechMessage(TTS_TOGGLE_VIDEOTRANSMISSION, tr("Video transmission enabled"));
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
        addTextToSpeechMessage(TTS_TOGGLE_VIDEOTRANSMISSION, tr("Video transmission disabled"));
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
            transmitOn(STREAMTYPE_DESKTOP);
            addTextToSpeechMessage(TTS_TOGGLE_DESKTOPTRANSMISSION, tr("Desktop sharing enabled"));
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
            addTextToSpeechMessage(TTS_TOGGLE_DESKTOPTRANSMISSION, tr("Desktop sharing disabled"));
    }
}

void MainWindow::slotMeEnableTTS(bool checked/*=false*/)
{
    if(checked)
    {
        ttSettings->setValue(SETTINGS_TTS_ENABLE, true);
        addTextToSpeechMessage(tr("Text-To-Speech enabled"));
    }
    else
    {
        ttSettings->setValue(SETTINGS_TTS_ENABLE, false);
        addTextToSpeechMessage(tr("Text-To-Speech disabled"));
    }
    slotUpdateUI();
}

void MainWindow::slotMeEnableSounds(bool checked/*=false*/)
{
    if(checked)
    {
        ttSettings->setValue(SETTINGS_SOUNDEVENT_ENABLE, true);
        addTextToSpeechMessage(TTS_MENU_ACTIONS, tr("Sound events enabled"));
    }
    else
    {
        ttSettings->setValue(SETTINGS_SOUNDEVENT_ENABLE, false);
        addTextToSpeechMessage(TTS_MENU_ACTIONS, tr("Sound events disabled"));
    }
    slotUpdateUI();
}

void MainWindow::slotUsersViewUserInformationGrid(bool /*checked =false */)
{
    slotUsersViewUserInformation(ui.channelsWidget->selectedUser());
}

void MainWindow::slotUsersSpeakUserInformationGrid(bool /*checked =false */)
{
    User user;
    if(ui.channelsWidget->getUser(ui.channelsWidget->selectedUser(), user))
        slotUsersSpeakUserInformation(ui.channelsWidget->selectedUser());
    else
        slotUsersSpeakUserInformation(ui.channelsWidget->selectedChannel(true));
}

void MainWindow::slotUsersMessagesGrid(bool /*checked =false */)
{
    int userid = ui.channelsWidget->selectedUser();
    slotUsersMessages(userid);
}

void MainWindow::slotUsersMuteVoiceGrid(bool checked /*=false */)
{
    QString listuser;
    foreach(int userid, ui.channelsWidget->selectedUsers())
    {
        User user;
        TT_GetUser(ttInst, userid, &user);
        listuser += getDisplayName(user) + ", ";
        slotUsersMuteVoice(userid, checked);
    }
    listuser.chop(2);
    QString ttsevent;
    if (checked == true)
        ttsevent = tr("Voice for %1 disabled").arg(listuser);
    else
        ttsevent = tr("Voice for %1 enabled").arg(listuser);
    addTextToSpeechMessage(TTS_MENU_ACTIONS, ttsevent);
}

void MainWindow::slotUsersMuteMediaFileGrid(bool checked /*=false */)
{
    QString listuser;
    foreach(int userid, ui.channelsWidget->selectedUsers())
    {
        User user;
        TT_GetUser(ttInst, userid, &user);
        listuser += getDisplayName(user) + ", ";
        slotUsersMuteMediaFile(userid, checked);
    }
    listuser.chop(2);
    QString ttsevent;
    if (checked == true)
        ttsevent = tr("Media files for %1 disabled").arg(listuser);
    else
        ttsevent = tr("Media files for %1 enabled").arg(listuser);
    addTextToSpeechMessage(TTS_MENU_ACTIONS, ttsevent);
}

void MainWindow::slotUsersVolumeGrid(bool /*checked =false */)
{
    int userid = ui.channelsWidget->selectedUser();
    slotUsersVolume(userid);
}

void MainWindow::slotUsersMuteVoiceAll(bool checked /*=false */)
{
    TT_SetSoundOutputMute(ttInst, checked);
    if(checked == true)
    {
        playSoundEvent(SOUNDEVENT_MUTEALLON);
        addTextToSpeechMessage(TTS_MENU_ACTIONS, tr("Master volume disabled"));
    }
    else
    {
        playSoundEvent(SOUNDEVENT_MUTEALLOFF);
        addTextToSpeechMessage(TTS_MENU_ACTIONS, tr("Master volume enabled"));
    }
}

void MainWindow::slotUsersOpGrid(bool /*checked =false */)
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
        slotUsersKickBan(u);
}

void MainWindow::slotUsersKickFromServer(bool /*checked =false */)
{
    foreach(User u, ui.channelsWidget->getSelectedUsers())
        slotUsersKick(u.nUserID, 0);
}

void MainWindow::slotUsersKickBanFromServer(bool /*checked =false */)
{
    foreach(User u, ui.channelsWidget->getSelectedUsers())
    {
        u.nChannelID = 0; //ban from server
        slotUsersKickBan(u);
    }
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
        addStatusMsg(STATUSBAR_BYPASS, QString(tr("%1 granted desktop access")
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
    std::for_each(users.begin(), users.end(), std::bind(incVolume, _1, STREAMTYPE_VOICE));
    for(int i=0; i<users.size(); i++)
    {
        User user;
        TT_GetUser(ttInst, users.value(i), &user);
        addTextToSpeechMessage(TTS_MENU_ACTIONS, tr("Voice volume for %1 increased to %2%").arg(getDisplayName(user)).arg(refVolumeToPercent(user.nVolumeVoice)));
    }
    slotUpdateUI();
}

void MainWindow::slotUsersAdvancedDecVolumeVoice()
{
    userids_t users = ui.channelsWidget->selectedUsers();
    std::for_each(users.begin(), users.end(), std::bind(decVolume, _1, STREAMTYPE_VOICE));
    for(int i=0; i<users.size(); i++)
    {
        User user;
        TT_GetUser(ttInst, users.value(i), &user);
        addTextToSpeechMessage(TTS_MENU_ACTIONS, tr("Voice volume for %1 decreased to %2%").arg(getDisplayName(user)).arg(refVolumeToPercent(user.nVolumeVoice)));
    }
    slotUpdateUI();
}

void MainWindow::slotUsersAdvancedIncVolumeMediaFile()
{
    userids_t users = ui.channelsWidget->selectedUsers();
    std::for_each(users.begin(), users.end(), std::bind(incVolume, _1, STREAMTYPE_MEDIAFILE_AUDIO));
    for(int i=0; i<users.size(); i++)
    {
        User user;
        TT_GetUser(ttInst, users.value(i), &user);
        addTextToSpeechMessage(TTS_MENU_ACTIONS, tr("Media files volume for %1 increased to %2%").arg(getDisplayName(user)).arg(refVolumeToPercent(user.nVolumeMediaFile)));
    }
    slotUpdateUI();
}

void MainWindow::slotUsersAdvancedDecVolumeMediaFile()
{
    userids_t users = ui.channelsWidget->selectedUsers();
    std::for_each(users.begin(), users.end(), std::bind(decVolume, _1, STREAMTYPE_MEDIAFILE_AUDIO));
    for(int i=0; i<users.size(); i++)
    {
        User user;
        TT_GetUser(ttInst, users.value(i), &user);
        addTextToSpeechMessage(TTS_MENU_ACTIONS, tr("Media files volume for %1 decreased to %2%").arg(getDisplayName(user)).arg(refVolumeToPercent(user.nVolumeMediaFile)));
    }
    slotUpdateUI();
}

void MainWindow::slotUsersAdvancedStoreForMove(int userid /*= 0*/)
{
    if(userid == 0)
        m_moveusers = ui.channelsWidget->selectedUsers();
    else
        m_moveusers.append(userid);
    QString listuser;
    for(int i=0; i<m_moveusers.size(); i++)
    {
        User user;
        TT_GetUser(ttInst, m_moveusers.value(i), &user);
        listuser += getDisplayName(user) + ", ";
    }
    listuser.chop(2);
    addTextToSpeechMessage(TTS_MENU_ACTIONS, tr("%1 selected for move").arg(listuser));
    slotUpdateUI();
}

void MainWindow::slotUsersAdvancedMoveUsers()
{
    int chanid = ui.channelsWidget->selectedChannel(true);
    if (chanid > 0)
        moveUsersToChannel(chanid);
}

void MainWindow::slotUsersAdvancedMoveUsersDialog()
{
    MoveUsersDlg dlg(ui.channelsWidget->getUsers(), ui.channelsWidget->getChannels());
    if (dlg.exec() == QDialog::Accepted)
    {
        m_moveusers = dlg.getSelectedUserIds();
        moveUsersToChannel(dlg.getSelectedChannelId());
    }
}

void MainWindow::moveUsersToChannel(int chanid)
{
    for (auto userid : m_moveusers)
        TT_DoMoveUser(ttInst, userid, chanid);

    Channel chan;
    ui.channelsWidget->getChannel(chanid, chan);
    QString usersmoved;
    if(chan.nParentID == 0)
    {
        QString rootchan = tr("root");
        usersmoved = tr("Selected users has been moved to channel %1").arg(rootchan);
    }
    else
    {
        usersmoved = tr("Selected users has been moved to channel %1").arg(_Q(chan.szName));
    }
    addTextToSpeechMessage(TTS_MENU_ACTIONS, usersmoved);
    slotUpdateUI();
    m_moveusers.clear();
}

void MainWindow::slotUsersAdvancedChanMsgAllowed(bool checked/*=false*/)
{
    if (QObject::sender() == ui.actionAllowChannelTextMessages)
        toggleAllowStreamType(checked, STREAMTYPE_CHANNELMSG);
    else
        toggleAllowStreamTypeForAll(checked, STREAMTYPE_CHANNELMSG);
}

void MainWindow::slotUsersAdvancedVoiceAllowed(bool checked/*=false*/)
{
    if (QObject::sender() == ui.actionAllowVoiceTransmission)
        toggleAllowStreamType(checked, STREAMTYPE_VOICE);
    else
        toggleAllowStreamTypeForAll(checked, STREAMTYPE_VOICE);
}

void MainWindow::slotUsersAdvancedVideoAllowed(bool checked/*=false*/)
{
    if (QObject::sender() == ui.actionAllowVideoTransmission)
        toggleAllowStreamType(checked, STREAMTYPE_VIDEOCAPTURE);
    else
        toggleAllowStreamTypeForAll(checked, STREAMTYPE_VIDEOCAPTURE);
}

void MainWindow::slotUsersAdvancedDesktopAllowed(bool checked/*=false*/)
{
    if (QObject::sender() == ui.actionAllowDesktopTransmission)
        toggleAllowStreamType(checked, STREAMTYPE_DESKTOP);
    else
        toggleAllowStreamTypeForAll(checked, STREAMTYPE_DESKTOP);
}

void MainWindow::slotUsersAdvancedMediaFileAllowed(bool checked/*=false*/)
{
    if (QObject::sender() == ui.actionAllowMediaFileTransmission)
        toggleAllowStreamType(checked, STREAMTYPE_MEDIAFILE);
    else
        toggleAllowStreamTypeForAll(checked, STREAMTYPE_MEDIAFILE);
}

void MainWindow::slotUsersAdvancedRelayUserVoice(bool checked/*=false*/)
{
    User user;
    if (!ui.channelsWidget->getSelectedUser(user))
        return;

    if (checked && user.nChannelID != m_mychannel.nChannelID &&
        (user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_VOICE) == SUBSCRIBE_NONE)
    {
        if (QMessageBox::information(this, MENUTEXT(ui.actionRelayVoiceStream->text()),
            tr("To relay voice stream from other channel you must enable subscription \"Intercept Voice\".\n"
                "Do you wish to do this now?"), QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
        {
            slotUsersSubscriptionsInterceptVoice(checked);
        }
    }

    relayAudioStream(user.nUserID, STREAMTYPE_VOICE, checked);
}

void MainWindow::slotUsersAdvancedRelayUserMediaFile(bool checked/* = false*/)
{
    User user;
    if (!ui.channelsWidget->getSelectedUser(user))
        return;

    if (checked && user.nChannelID != m_mychannel.nChannelID &&
        (user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_MEDIAFILE) == SUBSCRIBE_NONE)
    {
        if (QMessageBox::information(this, MENUTEXT(ui.actionRelayMediaFileStream->text()),
            tr("To relay media file stream from other channel you must enable subscription \"Intercept Media File\".\n"
                "Do you wish to do this now?"), QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
        {
            slotUsersSubscriptionsInterceptMediaFile(checked);
        }
    }

    relayAudioStream(user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO, checked);
}

void MainWindow::slotChannelsCreateChannel(bool /*checked =false */)
{
    Channel chan = {};

    if(ui.channelsWidget->selectedChannel() && (TT_GetMyUserRights(ttInst) & USERRIGHT_MODIFY_CHANNELS))
        chan.nParentID = ui.channelsWidget->selectedChannel();
    else
    {
        chan.nParentID = m_mychannel.nChannelID;
        if(!chan.nParentID)
            chan.nParentID = TT_GetRootChannelID(ttInst);
    }

    ChannelDlg dlg(ChannelDlg::CHANNEL_CREATE, chan, this);
    if(!dlg.exec())return;
    chan = dlg.GetChannel();

    //only admins can create channels. Users can only create a new channel
    //if they at the same time join it.
    if (!dlg.joinChannel())
    {
        if(TT_DoMakeChannel(ttInst, &chan)<0)
            QMessageBox::critical(this, MENUTEXT(ui.actionCreateChannel->text()), 
            tr("Failed to issue command to create channel"));
    }
    else
    {
        int cmdid = TT_DoJoinChannel(ttInst, &chan);
        if(cmdid>0)
        {
            m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
            m_last_channel = chan;
            m_channel_passwd[CHANNELID_TEMPPASSWORD] = _Q(chan.szPassword);
        }
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
    QMessageBox answer;
    answer.setText(tr("Are you sure you want to delete channel \"%1\"?").arg(_Q(buff)));
    QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
    QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
    Q_UNUSED(YesButton);
    answer.setIcon(QMessageBox::Information);
    answer.setWindowTitle(MENUTEXT(ui.actionDeleteChannel->text()));
    answer.exec();
    if(answer.clickedButton() == NoButton)
        return;

    if(TT_DoRemoveChannel(ttInst, chanid)<0)
        QMessageBox::critical(this, MENUTEXT(ui.actionDeleteChannel->text()), 
        tr("Failed to issue command to delete channel"));
}

void MainWindow::slotChannelsJoinChannel(bool /*checked=false*/)
{
    Channel chan;
    DoubleClickChannelAction dbClickAct = DoubleClickChannelAction(ttSettings->value(SETTINGS_DISPLAY_CHANDBCLICK, SETTINGS_DISPLAY_CHANDBCLICK_DEFAULT).toUInt());
    if (!ui.channelsWidget->getSelectedChannel(chan))
        return;

    if (chan.nChannelID == m_mychannel.nChannelID && ((dbClickAct & ACTION_LEAVE) == ACTION_LEAVE || QObject::sender() == ui.actionJoinChannel))
    {
        slotChannelsLeaveChannel();
    }

    if (chan.nChannelID != TT_GetMyChannelID(ttInst) && ((dbClickAct & ACTION_JOIN) == ACTION_JOIN || QObject::sender() == ui.actionJoinChannel))
    {
        m_last_channel = chan;
        QString password = m_channel_passwd[chan.nChannelID];
        if(chan.bPassword)
        {
            bool ok = false;
            PasswordDialog passDialog(MENUTEXT(ui.actionJoinChannel->text()), tr("Specify password"), password, this);
            ok = (passDialog.exec() == QDialog::Accepted);
            if (!ok)
                return;
            password = passDialog.getPassword();
        }
        m_channel_passwd[chan.nChannelID] = password;

        int cmdid = TT_DoJoinChannelByID(ttInst, chan.nChannelID, _W(password));
        if(cmdid>0)
        {
            m_commands.insert(cmdid, CMD_COMPLETE_JOINCHANNEL);
        }
        else
            QMessageBox::critical(this, MENUTEXT(ui.actionJoinChannel->text()),
                                  tr("Failed to issue command to join channel"));
    }
}

void MainWindow::slotChannelsLeaveChannel(bool /*checked=false*/)
{
    Channel chan;
    if (!ui.channelsWidget->getChannel(m_mychannel.nChannelID, chan))
        return;

    int cmdid = TT_DoLeaveChannel(ttInst);
    m_commands.insert(cmdid, CMD_COMPLETE_LEAVECHANNEL);
    return;
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

void MainWindow::slotChannelsSpeakChannelInformationGrid(bool /*checked =false */)
{
    slotUsersSpeakUserInformation(m_mychannel.nChannelID);
}

void MainWindow::slotChannelsSpeakChannelStatusGrid(bool /*checked =false */)
{
    slotChannelsSpeakChannelStatus();
}

void MainWindow::slotChannelsSpeakChannelStatus()
{
    QString speakList, voice, mediafile, video, desktop;

    QVector<int> users = ui.channelsWidget->getUsersInChannel(m_mychannel.nChannelID);
    QVector<QString> voice1, mediafile1, video1, desktop1;
    for (int i=0;i<users.size();i++)
    {
        User user = {};
        ui.channelsWidget->getUser(users[i], user);
        if (((user.uUserState & USERSTATE_VOICE) != USERSTATE_NONE) ||
            ((user.nUserID == TT_GetMyUserID(ttInst) && isMyselfTalking()) &&
             userCanVoiceTx(TT_GetMyUserID(ttInst), m_mychannel)))
            voice1.push_back(getDisplayName(user));
        if(user.uUserState & USERSTATE_MEDIAFILE)
            mediafile1.push_back(getDisplayName(user));
        if(user.uUserState & USERSTATE_VIDEOCAPTURE)
            video1.push_back(getDisplayName(user));
        if(user.uUserState & USERSTATE_DESKTOP)
            desktop1.push_back(getDisplayName(user));
    }
    if(voice1.count() > 0)
    {
        voice += tr("Talking") + ": ";
        int vo = 0;
        while(vo<voice1.count())
        {
            voice += voice1[vo] + ", ";
            vo++;
        }
        speakList += voice;
    }
    if(mediafile1.count() > 0)
    {
        mediafile += tr("Streaming") + ": ";
        int mf = 0;
        while(mf<mediafile1.count())
        {
            mediafile += mediafile1[mf] + ", ";
            mf++;
        }
        speakList += mediafile;
    }
    if(video1.count() > 0)
    {
        video += tr("Webcam") + ": ";
        int vi = 0;
        while(vi<video1.count())
        {
            video += video1[vi] + ", ";
            vi++;
        }
        speakList += video;
    }
    if(desktop1.count() > 0)
    {
        desktop += tr("Desktop") + ": ";
        int de = 0;
        while(de<desktop1.count())
        {
            desktop += desktop1[de] + ", ";
            de++;
        }
        speakList += desktop;
    }
    if(speakList.isEmpty())
        speakList = tr("Nobody is active in this channel");
    else
        speakList.chop(2);
    addTextToSpeechMessage(speakList);
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
    if (!checked)
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

void MainWindow::startStreamMediaFile()
{
    QString fileName = ttSettings->value(QString(SETTINGS_STREAMMEDIA_FILENAME).arg(0)).toString();
#if defined(Q_OS_WINDOWS)
    fileName = fileName.remove('"');
#endif

    m_mfp_videocodec = {};
    m_mfp_videocodec.nCodec = (Codec)ttSettings->value(SETTINGS_STREAMMEDIA_CODEC).toInt();
    switch(m_mfp_videocodec.nCodec)
    {
    case WEBM_VP8_CODEC :
        m_mfp_videocodec.webm_vp8.nRcTargetBitrate = ttSettings->value(SETTINGS_STREAMMEDIA_WEBMVP8_BITRATE).toInt();
        m_mfp_videocodec.webm_vp8.nEncodeDeadline = DEFAULT_WEBMVP8_DEADLINE;
        break;
    default :
        break;
    }

    m_mfp = {};
    AudioPreprocessorType apt = AudioPreprocessorType(ttSettings->value(SETTINGS_STREAMMEDIA_AUDIOPREPROCESSOR,
                                                      SETTINGS_STREAMMEDIA_AUDIOPREPROCESSOR_DEFAULT).toInt());
    m_mfp.audioPreprocessor = loadAudioPreprocessor(apt);
    m_mfp.bPaused = false;
    m_mfp.uOffsetMSec = ttSettings->value(SETTINGS_STREAMMEDIA_OFFSET, SETTINGS_STREAMMEDIA_OFFSET_DEFAULT).toUInt();
    if (!TT_StartStreamingMediaFileToChannelEx(ttInst, _W(fileName), &m_mfp, &m_mfp_videocodec))
    {
        QMessageBox::information(this,
                                 MENUTEXT(ui.actionStreamMediaFileToChannel->text()),
                                 QString(tr("Failed to stream media file %1").arg(fileName)));
        stopStreamMediaFile();
    }
    else
    {
        QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();
        m_statusmode |= STATUSMODE_STREAM_MEDIAFILE;
        m_statusmode &= ~STATUSMODE_STREAM_MEDIAFILE_PAUSED;
        if(ttSettings->value(SETTINGS_GENERAL_STREAMING_STATUS, SETTINGS_GENERAL_STREAMING_STATUS_DEFAULT).toBool() == true)
            statusmsg = QFileInfo(fileName).fileName();

        TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));
        ////since streaming video takes over webcam stream we show as 
        ////transmitting video
        //if(tx_mode & TRANSMIT_VIDEO)
        //    m_statusmode |= STATUSMODE_VIDEOTX;

        transmitOn(STREAMTYPE_MEDIAFILE);

        ui.actionPauseResumeStream->setEnabled(true);
        ui.actionPauseResumeStream->setText(tr("&Pause Stream"));
    }
}

void MainWindow::stopStreamMediaFile()
{
    TT_StopStreamingMediaFileToChannel(ttInst);

    ////clear video if not transmitting
    //if(TT_IsTransmitting(ttInst, TRANSMIT_VIDEO) == TRANSMIT_NONE)
    //    m_statusmode &= ~STATUSMODE_VIDEOTX;

    m_statusmode &= ~(STATUSMODE_STREAM_MEDIAFILE | STATUSMODE_STREAM_MEDIAFILE_PAUSED);

    QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();
    TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));

    ui.actionPauseResumeStream->setEnabled(false);
    ui.actionPauseResumeStream->setText(tr("Pause/Resume Stream"));
    m_mfp = {};

    slotUpdateUI();
}

void MainWindow::slotPauseResumeStream()
{
    QString statusmsg = ttSettings->value(SETTINGS_GENERAL_STATUSMESSAGE).toString();
    QString fileName = ttSettings->value(QString(SETTINGS_STREAMMEDIA_FILENAME).arg(0)).toString();
#if defined(Q_OS_WINDOWS)
    fileName = fileName.remove('"');
#endif
    if(ttSettings->value(SETTINGS_GENERAL_STREAMING_STATUS, SETTINGS_GENERAL_STREAMING_STATUS_DEFAULT).toBool() == true)
        statusmsg = QFileInfo(fileName).fileName();

    if (m_mfp.bPaused)
    {
        m_mfp.bPaused = false;
        if (!TT_UpdateStreamingMediaFileToChannel(ttInst, &m_mfp, &m_mfp_videocodec))
        {
            QMessageBox::critical(this, MENUTEXT(ui.actionPauseResumeStream->text()), tr("Failed to resume the stream"));
        }
        else
        {
            ui.actionPauseResumeStream->setText(tr("&Pause Stream"));
            m_statusmode |= STATUSMODE_STREAM_MEDIAFILE;
            m_statusmode &= ~STATUSMODE_STREAM_MEDIAFILE_PAUSED;
        }
    }
    else
    {
        m_mfp.bPaused = true;
        if (!TT_UpdateStreamingMediaFileToChannel(ttInst, &m_mfp, &m_mfp_videocodec))
        {
            QMessageBox::critical(this, MENUTEXT(ui.actionPauseResumeStream->text()), tr("Failed to pause the stream"));
        }
        else
        {
            ui.actionPauseResumeStream->setText(tr("&Resume Stream"));
            m_mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
            m_statusmode |= STATUSMODE_STREAM_MEDIAFILE_PAUSED;
            m_statusmode &= ~STATUSMODE_STREAM_MEDIAFILE;
        }
    }
    TT_DoChangeStatus(ttInst, m_statusmode, _W(statusmsg));
}

void MainWindow::updateMediaFileProgress(const MediaFileInfo& mfi)
{
    switch (mfi.nStatus)
    {
    case MFS_CLOSED :
    case MFS_ABORTED :
    case MFS_ERROR :
    case MFS_FINISHED :
        ui.durationLabel->setText(durationToString(0));
        ui.playbackTimeLabel->setText(durationToString(0));
        ui.audioLabel->setText("");
        ui.videoLabel->setText("");
        break;
    case MFS_PAUSED :
    case MFS_STARTED :
    case MFS_PLAYING :
        ui.durationLabel->setText(durationToString(mfi.uDurationMSec));
        ui.playbackTimeLabel->setText(durationToString(mfi.uElapsedMSec));
        ui.audioLabel->setText(getMediaAudioDescription(mfi.audioFmt));
        ui.videoLabel->setText(getMediaVideoDescription(mfi.videoFmt));
        break;
    }
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
    auto index = ui.filesView->currentIndex();
    int fileid = (int)m_proxyFilesModel->mapToSource(index).internalId();
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
    QStringList filenames = ui.filesView->selectedFiles();
    QItemSelectionModel* sel = ui.filesView->selectionModel();
    QModelIndexList files = sel->selectedRows();
    bool delete_ok = false;
    QMessageBox answer;
    QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
    QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
    Q_UNUSED(NoButton);
    answer.setIcon(QMessageBox::Information);
    answer.setWindowTitle(tr("Delete %1 files").arg(filenames.size()));
    if(filenames.size() == 1)
    {
        answer.setText(tr("Are you sure you want to delete \"%1\"?").arg(filenames[0]));
        answer.exec();
        if(answer.clickedButton() == YesButton)
            delete_ok = true;
        else
            delete_ok = false;
    }
    else if(filenames.size()>1)
    {
        answer.setText(tr("Are you sure you want to delete %1 file(s)?").arg(filenames.size()));
        answer.exec();
        if(answer.clickedButton() == YesButton)
            delete_ok = true;
        else
            delete_ok = false;
    }

    if (delete_ok)
    {
        for (QModelIndex index : files)
        {
            index = m_proxyFilesModel->mapToSource(index);
            TT_DoDeleteFile(ttInst, channelid, index.internalId());
        }
    }
}

void MainWindow::slotChannelsGenerateTTUrl(bool checked/*=false*/)
{
    Q_UNUSED(checked);

    QClipboard *cp = QApplication::clipboard();
    QString link = QString("%5//%1?tcpport=%2&udpport=%3&encrypted=%4").arg(m_host.ipaddr).arg(m_host.tcpport).arg(m_host.udpport).arg(m_host.encrypted).arg(TTLINK_PREFIX);

    bool ok = false;
    LoginInfoDialog loginDialog(tr("Share channel"), tr("Specify User Account"), m_host.username, m_host.password, this);
    ok = (loginDialog.exec() == QDialog::Accepted);
    if (ok && loginDialog.getUsername().size() > 0)
    {
        QString username = QUrl::toPercentEncoding(loginDialog.getUsername());
        link += QString("&username=%1").arg(username);
        if (loginDialog.getPassword().size() > 0)
        {
            QString password = QUrl::toPercentEncoding(loginDialog.getPassword());
            link += QString("&password=%1").arg(password);
        }
    }

    int chanid = ui.channelsWidget->selectedChannel(true);
    Channel chan;
    TTCHAR chanpath[TT_STRLEN];
    if (ui.channelsWidget->getChannel(chanid, chan) && TT_GetChannelPath(ttInst, chanid, chanpath))
    {
        QString channel = QUrl::toPercentEncoding(_Q(chanpath));
        link += QString("&channel=%1").arg(channel);
        QString chpasswd = m_channel_passwd[chan.nChannelID];
        if (chan.bPassword)
        {
            PasswordDialog channelPassDialog(tr("Share channel"), tr("Type password of channel:"), chpasswd, this);
            ok = (channelPassDialog.exec() == QDialog::Accepted);
            if (ok && channelPassDialog.getPassword().size() > 0)
                chpasswd = channelPassDialog.getPassword();
        }

        if (chpasswd.size())
        {
            chpasswd = QUrl::toPercentEncoding(chpasswd);
            link += QString("&chanpasswd=%1").arg(chpasswd);
        }
    }
    cp->setText(link);
    addStatusMsg(STATUSBAR_BYPASS, tr("Link copied to clipboard"));
    addTextToSpeechMessage(tr("Link copied to clipboard"));
}

void MainWindow::slotFilesContextMenu(const QPoint &/* pos*/)   
{
    QMenu menu(this);
    QMenu* sortMenu = menu.addMenu(tr("Sort By..."));
    QString asc = tr("Ascending"), desc = tr("Descending");
    QAction* sortName = new QAction(sortMenu);
    sortName->setText(tr("&Name (%1)").arg(m_proxyFilesModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortName->setCheckable(true);
    const QString name = "name";
    sortName->setChecked((ttSettings->value(SETTINGS_DISPLAY_FILESLIST_SORT, SETTINGS_DISPLAY_FILESLIST_SORT_DEFAULT).toString() == name)?true:false);
    sortMenu->addAction(sortName);
    QAction* sortSize = new QAction(sortMenu);
    sortSize->setText(tr("&Size (%1)").arg(m_proxyFilesModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortSize->setCheckable(true);
    const QString size = "size";
    sortSize->setChecked((ttSettings->value(SETTINGS_DISPLAY_FILESLIST_SORT, SETTINGS_DISPLAY_FILESLIST_SORT_DEFAULT).toString() == size)?true:false);
    sortMenu->addAction(sortSize);
    QAction* sortOwner = new QAction(sortMenu);
    sortOwner->setText(tr("&Owner (%1)").arg(m_proxyFilesModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortOwner->setCheckable(true);
    const QString owner = "owner";
    sortOwner->setChecked((ttSettings->value(SETTINGS_DISPLAY_FILESLIST_SORT, SETTINGS_DISPLAY_FILESLIST_SORT_DEFAULT).toString() == owner)?true:false);
    sortMenu->addAction(sortOwner);
    QAction* sortUpload = new QAction(sortMenu);
    sortUpload->setText(tr("&Upload Date (%1)").arg(m_proxyFilesModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortUpload->setCheckable(true);
    const QString uploadstr = "upload";
    sortUpload->setChecked((ttSettings->value(SETTINGS_DISPLAY_FILESLIST_SORT, SETTINGS_DISPLAY_FILESLIST_SORT_DEFAULT).toString() == uploadstr)?true:false);
    sortMenu->addAction(sortUpload);
    QAction* upload = menu.addAction(ui.actionUploadFile->text());
    QAction* download = menu.addAction(ui.actionDownloadFile->text());
    QAction* del = menu.addAction(ui.actionDeleteFile->text());
    auto index = ui.filesView->currentIndex();
    upload->setEnabled(m_myuseraccount.uUserRights & USERRIGHT_UPLOAD_FILES);
    download->setEnabled(index.isValid() && m_myuseraccount.uUserRights & USERRIGHT_DOWNLOAD_FILES);
    del->setEnabled(index.isValid());

    if (QAction* action = menu.exec(QCursor::pos()))
    {
        auto sortToggle = m_proxyFilesModel->sortOrder() == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        if (action == sortName)
        {
            ui.filesView->horizontalHeader()->setSortIndicator(COLUMN_INDEX_NAME, m_proxyFilesModel->sortColumn() == COLUMN_INDEX_NAME ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValue(SETTINGS_DISPLAY_FILESLIST_SORT, name);
        }
        else if (action == sortSize)
        {
            ui.filesView->horizontalHeader()->setSortIndicator(COLUMN_INDEX_SIZE, m_proxyFilesModel->sortColumn() == COLUMN_INDEX_SIZE ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValue(SETTINGS_DISPLAY_FILESLIST_SORT, size);
        }
        else if (action == sortOwner)
        {
            ui.filesView->horizontalHeader()->setSortIndicator(COLUMN_INDEX_OWNER, m_proxyFilesModel->sortColumn() == COLUMN_INDEX_OWNER? sortToggle : Qt::AscendingOrder);
            ttSettings->setValue(SETTINGS_DISPLAY_FILESLIST_SORT, owner);
        }
        else if (action == sortUpload)
        {
            ui.filesView->horizontalHeader()->setSortIndicator(COLUMN_INDEX_UPLOADED, m_proxyFilesModel->sortColumn() == COLUMN_INDEX_UPLOADED? sortToggle : Qt::AscendingOrder);
            ttSettings->setValue(SETTINGS_DISPLAY_FILESLIST_SORT, uploadstr);
        }
        else if (action == upload)
            slotChannelsUploadFile();
        else if (action == download)
            slotChannelsDownloadFile();
        else if (action == del)
            slotChannelsDeleteFile();
    }
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
        UserAccountDlg dlg(UserAccountDlg::USER_READONLY, m_myuseraccount, this);
        dlg.exec();
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
        connect(m_onlineusersdlg, &QDialog::finished,
                this, &MainWindow::slotClosedOnlineUsersDlg);
        m_onlineusersdlg->setAttribute(Qt::WA_DeleteOnClose);
    }

    connect(this, &MainWindow::userLogin, m_onlineusersdlg,
            &OnlineUsersDlg::slotUserLoggedIn);
    connect(this, &MainWindow::userLogout, m_onlineusersdlg,
            &OnlineUsersDlg::slotUserLoggedOut);
    connect(this, &MainWindow::userUpdate, m_onlineusersdlg,
            &OnlineUsersDlg::slotUserUpdate);
    connect(this, &MainWindow::userJoined, m_onlineusersdlg,
            &OnlineUsersDlg::slotUserJoin);
    connect(this, &MainWindow::userLeft, m_onlineusersdlg,
            &OnlineUsersDlg::slotUserLeft);

    connect(m_onlineusersdlg, &OnlineUsersDlg::viewUserInformation,
            this, &MainWindow::slotUsersViewUserInformation);
    connect(m_onlineusersdlg, &OnlineUsersDlg::sendUserMessage,
            this, &MainWindow::slotUsersMessages);
    connect(m_onlineusersdlg, &OnlineUsersDlg::muteUser,
            this, &MainWindow::slotUsersMuteVoice);
    connect(m_onlineusersdlg, &OnlineUsersDlg::changeUserVolume,
            this, &MainWindow::slotUsersVolume);
    connect(m_onlineusersdlg, &OnlineUsersDlg::opUser,
            this, &MainWindow::slotUsersOp);
    connect(m_onlineusersdlg, &OnlineUsersDlg::kickUser,
            this, &MainWindow::slotUsersKick);
    connect(m_onlineusersdlg, &OnlineUsersDlg::kickbanUser,
            this, &MainWindow::slotUsersKickBan);
    connect(m_onlineusersdlg, &OnlineUsersDlg::moveUser,
            this, &MainWindow::slotUsersAdvancedStoreForMove);
    m_onlineusersdlg->show();
}

void MainWindow::slotServerBroadcastMessage(bool /*checked=false*/)
{
    bool ok = false;
    QInputDialog inputDialog;
    inputDialog.setOkButtonText(tr("&OK"));
    inputDialog.setCancelButtonText(tr("&Cancel"));
    inputDialog.setInputMode(QInputDialog::TextInput);
    inputDialog.setWindowTitle(MENUTEXT(ui.actionBroadcastMessage->text()));
    inputDialog.setLabelText(tr("Message to broadcast:"));
    ok = inputDialog.exec();
    QString bcast = inputDialog.textValue();
    if (!ok || bcast.isEmpty())
        return;

    MyTextMessage msg;
    msg.nMsgType = MSGTYPE_BROADCAST;
    msg.nFromUserID = TT_GetMyUserID(ttInst);
    sendTextMessage(msg, bcast);
    addTextToSpeechMessage(TTS_USER_TEXTMSG_BROADCAST_SEND, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_BROADCASTMSGSEND, {{"{message}", bcast}}));
}

void MainWindow::slotServerServerProperties(bool /*checked =false */)
{
    ServerPropertiesDlg dlg(this);
    if(!dlg.exec())return;
}

void MainWindow::slotServerSaveConfiguration(bool /*checked =false */)
{
    int cmdid = TT_DoSaveConfig(ttInst);
    m_commands[cmdid] =  CMD_COMPLETE_SAVECONFIG;
}

void MainWindow::slotServerServerStatistics(bool /*checked=false*/)
{
    if(m_serverstatsdlg)
        m_serverstatsdlg->activateWindow();
    else
    {
        m_serverstatsdlg = new ServerStatisticsDlg();
        connect(this, &MainWindow::cmdSuccess, m_serverstatsdlg,
                &ServerStatisticsDlg::slotCmdSuccess);
        connect(m_serverstatsdlg, &QDialog::finished,
                this, &MainWindow::slotClosedServerStatsDlg);
        connect(this, &MainWindow::serverStatistics,
                m_serverstatsdlg, &ServerStatisticsDlg::slotUpdateStats);
        m_serverstatsdlg->setAttribute(Qt::WA_DeleteOnClose);
        m_serverstatsdlg->show();
    }
}

void MainWindow::slotHelpResetPreferences(bool /*checked=false*/)
{
    QMessageBox answer;
    answer.setText(tr("Are you sure you want to delete your existing settings?"));
    QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
    QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
    Q_UNUSED(NoButton);
    answer.setIcon(QMessageBox::Question);
    answer.setWindowTitle(MENUTEXT(ui.actionResetPreferencesToDefault->text()));
    answer.exec();
    if (answer.clickedButton() == YesButton)
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
    if(ttSettings->value(SETTINGS_CONNECTION_AUTOCONNECT, SETTINGS_CONNECTION_AUTOCONNECT_DEFAULT).toBool() &&
        getServerEntry(0, lasthost, true))
    {
        m_host = lasthost;
        connectToServer();
    }
}

void MainWindow::slotUsersViewUserInformation(int userid)
{
    UserInfoDlg dlg(userid, this);
    dlg.exec();
}

void MainWindow::slotUsersSpeakUserInformation(int id)
{
    QString speakList;

    if (id > 0 && id == ui.channelsWidget->selectedUser())
    {
        User user;
        if(!ui.channelsWidget->getUser(id, user))
            return;

        QString userString, voice = tr("Talking"), mute = tr("Mute"), mediaFile = tr("Streaming"), muteMediaFile = tr("Mute media file"), videoCapture = tr("Webcam"), desktop = tr("Desktop");
        speakList += QString("%1: ").arg(getDisplayName(user));
        if (user.uUserType & USERTYPE_ADMIN)
        {
            userString = ((user.nStatusMode & STATUSMODE_FEMALE)?tr("Administrator", "For female"):tr("Administrator", "For male and neutral"));
        }
        else
        {
            userString = ((user.nStatusMode & STATUSMODE_FEMALE)?tr("User", "For female"):tr("User", "For male and neutral"));
        }
        speakList += userString;

        QString status;

        if(m_moveusers.indexOf(user.nUserID) >= 0)
           speakList += ", " + ((user.nStatusMode & STATUSMODE_FEMALE)?tr("Selected for move", "For female"):tr("Selected for move", "For male and neutral"));

        if(TT_IsChannelOperator(ttInst, user.nUserID, user.nChannelID))
            speakList += ", " + ((user.nStatusMode & STATUSMODE_FEMALE)?tr("Channel operator", "For female"):tr("Channel operator", "For male and neutral"));

        if((user.uUserState & USERSTATE_VOICE) || (user.nUserID == TT_GetMyUserID(ttInst) && isMyselfTalking() == TRUE))
            speakList += ", " + voice;

        switch(user.nStatusMode & STATUSMODE_MODE)
        {
        case STATUSMODE_AVAILABLE :
            status = ((user.nStatusMode & STATUSMODE_FEMALE)?tr("Available", "For female"):tr("Available", "For male and neutral"));
            break;
        case STATUSMODE_AWAY :
            status = ((user.nStatusMode & STATUSMODE_FEMALE)?tr("Away", "For female"):tr("Away", "For male and neutral"));
            break;
        case STATUSMODE_QUESTION :
            status = tr("Question");
            break;
        }
        if(status.size())
            speakList += ", " + status;
        if(user.uUserState & USERSTATE_MUTE_VOICE)
            speakList += ", " + mute;
        if((user.uUserState & USERSTATE_MEDIAFILE) ||
           (user.nStatusMode & STATUSMODE_STREAM_MEDIAFILE))
            speakList += ", " + mediaFile;
        if(user.uUserState & USERSTATE_MUTE_MEDIAFILE)
            speakList += ", " + muteMediaFile;
        if((user.uUserState & USERSTATE_VIDEOCAPTURE) ||
           (user.nStatusMode & STATUSMODE_VIDEOTX))
            speakList += ", " + videoCapture;
        if((user.uUserState & USERSTATE_DESKTOP) ||
           (user.nStatusMode & STATUSMODE_DESKTOP))
            speakList += ", " + desktop;
    }
    else if (id > 0 && (id == ui.channelsWidget->selectedChannel(true) || id == m_mychannel.nChannelID))
    {
        Channel chan;
        if(!ui.channelsWidget->getChannel(id, chan))
            return;

        QString channel = tr("Channel"), passwd = tr("Password protected"), classroom = tr("Classroom"), topic, rootChan = tr("root"), hidden = tr("Hidden");
        if(chan.nChannelID == TT_GetRootChannelID(ttInst))
        {
            speakList += QString("%1: ").arg(_Q(m_srvprop.szServerName));
        }
        else
            speakList += QString("%1: ").arg(_Q(chan.szName));
        if(chan.nChannelID>0 && TT_GetRootChannelID(ttInst) == chan.nChannelID)
            channel += " " + rootChan;
        topic = _Q(chan.szTopic);

        speakList += channel;
        if(chan.uChannelType & CHANNEL_CLASSROOM)
            speakList += ", " + classroom;
        if(chan.uChannelType & CHANNEL_HIDDEN)
            speakList += ", " + hidden;
        if(chan.bPassword)
            speakList += ", " + passwd;
        if (topic.size())
            speakList += ", " + QString(tr("Topic: %1").arg(topic));

        int usersCount = ui.channelsWidget->getUsersInChannel(chan.nChannelID).size();
        if (usersCount > 0)
            speakList += ", " + QString(tr("%1 users").arg(usersCount));

        if ((id == m_mychannel.nChannelID || TT_GetMyUserType(ttInst) & USERTYPE_ADMIN) && (m_filesmodel->rowCount() > 0))
            speakList += ", " + QString(tr("%1 files").arg(m_filesmodel->rowCount()));
    }
    addTextToSpeechMessage(speakList);
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
    TT_PumpMessage(ttInst, CLIENTEVENT_USER_STATECHANGE, userid);
    slotUpdateUI();
}

void MainWindow::slotUsersOp(int userid, int chanid)
{
    bool op = (bool)TT_IsChannelOperator(ttInst, userid, chanid);

    if ((TT_GetMyUserRights(ttInst) & USERRIGHT_OPERATOR_ENABLE) != USERRIGHT_NONE) //don't need password in this case
        TT_DoChannelOp(ttInst, userid, chanid, !op);
    else
    {
        Channel chan = {};
        ui.channelsWidget->getChannel(chanid, chan);

        PasswordDialog passDialog(MENUTEXT(ui.actionOp->text()), tr("Specify password"), _Q(chan.szOpPassword), this);
        if (passDialog.exec() == QDialog::Accepted)
        {
            QString oppasswd = passDialog.getPassword();
            TT_DoChannelOpEx(ttInst, userid, chanid, _W(oppasswd), !op);
        }
    }
}

void MainWindow::slotUsersKick(int userid, int chanid)
{
    bool confirm = true;
    if (userid == TT_GetMyUserID(ttInst))
    {
        QMessageBox answer;
        answer.setText(tr("Are you sure you want to kick yourself?"));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        Q_UNUSED(NoButton);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle((QObject::sender() == ui.actionKickFromChannel?ui.actionKickFromChannel->text():ui.actionKickFromServer->text()));
        answer.exec();
        if(answer.clickedButton() != YesButton)
            confirm = false;
    }
    if (confirm == true)
        TT_DoKickUser(ttInst, userid, chanid);
}

void MainWindow::slotUsersKickBan(const User& user)
{
    bool confirm = true;
    if (user.nUserID == TT_GetMyUserID(ttInst))
    {
        QMessageBox answer;
        answer.setText(tr("Are you sure you want to kick and ban yourself?"));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        Q_UNUSED(NoButton);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle((QObject::sender() == ui.actionKickAndBanFromChannel?ui.actionKickAndBanFromChannel->text().remove('&'):ui.actionKickBan->text().remove('&')));
        answer.exec();
        if(answer.clickedButton() != YesButton)
            confirm = false;
    }
    if (confirm == true)
    {
        QStringList items = { tr("IP-address"), tr("Username") };
        bool ok = false;
        QInputDialog inputDialog;
        inputDialog.setOkButtonText(tr("&OK"));
        inputDialog.setCancelButtonText(tr("&Cancel"));
        inputDialog.setComboBoxItems(items);
        inputDialog.setComboBoxEditable(false);
        inputDialog.setWindowTitle(tr("Ban user #%1").arg(user.nUserID));
        if (user.nChannelID > 0)
            inputDialog.setLabelText(tr("Ban User From Channel"));
        else
            inputDialog.setLabelText(tr("Ban User From Server"));
        ok = inputDialog.exec();
        QString choice = inputDialog.textValue();
        if (ok)
        {
            if (ui.channelsWidget->getUser(user.nUserID).nUserID != 0)
            {
                //ban first since the user will otherwise have disappeared
                if (choice == items[0])
                    TT_DoBanUserEx(ttInst, user.nUserID, user.nChannelID > 0 ? BANTYPE_CHANNEL | BANTYPE_IPADDR : BANTYPE_IPADDR);
                else
                    TT_DoBanUserEx(ttInst, user.nUserID, user.nChannelID > 0 ? BANTYPE_CHANNEL | BANTYPE_USERNAME : BANTYPE_USERNAME);
                TT_DoKickUser(ttInst, user.nUserID, user.nChannelID);
            }
            else // ban offline user
            {
                BannedUser ban = {};
                TT_GetChannelPath(ttInst, user.nChannelID, ban.szChannelPath);
                COPY_TTSTR(ban.szUsername, _Q(user.szUsername));
                COPY_TTSTR(ban.szIPAddress, _Q(user.szIPAddress));
                COPY_TTSTR(ban.szNickname, _Q(user.szNickname));
                if (choice == items[0])
                {
                    if (versionSameOrLater(_Q(m_srvprop.szServerProtocolVersion), "5.13"))
                    {
                        bool ok;
                        QString ipaddr = QInputDialog::getText(this, tr("Ban IP-address"), tr("IP-address ('/' for subnet, e.g. 192.168.0.0/16)"),
                                                               QLineEdit::Normal, _Q(user.szIPAddress), &ok);
                        if (ok && !ipaddr.isEmpty())
                            COPY_TTSTR(ban.szIPAddress, ipaddr);
                        else return;
                    }
                    ban.uBanTypes |= user.nChannelID > 0 ? BANTYPE_CHANNEL | BANTYPE_IPADDR : BANTYPE_IPADDR;
                }
                else
                    ban.uBanTypes |= user.nChannelID > 0 ? BANTYPE_CHANNEL | BANTYPE_USERNAME : BANTYPE_USERNAME;
                TT_DoBan(ttInst, &ban);
            }
        }
    }
}

void MainWindow::slotTreeSelectionChanged()
{
    slotUpdateUI();
    int channelid = ui.channelsWidget->selectedChannel(true);
    if (m_filesmodel->getChannelID() != channelid && ((TT_GetMyUserType(ttInst) & USERTYPE_ADMIN) != USERTYPE_NONE))
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
    int mychannel = m_mychannel.nChannelID;
    int filescount = ui.filesView->selectedFiles().size();
    ClientFlags statemask = TT_GetFlags(ttInst);
    UserRights userrights = TT_GetMyUserRights(ttInst);
    bool auth = (statemask & CLIENT_AUTHORIZED);
    bool me_admin = (TT_GetMyUserType(ttInst) & USERTYPE_ADMIN);
    bool me_op = TT_IsChannelOperator(ttInst, TT_GetMyUserID(ttInst), user_chanid);
    bool tts = ttSettings->value(SETTINGS_TTS_ENGINE, SETTINGS_TTS_ENGINE_DEFAULT).toUInt() != TTSENGINE_NONE;
    bool voiceactivated = (statemask & CLIENT_SNDINPUT_VOICEACTIVATED);
    bool voicetx = (statemask & CLIENT_TX_VOICE);

    ui.actionConnect->setChecked( (statemask & CLIENT_CONNECTING) || (statemask & CLIENT_CONNECTED));
    ui.actionEnableEchoCancel->setChecked(ttSettings->value(SETTINGS_SOUND_ECHOCANCEL, SETTINGS_SOUND_ECHOCANCEL_DEFAULT).toBool());
    ui.actionEnableAGC->setChecked(ttSettings->value(SETTINGS_SOUND_AGC, SETTINGS_SOUND_AGC_DEFAULT).toBool());
    ui.actionEnableDenoising->setChecked(ttSettings->value(SETTINGS_SOUND_DENOISING, SETTINGS_SOUND_DENOISING_DEFAULT).toBool());
    ui.actionSpeakClientStats->setEnabled(tts);
    ui.actionChangeStatus->setEnabled(auth);
#ifdef Q_OS_WIN32
    ui.actionEnablePushToTalk->setChecked(TT_HotKey_IsActive(ttInst, HOTKEY_PUSHTOTALK) >= 0);
#elif defined(Q_OS_LINUX) || defined(Q_OS_DARWIN)
    ui.actionEnablePushToTalk->setChecked(m_hotkeys.find(HOTKEY_PUSHTOTALK) != m_hotkeys.end());
#endif
    ui.actionEnableVoiceActivation->setChecked(voiceactivated);
    ui.actionHearMyself->setEnabled(m_mychannel.nChannelID > 0);
    User myself;
    if (ui.channelsWidget->getUser(TT_GetMyUserID(ttInst), myself))
    {
        ui.actionHearMyself->setChecked(myself.uLocalSubscriptions & SUBSCRIBE_VOICE);
    }
    //don't allow web cam to stream when video streaming is active
    ui.actionEnableVideoTransmission->setChecked((CLIENT_VIDEOCAPTURE_READY & statemask) && 
                                                 (CLIENT_TX_VIDEOCAPTURE & statemask));
    ui.actionEnableDesktopSharing->setEnabled(mychannel>0);
    ui.actionEnableDesktopSharing->setChecked(statemask & CLIENT_DESKTOP_ACTIVE);
    ui.actionEnableTTS->setChecked(ttSettings->value(SETTINGS_TTS_ENABLE, SETTINGS_TTS_ENABLE_DEFAULT).toBool());
    ui.actionEnableTTS->setEnabled(tts);
    ui.actionEnableSounds->setChecked(ttSettings->value(SETTINGS_SOUNDEVENT_ENABLE, SETTINGS_SOUNDEVENT_ENABLE_DEFAULT).toBool());

    User user  = {};
    if (TT_GetUser(ttInst, userid, &user))
    {
        ui.actionMuteVoice->setChecked(user.uUserState & USERSTATE_MUTE_VOICE);
        ui.actionMuteMediaFile->setChecked(user.uUserState & USERSTATE_MUTE_MEDIAFILE);
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
    ui.actionSpeakUserInformation->setEnabled(tts);
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
    ui.actionMoveUsersDialog->setEnabled(userrights & USERRIGHT_MOVE_USERS);
    ui.actionRelayVoiceStream->setEnabled(userid > 0 && !voiceactivated && !voicetx);
    ui.actionRelayVoiceStream->setChecked(userid > 0 && userid == m_relayvoice_userid);
    ui.actionRelayMediaFileStream->setEnabled(userid > 0 && !voiceactivated && !voicetx);
    ui.actionRelayMediaFileStream->setChecked(userid > 0 && userid == m_relaymediafile_userid);

    //ui.actionMuteAll->setEnabled(statemask & CLIENT_SOUND_READY);
    ui.actionMuteAll->setChecked(statemask & CLIENT_SNDOUTPUT_MUTE);
    bool storetextmsgs = ttSettings->value(SETTINGS_MEDIASTORAGE_CHANLOGFOLDER).toString().size() ||
            ttSettings->value(SETTINGS_MEDIASTORAGE_USERLOGFOLDER).toString().size();
    ui.actionMediaStorage->setChecked(m_audiostorage_mode != AUDIOSTORAGE_NONE || storetextmsgs);

    //Channel-menu items
    Channel chan = {};
    if (TT_GetChannel(ttInst, user_chanid, &chan))
    {
    }

    ui.actionLeaveChannel->setEnabled(m_mychannel.nChannelID > 0);
    ui.actionLeaveChannel->setVisible(m_mychannel.nChannelID > 0);
    ui.actionJoinChannel->setEnabled(chanid != m_mychannel.nChannelID && userid <= 0);
    ui.actionJoinChannel->setVisible(chanid != m_mychannel.nChannelID && userid <= 0);
    ui.actionViewChannelInfo->setEnabled(chanid>0);
    ui.actionGenerateTTURL->setEnabled(chanid > 0);
    ui.actionSpeakChannelInfo->setEnabled(tts);
    ui.actionSpeakChannelStat->setEnabled(tts);
    ui.actionBannedUsersInChannel->setEnabled(chanid>0);
    ui.actionCreateChannel->setEnabled(chanid>0 || mychannel>0);
    ui.actionUpdateChannel->setEnabled(chanid>0);
    ui.actionDeleteChannel->setEnabled(chanid>0);
    ui.actionStreamMediaFileToChannel->setChecked(statemask & 
                                                  (CLIENT_STREAM_AUDIO | CLIENT_STREAM_VIDEO));
    ui.actionUploadFile->setEnabled(m_myuseraccount.uUserRights & USERRIGHT_UPLOAD_FILES);
    ui.actionDownloadFile->setEnabled(m_myuseraccount.uUserRights & USERRIGHT_DOWNLOAD_FILES);
    ui.actionDeleteFile->setEnabled(filescount>0);

    //Users-menu items dependent on Channel
    bool modchan = (userrights & USERRIGHT_MODIFY_CHANNELS) == USERRIGHT_MODIFY_CHANNELS;
    ui.actionAllowChannelTextMessages->setChecked(userCanChanMessage(userid, chan));
    ui.actionAllowChannelTextMessages->setEnabled(userid > 0 && (me_op || modchan));
    ui.actionAllowVoiceTransmission->setChecked(userCanVoiceTx(userid, chan));
    ui.actionAllowVoiceTransmission->setEnabled(userid>0 && (me_op || modchan));
    ui.actionAllowVideoTransmission->setChecked(userCanVideoTx(userid, chan));
    ui.actionAllowVideoTransmission->setEnabled(userid>0 && (me_op || modchan));
    ui.actionAllowDesktopTransmission->setChecked(userCanDesktopTx(userid, chan));
    ui.actionAllowDesktopTransmission->setEnabled(userid>0 && (me_op || modchan));
    ui.actionAllowMediaFileTransmission->setChecked(userCanMediaFileTx(userid, chan));
    ui.actionAllowMediaFileTransmission->setEnabled(userid>0 && (me_op || modchan));

    bool classroom = (chan.uChannelType & CHANNEL_CLASSROOM) == CHANNEL_CLASSROOM;
    ui.actionAllowAllChannelTextMessages->setChecked(userCanChanMessage(TT_TRANSMITUSERS_FREEFORALL, chan));
    ui.actionAllowAllChannelTextMessages->setEnabled(classroom && (me_op || modchan));
    ui.actionAllowAllVoiceTransmission->setChecked(userCanVoiceTx(TT_TRANSMITUSERS_FREEFORALL, chan));
    ui.actionAllowAllVoiceTransmission->setEnabled(classroom && (me_op || modchan));
    ui.actionAllowAllVideoTransmission->setChecked(userCanVideoTx(TT_TRANSMITUSERS_FREEFORALL, chan));
    ui.actionAllowAllVideoTransmission->setEnabled(classroom && (me_op || modchan));
    ui.actionAllowAllDesktopTransmission->setChecked(userCanDesktopTx(TT_TRANSMITUSERS_FREEFORALL, chan));
    ui.actionAllowAllDesktopTransmission->setEnabled(classroom && (me_op || modchan));
    ui.actionAllowAllMediaFileTransmission->setChecked(userCanMediaFileTx(TT_TRANSMITUSERS_FREEFORALL, chan));
    ui.actionAllowAllMediaFileTransmission->setEnabled(classroom && (me_op || modchan));

    //Server-menu items
    ui.actionUserAccounts->setEnabled(auth);
    ui.actionBannedUsers->setEnabled(me_op || (userrights & USERRIGHT_BAN_USERS));
    ui.actionOnlineUsers->setEnabled(auth);
    ui.actionBroadcastMessage->setEnabled(auth && (userrights & USERRIGHT_TEXTMESSAGE_BROADCAST));
    ui.actionServerProperties->setEnabled(auth);
    ui.actionSaveConfiguration->setEnabled(auth && me_admin);
    ui.actionServerStatistics->setEnabled(auth && me_admin);

    ui.uploadButton->setEnabled(m_myuseraccount.uUserRights & USERRIGHT_UPLOAD_FILES);
    ui.downloadButton->setEnabled(m_myuseraccount.uUserRights & USERRIGHT_DOWNLOAD_FILES);
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
    User user = {};
    ui.channelsWidget->getUser(userid, user);
    
    ui.detachDesktopButton->setEnabled(userid>0);
    ui.removeDesktopButton->setEnabled(ui.desktopgridWidget->getActiveUsersCount());
    ui.desktopaccessButton->setEnabled(userid>0);
    ui.desktopaccessButton->setChecked(user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT);
}

void MainWindow::slotUploadFiles(const QStringList& files)
{
    int channelid = m_filesmodel->getChannelID();
    Q_ASSERT(channelid > 0);

    for (const QString& filepath : files)
    {
        QString filename = QFileInfo(filepath).fileName();
        int remoteFileID = getRemoteFileID(channelid, filename);

        if (remoteFileID != -1)
        {
            RemoteFile remoteFile;
            if (TT_GetChannelFile(ttInst, channelid, remoteFileID, &remoteFile))
            {
                bool me_admin = (TT_GetMyUserType(ttInst) & USERTYPE_ADMIN);
                bool myusername = m_host.username == _Q(remoteFile.szUsername);
                bool op = TT_IsChannelOperator(ttInst, TT_GetMyUserID(ttInst), channelid);

                if (me_admin || myusername || op)
                {
                    QMessageBox answer;
                    answer.setText(tr("File %1 already exists on the server. Do you want to replace it?").arg(filename));
                    QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
                    QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
                    Q_UNUSED(NoButton);
                    answer.setIcon(QMessageBox::Question);
                    answer.setWindowTitle(tr("File exists"));
                    answer.exec();
                    if(answer.clickedButton() == YesButton)
                    {
                        if (TT_DoDeleteFile(ttInst, channelid, remoteFileID) < 0)
                        {
                            QMessageBox::critical(this, MENUTEXT(ui.actionUploadFile->text()),
                                                  tr("Failed to delete existing file %1").arg(filename));
                            continue;
                        }
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    QMessageBox::critical(this, MENUTEXT(ui.actionUploadFile->text()),
                                          tr("You do not have permission to replace the file %1").arg(filename));
                    continue;
                }
            }
        }

        QString nativeFilepath = QDir::toNativeSeparators(filepath);
        if (!TT_DoSendFile(ttInst, channelid, _W(nativeFilepath)))
        {
            QMessageBox::critical(this, MENUTEXT(ui.actionUploadFile->text()),
                                  tr("Failed to upload file %1").arg(nativeFilepath));
        }
    }
}

int MainWindow::getRemoteFileID(int channelid, const QString& filename)
{
    int count = 0;
    if (TT_GetChannelFiles(ttInst, channelid, nullptr, &count))
    {
        QVector<RemoteFile> files(count);
        if (TT_GetChannelFiles(ttInst, channelid, files.data(), &count))
        {
            for (const RemoteFile& file : files)
            {
                if (_Q(file.szFileName) == filename)
                {
                    return file.nFileID;
                }
            }
        }
    }
    return -1;
}

void MainWindow::slotSendChannelMessage()
{
    if (m_mychannel.nChannelID <= 0)
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

    MyTextMessage msg;
    msg.nFromUserID = TT_GetMyUserID(ttInst);
    msg.nChannelID = m_mychannel.nChannelID;
    msg.nMsgType = MSGTYPE_CHANNEL;

    if (sendTextMessage(msg, txtmsg).size() == 0)
    {
        switch(ui.tabWidget->currentIndex())
        {
        case TAB_CHAT :
            ui.msgEdit->setText(txtmsg);
            break;
        case TAB_VIDEO :
            ui.videomsgEdit->setText(txtmsg);
            break;
        case TAB_DESKTOP :
            ui.desktopmsgEdit->setText(txtmsg);
            break;
        default :
            break;
        }
    }

    transmitOn(STREAMTYPE_CHANNELMSG);
}

void MainWindow::slotUserDoubleClicked(int id)
{
    if(QApplication::keyboardModifiers() & Qt::ControlModifier)
    {
        User user;
        if (!ui.channelsWidget->getUser(id, user))
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
        slotUsersMessages(id);
}

void MainWindow::slotChannelDoubleClicked(int)
{
    slotChannelsJoinChannel(false);
}

void MainWindow::slotNewMyselfTextMessage(const MyTextMessage& textmsg)
{
    if(textmsg.nMsgType != MSGTYPE_USER)
        return;

    m_textmessages.addTextMessage(textmsg);
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

    // Solo transmission
    if(chan.transmitUsersQueue[0] == TT_GetMyUserID(ttInst) &&
        oldchan.transmitUsersQueue[0] != TT_GetMyUserID(ttInst))
        playSoundEvent(SOUNDEVENT_TRANSMITQUEUE_HEAD);

    if(chan.transmitUsersQueue[0] != TT_GetMyUserID(ttInst) &&
        oldchan.transmitUsersQueue[0] == TT_GetMyUserID(ttInst))
        playSoundEvent(SOUNDEVENT_TRANSMITQUEUE_STOP);
    if (chan.nChannelID == m_mychannel.nChannelID)
        updateClassroomChannel(oldchan, chan);
}

void MainWindow::updateClassroomChannel(const Channel& oldchan, const Channel& newchan)
{
    auto userids = ui.channelsWidget->getUsersInChannel(newchan.nChannelID);
    userids.push_back(TT_CLASSROOM_FREEFORALL);
    for (auto id : userids)
    {
        User user = {};
        ui.channelsWidget->getUser(id, user);
        QString nick = getDisplayName(user);
        if (id == TT_CLASSROOM_FREEFORALL)
            nick = tr("Everyone");
        if (user.nUserID == TT_GetMyUserID(ttInst))
            nick = tr("You");

        QString type, state;
        TextToSpeechEvent ttsType;
        StatusBarEvent statusType;
        bool before = false, after = false, change = false;
        before = userCanChanMessage(id, oldchan);
        after = userCanChanMessage(id, newchan);
        if (before != after)
        {
            type = tr("Channel messages");
            if (after)
                state = tr("Enabled");
            else
                state = tr("Disabled");
            ttsType = TTS_CLASSROOM_CHANMSG_TX;
            statusType = STATUSBAR_CLASSROOM_CHANMSG_TX;
            change = true;
        }
        before = userCanVoiceTx(id, oldchan);
        after = userCanVoiceTx(id, newchan);
        if (before != after)
        {
            type = tr("Voice");
            if (after)
                state = tr("Enabled");
            else
                state = tr("Disabled");
            statusType = STATUSBAR_CLASSROOM_VOICE_TX;
            ttsType = TTS_CLASSROOM_VOICE_TX;
            change = true;
        }
        before = userCanVideoTx(id, oldchan);
        after = userCanVideoTx(id, newchan);
        if (before != after)
        {
            type = tr("Video");
            if (after)
                state = tr("Enabled");
            else
                state = tr("Disabled");
            statusType = STATUSBAR_CLASSROOM_VIDEO_TX;
            ttsType = TTS_CLASSROOM_VIDEO_TX;
            change = true;
        }
        before = userCanDesktopTx(id, oldchan);
        after = userCanDesktopTx(id, newchan);
        if (before != after)
        {
            type = tr("Desktop windows");
            if (after)
                state = tr("Enabled");
            else
                state = tr("Disabled");
            statusType = STATUSBAR_CLASSROOM_DESKTOP_TX;
            ttsType = TTS_CLASSROOM_DESKTOP_TX;
            change = true;
        }
        before = userCanMediaFileTx(id, oldchan);
        after = userCanMediaFileTx(id, newchan);
        if (before != after)
        {
            type = tr("Media files");
            if (after)
                state = tr("Enabled");
            else
                state = tr("Disabled");
            statusType = STATUSBAR_CLASSROOM_MEDIAFILE_TX;
            ttsType = TTS_CLASSROOM_MEDIAFILE_TX;
            change = true;
        }
        if (change)
        {
            addStatusMsg(statusType, UtilUI::getStatusBarMessage(SETTINGS_STATUSBARMSG_CLASSROOM, {{"{type}", type}, {"{state}", state}, {"{user}", nick}}));
            addTextToSpeechMessage(ttsType, UtilTTS::getTTSMessage(SETTINGS_TTSMSG_CLASSROOM, {{"{type}", type}, {"{state}", state}, {"{user}", nick}}));
        }
    }
}

void MainWindow::slotInitVideo()
{
    if((TT_GetFlags(ttInst) & CLIENT_VIDEOCAPTURE_READY) == 0)
    {
        if(!initVideoCaptureFromSettings())
            QMessageBox::warning(this, tr("Start Webcam"), 
            tr("Video device hasn't been configured properly. Check settings in 'Preferences'"));
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
    QVector<int> users = ui.channelsWidget->getUsersInChannel(m_mychannel.nChannelID);
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
void MainWindow::slotRemoveUserVideoGrid()
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
    User user = {};

    //user might not exist in channels tree since it can be local video (userid=0)
    ui.channelsWidget->getUser(userid & VIDEOTYPE_USERMASK, user);

    UserVideoDlg* dlg;
    if(size.isValid())
        dlg = new UserVideoDlg(userid, user, size, nullptr);
    else
        dlg = new UserVideoDlg(userid, user, nullptr);

    connect(this, &MainWindow::userUpdate, dlg,
            &UserVideoDlg::slotUserUpdate);
    connect(dlg, &UserVideoDlg::userVideoDlgClosing,
            this, &MainWindow::slotUserVideoDlgClosing);
    connect(this, &MainWindow::preferencesModified, dlg->uservideoWidget,
            &UserVideoWidget::slotUpdateVideoTextBox);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    switch(userid & VIDEOTYPE_MASK)
    {
    case VIDEOTYPE_CAPTURE :
        connect(this, &MainWindow::newVideoCaptureFrame,
                dlg->uservideoWidget, &UserVideoWidget::slotNewVideoFrame);
        connect(this, &MainWindow::newVideoCaptureFrame, dlg,
                &UserVideoDlg::slotNewVideoFrame);
        m_user_video[userid] = dlg;
        break;
    case VIDEOTYPE_MEDIAFILE :
        connect(this, &MainWindow::newMediaVideoFrame,
                dlg->uservideoWidget, &UserVideoWidget::slotNewVideoFrame);
        connect(this, &MainWindow::newMediaVideoFrame, dlg,
                &UserVideoDlg::slotNewVideoFrame);
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
        ui.videogridWidget->slotAddUser(userid);
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

        ui.videogridWidget->slotAddUser(userid);
    }
    break;
    }
}

void MainWindow::slotAddUserDesktopGrid()
{
    QVector<int> users = ui.channelsWidget->getUsersInChannel(m_mychannel.nChannelID);
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

void MainWindow::slotRemoveUserDesktopGrid()
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
    sendTextMessage(msg, cmd);

    slotUpdateDesktopTabUI();
}

void MainWindow::slotDetachUserDesktopGrid()
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

    connect(this, &MainWindow::newDesktopWindow,
            dlg, &UserDesktopDlg::userDesktopWindowUpdate);
    connect(this, &MainWindow::userUpdate, dlg,
            &UserDesktopDlg::slotUserUpdate);
    connect(this, &MainWindow::userUpdate, dlg,
            &UserDesktopDlg::userUpdated);
    connect(this, &MainWindow::userDesktopCursor,
            dlg, &UserDesktopDlg::userDesktopCursorUpdate);
    connect(dlg, &UserDesktopDlg::userDesktopDlgClosing,
            this, &MainWindow::slotUserDesktopDlgClosing);
    connect(dlg, &UserDesktopDlg::userDesktopWindowEnded,
            this, &MainWindow::slotRemoveUserDesktop);
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
    }

    //we cannot get user from channels-widget since user has left channel
    if(m_mychannel.nChannelID == channelid &&
       (user.uLocalSubscriptions & SUBSCRIBE_DESKTOPINPUT))
       subscribeCommon(false, SUBSCRIBE_DESKTOPINPUT, user.nUserID);
}

void MainWindow::slotUserUpdate(const User& user)
{
    User oldUser;
    if(ui.channelsWidget->getUser(user.nUserID, oldUser))
    {
        QString nickname = getDisplayName(user);
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_USER_MSG) !=
            (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_USER_MSG))
        {
            addStatusMsg(STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE, tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionInterceptUserMessages->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_USER_MSG?
                     tr("On"):tr("Off")));
            addTextToSpeechMessage(TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionInterceptUserMessages->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_USER_MSG?tr("On"):tr("Off")));
            playSoundEvent(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_USER_MSG?SOUNDEVENT_INTERCEPT:SOUNDEVENT_INTERCEPTEND);
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG) !=
            (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG))
        {
            addStatusMsg(STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL, tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionInterceptChannelMessages->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG?
                     tr("On"):tr("Off")));
            addTextToSpeechMessage(TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionInterceptChannelMessages->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG?tr("On"):tr("Off")));
            playSoundEvent(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG?SOUNDEVENT_INTERCEPT:SOUNDEVENT_INTERCEPTEND);
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VOICE) !=
            (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VOICE))
        {
            addStatusMsg(STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VOICE, tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionInterceptVoice->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VOICE?
                     tr("On"):tr("Off")));
            addTextToSpeechMessage(TTS_SUBSCRIPTIONS_INTERCEPT_VOICE, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionInterceptVoice->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VOICE?tr("On"):tr("Off")));
            playSoundEvent(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VOICE?SOUNDEVENT_INTERCEPT:SOUNDEVENT_INTERCEPTEND);
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE) !=
            (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE))
        {
            addStatusMsg(STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VIDEO, tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionInterceptVideo->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE?
                     tr("On"):tr("Off")));
            addTextToSpeechMessage(TTS_SUBSCRIPTIONS_INTERCEPT_VIDEO, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionInterceptVideo->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE?tr("On"):tr("Off")));
            playSoundEvent(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE?SOUNDEVENT_INTERCEPT:SOUNDEVENT_INTERCEPTEND);
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_DESKTOP) !=
            (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_DESKTOP))
        {
            addStatusMsg(STATUSBAR_SUBSCRIPTIONS_INTERCEPT_DESKTOP, tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionInterceptDesktop->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_DESKTOP?
                     tr("On"):tr("Off")));
            addTextToSpeechMessage(TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOP, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionInterceptDesktop->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_DESKTOP?tr("On"):tr("Off")));
            playSoundEvent(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_DESKTOP?SOUNDEVENT_INTERCEPT:SOUNDEVENT_INTERCEPTEND);
        }
        if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_MEDIAFILE) !=
            (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_MEDIAFILE))
        {
            addStatusMsg(STATUSBAR_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE, tr("%1 changed subscription \"%2\" to: %3")
                .arg(nickname)
                .arg(MENUTEXT(ui.actionInterceptMediaFile->text()))
                .arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_MEDIAFILE?
                     tr("On"):tr("Off")));
            addTextToSpeechMessage(TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionInterceptMediaFile->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_MEDIAFILE?tr("On"):tr("Off")));
            playSoundEvent(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_MEDIAFILE?SOUNDEVENT_INTERCEPT:SOUNDEVENT_INTERCEPTEND);
        }
        if(m_mychannel.nChannelID == user.nChannelID && user.nChannelID)
        {
            if((oldUser.uPeerSubscriptions & SUBSCRIBE_USER_MSG) !=
                (user.uPeerSubscriptions & SUBSCRIBE_USER_MSG))
            {
                addStatusMsg(STATUSBAR_SUBSCRIPTIONS_TEXTMSG_PRIVATE, tr("%1 changed subscription \"%2\" to: %3")
                    .arg(nickname)
                    .arg(MENUTEXT(ui.actionUserMessages->text()))
                    .arg(user.uPeerSubscriptions & SUBSCRIBE_USER_MSG?
                         tr("On"):tr("Off")));
            addTextToSpeechMessage(TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionUserMessages->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_USER_MSG?tr("On"):tr("Off")));
            }
            if((oldUser.uPeerSubscriptions & SUBSCRIBE_CHANNEL_MSG) !=
                (user.uPeerSubscriptions & SUBSCRIBE_CHANNEL_MSG))
            {
                addStatusMsg(STATUSBAR_SUBSCRIPTIONS_TEXTMSG_CHANNEL, tr("%1 changed subscription \"%2\" to: %3")
                    .arg(nickname)
                    .arg(MENUTEXT(ui.actionChannelMessages->text()))
                    .arg(user.uPeerSubscriptions & SUBSCRIBE_CHANNEL_MSG?
                         tr("On"):tr("Off")));
            addTextToSpeechMessage(TTS_SUBSCRIPTIONS_TEXTMSG_CHANNEL, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionChannelMessages->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_CHANNEL_MSG?tr("On"):tr("Off")));
            }
            if((oldUser.uPeerSubscriptions & SUBSCRIBE_BROADCAST_MSG) !=
                (user.uPeerSubscriptions & SUBSCRIBE_BROADCAST_MSG))
            {
                addStatusMsg(STATUSBAR_SUBSCRIPTIONS_TEXTMSG_BROADCAST, tr("%1 changed subscription \"%2\" to: %3")
                    .arg(nickname)
                    .arg(MENUTEXT(ui.actionBroadcastMessages->text()))
                    .arg(user.uPeerSubscriptions & SUBSCRIBE_BROADCAST_MSG?
                         tr("On"):tr("Off")));
                addTextToSpeechMessage(TTS_SUBSCRIPTIONS_TEXTMSG_BROADCAST, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionBroadcastMessages->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_BROADCAST_MSG?tr("On"):tr("Off")));
            }
            if((oldUser.uPeerSubscriptions & SUBSCRIBE_VOICE) !=
                (user.uPeerSubscriptions & SUBSCRIBE_VOICE))
            {
                addStatusMsg(STATUSBAR_SUBSCRIPTIONS_VOICE, tr("%1 changed subscription \"%2\" to: %3")
                    .arg(nickname)
                    .arg(MENUTEXT(ui.actionVoice->text()))
                    .arg(user.uPeerSubscriptions & SUBSCRIBE_VOICE?
                         tr("On"):tr("Off")));
                addTextToSpeechMessage(TTS_SUBSCRIPTIONS_VOICE, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionVoice->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_VOICE?tr("On"):tr("Off")));
            }
            if((oldUser.uPeerSubscriptions & SUBSCRIBE_VIDEOCAPTURE) !=
                (user.uPeerSubscriptions & SUBSCRIBE_VIDEOCAPTURE))
            {
                addStatusMsg(STATUSBAR_SUBSCRIPTIONS_VIDEO, tr("%1 changed subscription \"%2\" to: %3")
                    .arg(nickname)
                    .arg(MENUTEXT(ui.actionVideo->text()))
                    .arg(user.uPeerSubscriptions & SUBSCRIBE_VIDEOCAPTURE?
                         tr("On"):tr("Off")));
                addTextToSpeechMessage(TTS_SUBSCRIPTIONS_VIDEO, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionVideo->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_VIDEOCAPTURE?tr("On"):tr("Off")));
            }
            if((oldUser.uPeerSubscriptions & SUBSCRIBE_DESKTOP) !=
                (user.uPeerSubscriptions & SUBSCRIBE_DESKTOP))
            {
                addStatusMsg(STATUSBAR_SUBSCRIPTIONS_DESKTOP, tr("%1 changed subscription \"%2\" to: %3")
                    .arg(nickname)
                    .arg(MENUTEXT(ui.actionDesktop->text()))
                    .arg(user.uPeerSubscriptions & SUBSCRIBE_DESKTOP?
                         tr("On"):tr("Off")));
                addTextToSpeechMessage(TTS_SUBSCRIPTIONS_DESKTOP, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionDesktop->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_DESKTOP?tr("On"):tr("Off")));
            }
            if((oldUser.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT) !=
                (user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT))
            {
                addStatusMsg(STATUSBAR_SUBSCRIPTIONS_DESKTOPINPUT, tr("%1 changed subscription \"%2\" to: %3")
                    .arg(nickname)
                    .arg(MENUTEXT(ui.actionDesktopInput->text()))
                    .arg(user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT?
                         tr("On"):tr("Off")));
                addTextToSpeechMessage(TTS_SUBSCRIPTIONS_DESKTOPINPUT, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionDesktopInput->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT?tr("On"):tr("Off")));
            }
            if((oldUser.uPeerSubscriptions & SUBSCRIBE_MEDIAFILE) !=
                (user.uPeerSubscriptions & SUBSCRIBE_MEDIAFILE))
            {
                addStatusMsg(STATUSBAR_SUBSCRIPTIONS_MEDIAFILE, tr("%1 changed subscription \"%2\" to: %3")
                    .arg(nickname)
                    .arg(MENUTEXT(ui.actionMediaFile->text()))
                    .arg(user.uPeerSubscriptions & SUBSCRIBE_MEDIAFILE?
                     tr("On"):tr("Off")));
                addTextToSpeechMessage(TTS_SUBSCRIPTIONS_MEDIAFILE, tr("%1 changed subscription \"%2\" to: %3").arg(nickname).arg(MENUTEXT(ui.actionMediaFile->text())).arg(user.uPeerSubscriptions & SUBSCRIBE_MEDIAFILE?tr("On"):tr("Off")));
            }
        }
    }
}

void MainWindow::slotToggleQuestionMode(bool checked)
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
        ui.tabWidget->setTabText(TAB_VIDEO, tr("&Video"));
    else
        ui.tabWidget->setTabText(TAB_VIDEO, tr("&Video (%1)").arg(count));
}

void MainWindow::slotUpdateDesktopCount(int count)
{
    if(count == 0)
        ui.tabWidget->setTabText(TAB_DESKTOP, tr("&Desktops"));
    else
        ui.tabWidget->setTabText(TAB_DESKTOP, tr("&Desktops (%1)").arg(count));
}

void MainWindow::slotMasterVolumeChanged(int value)
{
    int vol = refVolume(value);
    TT_SetSoundOutputVolume(ttInst, vol);
}

void MainWindow::slotMicrophoneGainChanged(int value)
{
    AudioPreprocessor preprocessor = initDefaultAudioPreprocessor(NO_AUDIOPREPROCESSOR);

    TT_GetSoundInputPreprocessEx(ttInst, &preprocessor);
    switch (preprocessor.nPreprocessor)
    {
    case NO_AUDIOPREPROCESSOR :
        preprocessor = initDefaultAudioPreprocessor(NO_AUDIOPREPROCESSOR);
        TT_SetSoundInputPreprocessEx(ttInst, &preprocessor);
        TT_SetSoundInputGainLevel(ttInst, refGain(value));
        break;
    case TEAMTALK_AUDIOPREPROCESSOR :
        preprocessor.ttpreprocessor.nGainLevel = refGain(value);
        TT_SetSoundInputPreprocessEx(ttInst, &preprocessor);
        break;
    case SPEEXDSP_AUDIOPREPROCESSOR :
        // Only no audio preprocessor or webrtc is currently supported.
        Q_ASSERT(preprocessor.nPreprocessor == WEBRTC_AUDIOPREPROCESSOR);
        break;
    case WEBRTC_AUDIOPREPROCESSOR :
    {
        bool agc = ttSettings->value(SETTINGS_SOUND_AGC, SETTINGS_SOUND_AGC_DEFAULT).toBool();
        float percent = float(value);
        percent /= 100.;
        preprocessor.webrtc.gaincontroller2.bEnable = agc;
        preprocessor.webrtc.gaincontroller2.fixeddigital.fGainDB = float(WEBRTC_GAINCONTROLLER2_FIXEDGAIN_MAX * percent);
        TT_SetSoundInputPreprocessEx(ttInst, &preprocessor);
        TT_SetSoundInputGainLevel(ttInst, agc ? SOUND_GAIN_DEFAULT : refGain(value));
        break;
    }
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
    if (!element.firstChildElement(CLIENTSETUP_TAG).isNull())
    {
        QMessageBox answer;
        answer.setText(tr("The file %1 contains %2 setup information.\r\nShould these settings be applied?").arg(filepath).arg(APPNAME_SHORT));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        Q_UNUSED(NoButton);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle(tr("Load %1 File").arg(TTFILE_EXT));
        answer.exec();
        if(answer.clickedButton() == YesButton)
        {
            //if no nickname specified use from .tt file
            if(m_host.nickname.size())
                ttSettings->setValue(SETTINGS_GENERAL_NICKNAME, m_host.nickname);

            //if no gender specified use from .tt file
            if (m_host.gender != GENDER_NONE)
                ttSettings->setValue(SETTINGS_GENERAL_GENDER, m_host.gender);
        
            //if no PTT-key specified use from .tt file
            hotkey_t hotkey;
            if(m_host.hotkey.size())
            {
                saveHotKeySettings(HOTKEY_PUSHTOTALK, m_host.hotkey);
                enableHotKey(HOTKEY_PUSHTOTALK, m_host.hotkey);
            }

            //voice activation
            if(m_host.voiceact >= 0)
                enableVoiceActivation(m_host.voiceact>0);

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
    }

    disconnectFromServer();
    connectToServer();
}

void MainWindow::slotSoftwareUpdateReply(QNetworkReply* reply, bool manualCheck)
{
    QByteArray data = reply->readAll();

    QDomDocument doc("foo");
    bool updateFound = false;
    if(doc.setContent(data))
    {
        QString version = newVersionAvailable(doc);
        QString betaVersion = newBetaVersionAvailable(doc);
        if(version.size() || betaVersion.size())
        {
            updateFound = true;
            if(ttSettings->value(SETTINGS_DISPLAY_APPUPDATE, SETTINGS_DISPLAY_APPUPDATE_DEFAULT).toBool() || manualCheck)
            {
                if (version.size())
                {
                    QString downloadurl = downloadUpdateURL(doc);
                    if(ttSettings->value(SETTINGS_DISPLAY_APPUPDATE_DLG, SETTINGS_DISPLAY_APPUPDATE_DLG_DEFAULT).toBool() || manualCheck)
                    {
                        QMessageBox answer;
                        answer.setText(tr("A new version of %1 is available: %2. Do you wish to open the download page now?").arg(APPNAME_SHORT).arg(version));
                        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
                        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
                        Q_UNUSED(NoButton);
                        answer.setIcon(QMessageBox::Question);
                        answer.setWindowTitle(tr("New version available"));
                        answer.exec();

                        if(answer.clickedButton() == YesButton)
                            QDesktopServices::openUrl(downloadurl);
                    }
                    else
                        addStatusMsg(STATUSBAR_BYPASS, tr("New version available: %1\r\nYou can download it on the page below:\r\n%2").arg(version).arg(downloadurl));
                }
            }

            if(ttSettings->value(SETTINGS_DISPLAY_APPUPDATE_BETA, SETTINGS_DISPLAY_APPUPDATE_BETA_DEFAULT).toBool())
            {
                if (betaVersion.size())
                {
                    QString downloadurl = downloadBetaUpdateURL(doc);
                    if(ttSettings->value(SETTINGS_DISPLAY_APPUPDATE_DLG, SETTINGS_DISPLAY_APPUPDATE_DLG_DEFAULT).toBool() || manualCheck)
                    {
                        QMessageBox answer;
                        answer.setText(tr("A new beta version of %1 is available: %2. Do you wish to open the download page now?").arg(APPNAME_SHORT).arg(betaVersion));
                        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
                        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
                        Q_UNUSED(NoButton);
                        answer.setIcon(QMessageBox::Question);
                        answer.setWindowTitle(tr("New beta version available"));
                        answer.exec();

                        if(answer.clickedButton() == YesButton)
                            QDesktopServices::openUrl(downloadurl);
                    }
                    else
                        addStatusMsg(STATUSBAR_BYPASS, tr("New beta version available: %1\r\nYou can download it on the page below:\r\n%2").arg(betaVersion).arg(downloadurl));
                }
            }
        }

        BearWareLoginDlg::registerUrl = getBearWareRegistrationUrl(doc);
    }
    if(!updateFound && manualCheck)
    {
        QMessageBox::information(this, tr("Check for Update"), tr("%1 is up to date.").arg(APPNAME_SHORT));
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

void MainWindow::startTTS()
{
    switch (ttSettings->value(SETTINGS_TTS_ENGINE, SETTINGS_TTS_ENGINE_DEFAULT).toUInt())
    {
#if defined(QT_TEXTTOSPEECH_LIB)
    case TTSENGINE_QT :
    {
        delete ttSpeech;
        ttSpeech = new QTextToSpeech(this);
        ttSpeech->setRate(ttSettings->value(SETTINGS_TTS_RATE, SETTINGS_TTS_RATE_DEFAULT).toDouble());
        ttSpeech->setVolume(ttSettings->value(SETTINGS_TTS_VOLUME, SETTINGS_TTS_VOLUME_DEFAULT).toDouble());
        QString locale = ttSettings->value(SETTINGS_TTS_LOCALE).toString();
        QVector<QLocale> locales = ttSpeech->availableLocales();
        auto selLocale = std::find_if(locales.begin(), locales.end(), [locale](const QLocale& l) {
           return l.nativeLanguageName() == locale;
        });
        if (selLocale != locales.end())
        {
            ttSpeech->setLocale(*selLocale);
        }
        else if (locales.size())
        {
            qDebug() << "Locales found";
        }
        else
        {
            addStatusMsg(STATUSBAR_BYPASS, tr("Language %1 not found for Text-To-Speech").arg(locale));
        }
        QString voice = ttSettings->value(SETTINGS_TTS_VOICE).toString();
        QVector<QVoice> voices = ttSpeech->availableVoices();
        auto selVoice = std::find_if(voices.begin(), voices.end(), [voice](const QVoice& v) {
           return v.name() == voice;
        });
        if (selVoice != voices.end())
        {
            ttSpeech->setVoice(*selVoice);
        }
        else if (voices.size())
        {
            addStatusMsg(STATUSBAR_BYPASS, tr("Voice %1 not found for Text-To-Speech. Switching to %2").arg(voice).arg(voices[0].name()));
            ttSpeech->setVoice(voices[0]);
        }
        else
        {
            addStatusMsg(STATUSBAR_BYPASS, tr("No available voices found for Text-To-Speech"));
        }
    }
    break;
#endif

#if defined(ENABLE_TOLK)
    case TTSENGINE_TOLK :
    {
        if (!Tolk_IsLoaded())
        {
            Tolk_Load();
            Tolk_TrySAPI(true);
        }
    }
    break;
#endif
    case TTSENGINE_QTANNOUNCEMENT :
#if QT_VERSION >= QT_VERSION_CHECK(6,8,0)
        announcerObject = this;
#endif
        break;
    }
}

void MainWindow::slotTextChanged()
{
    ui.sendButton->setVisible(ui.msgEdit->text().size()>0);
    ui.videosendButton->setVisible(ui.videomsgEdit->text().size()>0);
    ui.desktopsendButton->setVisible(ui.desktopmsgEdit->text().size()>0);
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
#if defined(Q_OS_DARWIN)
        if (ui.channelsWidget->hasFocus())
        {
            User user;
            if(ui.channelsWidget->getUser(ui.channelsWidget->selectedUser(), user))
                slotUserDoubleClicked(ui.channelsWidget->selectedUser());
            else
                slotChannelDoubleClicked(ui.channelsWidget->selectedChannel(true));
        }
#endif
        if (ui.filesView->hasFocus())
        {
            slotChannelsDownloadFile();
        }
    }
    if (ui.chatEdit->hasFocus() || ui.videochatEdit->hasFocus() || ui.desktopchatEdit->hasFocus())
    {
        QString key = e->text();
        if (!key.isEmpty() && key.size() == 1)
        {
            QChar keyText = key.at(0);    
            if (keyText.isPrint())
            {
                if (ui.chatEdit->hasFocus())
                {
                    ui.msgEdit->setFocus();
                    ui.msgEdit->kPress(e);
                }
                else if (ui.videochatEdit->hasFocus())
                {
                    ui.videomsgEdit->setFocus();
                    ui.videomsgEdit->kPress(e);
                }
                else if (ui.desktopchatEdit->hasFocus())
                {
                    ui.desktopmsgEdit->setFocus();
                    ui.desktopmsgEdit->kPress(e);
                }
            }
        }
    }
    QMainWindow::keyPressEvent(e);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
#if defined(Q_OS_DARWIN)
    QMainWindow::closeEvent(event);
#else
    slotClientExit();
#endif
}

void MainWindow::slotSpeakClientStats(bool /*checked = false*/)
{
    ClientStatistics stats = {};
    TT_GetClientStatistics(ttInst, &stats);
    float rx = float(stats.nUdpBytesRecv - m_clientstats.nUdpBytesRecv);
    float tx = float(stats.nUdpBytesSent - m_clientstats.nUdpBytesSent);
    int ping = stats.nUdpPingTimeMs;
    QString strstats = QString("RX: %1KB TX: %2KB").arg(rx / 1024.0, 2, 'f', 2, '0').arg(tx / 1024.0, 2, 'f', 2, '0');
    if (ping >= 0)
        strstats += QString(" PING: %3").arg(ping);
    addTextToSpeechMessage(strstats);
}
