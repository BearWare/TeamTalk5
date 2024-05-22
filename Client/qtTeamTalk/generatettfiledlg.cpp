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


#include "generatettfiledlg.h"
#include "keycompdlg.h"
#include "appinfo.h"
#include "settings.h"
#include "utilhotkey.h"
#include "utilxml.h"

#include <QFileDialog>
#include <QMessageBox>

extern QSettings* ttSettings;

GenerateTTFileDlg::GenerateTTFileDlg(const HostEntry& entry, QWidget * parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
, m_hostentry(entry)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    setWindowTitle(windowTitle() + " - " + entry.ipaddr + ":" +
                   QString::number(entry.tcpport));

    ui.usernameEdit->setText(m_hostentry.username);
    ui.passwordEdit->setText(m_hostentry.password);
    ui.nicknameEdit->setText(m_hostentry.nickname);

    connect(ui.saveBtn, &QAbstractButton::clicked, this, &GenerateTTFileDlg::slotSaveTTFile);
    connect(ui.closeBtn, &QAbstractButton::clicked, this, &QDialog::accept);
    connect(ui.overrideChkBox, &QAbstractButton::clicked, ui.nicknameEdit, &QWidget::setEnabled);
    connect(ui.overrideChkBox, &QAbstractButton::clicked, ui.maleRadioButton, &QWidget::setEnabled);
    connect(ui.overrideChkBox, &QAbstractButton::clicked, ui.femaleRadioButton, &QWidget::setEnabled);
    connect(ui.overrideChkBox, &QAbstractButton::clicked, ui.neutralRadioButton, &QWidget::setEnabled);
    connect(ui.overrideChkBox, &QAbstractButton::clicked, ui.pttChkBox, &QWidget::setEnabled);
    connect(ui.overrideChkBox, &QAbstractButton::clicked, ui.voiceactChkBox, &QWidget::setEnabled);
    connect(ui.overrideChkBox, &QAbstractButton::clicked, ui.captureformatsBox, &QWidget::setEnabled);
    connect(ui.overrideChkBox, &QAbstractButton::clicked, ui.vidcodecBox, &QWidget::setEnabled);

    connect(ui.pttChkBox, &QAbstractButton::clicked, ui.setupkeysButton, &QWidget::setEnabled);
    connect(ui.pttChkBox, &QAbstractButton::clicked, ui.keycompEdit, &QWidget::setEnabled);
    connect(ui.setupkeysButton, &QAbstractButton::clicked, this, &GenerateTTFileDlg::slotSetupHotkey);
    connect(ui.vidcodecBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GenerateTTFileDlg::slotVideoCodecChange);

    loadVideoFormats();

    ui.captureformatsBox->addItem(tr("Any"), 0);
    for(int i=1;i<m_vidcap_fmts.size();i++)
    {
        int fps = m_vidcap_fmts[i].nFPS_Denominator == 0? 0 : 
            m_vidcap_fmts[i].nFPS_Numerator / m_vidcap_fmts[i].nFPS_Denominator;

        QString res = QString("%1x%2, FPS %3").arg(m_vidcap_fmts[i].nWidth)
                                              .arg(m_vidcap_fmts[i].nHeight).arg(fps);
        ui.captureformatsBox->addItem(res, i);
    }

    ui.vidcodecBox->addItem(tr("Any"), NO_CODEC);
    ui.vidcodecBox->addItem("WebM VP8", WEBM_VP8_CODEC);
    ui.vidbitrateSpinBox->setValue(DEFAULT_WEBMVP8_BITRATE);
}

void GenerateTTFileDlg::loadVideoFormats()
{
    VideoFormat fmt = {};
    m_vidcap_fmts.push_back(fmt);

    fmt.picFourCC = FOURCC_RGB32;
    fmt.nFPS_Denominator = 1;

    fmt.nWidth = 160;
    fmt.nHeight = 120;

    fmt.nFPS_Numerator = 1;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 10;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 15;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 30;
    m_vidcap_fmts.push_back(fmt);
    
    fmt.nWidth = 320;
    fmt.nHeight = 240;

    fmt.nFPS_Numerator = 1;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 10;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 15;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 30;
    m_vidcap_fmts.push_back(fmt);

    fmt.nWidth = 640;
    fmt.nHeight = 480;

    fmt.nFPS_Numerator = 1;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 10;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 15;
    m_vidcap_fmts.push_back(fmt);
    fmt.nFPS_Numerator = 30;
    m_vidcap_fmts.push_back(fmt);

}

