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

#include "preferencesdlg.h"
#include "common.h"
#include "utilsound.h"
#include "keycompdlg.h"
#include "appinfo.h"
#include "videotextdlg.h"
#include "desktopaccessdlg.h"
#include "customvideofmtdlg.h"
#include "bearwarelogindlg.h"
#include "ttseventsmodel.h"
#include "statusbardlg.h"
#include "utilvideo.h"
#include "utiltts.h"
#include "utilui.h"
#include "settings.h"

#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QTranslator>
#include <QVariant>
#include <QKeyEvent>
#if defined(QT_TEXTTOSPEECH_LIB)
#include <QTextToSpeech>
#endif
#include "stdint.h"

extern TTInstance* ttInst;
extern QSettings* ttSettings;
extern QTranslator* ttTranslator;
#if defined(QT_TEXTTOSPEECH_LIB)
extern QTextToSpeech* ttSpeech;
#endif

#define CUSTOMVIDEOFORMAT_INDEX -2

PreferencesDlg::PreferencesDlg(SoundDevice& devin, SoundDevice& devout, QWidget * parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
, m_devin(devin)
, m_devout(devout)
, m_uservideo(nullptr)
, m_sndloop(nullptr)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_PREFERENCESWINDOWPOS).toByteArray());

    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&Ok"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    initDefaultVideoFormat(m_vidfmt);

    connect(ui.tabWidget, &QTabWidget::currentChanged, this, &PreferencesDlg::slotTabChange);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &PreferencesDlg::slotSaveChanges);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &PreferencesDlg::slotCancelChanges);

    //general tab
    connect(ui.awaySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &PreferencesDlg::slotUpdateASBAccessibleName);
    connect(ui.setupBearWareLoginButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEnableBearWareID);

    connect(ui.pttChkBox, &QAbstractButton::clicked, this, &PreferencesDlg::slotEnablePushToTalk);
    connect(ui.setupkeysButton, &QAbstractButton::clicked, this, &PreferencesDlg::slotSetupHotkey);

    //display tab
    connect(ui.languageBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PreferencesDlg::slotLanguageChange);
    connect(ui.vidtextsrcToolBtn, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotSelectVideoText);
    connect(ui.statusbarToolButton, &QAbstractButton::clicked, this, &PreferencesDlg::slotConfigureStatusBar);
    connect(ui.updatesChkBox, &QAbstractButton::clicked, this, &PreferencesDlg::slotUpdateUpdDlgChkBox);
    connect(ui.betaUpdatesChkBox, &QAbstractButton::clicked, this, &PreferencesDlg::slotUpdateUpdDlgChkBox);
    
    //connection tab
    connect(ui.subdeskinputBtn, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotDesktopAccess);

    //sound tab
    connect(ui.winmmButton, &QAbstractButton::clicked, this, &PreferencesDlg::slotSoundSystemChange);
    connect(ui.dsoundButton, &QAbstractButton::clicked, this, &PreferencesDlg::slotSoundSystemChange);
    connect(ui.wasapiButton, &QAbstractButton::clicked, this, &PreferencesDlg::slotSoundSystemChange);
    connect(ui.alsaButton, &QAbstractButton::clicked, this, &PreferencesDlg::slotSoundSystemChange);
    connect(ui.coreaudioButton, &QAbstractButton::clicked, this, &PreferencesDlg::slotSoundSystemChange);
#if defined(Q_OS_WIN32)
    ui.alsaButton->setDisabled(true);
    ui.alsaButton->hide();
    ui.coreaudioButton->setDisabled(true);
    ui.coreaudioButton->hide();
#elif defined(Q_OS_DARWIN)
    ui.wasapiButton->setDisabled(true);
    ui.wasapiButton->hide();
    ui.dsoundButton->setDisabled(true);
    ui.dsoundButton->hide();
    ui.winmmButton->setDisabled(true);
    ui.winmmButton->hide();
    ui.alsaButton->setDisabled(true);
    ui.alsaButton->hide();
    ui.winfwChkBox->hide();
#else
    ui.winmmButton->setDisabled(true);
    ui.winmmButton->hide();
    ui.dsoundButton->setDisabled(true);
    ui.dsoundButton->hide();
    ui.coreaudioButton->setDisabled(true);
    ui.coreaudioButton->hide();
    ui.wasapiButton->setDisabled(true);
    ui.wasapiButton->hide();
    ui.winfwChkBox->hide();
#endif
    connect(ui.inputdevBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PreferencesDlg::slotSoundInputChange);
    connect(ui.outputdevBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PreferencesDlg::slotSoundOutputChange);
    connect(ui.refreshinputButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotSoundRestart);
    connect(ui.refreshoutputButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotSoundRestart);
    connect(ui.sndtestButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotSoundTestDevices);
    connect(ui.snddefaultButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotSoundDefaults);

    //sound events
    connect(ui.spackBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PreferencesDlg::slotSPackChange);
    connect(ui.newuserButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventNewUser);
    connect(ui.rmuserButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventRemoveUser);
    connect(ui.srvlostButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventServerLost);
    connect(ui.usermsgButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventUserTextMsg);
    connect(ui.sentmsgButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventSentTextMsg);
    connect(ui.chanmsgButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventChannelTextMsg);
    connect(ui.sentchannelmsgButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventSentChannelMsg);
    connect(ui.bcastmsgButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventBroadcastTextMsg);
    connect(ui.hotkeyButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventHotKey);
    connect(ui.chansilentButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventSilence);
    connect(ui.videosessionButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventNewVideo);
    connect(ui.desktopsessionButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventNewDesktop);
    connect(ui.fileupdButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventFilesUpdated);
    connect(ui.transferdoneButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventFileTxDone);
    connect(ui.questionmodeBtn, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventQuestionMode);
    connect(ui.desktopaccessBtn, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventDesktopAccess);
    connect(ui.userloggedinButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventUserLoggedIn);
    connect(ui.userloggedoutButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventUserLoggedOut);
    connect(ui.voiceactonButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventVoiceActOn);
    connect(ui.voiceactoffButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventVoiceActOff);
    connect(ui.muteallonButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventMuteAllOn);
    connect(ui.mutealloffButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventMuteAllOff);
    connect(ui.transmitqueueheadButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventTransmitQueueHead);
    connect(ui.transmitqueuestopButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventTransmitQueueStop);
    connect(ui.voiceacttrigButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventVoiceActTrig);
    connect(ui.voiceactstopButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventVoiceActStop);
    connect(ui.voiceactmeonButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventVoiceActMeOn);
    connect(ui.voiceactmeoffButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventVoiceActMeOff);

    //text to speech
    m_ttsmodel = new TTSEventsModel(this);
    ui.ttsTreeView->setModel(m_ttsmodel);
    connect(ui.ttsTreeView, &QAbstractItemView::doubleClicked, this, &PreferencesDlg::slotTTSEventToggled);
    connect(ui.ttsengineComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PreferencesDlg::slotUpdateTTSTab);
    connect(ui.ttsEnableallButton, &QAbstractButton::clicked, this, &PreferencesDlg::slotTTSEnableAll);
    connect(ui.ttsClearallButton, &QAbstractButton::clicked, this, &PreferencesDlg::slotTTSClearAll);
    connect(ui.ttsRevertButton, &QAbstractButton::clicked, this, &PreferencesDlg::slotTTSRevert);
    ui.ttsTreeView->header()->restoreState(ttSettings->value(SETTINGS_DISPLAY_TTSHEADER).toByteArray());

    //keyboard shortcuts
    connect(ui.voiceactButton, &QAbstractButton::clicked,
            [&] (bool checked) { shortcutSetup(HOTKEY_VOICEACTIVATION, checked, ui.voiceactEdit); });
    connect(ui.volumeincButton, &QAbstractButton::clicked,
            [&] (bool checked) { shortcutSetup(HOTKEY_INCVOLUME, checked, ui.volumeincEdit); });
    connect(ui.volumedecButton, &QAbstractButton::clicked,
            [&] (bool checked) { shortcutSetup(HOTKEY_DECVOLUME, checked, ui.volumedecEdit); });
    connect(ui.muteallButton, &QAbstractButton::clicked,
            [&] (bool checked) { shortcutSetup(HOTKEY_MUTEALL, checked, ui.muteallEdit); });
    connect(ui.voicegainincButton, &QAbstractButton::clicked,
            [&] (bool checked) { shortcutSetup(HOTKEY_MICROPHONEGAIN_INC, checked, ui.voicegainincEdit); });
    connect(ui.voicegaindecButton, &QAbstractButton::clicked,
            [&] (bool checked) { shortcutSetup(HOTKEY_MICROPHONEGAIN_DEC, checked, ui.voicegaindecEdit); });
    connect(ui.videotxButton, &QAbstractButton::clicked,
            [&] (bool checked) { shortcutSetup(HOTKEY_VIDEOTX, checked, ui.videotxEdit); });
    connect (ui.initsoundButton, &QAbstractButton::clicked,
            [&] (bool checked) { shortcutSetup(HOTKEY_REINITSOUNDDEVS, checked, ui.initsoundEdit); });
    connect (ui.showhideButton, &QAbstractButton::clicked,
            [&] (bool checked) { shortcutSetup(HOTKEY_SHOWHIDE_WINDOW, checked, ui.showhideEdit); });

    //video tab
    connect(ui.vidcapdevicesBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PreferencesDlg::slotVideoCaptureDevChange);
    connect(ui.captureformatsBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PreferencesDlg::slotVideoResolutionChange);
    connect(ui.vidtestButton, &QAbstractButton::clicked, this, &PreferencesDlg::slotTestVideoFormat);
    connect(ui.vidrgb32RadioButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotImageFormatChange);
    connect(ui.vidi420RadioButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotImageFormatChange);
    connect(ui.vidyuy2RadioButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotImageFormatChange);
    connect(ui.vidcodecBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            ui.vidcodecStackedWidget, &QStackedWidget::setCurrentIndex);
    connect(ui.vidfmtToolButton, &QAbstractButton::clicked, this, &PreferencesDlg::slotCustomImageFormat);
    connect(ui.viddefaultButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotDefaultVideoSettings);

    m_video_ready = (TT_GetFlags(ttInst) & CLIENT_VIDEOCAPTURE_READY);

    slotTabChange(GENERAL_TAB);
}

PreferencesDlg::~PreferencesDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_PREFERENCESWINDOWPOS, saveGeometry());
    TT_CloseSoundLoopbackTest(m_sndloop);
}


void PreferencesDlg::initDevices()
{
    int default_inputid = SOUNDDEVICEID_DEFAULT, default_outputid = SOUNDDEVICEID_DEFAULT;
    TT_GetDefaultSoundDevices(&default_inputid, &default_outputid);
    
    m_sounddevices = getSoundDevices();

    for (auto s : m_sounddevices)
    {
        qDebug() << "#" << s.nDeviceID << "Sound system:" << s.nSoundSystem << _Q(s.szDeviceName) << _Q(s.szDeviceID);
    }

    //output device determines the selected sound system
    SoundSystem sndsys = (SoundSystem)ttSettings->value(SETTINGS_SOUND_SOUNDSYSTEM,
                                                        SOUNDSYSTEM_NONE).toInt();
    if(sndsys == SOUNDSYSTEM_NONE)
    {
        for(int i=0;i<m_sounddevices.size();i++)
        {
            int deviceid = ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE,
                                             TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL).toInt();
            QString uid = ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE_UID, "").toString();
            if(m_sounddevices[i].nDeviceID == deviceid &&
               _Q(m_sounddevices[i].szDeviceID) == uid)
            {
                sndsys = m_sounddevices[i].nSoundSystem;
                break;
            }
        }
    }

    if(sndsys == SOUNDSYSTEM_NONE)
    {
        for(int i=0;i<m_sounddevices.size();i++)
        {
            if(m_sounddevices[i].nDeviceID == default_outputid)
            {
                sndsys = m_sounddevices[i].nSoundSystem;
                break;
            }
        }
    }

    showDevices(sndsys);

    ui.echocancelBox->setChecked(ttSettings->value(SETTINGS_SOUND_ECHOCANCEL,
                                                   SETTINGS_SOUND_ECHOCANCEL_DEFAULT).toBool());
    ui.agcBox->setChecked(ttSettings->value(SETTINGS_SOUND_AGC, SETTINGS_SOUND_AGC_DEFAULT).toBool());
    ui.denoisingBox->setChecked(ttSettings->value(SETTINGS_SOUND_DENOISING,
                                                  SETTINGS_SOUND_DENOISING_DEFAULT).toBool());
    slotUpdateSoundCheckBoxes();
}

