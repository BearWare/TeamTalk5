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


#include "mediastoragedlg.h"
#include "appinfo.h"
#include "settings.h"
#include "common.h"

#include <QMessageBox>
#include <QFile>
#include <QFileDialog>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

MediaStorageDlg::MediaStorageDlg(QWidget * parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(ui.audioToolButton, SIGNAL(clicked()),
            SLOT(slotSetMediaFolder()));
    connect(ui.chanlogToolButton, SIGNAL(clicked()),
            SLOT(slotSetChanLogFolder()));
    connect(ui.usertextToolButton, SIGNAL(clicked()),
            SLOT(slotSetUserLogFolder()));

    ui.affComboBox->addItem("Wave-format", AFF_WAVE_FORMAT);
    ui.affComboBox->addItem("Ogg-format", AFF_CHANNELCODEC_FORMAT);
#if defined(Q_OS_WIN32)
    ui.affComboBox->addItem("MP3-format (16 kbit)", AFF_MP3_16KBIT_FORMAT);
    ui.affComboBox->addItem("MP3-format (32 kbit)", AFF_MP3_32KBIT_FORMAT);
    ui.affComboBox->addItem("MP3-format (64 kbit)", AFF_MP3_64KBIT_FORMAT);
    ui.affComboBox->addItem("MP3-format (128 kbit)", AFF_MP3_128KBIT_FORMAT);
    ui.affComboBox->addItem("MP3-format (256 kbit)", AFF_MP3_256KBIT_FORMAT);
#endif

    quint32 audiostorage_mode = ttSettings->value(SETTINGS_MEDIASTORAGE_MODE, 
                                              AUDIOSTORAGE_SINGLEFILE).toUInt();
    ui.singleCheckBox->setChecked(audiostorage_mode & AUDIOSTORAGE_SINGLEFILE);
    ui.multipleCheckBox->setChecked(audiostorage_mode & AUDIOSTORAGE_SEPARATEFILES);

    AudioFileFormat aff = (AudioFileFormat)ttSettings->value(SETTINGS_MEDIASTORAGE_FILEFORMAT, 
                                                             AFF_WAVE_FORMAT).toInt();
    
    int index = ui.affComboBox->findData(aff);
    if(index>=0)
        ui.affComboBox->setCurrentIndex(index);

    ui.audiopathEdit->setText(ttSettings->value(SETTINGS_MEDIASTORAGE_AUDIOFOLDER).toString());
    ui.chanlogEdit->setText(ttSettings->value(SETTINGS_MEDIASTORAGE_CHANLOGFOLDER).toString());
    ui.usertextEdit->setText(ttSettings->value(SETTINGS_MEDIASTORAGE_USERLOGFOLDER).toString());
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
        QMessageBox::information(this, tr("Folder for audio files"),
                                 tr("Folder for storing audio files does not exist"));
        return;
    }

    AudioFileFormat aff = (AudioFileFormat)ui.affComboBox->itemData(ui.affComboBox->currentIndex()).toInt();

    ttSettings->setValue(SETTINGS_MEDIASTORAGE_MODE, audiostorage_mode);
    ttSettings->setValue(SETTINGS_MEDIASTORAGE_AUDIOFOLDER, folder);
    ttSettings->setValue(SETTINGS_MEDIASTORAGE_FILEFORMAT, aff);

    folder = ui.chanlogEdit->text();
    ttSettings->setValue(SETTINGS_MEDIASTORAGE_CHANLOGFOLDER, folder);
    folder = ui.usertextEdit->text();
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
    dlg.setFileMode(QFileDialog::DirectoryOnly);

    if(!dlg.exec())
        return QString();

    QDir dir = dlg.directory();
    return dir.toNativeSeparators(dir.absolutePath());
}
