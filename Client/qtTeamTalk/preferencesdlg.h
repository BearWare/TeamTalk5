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

#ifndef PREFERENCESDLG_H
#define PREFERENCESDLG_H

#include "ui_preferences.h"

#include "common.h"
#include "utilhotkey.h"
#include "uservideodlg.h"

#include <QSet>
#include <QLineEdit>

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
    QSet<int> m_modtab;

private:
    void slotTabChange(int index);
    void slotSaveChanges();
    void slotCancelChanges();

    // general tab
    void initGeneralTab();
    void slotEnableBearWareID(bool checked);
    void slotEnablePushToTalk(bool checked);
    void slotSetupHotkey();
    void updatePushtoTalk();

    //display tab
    void initDisplayTab();
    void slotLanguageChange(int index);
    void slotSelectVideoText();
    void slotConfigureStatusBar();
    void slotEditChatTemplates();
    void retranslateStatusAndTTS();
    void slotUpdateUpdDlgChkBox();
    void insertTSFVariable();

    //connection tab
    void initConnectionTab();
    void slotDesktopAccess();

    //sound tab
    void initSoundSystemTab();
    void initDevices();
    SoundSystem getSoundSystem();
    void showDevices(SoundSystem snd);
    QVector<SoundDevice> m_sounddevices;
    void slotSoundInputChange(int index);
    void slotSoundOutputChange(int index);
    void slotSoundRestart();
    void slotSoundTestDevices(bool checked);
    void slotSoundDefaults();
    void slotUpdateSoundCheckBoxes();
    SoundDevice& m_devin, &m_devout;
    TTSoundLoop* m_sndloop;

    //sound events
    void initSoundEventsTab();
    void slotSoundEventToggled(const QModelIndex &index);
    void SoundEventSelected(const QModelIndex &index);
    void saveCurrentFile();
    void soundEventsRestoreDefaultFile();
    void slotSoundEventsEnableAll(bool checked);
    void slotSoundEventsClearAll(bool checked);
    void slotSoundEventsRevert(bool checked);
    bool getSoundFile(QString& filename);
    void slotBrowseSoundEvent();
    class SoundEventsModel* m_soundmodel = nullptr;
    QModelIndex m_currentSoundEventsIndex;

    //TTS
    void initTTSEventsTab();
    void slotUpdateTTSTab();
    void slotTTSLocaleChanged(const QString& locale);
    void slotTTSEventToggled(const QModelIndex &index);
    void TTSEventSelected(const QModelIndex &index);
    void insertVariable();
    void saveCurrentMessage();
    void TTSRestoreDefaultMessage();
    void TTSRestoreAllDefaultMessage();
    void slotTTSEnableAll(bool checked);
    void slotTTSClearAll(bool checked);
    void slotTTSRevert(bool checked);
    void slotSPackChange();
    void updateSoundEventFileEdit();
    QMenu* m_TTSVarMenu;
    QModelIndex m_currentTTSIndex;
    class TTSEventsModel* m_ttsmodel = nullptr;

    //keyboard shortcuts
    void initShortcutsTab();
    void shortcutSetup(const QModelIndex &index);
    class ShortcutsModel* m_shortcutsmodel = nullptr;
    hotkeys_t m_hotkeys;

    //video tab
    void initVideoCaptureTab();
    void slotVideoCaptureDevChange(int index);
    void slotTestVideoFormat();
    void slotVideoResolutionChange(int index);
    void slotImageFormatChange(int index);
    void slotCustomImageFormat();
    void slotDefaultVideoSettings();
    QVector<VideoCaptureDevice> m_videodevices;
    UserVideoDlg* m_uservideo;
    bool m_video_ready;
    VideoFormat m_vidfmt;

public:
    void slotNewVideoFrame(int userid, int stream_id);
};

#endif