SoundSystem PreferencesDlg::getSoundSystem()
{
    SoundSystem sndsys = SOUNDSYSTEM_NONE;
    
    if(ui.dsoundButton->isChecked())
        sndsys = SOUNDSYSTEM_DSOUND;
    if(ui.winmmButton->isChecked())
        sndsys = SOUNDSYSTEM_WINMM;
    if(ui.wasapiButton->isChecked())
        sndsys = SOUNDSYSTEM_WASAPI;
    if(ui.alsaButton->isChecked())
        sndsys = SOUNDSYSTEM_ALSA;
    if(ui.coreaudioButton->isChecked())
        sndsys = SOUNDSYSTEM_COREAUDIO;

    // ensure tab has been initialized, otherwise sound system will end up as 'none'
    Q_ASSERT(sndsys != SOUNDSYSTEM_NONE);

    return sndsys;
}

void PreferencesDlg::showDevices(SoundSystem snd)
{
    int default_inputid = SOUNDDEVICEID_DEFAULT, default_outputid = SOUNDDEVICEID_DEFAULT;
    TT_GetDefaultSoundDevicesEx(snd, &default_inputid, &default_outputid);

    ui.inputdevBox->clear();
    ui.outputdevBox->clear();

    switch(snd)
    {
    case SOUNDSYSTEM_DSOUND :
        ui.dsoundButton->setChecked(true);break;
    case SOUNDSYSTEM_WINMM :
        ui.winmmButton->setChecked(true);break;
    case SOUNDSYSTEM_WASAPI :
        ui.wasapiButton->setChecked(true);break;
    case SOUNDSYSTEM_ALSA :
        ui.alsaButton->setChecked(true);break;
    case SOUNDSYSTEM_COREAUDIO :
        ui.coreaudioButton->setChecked(true);break;
    case SOUNDSYSTEM_NONE :
    case SOUNDSYSTEM_OPENSLES_ANDROID :
    case SOUNDSYSTEM_AUDIOUNIT :
        break;
    }

    SoundDevice dev;
    int devid;
    QString uid;

    //for WASAPI, make a default device in the same way as DirectSound
    if(snd == SOUNDSYSTEM_WASAPI)
    {
        ui.inputdevBox->addItem(tr("Default Input Device"), SOUNDDEVICEID_DEFAULT);
        default_inputid = SOUNDDEVICEID_DEFAULT;
    }

    bool add_no_device = false;
    for(int i=0;i<m_sounddevices.size();i++)
    {
        add_no_device |= m_sounddevices[i].nDeviceID == TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL;

        if (m_sounddevices[i].nSoundSystem != snd || m_sounddevices[i].nMaxInputChannels == 0)
            continue;
        ui.inputdevBox->addItem(_Q(m_sounddevices[i].szDeviceName),
                                m_sounddevices[i].nDeviceID);
    }

    if(add_no_device)
    {
        ui.inputdevBox->addItem(tr("No Sound Device"), TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL);
    }

    //if possible use GUID to select correct device
    devid = ttSettings->value(SETTINGS_SOUND_INPUTDEVICE, default_inputid).toInt();
    uid = ttSettings->value(SETTINGS_SOUND_INPUTDEVICE_UID, "").toString();
    if (getSoundDevice(uid, true, m_sounddevices, dev) && dev.nDeviceID != devid)
        devid = dev.nDeviceID;

    int index = ui.inputdevBox->findData(devid);
    if(index >= 0)
        ui.inputdevBox->setCurrentIndex(index);

    //for WASAPI, make a default device in the same way as DirectSound
    if(snd == SOUNDSYSTEM_WASAPI)
    {
        ui.outputdevBox->addItem(tr("Default Output Device"), SOUNDDEVICEID_DEFAULT);
        default_outputid = SOUNDDEVICEID_DEFAULT;
    }

    for(int i=0;i<m_sounddevices.size();i++)
    {
        if(m_sounddevices[i].nSoundSystem != snd || m_sounddevices[i].nMaxOutputChannels == 0)
            continue;
        ui.outputdevBox->addItem(_Q(m_sounddevices[i].szDeviceName), m_sounddevices[i].nDeviceID);
    }

    if(add_no_device)
    {
        ui.outputdevBox->addItem(tr("No Sound Device"), TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL);
    }

    //if possible use GUID to select correct device
    devid = ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE, default_outputid).toInt();
    uid = ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE_UID, "").toString();
    if (getSoundDevice(uid, false, m_sounddevices, dev) && dev.nDeviceID != devid)
        devid = dev.nDeviceID;

    index = ui.outputdevBox->findData(devid);
    if(index >= 0)
        ui.outputdevBox->setCurrentIndex(index);
}

void PreferencesDlg::slotUpdateSoundCheckBoxes()
{
    int inputid = TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL, outputid = TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL;
    if(ui.inputdevBox->count())
        inputid = getCurrentItemData(ui.inputdevBox, inputid).toInt();
    if(ui.outputdevBox->count())
        outputid = getCurrentItemData(ui.outputdevBox, outputid).toInt();

    //if user selected SOUNDDEVICEID_DEFAULT then get the default device
    if(inputid == SOUNDDEVICEID_DEFAULT)
        TT_GetDefaultSoundDevicesEx(getSoundSystem(), &inputid, nullptr);
    if(outputid == SOUNDDEVICEID_DEFAULT)
        TT_GetDefaultSoundDevicesEx(getSoundSystem(), nullptr, &outputid);

    SoundDevice in_dev = {}, out_dev = {};
    getSoundDevice(inputid, m_sounddevices, in_dev);
    getSoundDevice(outputid, m_sounddevices, out_dev);

    bool echocapable = isSoundDeviceEchoCapable(in_dev, out_dev);
    ui.echocancelBox->setEnabled(echocapable);
    ui.echocancelBox->setChecked(ui.echocancelBox->isEnabled() && ui.echocancelBox->isChecked());
}

bool PreferencesDlg::getSoundFile(QString& filename)
{
    QString basename = QFileInfo(filename).fileName();
    QString dir = QFileInfo(filename).absolutePath();
    QString tmp = QFileDialog::getOpenFileName(this, tr("Open Wave File"), dir,
                                               tr("Wave files (*.wav)"), &basename);
    tmp = QDir::toNativeSeparators(tmp);
    if(tmp.size())
        filename = tmp;
    return tmp.size();
}

void PreferencesDlg::slotTabChange(int index)
{
    //don't fill again
    if(m_modtab.find(index) != m_modtab.end())
        return;

    switch(index)
    {
    case GENERAL_TAB : //general
    {
        ui.nicknameEdit->setText(ttSettings->value(SETTINGS_GENERAL_NICKNAME, SETTINGS_GENERAL_NICKNAME_DEFAULT).toString());
        switch (Gender(ttSettings->value(SETTINGS_GENERAL_GENDER, SETTINGS_GENERAL_GENDER_DEFAULT).toInt()))
        {
        case GENDER_MALE :
            ui.maleRadioButton->setChecked(true);
            break;
        case GENDER_FEMALE:
            ui.femaleRadioButton->setChecked(true);
            break;
        case GENDER_NEUTRAL:
        default:
            ui.neutralRadioButton->setChecked(true);
            break;
        }

        QString bearwareid = ttSettings->value(SETTINGS_GENERAL_BEARWARE_USERNAME).toString();
        ui.bearwareidEdit->setText(bearwareid);
        if (bearwareid.size())
            ui.setupBearWareLoginButton->setText(tr("&Reset"));
        ui.syncWebUserCheckBox->setChecked(ttSettings->value(SETTINGS_GENERAL_RESTOREUSERSETTINGS,
                                                             SETTINGS_GENERAL_RESTOREUSERSETTINGS_DEFAULT).toBool());

        ui.awaySpinBox->setValue(ttSettings->value(SETTINGS_GENERAL_AUTOAWAY, SETTINGS_GENERAL_AUTOAWAY_DEFAULT).toInt());
        slotUpdateASBAccessibleName();
        ui.pttChkBox->setChecked(ttSettings->value(SETTINGS_GENERAL_PUSHTOTALK).toBool());
        ui.pttlockChkBox->setChecked(ttSettings->value(SETTINGS_GENERAL_PUSHTOTALKLOCK,
                                                       SETTINGS_GENERAL_PUSHTOTALKLOCK_DEFAULT).toBool());
        slotEnablePushToTalk(ttSettings->value(SETTINGS_GENERAL_PUSHTOTALK).toBool());
        ui.voiceactChkBox->setChecked(ttSettings->value(SETTINGS_GENERAL_VOICEACTIVATED,
                                                        SETTINGS_GENERAL_VOICEACTIVATED_DEFAULT).toBool());
        loadHotKeySettings(HOTKEY_PUSHTOTALK, m_hotkey);
        ui.keycompEdit->setText(getHotKeyText(m_hotkey));
        break;
    }
    case DISPLAY_TAB : //display
    {
        ui.startminimizedChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_STARTMINIMIZED, false).toBool());
        ui.trayChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_TRAYMINIMIZE, false).toBool());
        ui.alwaysontopChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_ALWAYSONTOP, false).toBool());
        ui.vumeterChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_VU_METER_UPDATES,
                                                       SETTINGS_DISPLAY_VU_METER_UPDATES_DEFAULT).toBool());
        ui.msgpopupChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_MESSAGEPOPUP, true).toBool());
        ui.videodlgChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_VIDEOPOPUP, false).toBool());
        ui.returnvidChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_VIDEORETURNTOGRID,
                                                         SETTINGS_DISPLAY_VIDEORETURNTOGRID_DEFAULT).toBool());
        ui.vidtextChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_VIDEOTEXT_SHOW, false).toBool());
        ui.desktopdlgChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_DESKTOPPOPUP, false).toBool());
        ui.usercountChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_USERSCOUNT, 
                                                         SETTINGS_DISPLAY_USERSCOUNT_DEFAULT).toBool());
        ui.lasttalkChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_LASTTALK, 
                                                        SETTINGS_DISPLAY_LASTTALK_DEFAULT).toBool());
        ui.msgtimestampChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_MSGTIMESTAMP, false).toBool());
        ui.chanexpChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_CHANEXP, SETTINGS_DISPLAY_CHANEXP_DEFAULT).toBool());
        ui.logstatusbarChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_LOGSTATUSBAR, SETTINGS_DISPLAY_LOGSTATUSBAR_DEFAULT).toBool());
        ui.updatesChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_APPUPDATE, SETTINGS_DISPLAY_APPUPDATE_DEFAULT).toBool());
        ui.betaUpdatesChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_APPUPDATE_BETA, SETTINGS_DISPLAY_APPUPDATE_BETA_DEFAULT).toBool());
        slotUpdateUpdDlgChkBox();
        ui.updatesDlgChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_APPUPDATE_DLG,
                                                     SETTINGS_DISPLAY_APPUPDATE_DLG_DEFAULT).toBool());
        ui.maxtextSpinBox->setValue(ttSettings->value(SETTINGS_DISPLAY_MAX_STRING,
                                                      SETTINGS_DISPLAY_MAX_STRING_DEFAULT).toInt());
        ui.showusernameChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_SHOWUSERNAME,
                                                            SETTINGS_DISPLAY_SHOWUSERNAME_DEFAULT).toBool());
        ui.emojiChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_EMOJI,
                                                     SETTINGS_DISPLAY_EMOJI_DEFAULT).toBool());
        ui.ServnameChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_SERVNAME,
                                                     SETTINGS_DISPLAY_SERVNAME_DEFAULT).toBool());

        ui.languageBox->clear();
        ui.languageBox->addItem("");
        QDir dir( TRANSLATE_FOLDER, "*.qm", QDir::Name, QDir::Files);
        QStringList languages = dir.entryList();
        for(int i=0;i<languages.size();i++)
        {
            QString name = languages[i].left(languages[i].size()-3);
            ui.languageBox->addItem(name, name);
        }
        QString lang = ttSettings->value(SETTINGS_DISPLAY_LANGUAGE, "").toString();
        int index = ui.languageBox->findData(lang);;
        if(index>=0)
            ui.languageBox->setCurrentIndex(index);
        ui.chanDbClickBox->addItem(tr("Do nothing"), ACTION_NOTHING);
        ui.chanDbClickBox->addItem(tr("Join only"), ACTION_JOIN);
        ui.chanDbClickBox->addItem(tr("Leave only"), ACTION_LEAVE);
        ui.chanDbClickBox->addItem(tr("Join or leave"), ACTION_JOINLEAVE);
        DoubleClickChannelAction chanDbClickAction = DoubleClickChannelAction(ttSettings->value(SETTINGS_DISPLAY_CHANDBCLICK, SETTINGS_DISPLAY_CHANDBCLICK_DEFAULT).toUInt());
        setCurrentItemData(ui.chanDbClickBox, chanDbClickAction);

        ui.channelsortComboBox->addItem(tr("Ascending"), CHANNELSORT_ASCENDING);
        ui.channelsortComboBox->addItem(tr("Popularity"), CHANNELSORT_POPULARITY);
        setCurrentItemData(ui.channelsortComboBox, ttSettings->value(SETTINGS_DISPLAY_CHANNELSORT, SETTINGS_DISPLAY_CHANNELSORT_DEFAULT).toInt());

        ui.closeFileDlgChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_CLOSE_FILEDIALOG, SETTINGS_DISPLAY_CLOSE_FILEDIALOG_DEFAULT).toBool());
        ui.dlgExcludeChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_CHANEXCLUDE_DLG, SETTINGS_DISPLAY_CHANEXCLUDE_DLG_DEFAULT).toBool());
    }
    break;
    case CONNECTION_TAB :  //connection
    {
        ui.autoconnectChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_AUTOCONNECT, SETTINGS_CONNECTION_AUTOCONNECT_DEFAULT).toBool());
        ui.reconnectChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_RECONNECT, SETTINGS_CONNECTION_RECONNECT_DEFAULT).toBool());
        ui.autojoinChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_AUTOJOIN, SETTINGS_CONNECTION_AUTOJOIN_DEFAULT).toBool());
        ui.maxpayloadChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_QUERYMAXPAYLOAD, SETTINGS_CONNECTION_QUERYMAXPAYLOAD_DEFAULT).toBool());
