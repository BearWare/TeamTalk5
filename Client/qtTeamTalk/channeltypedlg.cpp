/*
 * Copyright (C) 2026, Bjørn D. Rasmussen, BearWare.dk
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

#include "channeltypedlg.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

ChannelTypeDlg::ChannelTypeDlg(ChannelTypes channelType, QWidget* parent)
    : QDialog(parent)
    , m_permanentBox(new QCheckBox(tr("Permanent channel (stored on server)"), this))
    , m_soloTransmitBox(new QCheckBox(tr("No interruptions (no simultaneous voice transmission)"), this))
    , m_classroomBox(new QCheckBox(tr("Classroom (operator-controlled transmissions)"), this))
    , m_operatorReceiveOnlyBox(new QCheckBox(tr("Operator receive only (only operators see and hear users)"), this))
    , m_noVoiceActivationBox(new QCheckBox(tr("No voice activation (only Push-to-Talk allowed)"), this))
    , m_noRecordingBox(new QCheckBox(tr("No audio recording allowed"), this))
    , m_hiddenBox(new QCheckBox(tr("Hidden channel (invisible and only known by name)"), this))
{
    setWindowTitle(tr("Channel Type"));
    setAccessibleDescription(tr("Select the channel types used when creating a missing channel"));

    auto layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Select one or more channel types. Clear all options for a default channel."), this));

    const QList<QPair<QCheckBox*, ChannelType>> options = {
        {m_permanentBox, CHANNEL_PERMANENT},
        {m_soloTransmitBox, CHANNEL_SOLO_TRANSMIT},
        {m_classroomBox, CHANNEL_CLASSROOM},
        {m_operatorReceiveOnlyBox, CHANNEL_OPERATOR_RECVONLY},
        {m_noVoiceActivationBox, CHANNEL_NO_VOICEACTIVATION},
        {m_noRecordingBox, CHANNEL_NO_RECORDING},
        {m_hiddenBox, CHANNEL_HIDDEN},
    };
    for (const auto& option : options)
    {
        option.first->setObjectName(QString("channelType_%1").arg(option.second));
        option.first->setChecked(channelType & option.second);
        layout->addWidget(option.first);
    }

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

ChannelTypes ChannelTypeDlg::channelType() const
{
    ChannelTypes channelType = CHANNEL_DEFAULT;
    if (m_permanentBox->isChecked())
        channelType |= CHANNEL_PERMANENT;
    if (m_soloTransmitBox->isChecked())
        channelType |= CHANNEL_SOLO_TRANSMIT;
    if (m_classroomBox->isChecked())
        channelType |= CHANNEL_CLASSROOM;
    if (m_operatorReceiveOnlyBox->isChecked())
        channelType |= CHANNEL_OPERATOR_RECVONLY;
    if (m_noVoiceActivationBox->isChecked())
        channelType |= CHANNEL_NO_VOICEACTIVATION;
    if (m_noRecordingBox->isChecked())
        channelType |= CHANNEL_NO_RECORDING;
    if (m_hiddenBox->isChecked())
        channelType |= CHANNEL_HIDDEN;
    return channelType;
}

QString ChannelTypeDlg::channelTypeText(ChannelTypes channelType)
{
    QStringList channelTypes;
    if (channelType & CHANNEL_PERMANENT)
        channelTypes.append(tr("Permanent"));
    if (channelType & CHANNEL_SOLO_TRANSMIT)
        channelTypes.append(tr("No interruptions"));
    if (channelType & CHANNEL_CLASSROOM)
        channelTypes.append(tr("Classroom"));
    if (channelType & CHANNEL_OPERATOR_RECVONLY)
        channelTypes.append(tr("Operator receive only"));
    if (channelType & CHANNEL_NO_VOICEACTIVATION)
        channelTypes.append(tr("No voice activation"));
    if (channelType & CHANNEL_NO_RECORDING)
        channelTypes.append(tr("No recording"));
    if (channelType & CHANNEL_HIDDEN)
        channelTypes.append(tr("Hidden"));
    return channelTypes.isEmpty() ? tr("Default") : channelTypes.join(", ");
}
