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

#include "channeldlg.h"
#include "appinfo.h"
#include "utilui.h"
#include "settings.h"

#include <QPushButton>
#include <QInputDialog>

extern TTInstance* ttInst;
extern NonDefaultSettings* ttSettings;

ChannelDlg::ChannelDlg(ChannelDlgType type, const Channel& chan, QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
    , m_channel(chan)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_CHANNELWINDOWPOS).toByteArray());

    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    connect(ui.singletxchanBox, &QAbstractButton::clicked,
            ui.singletxButton, &QAbstractButton::setEnabled);
    connect(ui.singletxButton, &QAbstractButton::clicked,
            this, &ChannelDlg::slotSoloTransmitDelay);

    connect(ui.audiocodecBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ChannelDlg::slotAudioCodecChanged);

    connect(ui.spx_qualitySlider, &QAbstractSlider::valueChanged,
            this, &ChannelDlg::slotUpdateSliderLabels);
    connect(ui.spxvbr_qualitySlider, &QAbstractSlider::valueChanged,
            this, &ChannelDlg::slotUpdateSliderLabels);
    connect(ui.nameEdit, &QLineEdit::textChanged,
            this, &ChannelDlg::slotUpdateChannelPath);
    
    connect(ui.agcBox, &QAbstractButton::toggled,
            ui.gainlevelSlider, &QWidget::setEnabled);
    connect(ui.gainlevelSlider, &QAbstractSlider::valueChanged,
            this, &ChannelDlg::slotUpdateSliderLabels);
    connect(ui.spx_srateBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&]()
    {
        ui.spx_srateBox->setAccessibleName(QString("%1 %2 %3").arg(ui.label_36->text()).arg(ui.spx_srateBox->currentText()).arg(ui.label_37->text()));
    });
    connect(ui.spxvbr_srateBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&]()
    {
        ui.spxvbr_srateBox->setAccessibleName(QString("%1 %2 %3").arg(ui.label_38->text()).arg(ui.spxvbr_srateBox->currentText()).arg(ui.label_39->text()));
    });
    connect(ui.opus_srateBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&]()
    {
        ui.opus_srateBox->setAccessibleName(QString("%1 %2 %3").arg(ui.label_43->text()).arg(ui.opus_srateBox->currentText()).arg(ui.label_40->text()));
    });
    connect(ui.opus_framesizeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&]()
    {
        ui.opus_framesizeComboBox->setAccessibleName(QString("%1 %2 %3").arg(ui.label_6->text()).arg(ui.opus_framesizeComboBox->currentText()).arg(ui.label_10->text()));
    });
    connect(ui.voiceTotDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&]()
    {
        ui.voiceTotDoubleSpinBox->setAccessibleName(QString("%1 %2 %3").arg(ui.voiceTotLabel->text()).arg(ui.voiceTotDoubleSpinBox->value()).arg(ui.voiceTotDoubleSpinBox->suffix()));
    });
    connect(ui.mfTotDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&]()
    {
        ui.mfTotDoubleSpinBox->setAccessibleName(QString("%1 %2 %3").arg(ui.mfTotLabel->text()).arg(ui.mfTotDoubleSpinBox->value()).arg(ui.mfTotDoubleSpinBox->suffix()));
    });

    ServerProperties prop = {};
    TT_GetServerProperties(ttInst, &prop);
    if (!versionSameOrLater(_Q(prop.szServerProtocolVersion), "5.8"))
        ui.hiddenchannelBox->hide();
    if (!versionSameOrLater(_Q(prop.szServerProtocolVersion), "5.10"))
        ui.singletxButton->hide();
    if (!versionSameOrLater(_Q(prop.szServerProtocolVersion), "5.12"))
        ui.streamGroupBox->hide();

    ui.audiocodecBox->addItem(tr("No Audio"), NO_CODEC);

    //Speex page
    ui.audiocodecBox->addItem("Speex", SPEEX_CODEC);
    ui.spx_srateBox->addItem("8000", 0); //narrow band
    ui.spx_srateBox->addItem("16000", 1); //wide band
    ui.spx_srateBox->addItem("32000", 2); //narrow band
    ui.spx_txdelaySpinBox->setSingleStep(20);
    ui.spx_txdelaySpinBox->setRange(20, 500);

    //Speex VBR page
    ui.audiocodecBox->addItem("Speex Variable Bit Rate", SPEEX_VBR_CODEC);
    ui.spxvbr_srateBox->addItem("8000", 0); //narrow band
    ui.spxvbr_srateBox->addItem("16000", 1); //wide band
    ui.spxvbr_srateBox->addItem("32000", 2); //narrow band
    ui.spxvbr_txdelaySpinBox->setSingleStep(20);
    ui.spxvbr_txdelaySpinBox->setRange(20, 500);

    //OPUS page
    ui.audiocodecBox->addItem("OPUS", OPUS_CODEC);
    ui.opus_channelsBox->addItem(tr("Mono"), 1);
    ui.opus_channelsBox->addItem(tr("Stereo"), 2);
    ui.opus_appBox->addItem(tr("VoIP"), OPUS_APPLICATION_VOIP);
    ui.opus_appBox->addItem(tr("Music"), OPUS_APPLICATION_AUDIO);
    ui.opus_bpsSpinBox->setRange(OPUS_MIN_BITRATE / 1000, OPUS_MAX_BITRATE / 1000);
    ui.opus_srateBox->addItem("8000", 8000);
    ui.opus_srateBox->addItem("12000", 12000);
    ui.opus_srateBox->addItem("16000", 16000);
    ui.opus_srateBox->addItem("24000", 24000);
    ui.opus_srateBox->addItem("48000", 48000);
    ui.opus_txdelaySpinBox->setSingleStep(20);
    ui.opus_txdelaySpinBox->setRange(20, 500);
    ui.opus_framesizeComboBox->addItem("0", 0);
    ui.opus_framesizeComboBox->addItem("2.5", OPUS_MIN_FRAMESIZE);
    ui.opus_framesizeComboBox->addItem("5", 5);
    ui.opus_framesizeComboBox->addItem("10", 10);
    ui.opus_framesizeComboBox->addItem("20", 20);
    ui.opus_framesizeComboBox->addItem("40", 40);
    ui.opus_framesizeComboBox->addItem("60", OPUS_MAX_FRAMESIZE);
    ui.opus_framesizeComboBox->addItem("80", 80);
    ui.opus_framesizeComboBox->addItem("100", 100);
    ui.opus_framesizeComboBox->addItem("120", OPUS_REALMAX_FRAMESIZE);

    bool modchannels = (TT_GetMyUserRights(ttInst) & USERRIGHT_MODIFY_CHANNELS) == USERRIGHT_MODIFY_CHANNELS;
    ui.staticchanBox->setEnabled(modchannels);
    ui.diskquotaSpinBox->setEnabled(modchannels);
    ui.maxusersSpinBox->setEnabled(modchannels);
    ui.joinchanBox->setEnabled(modchannels);
    ui.joinchanBox->setChecked(true);

    //default settings for Speex
    setCurrentItemData(ui.spx_srateBox, DEFAULT_SPEEX_BANDMODE);
    ui.spx_srateBox->setAccessibleName(QString("%1 %2 %3").arg(ui.label_36->text()).arg(ui.spx_srateBox->currentText()).arg(ui.label_37->text()));
    ui.spx_qualitySlider->setValue(DEFAULT_SPEEX_QUALITY);
    ui.spx_txdelaySpinBox->setValue(DEFAULT_SPEEX_DELAY);
    //default settings for Speex VBR
    setCurrentItemData(ui.spxvbr_srateBox, DEFAULT_SPEEX_VBR_BANDMODE);
    ui.spxvbr_srateBox->setAccessibleName(QString("%1 %2 %3").arg(ui.label_38->text()).arg(ui.spxvbr_srateBox->currentText()).arg(ui.label_39->text()));
    ui.spxvbr_qualitySlider->setValue(DEFAULT_SPEEX_VBR_QUALITY);
    ui.spxvbr_maxbpsSpinBox->setValue(DEFAULT_SPEEX_VBR_MAXBITRATE);
    ui.spxvbr_dtxBox->setChecked(DEFAULT_SPEEX_VBR_DTX);
    ui.spxvbr_txdelaySpinBox->setValue(DEFAULT_SPEEX_VBR_DELAY);
    //default settings for OPUS
    setCurrentItemData(ui.opus_srateBox, DEFAULT_OPUS_SAMPLERATE);
    ui.opus_srateBox->setAccessibleName(QString("%1 %2 %3").arg(ui.label_43->text()).arg(ui.opus_srateBox->currentText()).arg(ui.label_40->text()));
    setCurrentItemData(ui.opus_channelsBox, DEFAULT_OPUS_CHANNELS);
    setCurrentItemData(ui.opus_appBox, DEFAULT_OPUS_APPLICATION);
    ui.opus_bpsSpinBox->setValue(DEFAULT_OPUS_BITRATE / 1000);
    ui.opus_dtxBox->setChecked(DEFAULT_OPUS_DTX);
    ui.opus_txdelaySpinBox->setValue(DEFAULT_OPUS_DELAY);
    ui.opus_vbrCheckBox->setChecked(DEFAULT_OPUS_VBR);
    setCurrentItemData(ui.opus_framesizeComboBox, DEFAULT_OPUS_FRAMESIZE);
    ui.opus_framesizeComboBox->setAccessibleName(QString("%1 %2 %3").arg(ui.label_6->text()).arg(ui.opus_framesizeComboBox->currentText()).arg(ui.label_10->text()));

    switch(type)
    {
    case CHANNEL_CREATE :
        setWindowTitle(tr("Create Channel"));
        this->setAccessibleDescription(tr("Add channel on %1").arg(_Q(prop.szServerName)));
        initDefaultAudioCodec(m_channel.audiocodec);

        //set default channel options
        m_channel.nMaxUsers = prop.nMaxUsers;
        m_channel.uChannelType = CHANNEL_DEFAULT;
        //set AGC
        m_channel.audiocfg.bEnableAGC = DEFAULT_CHANNEL_AUDIOCONFIG_ENABLE;
        m_channel.audiocfg.nGainLevel = DEFAULT_CHANNEL_AUDIOCONFIG_LEVEL;

        m_channel.transmitUsers[0][TT_CLASSROOM_USERID_INDEX] = TT_CLASSROOM_FREEFORALL;
        m_channel.transmitUsers[0][TT_CLASSROOM_STREAMTYPE_INDEX] = STREAMTYPE_CLASSROOM_ALL;
        break;
    case CHANNEL_UPDATE :
    {
        setWindowTitle(tr("Update Channel"));
        if (m_channel.nChannelID == TT_GetRootChannelID(ttInst))
            this->setAccessibleDescription(tr("Modify root channel"));
        else
            this->setAccessibleDescription(tr("Modify channel %1").arg(_Q(m_channel.szName)));
        int count = 0;
        TT_GetChannelUsers(ttInst, chan.nChannelID, nullptr, &count);
        if (count > 0)
            setAudioCodecReadonly();

        // cannot modify hidden property
        ui.hiddenchannelBox->setEnabled(false);
    }
    break;
    case CHANNEL_READONLY :
        setWindowTitle(tr("View Channel Information"));
        if (m_channel.nChannelID == TT_GetRootChannelID(ttInst))
            this->setAccessibleDescription(tr("View root channel information"));
        else
            this->setAccessibleDescription(tr("View %1 information").arg(_Q(m_channel.szName)));
        ui.nameEdit->setReadOnly(true);
        ui.topicTextEdit->setReadOnly(true);
        ui.chanpasswdEdit->setReadOnly(true);
        ui.oppasswdEdit->setReadOnly(true);
        ui.maxusersSpinBox->setReadOnly(true);
        ui.diskquotaSpinBox->setReadOnly(true);
        ui.staticchanBox->setEnabled(false);
        ui.singletxchanBox->setEnabled(false);
        ui.singletxButton->setEnabled(false);
        ui.classroomchanBox->setEnabled(false);
        ui.oprecvonlychanBox->setEnabled(false);
        ui.novoiceactBox->setEnabled(false);
        ui.norecordBox->setEnabled(false);
        ui.hiddenchannelBox->setEnabled(false);

        setAudioCodecReadonly();

        ui.agcBox->setEnabled(false);
        ui.gainlevelSlider->setEnabled(false);

        ui.voiceTotDoubleSpinBox->setEnabled(false);
        ui.mfTotDoubleSpinBox->setEnabled(false);

        ui.buttonBox->setStandardButtons(QDialogButtonBox::Close);
        ui.buttonBox->button(QDialogButtonBox::Close)->setText(tr("&Close"));
        ui.joinchanBox->setEnabled(false);
        break;
    }

    ui.nameEdit->setText(_Q(m_channel.szName));
    ui.topicTextEdit->setPlainText(_Q(m_channel.szTopic));
    ui.chanpasswdEdit->setText(_Q(m_channel.szPassword));
    ui.oppasswdEdit->setText(_Q(m_channel.szOpPassword));
    ui.maxusersSpinBox->setValue(m_channel.nMaxUsers);
    ui.diskquotaSpinBox->setValue(m_channel.nDiskQuota/1024);
    ui.staticchanBox->setChecked(m_channel.uChannelType & CHANNEL_PERMANENT);
    ui.singletxchanBox->setChecked(m_channel.uChannelType & CHANNEL_SOLO_TRANSMIT);
    ui.singletxButton->setEnabled(ui.singletxchanBox->isEnabled() && ui.singletxchanBox->isChecked());
    ui.classroomchanBox->setChecked(m_channel.uChannelType & CHANNEL_CLASSROOM);
    ui.oprecvonlychanBox->setChecked(m_channel.uChannelType & CHANNEL_OPERATOR_RECVONLY);
    ui.novoiceactBox->setChecked(m_channel.uChannelType & CHANNEL_NO_VOICEACTIVATION);
    ui.norecordBox->setChecked(m_channel.uChannelType & CHANNEL_NO_RECORDING);
    ui.hiddenchannelBox->setChecked(m_channel.uChannelType & CHANNEL_HIDDEN);

    //codec
    switch(m_channel.audiocodec.nCodec)
    {
    case SPEEX_CODEC :
        setCurrentItemData(ui.spx_srateBox, m_channel.audiocodec.speex.nBandmode);
        ui.spx_qualitySlider->setValue(m_channel.audiocodec.speex.nQuality);
        ui.spx_txdelaySpinBox->setValue(m_channel.audiocodec.speex.nTxIntervalMSec);
        break;
    case SPEEX_VBR_CODEC :
        setCurrentItemData(ui.spxvbr_srateBox, m_channel.audiocodec.speex_vbr.nBandmode);
        ui.spxvbr_qualitySlider->setValue(m_channel.audiocodec.speex_vbr.nQuality);
        ui.spxvbr_maxbpsSpinBox->setValue(m_channel.audiocodec.speex_vbr.nMaxBitRate);
        ui.spxvbr_dtxBox->setChecked(m_channel.audiocodec.speex_vbr.bDTX);
        ui.spxvbr_txdelaySpinBox->setValue(m_channel.audiocodec.speex_vbr.nTxIntervalMSec);
        break;
    case OPUS_CODEC :
        setCurrentItemData(ui.opus_srateBox, m_channel.audiocodec.opus.nSampleRate);
        setCurrentItemData(ui.opus_channelsBox, m_channel.audiocodec.opus.nChannels);
        setCurrentItemData(ui.opus_appBox, m_channel.audiocodec.opus.nApplication);
        ui.opus_bpsSpinBox->setValue(m_channel.audiocodec.opus.nBitRate / 1000);
        ui.opus_vbrCheckBox->setChecked(m_channel.audiocodec.opus.bVBR);
        ui.opus_dtxBox->setChecked(m_channel.audiocodec.opus.bDTX);
        ui.opus_txdelaySpinBox->setValue(m_channel.audiocodec.opus.nTxIntervalMSec);
        setCurrentItemData(ui.opus_framesizeComboBox, m_channel.audiocodec.opus.nFrameSizeMSec);
        break;
    default :
        break;
    }
    setCurrentItemData(ui.audiocodecBox, m_channel.audiocodec.nCodec);
    //AGC
    ui.agcBox->setChecked(m_channel.audiocfg.bEnableAGC);
    ui.gainlevelSlider->setEnabled(m_channel.audiocfg.bEnableAGC);
    ui.gainlevelSlider->setValue(m_channel.audiocfg.nGainLevel / 1000);
    // Stream Timeout
    ui.voiceTotDoubleSpinBox->setValue(m_channel.nTimeOutTimerVoiceMSec / 1000.);
    ui.voiceTotDoubleSpinBox->setAccessibleName(QString("%1 %2 %3").arg(ui.voiceTotLabel->text()).arg(ui.voiceTotDoubleSpinBox->value()).arg(ui.voiceTotDoubleSpinBox->suffix()));
    ui.mfTotDoubleSpinBox->setValue(m_channel.nTimeOutTimerMediaFileMSec / 1000.);
    ui.mfTotDoubleSpinBox->setAccessibleName(QString("%1 %2 %3").arg(ui.mfTotLabel->text()).arg(ui.mfTotDoubleSpinBox->value()).arg(ui.mfTotDoubleSpinBox->suffix()));

    slotUpdateSliderLabels();
    slotUpdateChannelPath(_Q(m_channel.szName));
    ui.nameEdit->setFocus();
}