#ifdef Q_OS_WIN32
        QString appPath = QApplication::applicationFilePath();
        appPath = QDir::toNativeSeparators(appPath);
        ui.winfwChkBox->setChecked(TT_Firewall_AppExceptionExists(_W(appPath)));
#endif
        ui.subusermsgChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_USERMSG, SETTINGS_CONNECTION_SUBSCRIBE_USERMSG_DEFAULT).toBool());
        ui.subchanmsgChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_CHANNELMSG, SETTINGS_CONNECTION_SUBSCRIBE_CHANNELMSG_DEFAULT).toBool());
        ui.subbcastmsgChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_BROADCASTMSG, SETTINGS_CONNECTION_SUBSCRIBE_BROADCASTMSG_DEFAULT).toBool());
        ui.subvoiceChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_VOICE, SETTINGS_CONNECTION_SUBSCRIBE_VOICE_DEFAULT).toBool());
        ui.subvidcapChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_VIDEOCAPTURE, SETTINGS_CONNECTION_SUBSCRIBE_VIDEOCAPTURE_DEFAULT).toBool());
        ui.subdesktopChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_DESKTOP, SETTINGS_CONNECTION_SUBSCRIBE_DESKTOP_DEFAULT).toBool());
        ui.submediafileChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_MEDIAFILE, SETTINGS_CONNECTION_SUBSCRIBE_MEDIAFILE_DEFAULT).toBool());
        ui.tcpportSpinBox->setValue(ttSettings->value(SETTINGS_CONNECTION_TCPPORT, 0).toInt());
        ui.udpportSpinBox->setValue(ttSettings->value(SETTINGS_CONNECTION_UDPPORT, 0).toInt());
    }
    break;
    case SOUND_TAB :  //sound system
        initDevices();
        ui.mediavsvoiceSlider->setValue(ttSettings->value(SETTINGS_SOUND_MEDIASTREAM_VOLUME,
                                        SETTINGS_SOUND_MEDIASTREAM_VOLUME_DEFAULT).toInt());
        break;
    case SOUNDEVENTS_TAB :  //sound events
    {
        ui.spackBox->clear();
        ui.spackBox->addItem(tr("Default"));
        QDir dir( SOUNDSPATH, "", QDir::Name, QDir::AllDirs|QDir::NoSymLinks|QDir::NoDotAndDotDot);
        QStringList aspack = dir.entryList();
        for(int i=0;i<aspack.size();i++)
        {
            QString packname = aspack[i].left(aspack[i].size());
            ui.spackBox->addItem(packname, packname);
        }
        QString pack = ttSettings->value(SETTINGS_SOUNDS_PACK, QCoreApplication::translate("MainWindow", SETTINGS_SOUNDS_PACK_DEFAULT)).toString();
        int index = ui.spackBox->findData(pack);
        if(index>=0)
            ui.spackBox->setCurrentIndex(index);
        ui.newuserEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_NEWUSER, SETTINGS_SOUNDEVENT_NEWUSER_DEFAULT).toString());
        ui.rmuserEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_REMOVEUSER, SETTINGS_SOUNDEVENT_REMOVEUSER_DEFAULT).toString());
        ui.srvlostEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_SERVERLOST, SETTINGS_SOUNDEVENT_SERVERLOST_DEFAULT).toString());
        ui.usermsgEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_USERMSG, SETTINGS_SOUNDEVENT_USERMSG_DEFAULT).toString());
        ui.sentmsgEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_USERMSGSENT, SETTINGS_SOUNDEVENT_USERMSGSENT_DEFAULT).toString());
        ui.chanmsgEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_CHANNELMSG, SETTINGS_SOUNDEVENT_CHANNELMSG_DEFAULT).toString());
        ui.sentchannelmsgEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_CHANNELMSGSENT, SETTINGS_SOUNDEVENT_CHANNELMSGSENT_DEFAULT).toString());
        ui.bcastmsgEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_BROADCASTMSG, SETTINGS_SOUNDEVENT_BROADCASTMSG_DEFAULT).toString());
        ui.hotkeyEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_HOTKEY, SETTINGS_SOUNDEVENT_HOTKEY_DEFAULT).toString());
        ui.chansilentEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_SILENCE).toString());
        ui.videosessionEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_NEWVIDEO, SETTINGS_SOUNDEVENT_NEWVIDEO_DEFAULT).toString());
        ui.desktopsessionEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_NEWDESKTOP, SETTINGS_SOUNDEVENT_NEWDESKTOP_DEFAULT).toString());
        ui.fileupdEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_FILESUPD, SETTINGS_SOUNDEVENT_FILESUPD_DEFAULT).toString());
        ui.transferdoneEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_FILETXDONE, SETTINGS_SOUNDEVENT_FILETXDONE_DEFAULT).toString());
        ui.questionmodeEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_QUESTIONMODE, SETTINGS_SOUNDEVENT_QUESTIONMODE_DEFAULT).toString());
        ui.desktopaccessEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_DESKTOPACCESS, SETTINGS_SOUNDEVENT_DESKTOPACCESS_DEFAULT).toString());
        ui.userloggedinEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_USERLOGGEDIN, SETTINGS_SOUNDEVENT_USERLOGGEDIN_DEFAULT).toString());
        ui.userloggedoutEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_USERLOGGEDOUT, SETTINGS_SOUNDEVENT_USERLOGGEDOUT_DEFAULT).toString());
        ui.voiceactonEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTON, SETTINGS_SOUNDEVENT_VOICEACTON_DEFAULT).toString());
        ui.voiceactoffEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTOFF, SETTINGS_SOUNDEVENT_VOICEACTOFF_DEFAULT).toString());
        ui.muteallonEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_MUTEALLON, SETTINGS_SOUNDEVENT_MUTEALLON_DEFAULT).toString());
        ui.mutealloffEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_MUTEALLOFF, SETTINGS_SOUNDEVENT_MUTEALLOFF_DEFAULT).toString());
        ui.transmitqueueheadEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_TRANSMITQUEUE_HEAD, SETTINGS_SOUNDEVENT_TRANSMITQUEUE_HEAD_DEFAULT).toString());
        ui.transmitqueuestopEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_TRANSMITQUEUE_STOP, SETTINGS_SOUNDEVENT_TRANSMITQUEUE_STOP_DEFAULT).toString());
        ui.voiceacttrigEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTTRIG, SETTINGS_SOUNDEVENT_VOICEACTTRIG_DEFAULT).toString());
        ui.voiceactstopEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTSTOP, SETTINGS_SOUNDEVENT_VOICEACTSTOP_DEFAULT).toString());
        ui.voiceactmeonEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTMEON, SETTINGS_SOUNDEVENT_VOICEACTMEON_DEFAULT).toString());
        ui.voiceactmeoffEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTMEOFF, SETTINGS_SOUNDEVENT_VOICEACTMEOFF_DEFAULT).toString());
        break;
    }
    case TTSEVENTS_TAB :
    {
        TTSEvents events = ttSettings->value(SETTINGS_TTS_ACTIVEEVENTS, SETTINGS_TTS_ACTIVEEVENTS_DEFAULT).toULongLong();
        m_ttsmodel->setTTSEvents(events);
        ui.ttsengineComboBox->addItem(tr("None"), TTSENGINE_NONE);
#if defined(QT_TEXTTOSPEECH_LIB)
        ui.ttsengineComboBox->addItem(tr("Default"), TTSENGINE_QT);
#endif
#if defined(Q_OS_WIN)
        ui.ttsengineComboBox->addItem(tr("Tolk"), TTSENGINE_TOLK);
#elif defined(Q_OS_LINUX)
        if (QFile::exists(TTSENGINE_NOTIFY_PATH))
            ui.ttsengineComboBox->addItem(tr("Libnotify"), TTSENGINE_NOTIFY);
#elif defined(Q_OS_MACX)

#endif

        TextToSpeechEngine ttsEngine = TextToSpeechEngine(ttSettings->value(SETTINGS_TTS_ENGINE, SETTINGS_TTS_ENGINE_DEFAULT).toUInt());
        setCurrentItemData(ui.ttsengineComboBox, ttsEngine);

        slotUpdateTTSTab();

        break;
    }
    case SHORTCUTS_TAB :  //shortcuts
    {
        hotkey_t hotkey;
        if(loadHotKeySettings(HOTKEY_VOICEACTIVATION, hotkey))
        {
            m_hotkeys.insert(HOTKEY_VOICEACTIVATION, hotkey);
            ui.voiceactEdit->setText(getHotKeyText(hotkey));
            ui.voiceactButton->setChecked(true);
        }
        hotkey.clear();
        if(loadHotKeySettings(HOTKEY_INCVOLUME, hotkey))
        {
            m_hotkeys.insert(HOTKEY_INCVOLUME, hotkey);
            ui.volumeincEdit->setText(getHotKeyText(hotkey));
            ui.volumeincButton->setChecked(true);
        }
        hotkey.clear();
        if(loadHotKeySettings(HOTKEY_DECVOLUME, hotkey))
        {
            m_hotkeys.insert(HOTKEY_DECVOLUME, hotkey);
            ui.volumedecEdit->setText(getHotKeyText(hotkey));
            ui.volumedecButton->setChecked(true);
        }
        hotkey.clear();
        if(loadHotKeySettings(HOTKEY_MUTEALL, hotkey))
        {
            m_hotkeys.insert(HOTKEY_MUTEALL, hotkey);
            ui.muteallEdit->setText(getHotKeyText(hotkey));
            ui.muteallButton->setChecked(true);
        }
        hotkey.clear();
        if(loadHotKeySettings(HOTKEY_MICROPHONEGAIN_INC, hotkey))
        {
            m_hotkeys.insert(HOTKEY_MICROPHONEGAIN_INC, hotkey);
            ui.voicegainincEdit->setText(getHotKeyText(hotkey));
            ui.voicegainincButton->setChecked(true);
        }
        hotkey.clear();
        if(loadHotKeySettings(HOTKEY_MICROPHONEGAIN_DEC, hotkey))
        {
            m_hotkeys.insert(HOTKEY_MICROPHONEGAIN_DEC, hotkey);
            ui.voicegaindecEdit->setText(getHotKeyText(hotkey));
            ui.voicegaindecButton->setChecked(true);
        }
        hotkey.clear();
        if(loadHotKeySettings(HOTKEY_VIDEOTX, hotkey))
        {
            m_hotkeys.insert(HOTKEY_VIDEOTX, hotkey);
            ui.videotxEdit->setText(getHotKeyText(hotkey));
            ui.videotxButton->setChecked(true);
        }
        hotkey.clear();
        if (loadHotKeySettings(HOTKEY_REINITSOUNDDEVS, hotkey))
        {
            m_hotkeys.insert(HOTKEY_REINITSOUNDDEVS, hotkey);
            ui.initsoundEdit->setText(getHotKeyText(hotkey));
            ui.initsoundButton->setChecked(true);
        }
        hotkey.clear();
        if (loadHotKeySettings(HOTKEY_SHOWHIDE_WINDOW, hotkey))
        {
            m_hotkeys.insert(HOTKEY_SHOWHIDE_WINDOW, hotkey);
            ui.showhideEdit->setText(getHotKeyText(hotkey));
            ui.showhideButton->setChecked(true);
        }
    }
    break;
    case VIDCAP_TAB : //video capture
    {
        int count = 0;
        TT_GetVideoCaptureDevices(nullptr, &count);
        m_videodevices.resize(count);
        if(count)
            TT_GetVideoCaptureDevices(&m_videodevices[0], &count);
        else
        {
            ui.vidcapdevicesBox->setEnabled(false);
            ui.captureformatsBox->setEnabled(false);
            ui.vidyuy2RadioButton->setEnabled(false);
            ui.vidi420RadioButton->setEnabled(false); 
            ui.vidrgb32RadioButton->setEnabled(false);
            ui.vidtestButton->setEnabled(false);
            ui.viddefaultButton->setEnabled(false);
        }

        initDefaultVideoFormat(m_vidfmt);
        loadVideoFormat(m_vidfmt);

        switch(m_vidfmt.picFourCC)
        {
        case FOURCC_I420 :
            ui.vidi420RadioButton->setChecked(true);
            break;
        case FOURCC_YUY2 :
            ui.vidyuy2RadioButton->setChecked(true);
            break;
        case FOURCC_RGB32 :
            ui.vidrgb32RadioButton->setChecked(true);
            break;
        case FOURCC_NONE :
            break;
        }

        for(int i=0;i<count;i++)
            ui.vidcapdevicesBox->addItem(_Q(m_videodevices[i].szDeviceName), 
                                         _Q(m_videodevices[i].szDeviceID));

        int index = ui.vidcapdevicesBox->findData(ttSettings->value(SETTINGS_VIDCAP_DEVICEID).toString());
        if(index>=0)
            ui.vidcapdevicesBox->setCurrentIndex(index);
        else
            slotDefaultVideoSettings();

        int vidcodec = ttSettings->value(SETTINGS_VIDCAP_CODEC, DEFAULT_VIDEO_CODEC).toInt();

        ui.vidcodecBox->addItem("WebM VP8", WEBM_VP8_CODEC);

        ui.vp8bitrateSpinBox->setValue(ttSettings->value(SETTINGS_VIDCAP_WEBMVP8_BITRATE,
                                                         DEFAULT_WEBMVP8_BITRATE).toInt());

        int vidindex = ui.vidcodecBox->findData(vidcodec);
        ui.vidcodecBox->setCurrentIndex(vidindex);
    }
    break;
    default :
        Q_ASSERT(0);
    }
    m_modtab.insert(index);
}

