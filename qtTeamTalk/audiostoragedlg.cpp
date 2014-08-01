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


#include "audiostoragedlg.h"
#include "appinfo.h"
#include "settings.h"
#include "common.h"

#include <QMessageBox>
#include <QFile>
#include <QFileDialog>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

AudioStorageDlg::AudioStorageDlg(QWidget * parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(ui.folderToolButton, SIGNAL(clicked()),
            SLOT(slotShowDirectoryTreeDlg()));

#ifdef MP3ENCDLL_FILENAME
    if(QFile::exists(QString(MP3ENCDLL_FILENAME)))
    {
        ui.affComboBox->addItem("MP3-format (16 kbit)", AFF_MP3_16KBIT_FORMAT);
        ui.affComboBox->addItem("MP3-format (32 kbit)", AFF_MP3_32KBIT_FORMAT);
        ui.affComboBox->addItem("MP3-format (64 kbit)", AFF_MP3_64KBIT_FORMAT);
        ui.affComboBox->addItem("MP3-format (128 kbit)", AFF_MP3_128KBIT_FORMAT);
        ui.affComboBox->addItem("MP3-format (256 kbit)", AFF_MP3_256KBIT_FORMAT);
    }
#endif
    ui.affComboBox->addItem("Wave-format", AFF_WAVE_FORMAT);

    quint32 audiostorage_mode = ttSettings->value(SETTINGS_AUDIOSTORAGE_MODE, 
                                              AUDIOSTORAGE_SINGLEFILE).toUInt();
    ui.singleCheckBox->setChecked(audiostorage_mode & AUDIOSTORAGE_SINGLEFILE);
    ui.multipleCheckBox->setChecked(audiostorage_mode & AUDIOSTORAGE_SEPARATEFILES);

    AudioFileFormat aff = (AudioFileFormat)ttSettings->value(SETTINGS_AUDIOSTORAGE_FILEFORMAT, 
                                                             AFF_WAVE_FORMAT).toInt();
    
    int index = ui.affComboBox->findData(aff);
    if(index>=0)
        ui.affComboBox->setCurrentIndex(index);

    ui.filespathEdit->setText(ttSettings->value(SETTINGS_AUDIOSTORAGE_FOLDER).toString());
}

void AudioStorageDlg::accept()
{
    int audiostorage_mode = AUDIOSTORAGE_NONE;
    if(ui.multipleCheckBox->isChecked())
        audiostorage_mode |= AUDIOSTORAGE_SEPARATEFILES;
    if(ui.singleCheckBox->isChecked())
        audiostorage_mode |= AUDIOSTORAGE_SINGLEFILE;
    if(audiostorage_mode == AUDIOSTORAGE_NONE)
    {
        QMessageBox::information(this, tr("Storage mode"),
                                 tr("Storage mode not selected"));
        return;
    }

    QString audiofolder = ui.filespathEdit->text();
    if(!QDir(audiofolder).exists() || audiofolder.isEmpty())
    {
        QMessageBox::information(this, tr("Folder for audio files"),
                                 tr("Folder for storing audio files does not exist"));
        return;
    }

    AudioFileFormat aff = (AudioFileFormat)ui.affComboBox->itemData(ui.affComboBox->currentIndex()).toInt();

    ttSettings->setValue(SETTINGS_AUDIOSTORAGE_MODE, audiostorage_mode);
    ttSettings->setValue(SETTINGS_AUDIOSTORAGE_FOLDER, audiofolder);
    ttSettings->setValue(SETTINGS_AUDIOSTORAGE_FILEFORMAT, aff);

    QDialog::accept();
}

void AudioStorageDlg::slotShowDirectoryTreeDlg()
{
    QFileDialog dlg(this);
    dlg.setFileMode(QFileDialog::DirectoryOnly);

    if(!dlg.exec())
        return;

    QDir dir = dlg.directory();
    ui.filespathEdit->setText(dir.toNativeSeparators(dir.absolutePath()));
}
