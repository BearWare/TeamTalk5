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
#include "keycompdlg.h"
#include "appinfo.h"
#include "videotextdlg.h"
#include "desktopaccessdlg.h"
#include "customvideofmtdlg.h"
#include "bearwarelogindlg.h"
#include "settings.h"

#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QTranslator>

extern TTInstance* ttInst;
extern QSettings* ttSettings;
extern QTranslator* ttTranslator;

#define CUSTOMVIDEOFORMAT_INDEX -2

PreferencesDlg::PreferencesDlg(QWidget * parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
, m_uservideo(nullptr)
, m_sndloop(nullptr)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    initDefaultVideoFormat(m_vidfmt);

    connect(ui.tabWidget, SIGNAL(currentChanged(int)), SLOT(slotTabChange(int)));
    connect(ui.buttonBox, SIGNAL(accepted()), SLOT(slotSaveChanges()));
    connect(ui.buttonBox, SIGNAL(rejected()), SLOT(slotCancelChanges()));

    //general tab
    connect(ui.setupBearWareLoginButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEnableBearWareID);
    connect(ui.pttChkBox, SIGNAL(clicked(bool)), SLOT(slotEnablePushToTalk(bool)));
    connect(ui.setupkeysButton, SIGNAL(clicked()), SLOT(slotSetupHotkey()));

    //display tab
    connect(ui.languageBox, SIGNAL(currentIndexChanged(int)), 
            SLOT(slotLanguageChange(int)));
    connect(ui.vidtextsrcToolBtn, SIGNAL(clicked()),
            SLOT(slotSelectVideoText()));
    
    //connection tab
    connect(ui.subdeskinputBtn, SIGNAL(clicked()),
            SLOT(slotDesktopAccess()));

    //sound tab
    connect(ui.winmmButton, SIGNAL(clicked()), SLOT(slotSoundSystemChange()));
    connect(ui.dsoundButton, SIGNAL(clicked()), SLOT(slotSoundSystemChange()));
    connect(ui.wasapiButton, SIGNAL(clicked()), SLOT(slotSoundSystemChange()));
    connect(ui.alsaButton, SIGNAL(clicked()), SLOT(slotSoundSystemChange()));
    connect(ui.coreaudioButton, SIGNAL(clicked()), SLOT(slotSoundSystemChange()));
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
    connect(ui.inputdevBox, SIGNAL(currentIndexChanged(int)), 
            SLOT(slotSoundInputChange(int)));
    connect(ui.outputdevBox, SIGNAL(currentIndexChanged(int)), 
            SLOT(slotSoundOutputChange(int)));
    connect(ui.refreshinputButton, SIGNAL(clicked()),
            SLOT(slotSoundRestart()));
    connect(ui.refreshoutputButton, SIGNAL(clicked()),
            SLOT(slotSoundRestart()));
    connect(ui.sndtestButton, SIGNAL(clicked(bool)),
            SLOT(slotSoundTestDevices(bool)));
    connect(ui.snddefaultButton, SIGNAL(clicked()),
            SLOT(slotSoundDefaults()));
    connect(ui.sndduplexBox, SIGNAL(clicked()),
            SLOT(slotUpdateSoundCheckBoxes()));

    //sound events
    connect(ui.newuserButton, SIGNAL(clicked()),
            SLOT(slotEventNewUser()));
    connect(ui.rmuserButton, SIGNAL(clicked()),
            SLOT(slotEventRemoveUser()));
    connect(ui.srvlostButton, SIGNAL(clicked()),
            SLOT(slotEventServerLost()));
    connect(ui.usermsgButton, SIGNAL(clicked()),
            SLOT(slotEventUserTextMsg()));
    connect(ui.chanmsgButton, SIGNAL(clicked()),
            SLOT(slotEventChannelTextMsg()));
    connect(ui.bcastmsgButton, &QAbstractButton::clicked,
            this, &PreferencesDlg::slotEventBroadcastTextMsg);
    connect(ui.hotkeyButton, SIGNAL(clicked()),
            SLOT(slotEventHotKey()));
    connect(ui.chansilentButton, SIGNAL(clicked()),
            SLOT(slotEventSilence()));
    connect(ui.videosessionButton, SIGNAL(clicked()),
            SLOT(slotEventNewVideo()));
    connect(ui.desktopsessionButton, SIGNAL(clicked()),
            SLOT(slotEventNewDesktop()));
    connect(ui.fileupdButton, SIGNAL(clicked()),
            SLOT(slotEventFilesUpdated()));
    connect(ui.transferdoneButton, SIGNAL(clicked()),
            SLOT(slotEventFileTxDone()));
    connect(ui.questionmodeBtn, SIGNAL(clicked()),
            SLOT(slotEventQuestionMode()));
    connect(ui.desktopaccessBtn, SIGNAL(clicked()),
            SLOT(slotEventDesktopAccess()));

    //keyboard shortcuts
    connect(ui.voiceactButton, SIGNAL(clicked(bool)), 
            SLOT(slotShortcutVoiceActivation(bool)));
    connect(ui.volumeincButton, SIGNAL(clicked(bool)),
            SLOT(slotShortcutIncVolume(bool)));
    connect(ui.volumedecButton, SIGNAL(clicked(bool)),
            SLOT(slotShortcutDecVolume(bool)));
    connect(ui.muteallButton, SIGNAL(clicked(bool)),
            SLOT(slotShortcutMuteAll(bool)));
    connect(ui.voicegainincButton, SIGNAL(clicked(bool)),
            SLOT(slotShortcutIncVoiceGain(bool)));
    connect(ui.voicegaindecButton, SIGNAL(clicked(bool)),
            SLOT(slotShortcutDecVoiceGain(bool)));
    connect(ui.videotxButton, SIGNAL(clicked(bool)),
            SLOT(slotShortcutVideoTx(bool)));

    //video tab
    connect(ui.vidcapdevicesBox, SIGNAL(currentIndexChanged(int)), 
            SLOT(slotVideoCaptureDevChange(int)));
    connect(ui.captureformatsBox, SIGNAL(currentIndexChanged(int)),
            SLOT(slotVideoResolutionChange(int)));
    connect(ui.vidtestButton, SIGNAL(clicked()), SLOT(slotTestVideoFormat()));
    connect(ui.vidrgb32RadioButton, SIGNAL(clicked(bool)),
            SLOT(slotImageFormatChange(bool)));
    connect(ui.vidi420RadioButton, SIGNAL(clicked(bool)),
            SLOT(slotImageFormatChange(bool)));
    connect(ui.vidyuy2RadioButton, SIGNAL(clicked(bool)),
            SLOT(slotImageFormatChange(bool)));
    connect(ui.vidcodecBox, SIGNAL(currentIndexChanged(int)),
            ui.vidcodecStackedWidget, SLOT(setCurrentIndex(int)));
    connect(ui.vidfmtToolButton, SIGNAL(clicked()), SLOT(slotCustomImageFormat()));
    connect(ui.viddefaultButton, SIGNAL(clicked()),
            SLOT(slotDefaultVideoSettings()));

    m_video_ready = (TT_GetFlags(ttInst) & CLIENT_VIDEOCAPTURE_READY);

    slotTabChange(GENERAL_TAB);
}