void PreferencesDlg::slotSaveChanges()
{
    if(m_modtab.find(GENERAL_TAB) != m_modtab.end())
    {
        ttSettings->setValue(SETTINGS_GENERAL_NICKNAME, ui.nicknameEdit->text());
        if (ui.maleRadioButton->isChecked())
            ttSettings->setValue(SETTINGS_GENERAL_GENDER, GENDER_MALE);
        else if (ui.femaleRadioButton->isChecked())
            ttSettings->setValue(SETTINGS_GENERAL_GENDER, GENDER_FEMALE);
        else
            ttSettings->setValue(SETTINGS_GENERAL_GENDER, GENDER_NEUTRAL);
        ttSettings->setValue(SETTINGS_GENERAL_AUTOAWAY, ui.awaySpinBox->value());
        saveHotKeySettings(HOTKEY_PUSHTOTALK, m_hotkey);
        ttSettings->setValue(SETTINGS_GENERAL_PUSHTOTALK, ui.pttChkBox->isChecked());
        ttSettings->setValue(SETTINGS_GENERAL_PUSHTOTALKLOCK, ui.pttlockChkBox->isChecked());
        ttSettings->setValue(SETTINGS_GENERAL_VOICEACTIVATED, ui.voiceactChkBox->isChecked());
        ttSettings->setValue(SETTINGS_GENERAL_RESTOREUSERSETTINGS, ui.syncWebUserCheckBox->isChecked());
    }
    if(m_modtab.find(DISPLAY_TAB) != m_modtab.end())
    {
        ttSettings->setValue(SETTINGS_DISPLAY_STARTMINIMIZED, ui.startminimizedChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_TRAYMINIMIZE,  ui.trayChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_ALWAYSONTOP, ui.alwaysontopChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_VU_METER_UPDATES, ui.vumeterChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_MESSAGEPOPUP, ui.msgpopupChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_VIDEOPOPUP, ui.videodlgChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_VIDEORETURNTOGRID, ui.returnvidChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_VIDEOTEXT_SHOW, ui.vidtextChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_DESKTOPPOPUP, ui.desktopdlgChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_USERSCOUNT, ui.usercountChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_LASTTALK, ui.lasttalkChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_MSGTIMESTAMP, ui.msgtimestampChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_CHANEXP, ui.chanexpChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_LOGSTATUSBAR, ui.logstatusbarChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_APPUPDATE, ui.updatesChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_APPUPDATE_BETA, ui.betaUpdatesChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_APPUPDATE_DLG, ui.updatesDlgChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_MAX_STRING, ui.maxtextSpinBox->value());
        ttSettings->setValue(SETTINGS_DISPLAY_SHOWUSERNAME, ui.showusernameChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_EMOJI, ui.emojiChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_SERVNAME, ui.ServnameChkBox->isChecked());

        int index = ui.languageBox->currentIndex();
        if(index >= 0)
        {
            QString lang = ui.languageBox->itemData(index).toString();
            if(lang != ttSettings->value(SETTINGS_DISPLAY_LANGUAGE).toString())
            {
                QApplication::removeTranslator(ttTranslator);
                delete ttTranslator;
                ttTranslator = nullptr;
                if(!lang.isEmpty())
                {
                    ttTranslator = new QTranslator();
                    if(ttTranslator->load(lang, TRANSLATE_FOLDER))
                        QApplication::installTranslator(ttTranslator);
                    else
                    {
                        delete ttTranslator;
                        ttTranslator = nullptr;
                    }
                }
            }
            ttSettings->setValue(SETTINGS_DISPLAY_LANGUAGE,
                        ui.languageBox->itemData(index).toString());
        }
        ttSettings->setValue(SETTINGS_DISPLAY_CHANDBCLICK, getCurrentItemData(ui.chanDbClickBox, ACTION_JOINLEAVE));
        ttSettings->setValue(SETTINGS_DISPLAY_CHANNELSORT, getCurrentItemData(ui.channelsortComboBox, CHANNELSORT_ASCENDING));
        ttSettings->setValue(SETTINGS_DISPLAY_CLOSE_FILEDIALOG, ui.closeFileDlgChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_CHANEXCLUDE_DLG, ui.dlgExcludeChkBox->isChecked());
    }
    if(m_modtab.find(CONNECTION_TAB) != m_modtab.end())
    {
        ttSettings->setValue(SETTINGS_CONNECTION_AUTOCONNECT, ui.autoconnectChkBox->isChecked());
        ttSettings->setValue(SETTINGS_CONNECTION_RECONNECT, ui.reconnectChkBox->isChecked());
        ttSettings->setValue(SETTINGS_CONNECTION_AUTOJOIN, ui.autojoinChkBox->isChecked());
        ttSettings->setValue(SETTINGS_CONNECTION_QUERYMAXPAYLOAD, ui.maxpayloadChkBox->isChecked());

#ifdef Q_OS_WIN32
        QString appPath = QApplication::applicationFilePath();
        appPath = QDir::toNativeSeparators(appPath);
        if(ui.winfwChkBox->isChecked() != (bool)TT_Firewall_AppExceptionExists(_W(appPath)))
        {
            if(ui.winfwChkBox->isChecked())
            {
                if(!TT_Firewall_AddAppException(_W(QString(APPNAME_SHORT)), _W(appPath)))
                    QMessageBox::critical(this, tr("Windows Firewall"),
                            tr("Failed to add %1 to Windows Firewall exception list")
                            .arg(APPNAME_SHORT));
            }
            else
            {
                if(!TT_Firewall_RemoveAppException(_W(appPath)))
                    QMessageBox::critical(this, tr("Windows Firewall"),
                            tr("Failed to remove %1 from Windows Firewall exception list")
                            .arg(APPNAME_SHORT));
            }
        }
#endif

        ttSettings->setValue(SETTINGS_CONNECTION_SUBSCRIBE_USERMSG, ui.subusermsgChkBox->isChecked());
        ttSettings->setValue(SETTINGS_CONNECTION_SUBSCRIBE_CHANNELMSG, ui.subchanmsgChkBox->isChecked());
        ttSettings->setValue(SETTINGS_CONNECTION_SUBSCRIBE_BROADCASTMSG, ui.subbcastmsgChkBox->isChecked());
        ttSettings->setValue(SETTINGS_CONNECTION_SUBSCRIBE_VOICE, ui.subvoiceChkBox->isChecked());
        ttSettings->setValue(SETTINGS_CONNECTION_SUBSCRIBE_VIDEOCAPTURE, ui.subvidcapChkBox->isChecked());
        ttSettings->setValue(SETTINGS_CONNECTION_SUBSCRIBE_DESKTOP, ui.subdesktopChkBox->isChecked());
        ttSettings->setValue(SETTINGS_CONNECTION_SUBSCRIBE_MEDIAFILE, ui.submediafileChkBox->isChecked());

        ttSettings->setValue(SETTINGS_CONNECTION_TCPPORT, ui.tcpportSpinBox->value());
        ttSettings->setValue(SETTINGS_CONNECTION_UDPPORT, ui.udpportSpinBox->value());
    }
    if(m_modtab.find(SOUND_TAB) != m_modtab.end())
    {
        int inputid = TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL, outputid = TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL;
        if(ui.inputdevBox->count())
            inputid = ui.inputdevBox->itemData(ui.inputdevBox->currentIndex()).toInt();
        if(ui.outputdevBox->count())
            outputid = ui.outputdevBox->itemData(ui.outputdevBox->currentIndex()).toInt();

        int def_inputid = TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL, def_outputid = TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL;
        TT_GetDefaultSoundDevicesEx(getSoundSystem(), &def_inputid, &def_outputid);
        TT_CloseSoundLoopbackTest(m_sndloop);

        SoundSystem oldsndsys = SoundSystem(ttSettings->value(SETTINGS_SOUND_SOUNDSYSTEM, SOUNDSYSTEM_NONE).toInt());

        if(ui.wasapiButton->isChecked())
            ttSettings->setValue(SETTINGS_SOUND_SOUNDSYSTEM, SOUNDSYSTEM_WASAPI);
        else if(ui.dsoundButton->isChecked())
        {
            ttSettings->setValue(SETTINGS_SOUND_SOUNDSYSTEM, SOUNDSYSTEM_DSOUND);
            //in DirectSound 'Primary Sound Capture Driver' and 'Primary Sound Driver'
            //should be treated as default device
            int tmp_inputid, tmp_outputid;
            if(TT_GetDefaultSoundDevicesEx(SOUNDSYSTEM_DSOUND, &tmp_inputid, &tmp_outputid))
            {
                if(inputid == tmp_inputid)
                    inputid = SOUNDDEVICEID_DEFAULT;
                if(outputid == tmp_outputid)
                    outputid = SOUNDDEVICEID_DEFAULT;
            }
        }
        else if(ui.winmmButton->isChecked())
        {
            ttSettings->setValue(SETTINGS_SOUND_SOUNDSYSTEM, SOUNDSYSTEM_WINMM);
/*
            //in WinMM 'Sound Mapper - Input' and 'Sound Mapper - Output'
            //should be treated as default device
            int tmp_inputid, tmp_outputid;
            if(TT_GetDefaultSoundDevicesEx(SOUNDSYSTEM_WINMM, &tmp_inputid, &tmp_outputid))
            {
                if(inputid == tmp_inputid)
                    inputid = SOUNDDEVICEID_DEFAULT;
                if(outputid == tmp_outputid)
                    outputid = SOUNDDEVICEID_DEFAULT;
            }
*/
        }
        else if(ui.coreaudioButton->isChecked())
            ttSettings->setValue(SETTINGS_SOUND_SOUNDSYSTEM, SOUNDSYSTEM_COREAUDIO);
        else if(ui.alsaButton->isChecked())
            ttSettings->setValue(SETTINGS_SOUND_SOUNDSYSTEM, SOUNDSYSTEM_ALSA);

        ttSettings->setValue(SETTINGS_SOUND_INPUTDEVICE_UID, "");
        for(int i=0;i<m_sounddevices.size();i++)
        {
            if(inputid == m_sounddevices[i].nDeviceID)
                ttSettings->setValue(SETTINGS_SOUND_INPUTDEVICE_UID, 
                                     _Q(m_sounddevices[i].szDeviceID));
        }

        ttSettings->setValue(SETTINGS_SOUND_OUTPUTDEVICE_UID, "");
        for(int i=0;i<m_sounddevices.size();i++)
        {
            if(outputid == m_sounddevices[i].nDeviceID)
                ttSettings->setValue(SETTINGS_SOUND_OUTPUTDEVICE_UID, 
                                     _Q(m_sounddevices[i].szDeviceID));
        }

        // reinit sound device if anything has changed
        bool sndsysinit = oldsndsys != ttSettings->value(SETTINGS_SOUND_SOUNDSYSTEM).toInt();
        sndsysinit |= ttSettings->value(SETTINGS_SOUND_INPUTDEVICE, SETTINGS_SOUND_INPUTDEVICE_DEFAULT).toInt() != inputid;
        sndsysinit |= ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE, SETTINGS_SOUND_OUTPUTDEVICE_DEFAULT).toInt() != outputid;
        // Sound devices that has SoundDeviceEffects must also be restarted if AGC, AEC or denoise has changed
        sndsysinit |= ttSettings->value(SETTINGS_SOUND_ECHOCANCEL, SETTINGS_SOUND_ECHOCANCEL_DEFAULT).toBool() != ui.echocancelBox->isChecked();
        sndsysinit |= ttSettings->value(SETTINGS_SOUND_AGC, SETTINGS_SOUND_AGC_DEFAULT).toBool() != ui.agcBox->isChecked();
        sndsysinit |= ttSettings->value(SETTINGS_SOUND_DENOISING, SETTINGS_SOUND_DENOISING_DEFAULT).toBool() != ui.denoisingBox->isChecked();

        ttSettings->setValue(SETTINGS_SOUND_INPUTDEVICE, inputid);
        ttSettings->setValue(SETTINGS_SOUND_OUTPUTDEVICE, outputid);
        ttSettings->setValue(SETTINGS_SOUND_ECHOCANCEL, ui.echocancelBox->isChecked());
        ttSettings->setValue(SETTINGS_SOUND_AGC, ui.agcBox->isChecked());
        ttSettings->setValue(SETTINGS_SOUND_DENOISING, ui.denoisingBox->isChecked());

        ttSettings->setValue(SETTINGS_SOUND_MEDIASTREAM_VOLUME, ui.mediavsvoiceSlider->value());

        if (sndsysinit)
        {
            QStringList errs = initSelectedSoundDevices(m_devin, m_devout);
            for (QString err : errs)
            {
                QMessageBox::critical(this, tr("Sound System"), err);
            }

            if (errs.size())
            {
                initDefaultSoundDevices(m_devin, m_devout);
            }
        }
    }
    if(m_modtab.find(SOUNDEVENTS_TAB) != m_modtab.end())
    {
        ttSettings->setValue(SETTINGS_SOUNDS_PACK, ui.spackBox->currentText());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_NEWUSER, ui.newuserEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_REMOVEUSER, ui.rmuserEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_SERVERLOST, ui.srvlostEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_USERMSG, ui.usermsgEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_USERMSGSENT, ui.sentmsgEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_CHANNELMSG, ui.chanmsgEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_CHANNELMSGSENT, ui.sentchannelmsgEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_BROADCASTMSG, ui.bcastmsgEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_HOTKEY, ui.hotkeyEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_SILENCE, ui.chansilentEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_NEWVIDEO, ui.videosessionEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_NEWDESKTOP, ui.desktopsessionEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_FILESUPD, ui.fileupdEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_FILETXDONE, ui.transferdoneEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_QUESTIONMODE, ui.questionmodeEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_DESKTOPACCESS, ui.desktopaccessEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_USERLOGGEDIN, ui.userloggedinEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_USERLOGGEDOUT, ui.userloggedoutEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_VOICEACTON, ui.voiceactonEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_VOICEACTOFF, ui.voiceactoffEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_MUTEALLON, ui.muteallonEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_MUTEALLOFF, ui.mutealloffEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_TRANSMITQUEUE_HEAD, ui.transmitqueueheadEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_TRANSMITQUEUE_STOP, ui.transmitqueuestopEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_VOICEACTTRIG, ui.voiceacttrigEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_VOICEACTSTOP, ui.voiceactstopEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_VOICEACTMEON, ui.voiceactmeonEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_VOICEACTMEOFF, ui.voiceactmeoffEdit->text());
    }
    if(m_modtab.find(SHORTCUTS_TAB) != m_modtab.end())
    {
#ifdef Q_OS_WIN32
        TT_HotKey_Unregister(ttInst, HOTKEY_VOICEACTIVATION);
        TT_HotKey_Unregister(ttInst, HOTKEY_INCVOLUME);
        TT_HotKey_Unregister(ttInst, HOTKEY_DECVOLUME);
        TT_HotKey_Unregister(ttInst, HOTKEY_MUTEALL);
        TT_HotKey_Unregister(ttInst, HOTKEY_MICROPHONEGAIN_INC);
        TT_HotKey_Unregister(ttInst, HOTKEY_MICROPHONEGAIN_DEC);
        TT_HotKey_Unregister(ttInst, HOTKEY_VIDEOTX);
        TT_HotKey_Unregister(ttInst, HOTKEY_REINITSOUNDDEVS);
        TT_HotKey_Unregister(ttInst, HOTKEY_SHOWHIDE_WINDOW);
#endif
        deleteHotKeySettings(HOTKEY_VOICEACTIVATION);
        deleteHotKeySettings(HOTKEY_INCVOLUME);
        deleteHotKeySettings(HOTKEY_DECVOLUME);
        deleteHotKeySettings(HOTKEY_MUTEALL);
        deleteHotKeySettings(HOTKEY_MICROPHONEGAIN_INC);
        deleteHotKeySettings(HOTKEY_MICROPHONEGAIN_DEC);
        deleteHotKeySettings(HOTKEY_VIDEOTX);
        deleteHotKeySettings(HOTKEY_REINITSOUNDDEVS);
        deleteHotKeySettings(HOTKEY_SHOWHIDE_WINDOW);

        hotkeys_t::iterator ite = m_hotkeys.begin();
        while(ite != m_hotkeys.end())
        {
            saveHotKeySettings(ite.key(), *ite);
#ifdef Q_OS_WIN32
            TT_HotKey_Register(ttInst, ite.key(), &(*ite)[0], ite->size());
#endif
            ite++;
        }
    }
    if(m_modtab.find(VIDCAP_TAB) != m_modtab.end())
    {
        QString devapi, devid;
        int viddev_index = ui.vidcapdevicesBox->currentIndex();
        int fmt_itemdata = ui.captureformatsBox->itemData(ui.captureformatsBox->currentIndex()).toInt();
        bool modified = false;
        if( viddev_index >= 0 &&
            viddev_index < m_videodevices.size())
        {
            devapi = _Q(m_videodevices[viddev_index].szCaptureAPI);
            devid = _Q(m_videodevices[viddev_index].szDeviceID);
            
            if(fmt_itemdata == CUSTOMVIDEOFORMAT_INDEX)
            {
                if(ui.vidi420RadioButton->isChecked())
                    m_vidfmt.picFourCC = FOURCC_I420;
                if(ui.vidyuy2RadioButton->isChecked())
                    m_vidfmt.picFourCC = FOURCC_YUY2;
                if(ui.vidrgb32RadioButton->isChecked())
                    m_vidfmt.picFourCC = FOURCC_RGB32;
            }
            else if(fmt_itemdata >= 0 &&
                    fmt_itemdata < m_videodevices[viddev_index].nVideoFormatsCount)
            {
                m_vidfmt = m_videodevices[viddev_index].videoFormats[fmt_itemdata];
            }

            VideoFormat oldfmt;
            loadVideoFormat(oldfmt);

            modified = ttSettings->value(SETTINGS_VIDCAP_DEVICEID) != devid ||
                       memcmp(&m_vidfmt, &oldfmt, sizeof(oldfmt)) != 0;

            ttSettings->setValue(SETTINGS_VIDCAP_DEVICEID, devid);
            saveVideoFormat(m_vidfmt);
        }

        int codec_index = ui.vidcodecBox->currentIndex();

        int vp8_bitrate = ui.vp8bitrateSpinBox->value();
        modified |= ttSettings->value(SETTINGS_VIDCAP_CODEC) != ui.vidcodecBox->itemData(codec_index) ||
                    ttSettings->value(SETTINGS_VIDCAP_WEBMVP8_BITRATE) != vp8_bitrate;

        ttSettings->setValue(SETTINGS_VIDCAP_CODEC, ui.vidcodecBox->itemData(codec_index).toInt());
        ttSettings->setValue(SETTINGS_VIDCAP_WEBMVP8_BITRATE, vp8_bitrate);

        if(modified && m_video_ready)
        {
            TT_CloseVideoCaptureDevice(ttInst);
            if(!initVideoCaptureFromSettings())
                QMessageBox::critical(this, tr("Video Device"), 
                tr("Failed to initialize video device"));
        }
    }
    if (m_modtab.find(TTSEVENTS_TAB) != m_modtab.end())
    {
        ttSettings->setValue(SETTINGS_TTS_ACTIVEEVENTS, m_ttsmodel->getTTSEvents());
        ttSettings->setValue(SETTINGS_TTS_ENGINE, getCurrentItemData(ui.ttsengineComboBox, TTSENGINE_NONE));
        ttSettings->setValue(SETTINGS_TTS_VOICE, ui.ttsVoiceComboBox->currentIndex());
        ttSettings->setValue(SETTINGS_TTS_RATE, ui.ttsVoiceRateSpinBox->value());
        ttSettings->setValue(SETTINGS_TTS_VOLUME, ui.ttsVoiceVolumeSpinBox->value());
#if defined(Q_OS_LINUX)
        ttSettings->setValue(SETTINGS_TTS_TIMESTAMP, ui.ttsNotifTimestampSpinBox->value());
#elif defined(Q_OS_WIN)
        ttSettings->setValue(SETTINGS_TTS_SAPI, ui.ttsForceSapiChkBox->isChecked());
#elif defined(Q_OS_DARWIN)
        ttSettings->setValue(SETTINGS_TTS_SPEAKLISTS, ui.ttsSpeakListsChkBox->isChecked());
#endif
        ttSettings->setValue(SETTINGS_DISPLAY_TTSHEADER, ui.ttsTreeView->header()->saveState());
    }
}

