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

#ifndef PREFERENCESDLG_H
#define PREFERENCESDLG_H

#include <QSet>

#include "ui_preferences.h"

#include "common.h"
#include "uservideodlg.h"
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QTextToSpeech>
#endif

class PreferencesDlg : public QDialog
{
    Q_OBJECT

public:
    PreferencesDlg(SoundDevice& devin, SoundDevice& devout, QWidget * parent = 0);
    ~PreferencesDlg();

    enum
    {
        GENERAL_TAB,
        DISPLAY_TAB,
        CONNECTION_TAB,
        SOUND_TAB,
        SOUNDEVENTS_TAB,
        TTSEVENTS_TAB,
        SHORTCUTS_TAB,
        VIDCAP_TAB
    };

private:
    Ui::PreferencesDlg ui;
    SoundDevice& m_devin, &m_devout;
    /* hotkey tab */
    hotkey_t m_hotkey;
    /* sound tab */
    void initDevices();
    SoundSystem getSoundSystem();
    void showDevices(SoundSystem snd);
    QVector<SoundDevice> m_sounddevices;
    /* sound events */
    bool getSoundFile(QString& filename);
    typedef QMap<HotKeyID, hotkey_t> hotkeys_t;
    hotkeys_t m_hotkeys;
    /* video tab */
    QVector<VideoCaptureDevice> m_videodevices;
    UserVideoDlg* m_uservideo;
    /* text-to-speech */
    class TTSEventsModel* m_ttsmodel = nullptr;

    QSet<int> m_modtab;
    bool m_video_ready;
    VideoFormat m_vidfmt;

    TTSoundLoop* m_sndloop;

private:
    void slotTabChange(int index);
    void slotSaveChanges();
    void slotCancelChanges();
    // general tab
    void slotEnableBearWareID(bool checked);
    void slotEnablePushToTalk(bool checked);
    void slotSetupHotkey();
    //display tab
    void slotLanguageChange(int index);
    void slotSelectVideoText();
    void slotUpdateLIOChkBox(bool checked);
    //connection tab
    void slotDesktopAccess();
    //sound tab
    void slotSoundSystemChange();
    void slotSoundInputChange(int index);
    void slotSoundOutputChange(int index);
    void slotSoundRestart();
    void slotSoundTestDevices(bool checked);
    void slotSoundDefaults();
    void slotUpdateSoundCheckBoxes();
    //sound events
    void slotEventNewUser();
    void slotEventRemoveUser();
    void slotEventServerLost();
    void slotEventUserTextMsg();
    void slotEventSentTextMsg();
    void slotEventChannelTextMsg();
    void slotEventSentChannelMsg();
    void slotEventBroadcastTextMsg();
    void slotEventHotKey();
    void slotEventSilence();
    void slotEventNewVideo();
    void slotEventNewDesktop();
    void slotEventFilesUpdated();
    void slotEventFileTxDone();
    void slotEventQuestionMode();
    void slotEventDesktopAccess();
    void slotEventUserLoggedIn();
    void slotEventUserLoggedOut();
    void slotEventVoiceActOn();
    void slotEventVoiceActOff();
    void slotEventMuteAllOn();
    void slotEventMuteAllOff();
    //keyboard shortcuts
    void slotShortcutVoiceActivation(bool checked);
    void slotShortcutIncVolume(bool checked);
    void slotShortcutDecVolume(bool checked);
    void slotShortcutMuteAll(bool checked);
    void slotShortcutIncVoiceGain(bool checked);
    void slotShortcutDecVoiceGain(bool checked);
    void slotShortcutVideoTx(bool checked);
    //video tab
    void slotVideoCaptureDevChange(int index);
    void slotTestVideoFormat();
    void slotVideoResolutionChange(int index);
    void slotImageFormatChange(bool checked);
    void slotCustomImageFormat();
    void slotDefaultVideoSettings();
    //tts tab
    void slotTTSEventToggled(const QModelIndex &index);

public:
    void slotNewVideoFrame(int userid, int stream_id);
};

#endif