PreferencesDlg::~PreferencesDlg()
{
    TT_CloseSoundLoopbackTest(m_sndloop);
}

void PreferencesDlg::initDevices()
{
    int default_inputid = SOUNDDEVICEID_DEFAULT, default_outputid = SOUNDDEVICEID_DEFAULT;
    TT_GetDefaultSoundDevices(&default_inputid, &default_outputid);
    
    m_sounddevices = getSoundDevices();

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

    ui.sndduplexBox->setChecked(ttSettings->value(SETTINGS_SOUND_DUPLEXMODE,
                                                  SETTINGS_SOUND_DUPLEXMODE_DEFAULT).toBool());
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
    Q_ASSERT(ui.tabWidget->currentIndex() == SOUND_TAB);
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

        if(m_sounddevices[i].nSoundSystem != snd ||
           m_sounddevices[i].nMaxInputChannels == 0)
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
    if(getSoundDevice(uid, m_sounddevices, dev) && dev.nDeviceID != devid)
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
        if(m_sounddevices[i].nSoundSystem != snd ||
           m_sounddevices[i].nMaxOutputChannels == 0)
            continue;
        ui.outputdevBox->addItem(_Q(m_sounddevices[i].szDeviceName), 
                                 m_sounddevices[i].nDeviceID);
    }

    if(add_no_device)
    {
        ui.outputdevBox->addItem(tr("No Sound Device"), TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL);
    }

    //if possible use GUID to select correct device
    devid = ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE, default_outputid).toInt();
    uid = ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE_UID, "").toString();
    if(getSoundDevice(uid, m_sounddevices, dev) && dev.nDeviceID != devid)
        devid = dev.nDeviceID;

    index = ui.outputdevBox->findData(devid);
    if(index >= 0)
        ui.outputdevBox->setCurrentIndex(index);
}

