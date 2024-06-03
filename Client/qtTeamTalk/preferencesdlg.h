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

protected:
    void keyPressEvent(QKeyEvent* e) override;

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
    void slotBrowseSoundEvent();
    class SoundEventsModel* m_soundmodel = nullptr;
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
    void slotConfigureStatusBar();
    void slotUpdateUpdDlgChkBox();
    QMenu* m_TSFVarMenu;
    void insertTSFVariable();
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
    void slotSoundEventToggled(const QModelIndex &index);
    void SoundEventSelected(const QModelIndex &index);
    void saveCurrentFile();
    void soundEventsRestoreDefaultFile();
    QModelIndex m_currentSoundEventsIndex;
    void slotSoundEventsEnableAll(bool checked);
    void slotSoundEventsClearAll(bool checked);
    void slotSoundEventsRevert(bool checked);
    //TTS
    void slotUpdateTTSTab();
    void slotTTSLocaleChanged(const QString& locale);
    //keyboard shortcuts
    void shortcutSetup(HotKeyID hotkey, bool enable, QLineEdit* shortcutedit);
    //video tab
    void slotVideoCaptureDevChange(int index);
    void slotTestVideoFormat();
    void slotVideoResolutionChange(int index);
    void slotImageFormatChange(bool checked);
    void slotCustomImageFormat();
    void slotDefaultVideoSettings();
    //tts tab
    void slotTTSEventToggled(const QModelIndex &index);
    void TTSEventSelected(const QModelIndex &index);
    void insertVariable();
    void saveCurrentMessage();
    void TTSRestoreDefaultMessage();
    QMenu* m_TTSVarMenu;
    QModelIndex m_currentTTSIndex;
    void slotTTSEnableAll(bool checked);
    void slotTTSClearAll(bool checked);
    void slotTTSRevert(bool checked);
    void slotUpdateASBAccessibleName();
    void slotSPackChange();
    void updateSoundEventFileEdit();

public:
    void slotNewVideoFrame(int userid, int stream_id);
};

#endif
