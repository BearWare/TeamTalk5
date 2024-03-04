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

#include "serverdlg.h"
#include "appinfo.h"
#include "utilui.h"
#include "settings.h"

#include <QPushButton>
#include <QInputDialog>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

ServerDlg::ServerDlg(ServerDlgType type, const HostEntry& host, QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
    , m_hostentry(host)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_SERVERWINDOWPOS).toByteArray());

    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    ui.usernameBox->addItem(WEBLOGIN_BEARWARE_USERNAME);

    connect(ui.hostaddrBox, &QComboBox::editTextChanged,
            this, &ServerListDlg::slotGenerateEntryName);
    connect(ui.tcpportEdit, &QLineEdit::textChanged,
            this, &ServerListDlg::slotGenerateEntryName);
    connect(ui.cryptChkBox, &QCheckBox::toggled, ui.encsetupBtn, &QAbstractButton::setEnabled);
    connect(ui.encsetupBtn, &QAbstractButton::clicked, [&]()
    {
        HostEntry entry;
        if (getHostEntry(entry) && EncryptionSetupDlg(entry.encryption, this).exec())
            m_setup_encryption.reset(new HostEncryption(entry.encryption));
    });
    connect(ui.usernameBox, &QComboBox::editTextChanged,
            this, &ServerListDlg::slotGenerateEntryName);
    connect(ui.passwordChkBox, &QAbstractButton::clicked,
            this, [&](bool checked) { ui.passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password); } );
    connect(ui.chanpasswordChkBox, &QAbstractButton::clicked,
            this, [&](bool checked) { ui.chanpasswdEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password); } );

    switch(type)
    {
    case SERVER_CREATE :
        setWindowTitle(tr("Add Server"));
        this->setAccessibleDescription(tr("Add server"));
        break;
    case SERVER_UPDATE :
    {
        setWindowTitle(tr("Edit Server"));
        this->setAccessibleDescription(tr("Edit Server %1").arg(_Q(host.name)));
    }
    break;
    case SERVER_READONLY :
        setWindowTitle(tr("View Server Information"));
        this->setAccessibleDescription(tr("View %1 information").arg(_Q(host.name)));
        ui.nameEdit->setReadOnly(true);
        ui.hostaddrBox->setReadOnly(true);
        ui.tcpportEdit->setReadOnly(true);
        ui.udpportEdit->setReadOnly(true);
        ui.cryptChkBox->setReadOnly(true);
        ui.encsetupBtn->setEnabled(false);
        ui.usernameBox->setReadOnly(true);
        ui.passwordEdit->setReadOnly(true);
        ui.passwordChkBox->setReadOnly(true);
        ui.nicknameEdit->setReadOnly(true);
        ui.channelEdit->setReadOnly(true);
        ui.chanpasswdEdit->setReadOnly(true);
        ui.chanpasswordChkBox->setEnabled(false);
        break;
    }

    ui.nameEdit->setText(_Q(m_hostentry(name));
    ui.hostaddrBox->setText(_Q(m_hostentry.name));
    ui.tcpportEdit->setText(_Q(m_hostentry.tcpport));
    ui.udpportEdit->setText(_Q(m_hostentry.udpport));
    ui.cryptChkBox->setChecked(m_hostentry.encrpyted);
    ui.usernameBox->setText(_Q(m_hostentry.username));
    ui.passwordEdit->setText(_Q(m_hostentry.password));
    ui.nicknameEdit->setText(_Q(m_hostentry.nickname));
    ui.channelEdit->setText(_Q(m_hostentry.channel));
    ui.chanpasswdEdit->setText(_Q(m_hostentry.chanpasswd));
}

ServerDlg::~ServerDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_SERVERWINDOWPOS, saveGeometry());
}

HostEntry ServerDlg::GetHostEntry() const
{
    HostEntry newhostentry = m_hostentry;
    COPY_TTSTR(newhostentry.name, ui.nameEdit->text());
    COPY_TTSTR(newhostentry.ipaddr, ui.hostaddrBox->text());
    COPY_TTSTR(newhostentry.tcpport, ui.tcpportEdit->text());
    COPY_TTSTR(newhostentry.udpport, ui.udpportEdit->text());
    newhostentry.encrypted = ui.cryptChkBox->isChecked();
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

void ServerDlg::slotGenerateEntryName(const QString&)
{
    QString username = ui.usernameBox->lineEdit()->text();
    if(username.size())
        ui.nameEdit->setText(QString("%1@%2:%3")
                             .arg(username)
                             .arg(ui.hostaddrBox->lineEdit()->text())
                             .arg(ui.tcpportEdit->text()));
    else if(ui.hostaddrBox->lineEdit()->text().size())
        ui.nameEdit->setText(QString("%1:%2")
                             .arg(ui.hostaddrBox->lineEdit()->text())
                             .arg(ui.tcpportEdit->text()));
    else
        ui.nameEdit->setText(QString());

    ui.passwordEdit->setDisabled(username == WEBLOGIN_BEARWARE_USERNAME);
    ui.passwordChkBox->setDisabled(username == WEBLOGIN_BEARWARE_USERNAME);
    if (isWebLogin(username, true))
        ui.passwordEdit->setText("");
}