void PreferencesDlg::slotCancelChanges()
{
    //if user tested video settings, we need to revert to what it was before
    if(m_video_ready && (TT_GetFlags(ttInst) & CLIENT_VIDEOCAPTURE_READY) == 0)
    {
        if(!initVideoCaptureFromSettings())
            QMessageBox::critical(this, tr("Video Device"), 
            tr("Failed to initialize video device"));
    }
}

void PreferencesDlg::slotEnableBearWareID(bool /*checked*/)
{
    QString bearwareid = ttSettings->value(SETTINGS_GENERAL_BEARWARE_USERNAME).toString();

    if (bearwareid.size())
    {
        ttSettings->setValue(SETTINGS_GENERAL_BEARWARE_USERNAME, QString());
        ui.bearwareidEdit->setText(QString());
        ui.setupBearWareLoginButton->setText("&Activate");
    }
    else
    {
        BearWareLoginDlg dlg(this);
        if(dlg.exec())
        {
            bearwareid = ttSettings->value(SETTINGS_GENERAL_BEARWARE_USERNAME).toString();
            ui.bearwareidEdit->setText(bearwareid);
            ui.setupBearWareLoginButton->setText("&Reset");
        }
    }
}

void PreferencesDlg::slotEnablePushToTalk(bool checked)
{
    ui.setupkeysButton->setEnabled(checked);
    ui.keycompEdit->setEnabled(checked);
    ui.pttlockChkBox->setEnabled(checked);
#if defined(Q_OS_LINUX) && QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    // push/release only supported on X11
    ui.pttlockChkBox->setEnabled(false);
    ui.pttlockChkBox->setChecked(checked);
#endif
}

