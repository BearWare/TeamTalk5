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

#include "channeldlg.h"
#include "appinfo.h"

#include <QPushButton>

extern TTInstance* ttInst;

ChannelDlg::ChannelDlg(ChannelDlgType type, const Channel& chan, QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
    , m_channel(chan)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(ui.audiocodecBox, SIGNAL(currentIndexChanged(int)), 
            SLOT(slotAudioCodecChanged(int)));

    connect(ui.spx_qualitySlider, SIGNAL(valueChanged(int)),
            SLOT(slotUpdateSliderLabels()));
    connect(ui.spxvbr_qualitySlider, SIGNAL(valueChanged(int)),
            SLOT(slotUpdateSliderLabels()));
    connect(ui.nameEdit, SIGNAL(textChanged(const QString&)),
            SLOT(slotUpdateChannelPath(const QString&)));
    
    connect(ui.agcBox, SIGNAL(toggled(bool)),
            ui.gainlevelSlider, SLOT(setEnabled(bool)));
    connect(ui.gainlevelSlider, SIGNAL(valueChanged(int)),
            SLOT(slotUpdateSliderLabels()));

    ServerProperties prop = {};
    TT_GetServerProperties(ttInst, &prop);

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
    ui.opus_bpsSpinBox->setRange(1, 512);
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

    ui.staticchanBox->setEnabled(TT_GetMyUserRights(ttInst) & USERRIGHT_MODIFY_CHANNELS);
    
    if( (TT_GetMyUserType(ttInst) & USERTYPE_ADMIN) == 0)
    {
        ui.diskquotaSpinBox->setEnabled(false);
        ui.maxusersSpinBox->setEnabled(false);
    }

    //default settings for Speex
    setCurrentItemData(ui.spx_srateBox, DEFAULT_SPEEX_BANDMODE);
    ui.spx_qualitySlider->setValue(DEFAULT_SPEEX_QUALITY);
    ui.spx_txdelaySpinBox->setValue(DEFAULT_SPEEX_DELAY);
    //default settings for Speex VBR
    setCurrentItemData(ui.spxvbr_srateBox, DEFAULT_SPEEX_VBR_BANDMODE);
    ui.spxvbr_qualitySlider->setValue(DEFAULT_SPEEX_VBR_QUALITY);
    ui.spxvbr_maxbpsSpinBox->setValue(DEFAULT_SPEEX_VBR_MAXBITRATE);
    ui.spxvbr_dtxBox->setChecked(DEFAULT_SPEEX_VBR_DTX);
    ui.spxvbr_txdelaySpinBox->setValue(DEFAULT_SPEEX_VBR_DELAY);
    //default settings for OPUS
    setCurrentItemData(ui.opus_srateBox, DEFAULT_OPUS_SAMPLERATE);
    setCurrentItemData(ui.opus_channelsBox, DEFAULT_OPUS_CHANNELS);
    setCurrentItemData(ui.opus_appBox, DEFAULT_OPUS_APPLICATION);
    ui.opus_bpsSpinBox->setValue(DEFAULT_OPUS_BITRATE / 1000);
    ui.opus_dtxBox->setChecked(DEFAULT_OPUS_DTX);
    ui.opus_txdelaySpinBox->setValue(DEFAULT_OPUS_DELAY);
    ui.opus_vbrCheckBox->setChecked(DEFAULT_OPUS_VBR);
    setCurrentItemData(ui.opus_framesizeComboBox, DEFAULT_OPUS_FRAMESIZE);

    switch(type)
    {
    case CHANNEL_CREATE :
        setWindowTitle(tr("Create Channel"));
        initDefaultAudioCodec(m_channel.audiocodec);

        //set default channel options
        m_channel.nMaxUsers = prop.nMaxUsers;
        m_channel.uChannelType = CHANNEL_DEFAULT;
        //set AGC
        m_channel.audiocfg.bEnableAGC = DEFAULT_CHANNEL_AUDIOCONFIG;
        m_channel.audiocfg.nGainLevel = DEFAULT_AGC_GAINLEVEL;

        m_channel.transmitUsers[0][TT_CLASSROOM_USERID_INDEX] = TT_CLASSROOM_FREEFORALL;
        m_channel.transmitUsers[0][TT_CLASSROOM_STREAMTYPE_INDEX] = STREAMTYPE_CLASSROOM_ALL;
        break;
    case CHANNEL_UPDATE :
    {
        setWindowTitle(tr("Update Channel"));
        int count = 0;
        TT_GetChannelUsers(ttInst, chan.nChannelID, nullptr, &count);
        if(count>0)
        {
            ui.audiocodecBox->setEnabled(false);
            ui.spx_srateBox->setEnabled(false);
            ui.spx_qualitySlider->setEnabled(false);
            ui.spx_txdelaySpinBox->setEnabled(false);

            ui.spxvbr_srateBox->setEnabled(false);
            ui.spxvbr_qualitySlider->setEnabled(false);
            ui.spxvbr_maxbpsSpinBox->setEnabled(false);
            ui.spxvbr_dtxBox->setEnabled(false);
            ui.spxvbr_txdelaySpinBox->setEnabled(false);

            ui.opus_srateBox->setEnabled(false);
            ui.opus_channelsBox->setEnabled(false);
            ui.opus_appBox->setEnabled(false);
            ui.opus_bpsSpinBox->setEnabled(false);
            ui.opus_dtxBox->setEnabled(false);
            ui.opus_txdelaySpinBox->setEnabled(false);
        }
    }
    break;
    case CHANNEL_READONLY :
        setWindowTitle(tr("View Channel Information"));
        ui.nameEdit->setReadOnly(true);
        ui.topicTextEdit->setReadOnly(true);
        ui.chanpasswdEdit->setReadOnly(true);
        ui.oppasswdEdit->setReadOnly(true);
        ui.maxusersSpinBox->setReadOnly(true);
        ui.diskquotaSpinBox->setReadOnly(true);
        ui.staticchanBox->setEnabled(false);
        ui.singletxchanBox->setEnabled(false);
        ui.classroomchanBox->setEnabled(false);
        ui.oprecvonlychanBox->setEnabled(false);
        ui.novoiceactBox->setEnabled(false);
        ui.norecordBox->setEnabled(false);

        ui.audiocodecBox->setEnabled(false);

        ui.spx_srateBox->setEnabled(false);
        ui.spx_qualitySlider->setEnabled(false);
        ui.spx_txdelaySpinBox->setEnabled(false);

        ui.spxvbr_srateBox->setEnabled(false);
        ui.spxvbr_qualitySlider->setEnabled(false);
        ui.spxvbr_maxbpsSpinBox->setEnabled(false);
        ui.spxvbr_dtxBox->setEnabled(false);
        ui.spxvbr_txdelaySpinBox->setEnabled(false);

        ui.opus_srateBox->setEnabled(false);
        ui.opus_channelsBox->setEnabled(false);
        ui.opus_appBox->setEnabled(false);
        ui.opus_bpsSpinBox->setEnabled(false);
        ui.opus_dtxBox->setEnabled(false);
        ui.opus_txdelaySpinBox->setEnabled(false);

        ui.agcBox->setEnabled(false);
        ui.gainlevelSlider->setEnabled(false);
        ui.buttonBox->setStandardButtons(QDialogButtonBox::Close);
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
    ui.classroomchanBox->setChecked(m_channel.uChannelType & CHANNEL_CLASSROOM);
    ui.oprecvonlychanBox->setChecked(m_channel.uChannelType & CHANNEL_OPERATOR_RECVONLY);
    ui.novoiceactBox->setChecked(m_channel.uChannelType & CHANNEL_NO_VOICEACTIVATION);
    ui.norecordBox->setChecked(m_channel.uChannelType & CHANNEL_NO_RECORDING);

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

    slotUpdateSliderLabels();
    slotUpdateChannelPath(_Q(m_channel.szName));
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

    return newchannel;
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
    ui.spxvbr_QualityLabel->setText(QString("%1/10").arg(ui.spxvbr_qualitySlider->value()));
    ui.gainlevelLabel->setText(QString::number(ui.gainlevelSlider->value()*1000));   
}

void ChannelDlg::slotUpdateChannelPath(const QString& str)
{
    TTCHAR path[TT_STRLEN] = {};
    if(TT_GetChannelPath(ttInst, m_channel.nParentID, path))
        ui.chanpathLabel->setText(_Q(path) + str);
    else
        ui.chanpathLabel->setText(QString());

    QPushButton* btn = ui.buttonBox->button(QDialogButtonBox::Ok);
    if(btn)
        btn->setEnabled(str.length()>0 || 
            TT_GetRootChannelID(ttInst) == m_channel.nChannelID);
}

void ChannelDlg::slotAudioChannelChanged(int aud_channels)
{
    Q_UNUSED(aud_channels);
}
