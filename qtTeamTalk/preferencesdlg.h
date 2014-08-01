/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#ifndef PREFERENCESDLG_H
#define PREFERENCESDLG_H

#include <QSet>

#include "ui_preferences.h"

#include "common.h"
#include "uservideodlg.h"

class PreferencesDlg : public QDialog
{
    Q_OBJECT

public:
    PreferencesDlg(QWidget * parent = 0);
    ~PreferencesDlg();

    enum
    {
        GENERAL_TAB,
        DISPLAY_TAB,
        CONNECTION_TAB,
        SOUND_TAB,
        SOUNDEVENTS_TAB,
        SHORTCUTS_TAB,
        VIDCAP_TAB
    };

private:
    Ui::PreferencesDlg ui;
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

    QSet<int> m_modtab;
    bool m_video_ready;

    TTSoundLoop* m_sndloop;

private slots:
    void slotTabChange(int index);
    void slotSaveChanges();
    void slotCancelChanges();
    void slotEnablePushToTalk(bool checked);
    void slotSetupHotkey();
    //display tab
    void slotLanguageChange(int index);
    void slotSelectVideoText();
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
    void slotEventChannelTextMsg();
    void slotEventHotKey();
    void slotEventSilence();
    void slotEventNewVideo();
    void slotEventNewDesktop();
    void slotEventFilesUpdated();
    void slotEventFileTxDone();
    void slotEventQuestionMode();
    void slotEventDesktopAccess();
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
    void slotImageFormatChange(bool checked);
    void slotDefaultVideoSettings();

public slots:
    void slotNewVideoFrame(int userid, int stream_id);
};

#endif