void GenerateTTFileDlg::slotSetupHotkey()
{
    KeyCompDlg dlg(this);
    if(!dlg.exec())
        return;

    m_hostentry.hotkey = dlg.m_hotkey;
    ui.keycompEdit->setText(getHotKeyText(dlg.m_hotkey));
}

void GenerateTTFileDlg::slotVideoCodecChange(int)
{
    int c = ui.vidcodecBox->itemData(ui.vidcodecBox->currentIndex()).toInt();
    ui.vidbitrateSpinBox->setEnabled(c == WEBM_VP8_CODEC);
}

void GenerateTTFileDlg::exportTTFileToDirectory(const QString& dir)
{
    QByteArray xml = generateTTFile(m_hostentry);

    QString genname = !m_hostentry.name.isEmpty() ? m_hostentry.name : QString("%1@%2_%3").arg(m_hostentry.username).arg(m_hostentry.ipaddr).arg(m_hostentry.tcpport);
    QString filename = QString("%1%2%3.tt").arg(dir).arg(QDir::separator()).arg(genname);

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, tr("Save File"), tr("Unable to save file %1").arg(filename));
        return;
    }
    file.write(xml);
}

void GenerateTTFileDlg::slotSaveTTFile()
{
    m_hostentry.username = ui.usernameEdit->text();
    m_hostentry.password = ui.passwordEdit->text();

    if(ui.overrideChkBox->isChecked())
    {
        m_hostentry.nickname = ui.nicknameEdit->text();

        if (ui.maleRadioButton->isChecked())
            m_hostentry.gender = GENDER_MALE;
        else if (ui.femaleRadioButton->isChecked())
            m_hostentry.gender = GENDER_FEMALE;
        else if (ui.neutralRadioButton->isChecked())
            m_hostentry.gender = GENDER_NEUTRAL;

        if(!ui.pttChkBox->isChecked())
            m_hostentry.hotkey.clear();
        m_hostentry.voiceact = ui.voiceactChkBox->isChecked();
        int c = ui.vidcodecBox->itemData(ui.vidcodecBox->currentIndex()).toInt();
        m_hostentry.vidcodec.nCodec = (Codec)c;
        switch(m_hostentry.vidcodec.nCodec)
        {
        case WEBM_VP8_CODEC :
            m_hostentry.vidcodec.webm_vp8.nRcTargetBitrate = ui.vidbitrateSpinBox->value();
            m_hostentry.vidcodec.webm_vp8.nEncodeDeadline = DEFAULT_WEBMVP8_DEADLINE;
            break;
        case SPEEX_CODEC :
        case SPEEX_VBR_CODEC :
        case OPUS_CODEC :
        case NO_CODEC :
            break;
        }
        m_hostentry.capformat = m_vidcap_fmts[ui.captureformatsBox->currentIndex()];
    }
    QByteArray xml = generateTTFile(m_hostentry);

    QString start_dir = ttSettings->value(SETTINGS_LAST_DIRECTORY, QDir::homePath()).toString();
    QString genname = !m_hostentry.name.isEmpty() ? m_hostentry.name : QString("%1@%2_%3").arg(m_hostentry.username).arg(m_hostentry.ipaddr).arg(m_hostentry.tcpport);
    QString defaultfilename = QString("%1%2%3.tt").arg(start_dir).arg(QDir::separator()).arg(genname);
    QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
        defaultfilename, tr("%1 File (*%1)").arg(TTFILE_EXT));
    if(filename.size())
    {
        ttSettings->setValue(SETTINGS_LAST_DIRECTORY, QFileInfo(filename).absolutePath());

        QFile file(filename);
        if(!file.open(QIODevice::WriteOnly))
            QMessageBox::critical(this, tr("Save File"), tr("Unable to save file"));
        else
            file.write(xml);
    }
}