void PreferencesDlg::slotSetupHotkey()
{
    KeyCompDlg dlg(this);
    if(!dlg.exec())
        return;

    m_hotkey = dlg.m_hotkey;
    ui.keycompEdit->setText(getHotKeyText(m_hotkey));
}

void PreferencesDlg::slotLanguageChange(int /*index*/)
{

}

void PreferencesDlg::slotSelectVideoText()
{
    VideoTextDlg dlg(this);
    dlg.exec();
}

void PreferencesDlg::slotUpdateUpdDlgChkBox()
{
    if (ui.updatesChkBox->isChecked() || ui.betaUpdatesChkBox->isChecked())
        ui.updatesDlgChkBox->setEnabled(true);
    else
        ui.updatesDlgChkBox->setEnabled(false);
}

void PreferencesDlg::slotDesktopAccess()
{
    DesktopAccessDlg dlg(this);
    dlg.exec();
}

void PreferencesDlg::slotSoundSystemChange()
{
    showDevices(getSoundSystem());
}

void PreferencesDlg::slotSoundInputChange(int index)
{
    int channels = 0;
    int deviceid = ui.inputdevBox->itemData(index).toInt();

    if(deviceid == SOUNDDEVICEID_DEFAULT)
        TT_GetDefaultSoundDevicesEx(getSoundSystem(), &deviceid, nullptr);

    SoundDevice dev;
    QString devinfo;
    if(getSoundDevice(deviceid, m_sounddevices, dev))
    {
        channels = dev.nMaxInputChannels;
        devinfo = tr("Max Input Channels %1").arg(channels);
        devinfo += ". ";
        devinfo += tr("Sample Rates:");
        for(int i=0;dev.inputSampleRates[i]>0 && i < TT_SAMPLERATES_MAX;i++)
            devinfo += " " + QString::number(dev.inputSampleRates[i]);
    }
    ui.inputinfoLabel->setText(devinfo);
    slotUpdateSoundCheckBoxes();
}

void PreferencesDlg::slotSoundOutputChange(int index)
{
    int channels = 0;
    int deviceid = ui.outputdevBox->itemData(index).toInt();

    if(deviceid == SOUNDDEVICEID_DEFAULT)
        TT_GetDefaultSoundDevicesEx(getSoundSystem(), nullptr, &deviceid);

    SoundDevice dev;
    QString devinfo;
    if(getSoundDevice(deviceid, m_sounddevices, dev))
    {
        channels = dev.nMaxOutputChannels;
        devinfo = tr("Max Output Channels %1").arg(channels);
        devinfo += ". ";
        devinfo += tr("Sample Rates:");
        for(int i=0;dev.outputSampleRates[i]>0 && i < TT_SAMPLERATES_MAX;i++)
            devinfo += " " + QString::number(dev.outputSampleRates[i]);
    }
    ui.outputinfoLabel->setText(devinfo);
    slotUpdateSoundCheckBoxes();
}

void PreferencesDlg::slotSoundRestart()
{
    ClientFlags flags = TT_GetFlags(ttInst);

    if(flags & CLIENT_SNDINOUTPUT_DUPLEX)
        TT_CloseSoundDuplexDevices(ttInst);
    if(flags & CLIENT_SNDINPUT_READY)
        TT_CloseSoundInputDevice(ttInst);
    if(flags & CLIENT_SNDOUTPUT_READY)
        TT_CloseSoundOutputDevice(ttInst);

    bool success = true;
    if (TT_RestartSoundSystem())
    {
        initDevices();

        success = initSelectedSoundDevices(m_devin, m_devout).empty();
    }
    
    if(!success)
        QMessageBox::critical(this, tr("Refresh Sound Devices"),
                              tr("Failed to restart sound systems. Please restart application."));
}

void PreferencesDlg::slotSoundTestDevices(bool checked)
{
    if(checked)
    {
        SoundSystem sndsys = getSoundSystem();

        int inputid = ui.inputdevBox->itemData(ui.inputdevBox->currentIndex()).toInt();
        int outputid = ui.outputdevBox->itemData(ui.outputdevBox->currentIndex()).toInt();

        if(inputid == SOUNDDEVICEID_DEFAULT)
            TT_GetDefaultSoundDevicesEx(sndsys, &inputid, nullptr);
        if(outputid == SOUNDDEVICEID_DEFAULT)
            TT_GetDefaultSoundDevicesEx(sndsys, nullptr, &outputid);

        //adapt to output device's sample rate (required for WASAPI)
        SoundDevice in_dev = {}, out_dev = {};
        getSoundDevice(inputid, m_sounddevices, in_dev);
        getSoundDevice(outputid, m_sounddevices, out_dev);

        int samplerate = getSoundDuplexSampleRate(in_dev, out_dev);
        bool duplex = samplerate != 0;
        if (samplerate == 0)
            samplerate = out_dev.nDefaultSampleRate;
        int channels = 1;

        AudioPreprocessor preprocessor = initDefaultAudioPreprocessor(WEBRTC_AUDIOPREPROCESSOR);
        preprocessor.webrtc.gaincontroller2.bEnable = ui.agcBox->isChecked();
        preprocessor.webrtc.noisesuppression.bEnable = ui.denoisingBox->isChecked();

        if (!duplex && (in_dev.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_AEC))
        {
            SoundDeviceEffects effects = {};
            effects.bEnableEchoCancellation = ui.echocancelBox->isChecked();
            preprocessor.webrtc.echocanceller.bEnable = FALSE;

            duplex = effects.bEnableEchoCancellation && in_dev.nSoundSystem == SOUNDSYSTEM_WASAPI;

            QMessageBox::information(this, tr("Test Selected"),
                tr("This sound device configuration gives suboptimal echo cancellation. Check manual for details."));

            m_sndloop = TT_StartSoundLoopbackTestEx(inputid, outputid, samplerate,
                                                    channels, duplex, &preprocessor, &effects);
        }
        else
        {
            Q_ASSERT((ui.echocancelBox->isChecked() && duplex) || !ui.echocancelBox->isChecked());
            preprocessor.webrtc.echocanceller.bEnable = ui.echocancelBox->isChecked();

            //input and output devices MUST support the specified 'samplerate' in duplex mode
            m_sndloop = TT_StartSoundLoopbackTestEx(inputid, outputid, samplerate, channels,
                                                    duplex, &preprocessor, nullptr);
        }

        if (!m_sndloop)
        {
            QMessageBox::critical(this, tr("Sound Initialization"),
                                  tr("Failed to initialize new sound devices"));
            ui.sndtestButton->setChecked(false);
        }
    }
    else
    {
        TT_CloseSoundLoopbackTest(m_sndloop);
    }
}

void PreferencesDlg::slotSoundDefaults()
{
    int default_inputid = 0, default_outputid = 0;
    TT_GetDefaultSoundDevices(&default_inputid, &default_outputid);

    SoundDevice indev, outdev;
    getSoundDevice(default_inputid, m_sounddevices, indev);
    if (getSoundDevice(default_outputid, m_sounddevices, outdev))
        showDevices(outdev.nSoundSystem);

    setCurrentItemData(ui.inputdevBox, default_inputid);
    setCurrentItemData(ui.outputdevBox, default_outputid);
    
    bool echocapable = isSoundDeviceEchoCapable(indev, outdev);
    ui.echocancelBox->setChecked(DEFAULT_ECHO_ENABLE && echocapable);
    ui.agcBox->setChecked(DEFAULT_AGC_ENABLE);
    ui.denoisingBox->setChecked(DEFAULT_DENOISE_ENABLE);
    ui.mediavsvoiceSlider->setValue(SETTINGS_SOUND_MEDIASTREAM_VOLUME_DEFAULT);
}

void PreferencesDlg::slotEventNewUser()
{
    QString filename = ui.newuserEdit->text();
    if (getSoundFile(filename))
        ui.newuserEdit->setText(filename);
}

void PreferencesDlg::slotEventRemoveUser()
{
    QString filename = ui.rmuserEdit->text();
    if (getSoundFile(filename))
        ui.rmuserEdit->setText(filename);
}

void PreferencesDlg::slotEventServerLost()
{
    QString filename = ui.srvlostEdit->text();
    if(getSoundFile(filename))
        ui.srvlostEdit->setText(filename);
}

void PreferencesDlg::slotEventUserTextMsg()
{
    QString filename = ui.usermsgEdit->text();
    if(getSoundFile(filename))
        ui.usermsgEdit->setText(filename);
}

void PreferencesDlg::slotEventSentTextMsg()
{
    QString filename = ui.sentmsgEdit->text();
    if (getSoundFile(filename))
        ui.sentmsgEdit->setText(filename);
}