ChannelDlg::~ChannelDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_CHANNELWINDOWPOS, saveGeometry());
}

Channel ChannelDlg::GetChannel() const
{
    Channel newchannel = m_channel;
    COPY_TTSTR(newchannel.szName, ui.nameEdit->text().trimmed());
    COPY_TTSTR(newchannel.szTopic, ui.topicTextEdit->toPlainText());
    COPY_TTSTR(newchannel.szPassword, ui.chanpasswdEdit->text());
    COPY_TTSTR(newchannel.szOpPassword, ui.oppasswdEdit->text());
    newchannel.nMaxUsers = ui.maxusersSpinBox->value();
    newchannel.nDiskQuota = (quint64)ui.diskquotaSpinBox->value()*1024;

    if(ui.staticchanBox->isChecked())
        newchannel.uChannelType |= CHANNEL_PERMANENT;
    else
        newchannel.uChannelType &= ~CHANNEL_PERMANENT;

    if(ui.singletxchanBox->isChecked())
        newchannel.uChannelType |= CHANNEL_SOLO_TRANSMIT;
    else
        newchannel.uChannelType &= ~CHANNEL_SOLO_TRANSMIT;

    if(ui.classroomchanBox->isChecked())
        newchannel.uChannelType |= CHANNEL_CLASSROOM;
    else
        newchannel.uChannelType &= ~CHANNEL_CLASSROOM;

    if(ui.oprecvonlychanBox->isChecked())
        newchannel.uChannelType |= CHANNEL_OPERATOR_RECVONLY;
    else
        newchannel.uChannelType &= ~CHANNEL_OPERATOR_RECVONLY;

    if(ui.novoiceactBox->isChecked())
        newchannel.uChannelType |= CHANNEL_NO_VOICEACTIVATION;
    else
        newchannel.uChannelType &= ~CHANNEL_NO_VOICEACTIVATION;

    if(ui.norecordBox->isChecked())
        newchannel.uChannelType |= CHANNEL_NO_RECORDING;
    else
        newchannel.uChannelType &= ~CHANNEL_NO_RECORDING;

    if (ui.hiddenchannelBox->isChecked())
        newchannel.uChannelType |= CHANNEL_HIDDEN;
    else
        newchannel.uChannelType &= ~CHANNEL_HIDDEN;

    newchannel.audiocodec.nCodec = (Codec)ui.audiocodecBox->itemData(ui.audiocodecBox->currentIndex()).toInt();
    switch(newchannel.audiocodec.nCodec)
    {
    case SPEEX_CODEC :
        newchannel.audiocodec.speex.nBandmode = getCurrentItemData(ui.spx_srateBox).toInt();
        newchannel.audiocodec.speex.nQuality = ui.spx_qualitySlider->value();
        newchannel.audiocodec.speex.nTxIntervalMSec = ui.spx_txdelaySpinBox->value();
        newchannel.audiocodec.speex.bStereoPlayback = DEFAULT_SPEEX_SIMSTEREO;
        break;
    case SPEEX_VBR_CODEC :
        newchannel.audiocodec.speex_vbr.nBandmode = getCurrentItemData(ui.spxvbr_srateBox).toInt();
        newchannel.audiocodec.speex_vbr.nQuality = ui.spxvbr_qualitySlider->value();
        newchannel.audiocodec.speex_vbr.nBitRate = DEFAULT_SPEEX_VBR_BITRATE;
        newchannel.audiocodec.speex_vbr.nMaxBitRate = ui.spxvbr_maxbpsSpinBox->value();
        newchannel.audiocodec.speex_vbr.bDTX = ui.spxvbr_dtxBox->isChecked();
        newchannel.audiocodec.speex_vbr.nTxIntervalMSec = ui.spxvbr_txdelaySpinBox->value();
        newchannel.audiocodec.speex_vbr.bStereoPlayback = DEFAULT_SPEEX_VBR_SIMSTEREO;
        break;
    case OPUS_CODEC :
        newchannel.audiocodec.opus.nSampleRate = getCurrentItemData(ui.opus_srateBox).toInt();
        newchannel.audiocodec.opus.nChannels = getCurrentItemData(ui.opus_channelsBox).toInt();
        newchannel.audiocodec.opus.nApplication = getCurrentItemData(ui.opus_appBox).toInt();
        newchannel.audiocodec.opus.nComplexity = DEFAULT_OPUS_COMPLEXITY;
        newchannel.audiocodec.opus.bFEC = DEFAULT_OPUS_FEC;
        newchannel.audiocodec.opus.bDTX = ui.opus_dtxBox->isChecked();
        newchannel.audiocodec.opus.nBitRate = ui.opus_bpsSpinBox->value() * 1000;
        newchannel.audiocodec.opus.bVBR = ui.opus_vbrCheckBox->isChecked();
        newchannel.audiocodec.opus.bVBRConstraint = DEFAULT_OPUS_VBRCONSTRAINT;
        newchannel.audiocodec.opus.nTxIntervalMSec = ui.opus_txdelaySpinBox->value();
        newchannel.audiocodec.opus.nFrameSizeMSec = getCurrentItemData(ui.opus_framesizeComboBox).toInt();
        break;
    default :
        break;
    }

    newchannel.audiocfg.bEnableAGC = ui.agcBox->isChecked();
    
    //set default values since they may otherwise be 0
    if(newchannel.audiocfg.bEnableAGC)
    {
        newchannel.audiocfg.nGainLevel = ui.gainlevelSlider->value()*1000;
    }

    newchannel.nTimeOutTimerVoiceMSec = ui.voiceTotDoubleSpinBox->value() * 1000;
    newchannel.nTimeOutTimerMediaFileMSec = ui.mfTotDoubleSpinBox->value() * 1000;

    return newchannel;
}

