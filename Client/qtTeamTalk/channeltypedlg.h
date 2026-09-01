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

#ifndef CHANNELTYPEDLG_H
#define CHANNELTYPEDLG_H

#include "common.h"

#include <QDialog>

class QCheckBox;

class ChannelTypeDlg : public QDialog
{
public:
    explicit ChannelTypeDlg(ChannelTypes channelType, QWidget* parent = nullptr);

    ChannelTypes channelType() const;
    static QString channelTypeText(ChannelTypes channelType);

private:
    QCheckBox* m_permanentBox;
    QCheckBox* m_soloTransmitBox;
    QCheckBox* m_classroomBox;
    QCheckBox* m_operatorReceiveOnlyBox;
    QCheckBox* m_noVoiceActivationBox;
    QCheckBox* m_noRecordingBox;
    QCheckBox* m_hiddenBox;
};

#endif // CHANNELTYPEDLG_H