void PreferencesDlg::slotEventChannelTextMsg()
{
    QString filename = ui.chanmsgEdit->text();
    if(getSoundFile(filename))
        ui.chanmsgEdit->setText(filename);
}

void PreferencesDlg::slotEventSentChannelMsg()
{
    QString filename = ui.sentchannelmsgEdit->text();
    if (getSoundFile(filename))
        ui.sentchannelmsgEdit->setText(filename);
}

void PreferencesDlg::slotEventBroadcastTextMsg()
{
    QString filename = ui.bcastmsgEdit->text();
    if(getSoundFile(filename))
        ui.bcastmsgEdit->setText(filename);
}

void PreferencesDlg::slotEventHotKey()
{
    QString filename = ui.hotkeyEdit->text();
    if(getSoundFile(filename))
        ui.hotkeyEdit->setText(filename);
}

void PreferencesDlg::slotEventSilence()
{
    QString filename = ui.chansilentEdit->text();
    if(getSoundFile(filename))
        ui.chansilentEdit->setText(filename);
}

void PreferencesDlg::slotEventNewVideo()
{
    QString filename = ui.videosessionEdit->text();
    if(getSoundFile(filename))
        ui.videosessionEdit->setText(filename);
}

void PreferencesDlg::slotEventNewDesktop()
{
    QString filename = ui.desktopsessionEdit->text();
    if(getSoundFile(filename))
        ui.desktopsessionEdit->setText(filename);
}

void PreferencesDlg::slotEventFilesUpdated()
{
    QString filename = ui.fileupdEdit->text();
    if(getSoundFile(filename))
        ui.fileupdEdit->setText(filename);
}

void PreferencesDlg::slotEventFileTxDone()
{
    QString filename = ui.transferdoneEdit->text();
    if(getSoundFile(filename))
        ui.transferdoneEdit->setText(filename);
}

void PreferencesDlg::slotEventQuestionMode()
{
    QString filename = ui.questionmodeEdit->text();
    if(getSoundFile(filename))
        ui.questionmodeEdit->setText(filename);
}

void PreferencesDlg::slotEventDesktopAccess()
{
    QString filename = ui.desktopaccessEdit->text();
    if(getSoundFile(filename))
        ui.desktopaccessEdit->setText(filename);
}

void PreferencesDlg::slotEventUserLoggedIn()
{
    QString filename = ui.userloggedinEdit->text();
    if(getSoundFile(filename))
        ui.userloggedinEdit->setText(filename);
}

void PreferencesDlg::slotEventUserLoggedOut()
{
    QString filename = ui.userloggedoutEdit->text();
    if(getSoundFile(filename))
        ui.userloggedoutEdit->setText(filename);
}

void PreferencesDlg::slotEventVoiceActOn()
{
    QString filename = ui.voiceactonEdit->text();
    if(getSoundFile(filename))
        ui.voiceactonEdit->setText(filename);
}

void PreferencesDlg::slotEventVoiceActOff()
{
    QString filename = ui.voiceactoffEdit->text();
    if(getSoundFile(filename))
        ui.voiceactoffEdit->setText(filename);
}

void PreferencesDlg::slotEventMuteAllOn()
{
    QString filename = ui.muteallonEdit->text();
    if(getSoundFile(filename))
        ui.muteallonEdit->setText(filename);
}

void PreferencesDlg::slotEventMuteAllOff()
{
    QString filename = ui.mutealloffEdit->text();
    if(getSoundFile(filename))
        ui.mutealloffEdit->setText(filename);
}

void PreferencesDlg::slotEventTransmitQueueHead()
{
    QString filename = ui.transmitqueueheadEdit->text();
    if(getSoundFile(filename))
        ui.transmitqueueheadEdit->setText(filename);
}

void PreferencesDlg::slotEventTransmitQueueStop()
{
    QString filename = ui.transmitqueuestopEdit->text();
    if(getSoundFile(filename))
        ui.transmitqueuestopEdit->setText(filename);
}

void PreferencesDlg::slotEventVoiceActTrig()
{
    QString filename = ui.voiceacttrigEdit->text();
    if(getSoundFile(filename))
        ui.voiceacttrigEdit->setText(filename);
}

void PreferencesDlg::slotEventVoiceActStop()
{
    QString filename = ui.voiceactstopEdit->text();
    if(getSoundFile(filename))
        ui.voiceactstopEdit->setText(filename);
}

void PreferencesDlg::slotEventVoiceActMeOn()
{
    QString filename = ui.voiceactmeonEdit->text();
    if(getSoundFile(filename))
        ui.voiceactmeonEdit->setText(filename);
}

void PreferencesDlg::slotEventVoiceActMeOff()
{
    QString filename = ui.voiceactmeoffEdit->text();
    if(getSoundFile(filename))
        ui.voiceactmeoffEdit->setText(filename);
}

void PreferencesDlg::slotUpdateTTSTab()
{
    ui.label_ttsvoice->hide();
    ui.ttsVoiceComboBox->hide();

    ui.label_ttsvoicerate->hide();
    ui.ttsVoiceRateSpinBox->hide();

    ui.label_ttsvoicevolume->hide();
    ui.ttsVoiceVolumeSpinBox->hide();

    ui.label_ttsnotifTimestamp->hide();
    ui.ttsNotifTimestampSpinBox->hide();

    ui.ttsForceSapiChkBox->hide();
    ui.ttsSpeakListsChkBox->hide();

    switch (getCurrentItemData(ui.ttsengineComboBox).toUInt())
    {
    case TTSENGINE_QT :
    {
#if defined(QT_TEXTTOSPEECH_LIB)
        ui.label_ttsvoice->show();
        ui.ttsVoiceComboBox->show();
        ui.label_ttsvoicerate->show();
        ui.ttsVoiceRateSpinBox->show();
        ui.label_ttsvoicevolume->show();
        ui.ttsVoiceVolumeSpinBox->show();
#if defined(Q_OS_DARWIN)
        ui.ttsSpeakListsChkBox->show();
#endif
        delete ttSpeech;
        ttSpeech = new QTextToSpeech(this);

        ui.ttsVoiceRateSpinBox->setValue(ttSettings->value(SETTINGS_TTS_RATE, SETTINGS_TTS_RATE_DEFAULT).toDouble());
        ui.ttsVoiceVolumeSpinBox->setValue(ttSettings->value(SETTINGS_TTS_VOLUME, SETTINGS_TTS_VOLUME_DEFAULT).toDouble());
        ui.ttsVoiceComboBox->clear();
        QVector<QVoice> Voices = ttSpeech->availableVoices();
        foreach (const QVoice &voice, Voices)
        {
            ui.ttsVoiceComboBox->addItem(voice.name());
        }
        ui.ttsVoiceComboBox->setCurrentIndex(ttSettings->value(SETTINGS_TTS_VOICE).toInt());

#if defined(Q_OS_DARWIN)
        ui.ttsSpeakListsChkBox->setChecked(ttSettings->value(SETTINGS_TTS_SPEAKLISTS, SETTINGS_TTS_SPEAKLISTS_DEFAULT).toBool());
#endif
#endif /* QT_TEXTTOSPEECH_LIB */
    }
    break;
    case TTSENGINE_NOTIFY :
#if defined(Q_OS_LINUX)
    {
        ui.label_ttsnotifTimestamp->show();
        ui.ttsNotifTimestampSpinBox->show();

        ui.ttsNotifTimestampSpinBox->setValue(ttSettings->value(SETTINGS_TTS_TIMESTAMP, SETTINGS_TTS_TIMESTAMP_DEFAULT).toUInt());
    }
#endif
    break;
    case TTSENGINE_TOLK :
#if defined(ENABLE_TOLK)
    {
        ui.ttsForceSapiChkBox->show();

        bool tolkLoaded = Tolk_IsLoaded();
        if (!tolkLoaded)
            Tolk_Load();
        QString currentSR = QString("%1").arg(Tolk_DetectScreenReader());
        if (!tolkLoaded)
            Tolk_Unload();
        if(currentSR.size())
            ui.ttsForceSapiChkBox->setText(tr("Use SAPI instead of %1 screenreader").arg(currentSR));
        ui.ttsForceSapiChkBox->setChecked(ttSettings->value(SETTINGS_TTS_SAPI, SETTINGS_TTS_SAPI_DEFAULT).toBool());
    }
#endif
    break;
    case TTSENGINE_NONE :
    break;
    }
}

void PreferencesDlg::shortcutSetup(HotKeyID hotkey, bool enable, QLineEdit* shortcutedit)
{
    if (enable)
    {
        KeyCompDlg dlg(this);
        if(!dlg.exec())
            return;

        m_hotkeys.insert(hotkey, dlg.m_hotkey);
        shortcutedit->setText(getHotKeyText(dlg.m_hotkey));
    }
    else
    {
        shortcutedit->setText("");
        m_hotkeys.remove(hotkey);
    }
}

void PreferencesDlg::slotVideoCaptureDevChange(int index)
{
    if(m_videodevices.size() == 0)
        return;

    ui.captureformatsBox->clear();

    int cur_index = -1;
    for(int j=0;j<m_videodevices[index].nVideoFormatsCount;j++)
    {
        if(m_videodevices[index].videoFormats[j].nFPS_Denominator == 0)
            continue;

        int fps = m_videodevices[index].videoFormats[j].nFPS_Numerator /
            m_videodevices[index].videoFormats[j].nFPS_Denominator;
        switch(m_videodevices[index].videoFormats[j].picFourCC)
        {
        case FOURCC_I420 :
            if(ui.vidi420RadioButton->isChecked())
                break;
            else continue;
        case FOURCC_YUY2 :
            if(ui.vidyuy2RadioButton->isChecked())
                break;
            else continue;
        case FOURCC_RGB32 :
            if(ui.vidrgb32RadioButton->isChecked())
                break;
            else continue;
        default :
            //hm unknown
            qDebug() << "Unknown video format " << m_videodevices[index].videoFormats[j].picFourCC;
            continue;
        }

        QString res = QString("%1x%2, FPS %3").arg(m_videodevices[index].videoFormats[j].nWidth)
            .arg(m_videodevices[index].videoFormats[j].nHeight).arg(fps);
        ui.captureformatsBox->addItem(res, j);

        if(_Q(m_videodevices[index].szDeviceID) == ttSettings->value(SETTINGS_VIDCAP_DEVICEID) &&
            m_videodevices[index].videoFormats[j].nFPS_Numerator == m_vidfmt.nFPS_Numerator &&
            m_videodevices[index].videoFormats[j].nFPS_Denominator == m_vidfmt.nFPS_Denominator &&
            m_videodevices[index].videoFormats[j].nWidth == m_vidfmt.nWidth &&
            m_videodevices[index].videoFormats[j].nHeight == m_vidfmt.nHeight &&
            m_videodevices[index].videoFormats[j].picFourCC == m_vidfmt.picFourCC)
        {
            cur_index = j;
        }
    }

    //special case for custom format
    ui.captureformatsBox->addItem(tr("Custom video format"), CUSTOMVIDEOFORMAT_INDEX);
    if(_Q(m_videodevices[index].szDeviceID) == ttSettings->value(SETTINGS_VIDCAP_DEVICEID) &&
        cur_index == -1)
    {
        cur_index = CUSTOMVIDEOFORMAT_INDEX;
    }

    cur_index = ui.captureformatsBox->findData(cur_index);
    if(cur_index>=0)
        ui.captureformatsBox->setCurrentIndex(cur_index);
    else
    {
        ui.captureformatsBox->setCurrentIndex(0);
    }
}

