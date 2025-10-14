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


#include "mediastoragedlg.h"
#include "appinfo.h"
#include "settings.h"
#include "common.h"

#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QPushButton>

extern TTInstance* ttInst;
extern NonDefaultSettings* ttSettings;

MediaStorageDlg::MediaStorageDlg(QWidget * parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_MEDIASTORAGEWINDOWPOS).toByteArray());

    connect(ui.singleCheckBox, &QAbstractButton::clicked,
            this, &MediaStorageDlg::slotUpdateUI);
    connect(ui.multipleCheckBox, &QAbstractButton::clicked,
            this, &MediaStorageDlg::slotUpdateUI);
    connect(ui.startButton, &QAbstractButton::clicked,
            this, &MediaStorageDlg::accept);
    connect(ui.stopButton, &QAbstractButton::clicked,
            this, &MediaStorageDlg::slotStop);
    connect(ui.cancelButton, &QAbstractButton::clicked,
            this, &MediaStorageDlg::reject);

    connect(ui.audioToolButton, &QAbstractButton::clicked,
            this, &MediaStorageDlg::slotSetMediaFolder);
    connect(ui.chanlogToolButton, &QAbstractButton::clicked,
            this, &MediaStorageDlg::slotSetChanLogFolder);
    connect(ui.usertextToolButton, &QAbstractButton::clicked,
            this, &MediaStorageDlg::slotSetUserLogFolder);

    ui.affComboBox->addItem("Wave-format", AFF_WAVE_FORMAT);
    ui.affComboBox->addItem("Ogg-format", AFF_CHANNELCODEC_FORMAT);
#if defined(Q_OS_WIN32)
    ui.affComboBox->addItem("MP3-format (16 kbit)", AFF_MP3_16KBIT_FORMAT);
    ui.affComboBox->addItem("MP3-format (32 kbit)", AFF_MP3_32KBIT_FORMAT);
    ui.affComboBox->addItem("MP3-format (64 kbit)", AFF_MP3_64KBIT_FORMAT);
    ui.affComboBox->addItem("MP3-format (128 kbit)", AFF_MP3_128KBIT_FORMAT);
    ui.affComboBox->addItem("MP3-format (256 kbit)", AFF_MP3_256KBIT_FORMAT);
    ui.affComboBox->addItem("MP3-format (320 kbit)", AFF_MP3_320KBIT_FORMAT);
#endif

    quint32 audiostorage_mode = ttSettings->value(SETTINGS_MEDIASTORAGE_MODE, 
                                                  AUDIOSTORAGE_SINGLEFILE).toUInt();
    ui.stopButton->setEnabled(audiostorage_mode != AUDIOSTORAGE_NONE);
    ui.singleCheckBox->setChecked(audiostorage_mode & AUDIOSTORAGE_SINGLEFILE);
    ui.multipleCheckBox->setChecked(audiostorage_mode & AUDIOSTORAGE_SEPARATEFILES);

    quint32 sts = ttSettings->value(SETTINGS_MEDIASTORAGE_STREAMTYPES,
                                    SETTINGS_MEDIASTORAGE_STREAMTYPES_DEFAULT).toUInt();
    ui.voicestreamCheckBox->setChecked(sts & STREAMTYPE_VOICE);
    ui.mediafileCheckBox->setChecked(sts & STREAMTYPE_MEDIAFILE_AUDIO);

    AudioFileFormat aff = (AudioFileFormat)ttSettings->value(SETTINGS_MEDIASTORAGE_FILEFORMAT, 
                                                             AFF_WAVE_FORMAT).toInt();
    
    int index = ui.affComboBox->findData(aff);
    if(index>=0)
        ui.affComboBox->setCurrentIndex(index);

    ui.audiopathEdit->setText(ttSettings->value(SETTINGS_MEDIASTORAGE_AUDIOFOLDER).toString());
    ui.chanlogEdit->setText(ttSettings->value(SETTINGS_MEDIASTORAGE_CHANLOGFOLDER).toString());
    ui.usertextEdit->setText(ttSettings->value(SETTINGS_MEDIASTORAGE_USERLOGFOLDER).toString());

    slotUpdateUI();
}

MediaStorageDlg::~MediaStorageDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_MEDIASTORAGEWINDOWPOS, saveGeometry());
}

