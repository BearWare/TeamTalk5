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


#include "generatettfiledlg.h"
#include "keycompdlg.h"
#include "appinfo.h"
#include "settings.h"

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

    connect(ui.saveBtn, SIGNAL(clicked()), SLOT(slotSaveTTFile()));
    connect(ui.closeBtn, SIGNAL(clicked()), SLOT(accept()));
    connect(ui.overrideChkBox, SIGNAL(clicked(bool)), ui.nicknameEdit, SLOT(setEnabled(bool)));
    connect(ui.overrideChkBox, SIGNAL(clicked(bool)), ui.maleRadioButton, SLOT(setEnabled(bool)));
    connect(ui.overrideChkBox, SIGNAL(clicked(bool)), ui.femaleRadioButton, SLOT(setEnabled(bool)));
    connect(ui.overrideChkBox, SIGNAL(clicked(bool)), ui.pttChkBox, SLOT(setEnabled(bool)));
    connect(ui.overrideChkBox, SIGNAL(clicked(bool)), ui.voiceactChkBox, SLOT(setEnabled(bool)));
    connect(ui.overrideChkBox, SIGNAL(clicked(bool)), ui.captureformatsBox, SLOT(setEnabled(bool)));
    connect(ui.overrideChkBox, SIGNAL(clicked(bool)), ui.vidcodecBox, SLOT(setEnabled(bool)));

    connect(ui.pttChkBox, SIGNAL(clicked(bool)), ui.setupkeysButton, SLOT(setEnabled(bool)));
    connect(ui.pttChkBox, SIGNAL(clicked(bool)), ui.keycompEdit, SLOT(setEnabled(bool)));
    connect(ui.setupkeysButton, SIGNAL(clicked()), SLOT(slotSetupHotkey()));
    connect(ui.vidcodecBox, SIGNAL(currentIndexChanged(int)),
            SLOT(slotVideoCodecChange(int)));

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
    VideoFormat fmt;
    ZERO_STRUCT(fmt);
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

void GenerateTTFileDlg::slotSaveTTFile()
{
    if(ui.overrideChkBox->isChecked())
    {
        m_hostentry.username = ui.usernameEdit->text();
        m_hostentry.password = ui.passwordEdit->text();
        m_hostentry.nickname = ui.nicknameEdit->text();
        m_hostentry.gender = ui.femaleRadioButton->isChecked()?GENDER_FEMALE:GENDER_MALE;
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

    QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
        start_dir, tr("%1 File (*%1)").arg(TTFILE_EXT));
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