void PreferencesDlg::slotUpdateSoundCheckBoxes()
{
    int inputid = TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL, outputid = TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL;
    if(ui.inputdevBox->count())
        inputid = ui.inputdevBox->itemData(ui.inputdevBox->currentIndex()).toInt();
    if(ui.outputdevBox->count())
        outputid = ui.outputdevBox->itemData(ui.outputdevBox->currentIndex()).toInt();

    //if user selected SOUNDDEVICEID_DEFAULT then get the default device
    if(inputid == SOUNDDEVICEID_DEFAULT)
        TT_GetDefaultSoundDevicesEx(getSoundSystem(), &inputid, nullptr);
    if(outputid == SOUNDDEVICEID_DEFAULT)
        TT_GetDefaultSoundDevicesEx(getSoundSystem(), nullptr, &outputid);

    SoundDevice in_dev = {}, out_dev = {};
    getSoundDevice(inputid, m_sounddevices, in_dev);
    getSoundDevice(outputid, m_sounddevices, out_dev);

    if (out_dev.nSoundSystem == SOUNDSYSTEM_WASAPI)
    {
        // WASAPI has its own echo cancellor, AGC and denoiser so duplex mode doesn't apply
        ui.sndduplexBox->hide();
        ui.sndduplexBox->setChecked(false);
        ui.echocancelBox->setEnabled(true);
    }
    else
    {
        //sound duplex mode requires a valid input and output device with same sample rate
        ui.sndduplexBox->show();
        bool same_samplerate = false;
        for(int i=0;i<TT_SAMPLERATES_MAX && out_dev.outputSampleRates[i]>0;i++)
            same_samplerate |= in_dev.nDefaultSampleRate == out_dev.outputSampleRates[i];
        ui.sndduplexBox->setEnabled(inputid>=0 && outputid>=0 && same_samplerate);
        ui.sndduplexBox->setChecked(ui.sndduplexBox->isEnabled() &&
                                    ui.sndduplexBox->isChecked());
        //echo cancel only works in sound duplex mode
        ui.echocancelBox->setEnabled(ui.sndduplexBox->isChecked());
        ui.echocancelBox->setChecked(ui.echocancelBox->isEnabled() &&
                                     ui.echocancelBox->isChecked());
    }
}

bool PreferencesDlg::getSoundFile(QString& filename)
{
    QString tmp = QFileDialog::getOpenFileName(this, tr("Open Wave File"),
                                               "",
                                               tr("Wave files (*.wav)"));
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
        ui.nicknameEdit->setText(ttSettings->value(SETTINGS_GENERAL_NICKNAME).toString());
        ui.maleRadioButton->setChecked(ttSettings->value(SETTINGS_GENERAL_GENDER,
                                                         SETTINGS_GENERAL_GENDER_DEFAULT).toBool());
        ui.femaleRadioButton->setChecked(!ttSettings->value(SETTINGS_GENERAL_GENDER,
                                                            SETTINGS_GENERAL_GENDER_DEFAULT).toBool());
        QString bearwareid = ttSettings->value(SETTINGS_GENERAL_BEARWARE_USERNAME).toString();
        ui.bearwareidEdit->setText(bearwareid);
        if (bearwareid.size())
            ui.setupBearWareLoginButton->setText("&Reset");

        ui.awaySpinBox->setValue(ttSettings->value(SETTINGS_GENERAL_AUTOAWAY).toInt());
        ui.pttChkBox->setChecked(ttSettings->value(SETTINGS_GENERAL_PUSHTOTALK).toBool());
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
        ui.logstatusbarChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_LOGSTATUSBAR, true).toBool());
        ui.updatesChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_APPUPDATE, true).toBool());
        ui.maxtextSpinBox->setValue(ttSettings->value(SETTINGS_DISPLAY_MAX_STRING,
                                                      SETTINGS_DISPLAY_MAX_STRING_DEFAULT).toInt());
        ui.showusernameChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_SHOWUSERNAME,
                                                            SETTINGS_DISPLAY_SHOWUSERNAME_DEFAULT).toBool());

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
    }
    break;
    case CONNECTION_TAB :  //connection
    {
        ui.autoconnectChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_AUTOCONNECT, false).toBool());
        ui.reconnectChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_RECONNECT, true).toBool());
        ui.autojoinChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_AUTOJOIN, true).toBool());
        ui.maxpayloadChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_QUERYMAXPAYLOAD, false).toBool());