bool ChannelDlg::joinChannel() const
{
    return ui.joinchanBox->isChecked();
}

void ChannelDlg::setAudioCodecReadonly()
{
    ui.audiocodecBox->setEnabled(false);

    ui.spx_srateBox->setEnabled(false);
    ui.spx_qualitySlider->setEnabled(false);
    ui.spx_txdelaySpinBox->setReadOnly(true);

    ui.spxvbr_srateBox->setEnabled(false);
    ui.spxvbr_qualitySlider->setEnabled(false);
    ui.spxvbr_maxbpsSpinBox->setReadOnly(true);
    ui.spxvbr_dtxBox->setEnabled(false);
    ui.spxvbr_txdelaySpinBox->setReadOnly(true);

    ui.opus_channelsBox->setEnabled(false);
    ui.opus_appBox->setEnabled(false);
    ui.opus_bpsSpinBox->setReadOnly(true);
    ui.opus_srateBox->setEnabled(false);
    ui.opus_txdelaySpinBox->setReadOnly(true);
    ui.opus_framesizeComboBox->setEnabled(false);
    ui.opus_vbrCheckBox->setEnabled(false);
    ui.opus_dtxBox->setEnabled(false);
}

void ChannelDlg::slotAudioCodecChanged(int index)
{
    switch(ui.audiocodecBox->itemData(index).toInt())
    {
    case SPEEX_CODEC :
        ui.stackedWidget->setCurrentIndex(1);
        break;
    case SPEEX_VBR_CODEC :
        ui.stackedWidget->setCurrentIndex(2);
        break;
    case OPUS_CODEC :
        ui.stackedWidget->setCurrentIndex(3);
        break;
    case NO_CODEC :
    default :
        ui.stackedWidget->setCurrentIndex(0);
        break;
    }
}