void PreferencesDlg::slotTestVideoFormat()
{
    if(!ui.captureformatsBox->count())
        return;
    int dev_index = ui.vidcapdevicesBox->currentIndex();
    int fmt_itemdata = ui.captureformatsBox->itemData(ui.captureformatsBox->currentIndex()).toInt();
    if(TT_GetFlags(ttInst) & CLIENT_VIDEOCAPTURE_READY)
        TT_CloseVideoCaptureDevice(ttInst);

    if(fmt_itemdata != CUSTOMVIDEOFORMAT_INDEX)
        m_vidfmt = m_videodevices[dev_index].videoFormats[fmt_itemdata];

    if(TT_InitVideoCaptureDevice(ttInst, m_videodevices[dev_index].szDeviceID, 
        &m_vidfmt))
    {
        QSize size;
        size.setWidth(m_vidfmt.nWidth);
        size.setHeight(m_vidfmt.nHeight);
        User user = {};
        m_uservideo = new UserVideoDlg(0 | VIDEOTYPE_CAPTURE, user, size, this);
        m_uservideo->exec();
    }
    else
    {
        QMessageBox::information(this, ui.vidtestButton->text(),
                                 tr("Failed to initialize video device"));
    }

    delete m_uservideo;
    m_uservideo = nullptr;
    TT_CloseVideoCaptureDevice(ttInst);
}

void PreferencesDlg::slotVideoResolutionChange(int index)
{
    Q_UNUSED(index);
}

void PreferencesDlg::slotImageFormatChange(bool /*checked*/)
{
    if(ui.vidi420RadioButton->isChecked())
        m_vidfmt.picFourCC = FOURCC_I420;
    if(ui.vidyuy2RadioButton->isChecked())
        m_vidfmt.picFourCC = FOURCC_YUY2;
    if(ui.vidrgb32RadioButton->isChecked())
        m_vidfmt.picFourCC = FOURCC_RGB32;

    slotVideoCaptureDevChange(ui.vidcapdevicesBox->currentIndex());
}

void PreferencesDlg::slotCustomImageFormat()
{
    CustomVideoFmtDlg dlg(this, m_vidfmt);
    if(dlg.exec())
    {
        int i = ui.captureformatsBox->findData(CUSTOMVIDEOFORMAT_INDEX);
        if(i>=0)
            ui.captureformatsBox->setCurrentIndex(i);
    }
}

void PreferencesDlg::slotNewVideoFrame(int userid, int stream_id)
{
    if(m_uservideo)
        m_uservideo->uservideoWidget->slotNewVideoFrame(userid, stream_id);
}

void PreferencesDlg::slotDefaultVideoSettings()
{
    int index = ui.vidcapdevicesBox->currentIndex();
    if(index<0 || index >= m_videodevices.size())
        return;

    m_vidfmt = m_videodevices[index].videoFormats[0];
    if(m_vidfmt.picFourCC == FOURCC_NONE)
    {
        QMessageBox::information(this,
                                 tr("Default Video Capture"),
                                 tr("Unable to find preferred video capture settings"));
    }
    else
    {
        switch(m_vidfmt.picFourCC)
        {
        case FOURCC_I420 :
            ui.vidi420RadioButton->setChecked(true);
            break;
        case FOURCC_YUY2 :
            ui.vidyuy2RadioButton->setChecked(true);
            break;
        case FOURCC_RGB32 :
            ui.vidrgb32RadioButton->setChecked(true);
            break;
        case FOURCC_NONE :
            break;
        }
    }

    slotVideoCaptureDevChange(index);

    for(int i=0;i<ui.vidcodecBox->count();i++)
    {
        if(ui.vidcodecBox->itemData(i).toInt() == DEFAULT_VIDEO_CODEC)
            ui.vidcodecBox->setCurrentIndex(i);
    }

    switch(DEFAULT_VIDEO_CODEC)
    {
    case WEBM_VP8_CODEC :
        ui.vp8bitrateSpinBox->setValue(DEFAULT_WEBMVP8_BITRATE);
        break;
    case OPUS_CODEC :
    case SPEEX_VBR_CODEC :
    case SPEEX_CODEC :
    case NO_CODEC :
        break;
    }
}

void PreferencesDlg::slotTTSEventToggled(const QModelIndex &index)
{
    auto events = m_ttsmodel->getTTSEvents();
    TextToSpeechEvent e = TextToSpeechEvent(index.internalId());
    if (e & events)
        m_ttsmodel->setTTSEvents(events & ~e);
    else
        m_ttsmodel->setTTSEvents(events | e);
}

void PreferencesDlg::slotTTSEnableAll(bool /*checked*/)
{
    m_ttsmodel->setTTSEvents(~TTS_NONE);
}

void PreferencesDlg::slotTTSClearAll(bool /*checked*/)
{
    m_ttsmodel->setTTSEvents(TTS_NONE);
}

void PreferencesDlg::slotTTSRevert(bool /*checked*/)
{
    TTSEvents events = ttSettings->value(SETTINGS_TTS_ACTIVEEVENTS, SETTINGS_TTS_ACTIVEEVENTS_DEFAULT).toULongLong();
    m_ttsmodel->setTTSEvents(events);
}

void PreferencesDlg::slotUpdateASBAccessibleName()
{
    ui.awaySpinBox->setAccessibleName(QString("%1 %2 %3").arg(ui.label_2->text()).arg(ui.awaySpinBox->value()).arg(ui.label_3->text()));
}

void PreferencesDlg::slotSPackChange()
{
    ui.newuserEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_NEWUSER_DEFAULT, SETTINGS_SOUNDEVENT_NEWUSER_DEFAULT).toString());
    ui.rmuserEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_REMOVEUSER_DEFAULT, SETTINGS_SOUNDEVENT_REMOVEUSER_DEFAULT).toString());
    ui.srvlostEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_SERVERLOST_DEFAULT, SETTINGS_SOUNDEVENT_SERVERLOST_DEFAULT).toString());
    ui.usermsgEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_USERMSG_DEFAULT, SETTINGS_SOUNDEVENT_USERMSG_DEFAULT).toString());
    ui.chanmsgEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_CHANNELMSG_DEFAULT, SETTINGS_SOUNDEVENT_CHANNELMSG_DEFAULT).toString());
    ui.bcastmsgEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_BROADCASTMSG_DEFAULT, SETTINGS_SOUNDEVENT_BROADCASTMSG_DEFAULT).toString());
    ui.hotkeyEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_HOTKEY_DEFAULT, SETTINGS_SOUNDEVENT_HOTKEY_DEFAULT).toString());
    ui.videosessionEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_NEWVIDEO_DEFAULT, SETTINGS_SOUNDEVENT_NEWVIDEO_DEFAULT).toString());
    ui.desktopsessionEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_NEWDESKTOP_DEFAULT, SETTINGS_SOUNDEVENT_NEWDESKTOP_DEFAULT).toString());
    ui.fileupdEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_FILESUPD_DEFAULT, SETTINGS_SOUNDEVENT_FILESUPD_DEFAULT).toString());
    ui.transferdoneEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_FILETXDONE_DEFAULT, SETTINGS_SOUNDEVENT_FILETXDONE_DEFAULT).toString());
    ui.questionmodeEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_QUESTIONMODE_DEFAULT, SETTINGS_SOUNDEVENT_QUESTIONMODE_DEFAULT).toString());
    ui.desktopaccessEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_DESKTOPACCESS_DEFAULT, SETTINGS_SOUNDEVENT_DESKTOPACCESS_DEFAULT).toString());
    ui.userloggedinEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_USERLOGGEDIN_DEFAULT, SETTINGS_SOUNDEVENT_USERLOGGEDIN_DEFAULT).toString());
    ui.userloggedoutEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_USERLOGGEDOUT_DEFAULT, SETTINGS_SOUNDEVENT_USERLOGGEDOUT_DEFAULT).toString());
    ui.voiceactonEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTON_DEFAULT, SETTINGS_SOUNDEVENT_VOICEACTON_DEFAULT).toString());
    ui.voiceactoffEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTOFF_DEFAULT, SETTINGS_SOUNDEVENT_VOICEACTOFF_DEFAULT).toString());
    ui.muteallonEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_MUTEALLON_DEFAULT, SETTINGS_SOUNDEVENT_MUTEALLON_DEFAULT).toString());
    ui.mutealloffEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_MUTEALLOFF_DEFAULT, SETTINGS_SOUNDEVENT_MUTEALLOFF_DEFAULT).toString());
    ui.transmitqueueheadEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_TRANSMITQUEUE_HEAD_DEFAULT, SETTINGS_SOUNDEVENT_TRANSMITQUEUE_HEAD_DEFAULT).toString());
    ui.transmitqueuestopEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_TRANSMITQUEUE_STOP_DEFAULT, SETTINGS_SOUNDEVENT_TRANSMITQUEUE_STOP_DEFAULT).toString());
    ui.voiceacttrigEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTTRIG_DEFAULT, SETTINGS_SOUNDEVENT_VOICEACTTRIG_DEFAULT).toString());
    ui.voiceactstopEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTSTOP_DEFAULT, SETTINGS_SOUNDEVENT_VOICEACTSTOP_DEFAULT).toString());
    ui.voiceactmeonEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTMEON_DEFAULT, SETTINGS_SOUNDEVENT_VOICEACTMEON_DEFAULT).toString());
    ui.voiceactmeoffEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTMEOFF_DEFAULT, SETTINGS_SOUNDEVENT_VOICEACTMEOFF_DEFAULT).toString());
    QString dirtoscan = QString("%1/%2").arg(SOUNDSPATH).arg(ui.spackBox->currentText());
    QDir soundsdir( dirtoscan, "*.wav", QDir::Name, QDir::Files|QDir::NoSymLinks);
    QStringList packfile = soundsdir.entryList();
    for(int i=0;i<packfile.size();i++)
    {
        QString filename = packfile[i].left(packfile[i].size()-4);
        if(filename == "newuser")
           ui.newuserEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "removeuser")
           ui.rmuserEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "serverlost")
           ui.srvlostEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "user_msg")
           ui.usermsgEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "user_msg_sent")
           ui.sentmsgEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "channel_msg")
           ui.chanmsgEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "channel_msg_sent")
           ui.sentchannelmsgEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "broadcast_msg")
           ui.bcastmsgEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "hotkey")
           ui.hotkeyEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "videosession")
           ui.videosessionEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "desktopsession")
           ui.desktopsessionEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "fileupdate")
           ui.fileupdEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "filetx_complete")
           ui.transferdoneEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "questionmode")
           ui.questionmodeEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "desktopaccessreq")
           ui.desktopaccessEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "logged_on")
           ui.userloggedinEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "logged_off")
           ui.userloggedoutEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "vox_enable")
           ui.voiceactonEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "vox_disable")
           ui.voiceactoffEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "mute_all")
           ui.muteallonEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "unmute_all")
           ui.mutealloffEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "txqueue_start")
           ui.transmitqueueheadEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "txqueue_stop")
           ui.transmitqueuestopEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "voiceact_on")
           ui.voiceacttrigEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "voiceact_off")
           ui.voiceactstopEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "vox_me_enable")
           ui.voiceactmeonEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
        if(filename == "vox_me_disable")
           ui.voiceactmeoffEdit->setText(QString("%1/%2/%3.wav").arg(SOUNDSPATH).arg(ui.spackBox->currentText()).arg(filename));
    }
}

void PreferencesDlg::slotConfigureStatusBar()
{
    StatusBarDlg dlg(this, ttSettings->value(SETTINGS_STATUSBAR_ACTIVEEVENTS, SETTINGS_STATUSBAR_ACTIVEEVENTS_DEFAULT).toULongLong());
    dlg.exec();
}

void PreferencesDlg::keyPressEvent(QKeyEvent* e)
{
    if (ui.tabWidget->hasFocus())
    {
        if (e->key() == Qt::Key_Home && ui.tabWidget->currentIndex() != 0)
            ui.tabWidget->setCurrentIndex(0);
        else if (e->key() == Qt::Key_End && ui.tabWidget->currentIndex() != ui.tabWidget->count())
            ui.tabWidget->setCurrentIndex(ui.tabWidget->count()-1);
    }
    QDialog::keyPressEvent(e);
}