void MediaStorageDlg::accept()
{
    int audiostorage_mode = AUDIOSTORAGE_NONE;
    if(ui.multipleCheckBox->isChecked())
        audiostorage_mode |= AUDIOSTORAGE_SEPARATEFILES;
    if(ui.singleCheckBox->isChecked())
        audiostorage_mode |= AUDIOSTORAGE_SINGLEFILE;

    QString folder = ui.audiopathEdit->text();
    if((audiostorage_mode & (AUDIOSTORAGE_SEPARATEFILES | AUDIOSTORAGE_SINGLEFILE)) &&
       (folder.isEmpty() || !QDir(folder).exists()))
    {
        QMessageBox answer;
        answer.setText(tr("Folder for storing audio files does not exist. Do you want %1 to create it for you?").arg(APPNAME_SHORT));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        Q_UNUSED(NoButton);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle(tr("Folder for audio files"));
        answer.exec();
        if(answer.clickedButton() == YesButton)
            QDir(folder).mkpath(folder);
        else
            return;
    }

    StreamTypes sts = STREAMTYPE_NONE;
    if (ui.voicestreamCheckBox->isChecked())
        sts |= STREAMTYPE_VOICE;
    if (ui.mediafileCheckBox->isChecked())
        sts |= STREAMTYPE_MEDIAFILE_AUDIO;

    if (sts == STREAMTYPE_NONE)
    {
        QMessageBox::information(this, tr("Stream type to store"),
                                 tr("No stream type has been selected as audio input for recording"));
        return;
    }

    AudioFileFormat aff = (AudioFileFormat)ui.affComboBox->itemData(ui.affComboBox->currentIndex()).toInt();

    ttSettings->setValue(SETTINGS_MEDIASTORAGE_MODE, audiostorage_mode);
    ttSettings->setValue(SETTINGS_MEDIASTORAGE_AUDIOFOLDER, folder);
    ttSettings->setValue(SETTINGS_MEDIASTORAGE_FILEFORMAT, aff);

    ttSettings->setValueOrClear(SETTINGS_MEDIASTORAGE_STREAMTYPES, sts, SETTINGS_MEDIASTORAGE_STREAMTYPES_DEFAULT);

    folder = ui.chanlogEdit->text();
    if ((folder.size() > 0 && folder.isEmpty()) || !QDir(folder).exists())
    {
        QMessageBox answer;
        answer.setText(tr("Folder for storing channel messages does not exist. Do you want %1 to create it for you?").arg(APPNAME_SHORT));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        Q_UNUSED(NoButton);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle(tr("Folder for channel messages"));
        answer.exec();
        if(answer.clickedButton() == YesButton)
            QDir(folder).mkpath(folder);
        else
            return;
    }
    ttSettings->setValue(SETTINGS_MEDIASTORAGE_CHANLOGFOLDER, folder);
    folder = ui.usertextEdit->text();
    if ((folder.size() > 0 && folder.isEmpty()) || !QDir(folder).exists())
    {
        QMessageBox answer;
        answer.setText(tr("Folder for storing private text messages does not exist. Do you want %1 to create it for you?").arg(APPNAME_SHORT));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        Q_UNUSED(NoButton);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle(tr("Folder for private text messages"));
        answer.exec();
        if(answer.clickedButton() == YesButton)
            QDir(folder).mkpath(folder);
        else
            return;
    }
    ttSettings->setValue(SETTINGS_MEDIASTORAGE_USERLOGFOLDER, folder);

    QDialog::accept();
}

void MediaStorageDlg::slotSetMediaFolder()
{
    ui.audiopathEdit->setText(getFolder());
}

void MediaStorageDlg::slotSetChanLogFolder()
{
    ui.chanlogEdit->setText(getFolder());
}

void MediaStorageDlg::slotSetUserLogFolder()
{
    ui.usertextEdit->setText(getFolder());
}

QString MediaStorageDlg::getFolder()
{
    QFileDialog dlg(this);
    dlg.setFileMode(QFileDialog::Directory);

    if(!dlg.exec())
        return QString();

    QDir dir = dlg.directory();
    return dir.toNativeSeparators(dir.absolutePath());
}

void MediaStorageDlg::slotStop()
{
    ui.multipleCheckBox->setChecked(false);
    ui.singleCheckBox->setChecked(false);
    accept();
}

void MediaStorageDlg::slotUpdateUI()
{
    ui.voicestreamCheckBox->setEnabled(ui.singleCheckBox->isChecked() || ui.multipleCheckBox->isChecked());
    ui.mediafileCheckBox->setEnabled(ui.singleCheckBox->isChecked() || ui.multipleCheckBox->isChecked());
}