void ChannelDlg::slotUpdateSliderLabels()
{
    ui.spxQualityLabel->setText(QString("%1/10").arg(ui.spx_qualitySlider->value()));
    ui.spx_qualitySlider->setAccessibleName(QString("%1 %2 %3").arg(ui.label_8->text()).arg(ui.spx_qualitySlider->value()).arg(ui.spxQualityLabel->text()));
    ui.spxvbr_QualityLabel->setText(QString("%1/10").arg(ui.spxvbr_qualitySlider->value()));
    ui.spxvbr_qualitySlider->setAccessibleName(QString("%1 %2 %3").arg(ui.label_11->text()).arg(ui.spx_qualitySlider->value()).arg(ui.spxvbr_QualityLabel->text()));
    ui.gainlevelLabel->setText(QString::number(ui.gainlevelSlider->value()*1000));   
    ui.gainlevelSlider->setAccessibleName(QString("%1 %2 %3").arg(ui.label_9->text()).arg(ui.spx_qualitySlider->value()).arg(ui.gainlevelLabel->text()));
}

void ChannelDlg::slotUpdateChannelPath(const QString& str)
{
    TTCHAR path[TT_STRLEN] = {};
    if(TT_GetChannelPath(ttInst, m_channel.nParentID, path))
        ui.chanpathLabel->setText(_Q(path) + str);
    else
        ui.chanpathLabel->setText(QString());
    ui.chanpathLabel->setAccessibleName(QString("%1 %2").arg(ui.chanpathLabelFixed->text()).arg(ui.chanpathLabel->text()));

    QPushButton* btn = ui.buttonBox->button(QDialogButtonBox::Ok);
    if(btn)
        btn->setEnabled(str.length()>0 || 
            TT_GetRootChannelID(ttInst) == m_channel.nChannelID);
}

void ChannelDlg::slotAudioChannelChanged(int aud_channels)
{
    Q_UNUSED(aud_channels);
}

void ChannelDlg::slotSoloTransmitDelay()
{
    QInputDialog inputDialog;
    inputDialog.setOkButtonText(tr("&OK"));
    inputDialog.setCancelButtonText(tr("&Cancel"));
    inputDialog.setInputMode(QInputDialog::TextInput);
    inputDialog.setTextValue(QString::number(m_channel.nTransmitUsersQueueDelayMSec));
    inputDialog.setWindowTitle(tr("Transmission Queue Delay"));
    inputDialog.setLabelText(tr("Delay before switching to next user in queue (in msec)"));
    if (inputDialog.exec())
        m_channel.nTransmitUsersQueueDelayMSec = inputDialog.textValue().toInt();
}