#ifdef Q_OS_WIN32
        QString appPath = QApplication::applicationFilePath();
        appPath = QDir::toNativeSeparators(appPath);
        ui.winfwChkBox->setChecked(TT_Firewall_AppExceptionExists(_W(appPath)));
#endif
        ui.subusermsgChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_USERMSG, true).toBool());
        ui.subchanmsgChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_CHANNELMSG, true).toBool());
        ui.subbcastmsgChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_BROADCASTMSG, true).toBool());
        ui.subvoiceChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_VOICE, true).toBool());
        ui.subvidcapChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_VIDEOCAPTURE, true).toBool());
        ui.subdesktopChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_DESKTOP, true).toBool());
        ui.submediafileChkBox->setChecked(ttSettings->value(SETTINGS_CONNECTION_SUBSCRIBE_MEDIAFILE, true).toBool());
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
        ui.newuserEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_NEWUSER).toString());
        ui.rmuserEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_REMOVEUSER).toString());
        ui.srvlostEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_SERVERLOST).toString());
        ui.usermsgEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_USERMSG).toString());
        ui.chanmsgEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_CHANNELMSG).toString());
        ui.bcastmsgEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_BROADCASTMSG).toString());
        ui.hotkeyEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_HOTKEY).toString());
        ui.chansilentEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_SILENCE).toString());
        ui.videosessionEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_NEWVIDEO).toString());
        ui.desktopsessionEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_NEWDESKTOP).toString());
        ui.fileupdEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_FILESUPD).toString());
        ui.transferdoneEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_FILETXDONE).toString());
        ui.questionmodeEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_QUESTIONMODE).toString());
        ui.desktopaccessEdit->setText(ttSettings->value(SETTINGS_SOUNDEVENT_DESKTOPACCESS).toString());
        break;
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
        ttSettings->setValue(SETTINGS_GENERAL_GENDER, ui.maleRadioButton->isChecked());
        ttSettings->setValue(SETTINGS_GENERAL_AUTOAWAY, ui.awaySpinBox->value());
        saveHotKeySettings(HOTKEY_PUSHTOTALK, m_hotkey);
        ttSettings->setValue(SETTINGS_GENERAL_PUSHTOTALK, ui.pttChkBox->isChecked());
        ttSettings->setValue(SETTINGS_GENERAL_VOICEACTIVATED, ui.voiceactChkBox->isChecked());
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
        ttSettings->setValue(SETTINGS_DISPLAY_LOGSTATUSBAR, ui.logstatusbarChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_APPUPDATE, ui.updatesChkBox->isChecked());
        ttSettings->setValue(SETTINGS_DISPLAY_MAX_STRING, ui.maxtextSpinBox->value());
        ttSettings->setValue(SETTINGS_DISPLAY_SHOWUSERNAME, ui.showusernameChkBox->isChecked());

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
        sndsysinit |= ttSettings->value(SETTINGS_SOUND_DUPLEXMODE, SETTINGS_SOUND_DUPLEXMODE_DEFAULT).toBool() != ui.sndduplexBox->isChecked();
        // Sound devices that has SoundDeviceEffects must also be restarted if AGC, AEC or denoise has changed
        sndsysinit |= ttSettings->value(SETTINGS_SOUND_ECHOCANCEL, SETTINGS_SOUND_ECHOCANCEL_DEFAULT).toBool() != ui.echocancelBox->isChecked();
        sndsysinit |= ttSettings->value(SETTINGS_SOUND_AGC, SETTINGS_SOUND_AGC_DEFAULT).toBool() != ui.agcBox->isChecked();
        sndsysinit |= ttSettings->value(SETTINGS_SOUND_DENOISING, SETTINGS_SOUND_DENOISING_DEFAULT).toBool() != ui.denoisingBox->isChecked();

        ttSettings->setValue(SETTINGS_SOUND_INPUTDEVICE, inputid);
        ttSettings->setValue(SETTINGS_SOUND_OUTPUTDEVICE, outputid);
        ttSettings->setValue(SETTINGS_SOUND_DUPLEXMODE, ui.sndduplexBox->isChecked());
        ttSettings->setValue(SETTINGS_SOUND_ECHOCANCEL, ui.echocancelBox->isChecked());
        ttSettings->setValue(SETTINGS_SOUND_AGC, ui.agcBox->isChecked());
        ttSettings->setValue(SETTINGS_SOUND_DENOISING, ui.denoisingBox->isChecked());

        ttSettings->setValue(SETTINGS_SOUND_MEDIASTREAM_VOLUME, ui.mediavsvoiceSlider->value());

        if (sndsysinit)
        {
            QStringList errs = initSelectedSoundDevices();
            for (QString err : errs)
            {
                QMessageBox::critical(this, tr("Sound System"), err);
            }

            if (errs.size())
                initDefaultSoundDevices();
        }
    }
    if(m_modtab.find(SOUNDEVENTS_TAB) != m_modtab.end())
    {
        ttSettings->setValue(SETTINGS_SOUNDEVENT_NEWUSER, ui.newuserEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_REMOVEUSER, ui.rmuserEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_SERVERLOST, ui.srvlostEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_USERMSG, ui.usermsgEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_CHANNELMSG, ui.chanmsgEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_BROADCASTMSG, ui.bcastmsgEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_HOTKEY, ui.hotkeyEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_SILENCE, ui.chansilentEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_NEWVIDEO, ui.videosessionEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_NEWDESKTOP, ui.desktopsessionEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_FILESUPD, ui.fileupdEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_FILETXDONE, ui.transferdoneEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_QUESTIONMODE, ui.questionmodeEdit->text());
        ttSettings->setValue(SETTINGS_SOUNDEVENT_DESKTOPACCESS, ui.desktopaccessEdit->text());
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
#endif
        deleteHotKeySettings(HOTKEY_VOICEACTIVATION);
        deleteHotKeySettings(HOTKEY_INCVOLUME);
        deleteHotKeySettings(HOTKEY_DECVOLUME);
        deleteHotKeySettings(HOTKEY_MUTEALL);
        deleteHotKeySettings(HOTKEY_MICROPHONEGAIN_INC);
        deleteHotKeySettings(HOTKEY_MICROPHONEGAIN_DEC);
        deleteHotKeySettings(HOTKEY_VIDEOTX);

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
    if(TT_RestartSoundSystem())
    {
        initDevices();

        success = initSelectedSoundDevices().empty();
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

        int samplerate = 16000;
        int channels = 1;

        //adapt to output device's sample rate (required for WASAPI)
        SoundDevice out_dev;
        if (getSoundDevice(outputid, m_sounddevices, out_dev))
            samplerate = out_dev.nDefaultSampleRate;

        if (out_dev.nSoundSystem == SOUNDSYSTEM_WASAPI)
        {
            SoundDeviceEffects effects = {};
            effects.bEnableAGC = ui.agcBox->isChecked();
            effects.bEnableDenoise = ui.denoisingBox->isChecked();
            effects.bEnableEchoCancellation = ui.echocancelBox->isChecked();

            bool duplex = (effects.bEnableAGC || effects.bEnableDenoise || effects.bEnableEchoCancellation);

            m_sndloop = TT_StartSoundLoopbackTestEx(inputid, outputid, samplerate,
                                                    channels, duplex, nullptr, &effects);
        }
        else
        {
            SpeexDSP spxdsp;
            ZERO_STRUCT(spxdsp);
            spxdsp.bEnableAGC = ui.agcBox->isChecked();
            spxdsp.nGainLevel = DEFAULT_AGC_GAINLEVEL;
            spxdsp.nMaxIncDBSec = DEFAULT_AGC_INC_MAXDB;
            spxdsp.nMaxDecDBSec = DEFAULT_AGC_DEC_MAXDB;
            spxdsp.nMaxGainDB = DEFAULT_AGC_GAINMAXDB;

            spxdsp.bEnableDenoise = ui.denoisingBox->isChecked();
            spxdsp.nMaxNoiseSuppressDB = DEFAULT_DENOISE_SUPPRESS;

            spxdsp.bEnableEchoCancellation = ui.echocancelBox->isChecked();
            spxdsp.nEchoSuppress = DEFAULT_ECHO_SUPPRESS;
            spxdsp.nEchoSuppressActive = DEFAULT_ECHO_SUPPRESSACTIVE;

            //input and output devices MUST support the specified 'samplerate' in duplex mode
            m_sndloop = TT_StartSoundLoopbackTest(inputid, outputid, 
                                                  samplerate, channels, 
                                                  ui.sndduplexBox->isChecked(), 
                                                  &spxdsp);
        }
        if(!m_sndloop)
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

    SoundDevice dev;
    if(getSoundDevice(default_outputid, m_sounddevices, dev))
        showDevices(dev.nSoundSystem);

    int index = ui.inputdevBox->findData(default_inputid);
    if(index>=0)
        ui.inputdevBox->setCurrentIndex(index);
    index = ui.outputdevBox->findData(default_outputid);
    if(index>=0)
        ui.outputdevBox->setCurrentIndex(index);
    
    ui.sndduplexBox->setChecked(DEFAULT_SOUND_DUPLEXMODE);
    ui.echocancelBox->setChecked(DEFAULT_ECHO_ENABLE);
    ui.agcBox->setChecked(DEFAULT_AGC_ENABLE);
    ui.denoisingBox->setChecked(DEFAULT_DENOISE_ENABLE);
    ui.mediavsvoiceSlider->setValue(SETTINGS_SOUND_MEDIASTREAM_VOLUME_DEFAULT);
}

void PreferencesDlg::slotEventNewUser()
{
    QString filename;
    if(getSoundFile(filename))
        ui.newuserEdit->setText(filename);
}

void PreferencesDlg::slotEventRemoveUser()
{
    QString filename;
    if(getSoundFile(filename))
        ui.rmuserEdit->setText(filename);
}

void PreferencesDlg::slotEventServerLost()
{
    QString filename;
    if(getSoundFile(filename))
        ui.srvlostEdit->setText(filename);
}

void PreferencesDlg::slotEventUserTextMsg()
{
    QString filename;
    if(getSoundFile(filename))
        ui.usermsgEdit->setText(filename);
}

void PreferencesDlg::slotEventChannelTextMsg()
{
    QString filename;
    if(getSoundFile(filename))
        ui.chanmsgEdit->setText(filename);
}

void PreferencesDlg::slotEventBroadcastTextMsg()
{
    QString filename;
    if(getSoundFile(filename))
        ui.bcastmsgEdit->setText(filename);
}

void PreferencesDlg::slotEventHotKey()
{
    QString filename;
    if(getSoundFile(filename))
        ui.hotkeyEdit->setText(filename);
}

void PreferencesDlg::slotEventSilence()
{
    QString filename;
    if(getSoundFile(filename))
        ui.chansilentEdit->setText(filename);
}

void PreferencesDlg::slotEventNewVideo()
{
    QString filename;
    if(getSoundFile(filename))
        ui.videosessionEdit->setText(filename);
}

void PreferencesDlg::slotEventNewDesktop()
{
    QString filename;
    if(getSoundFile(filename))
        ui.desktopsessionEdit->setText(filename);
}

void PreferencesDlg::slotEventFilesUpdated()
{
    QString filename;
    if(getSoundFile(filename))
        ui.fileupdEdit->setText(filename);
}

void PreferencesDlg::slotEventFileTxDone()
{
    QString filename;
    if(getSoundFile(filename))
        ui.transferdoneEdit->setText(filename);
}

void PreferencesDlg::slotEventQuestionMode()
{
    QString filename;
    if(getSoundFile(filename))
        ui.questionmodeEdit->setText(filename);
}

void PreferencesDlg::slotEventDesktopAccess()
{
    QString filename;
    if(getSoundFile(filename))
        ui.desktopaccessEdit->setText(filename);
}

void PreferencesDlg::slotShortcutVoiceActivation(bool checked)
{
    if(checked)
    {
        KeyCompDlg dlg(this);
        if(!dlg.exec())
            return;

        m_hotkeys.insert(HOTKEY_VOICEACTIVATION, dlg.m_hotkey);
        ui.voiceactEdit->setText(getHotKeyText(dlg.m_hotkey));
    }
    else
    {
        ui.voiceactEdit->setText("");
        m_hotkeys.remove(HOTKEY_VOICEACTIVATION);
    }
}

void PreferencesDlg::slotShortcutIncVolume(bool checked)
{
    if(checked)
    {
        KeyCompDlg dlg(this);
        if(!dlg.exec())
            return;

        m_hotkeys.insert(HOTKEY_INCVOLUME, dlg.m_hotkey);
        ui.volumeincEdit->setText(getHotKeyText(dlg.m_hotkey));
    }
    else
    {
        ui.volumeincEdit->setText("");
        m_hotkeys.remove(HOTKEY_INCVOLUME);
    }
}

void PreferencesDlg::slotShortcutDecVolume(bool checked)
{
    if(checked)
    {
        KeyCompDlg dlg(this);
        if(!dlg.exec())
            return;

        m_hotkeys.insert(HOTKEY_DECVOLUME, dlg.m_hotkey);
        ui.volumedecEdit->setText(getHotKeyText(dlg.m_hotkey));
    }
    else
    {
        ui.volumedecEdit->setText("");
        m_hotkeys.remove(HOTKEY_DECVOLUME);
    }
}

void PreferencesDlg::slotShortcutMuteAll(bool checked)
{
    if(checked)
    {
        KeyCompDlg dlg(this);
        if(!dlg.exec())
            return;

        m_hotkeys.insert(HOTKEY_MUTEALL, dlg.m_hotkey);
        ui.muteallEdit->setText(getHotKeyText(dlg.m_hotkey));
    }
    else
    {
        ui.muteallEdit->setText("");
        m_hotkeys.remove(HOTKEY_MUTEALL);
    }
}

void PreferencesDlg::slotShortcutIncVoiceGain(bool checked)
{
    if(checked)
    {
        KeyCompDlg dlg(this);
        if(!dlg.exec())
            return;

        m_hotkeys.insert(HOTKEY_MICROPHONEGAIN_INC, dlg.m_hotkey);
        ui.voicegainincEdit->setText(getHotKeyText(dlg.m_hotkey));
    }
    else
    {
        ui.voicegainincEdit->setText("");
        m_hotkeys.remove(HOTKEY_MICROPHONEGAIN_INC);
    }
}

void PreferencesDlg::slotShortcutDecVoiceGain(bool checked)
{
    if(checked)
    {
        KeyCompDlg dlg(this);
        if(!dlg.exec())
            return;

        m_hotkeys.insert(HOTKEY_MICROPHONEGAIN_DEC, dlg.m_hotkey);
        ui.voicegaindecEdit->setText(getHotKeyText(dlg.m_hotkey));
    }
    else
    {
        ui.voicegaindecEdit->setText("");
        m_hotkeys.remove(HOTKEY_MICROPHONEGAIN_DEC);
    }
}

void PreferencesDlg::slotShortcutVideoTx(bool checked)
{
    if(checked)
    {
        KeyCompDlg dlg(this);
        if(!dlg.exec())
            return;

        m_hotkeys.insert(HOTKEY_VIDEOTX, dlg.m_hotkey);
        ui.videotxEdit->setText(getHotKeyText(dlg.m_hotkey));
    }
    else
    {
        ui.videotxEdit->setText("");
        m_hotkeys.remove(HOTKEY_VIDEOTX);
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
        User user;
        ZERO_STRUCT(user);
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
