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

#ifndef CHANNELDLG_H
#define CHANNELDLG_H

#include "common.h"
#include "ui_channel.h"

class ChannelDlg : public QDialog
{
    Q_OBJECT

public:
    enum ChannelDlgType
    {
        CHANNEL_CREATE,
        CHANNEL_UPDATE,
        CHANNEL_READONLY
    };

    ChannelDlg(ChannelDlgType type, const Channel& chan, QWidget * parent = 0);
    ~ChannelDlg();

    Channel GetChannel() const;
    bool joinChannel() const;

private:
    void setAudioCodecReadonly();
    void slotAudioCodecChanged(int index);
    void slotUpdateSliderLabels();
    void slotUpdateChannelPath(const QString &);
    void slotAudioChannelChanged(int aud_channels);
    void slotSoloTransmitDelay();

private:
    Ui::ChannelDlg ui;
    void initValues();
    Channel m_channel;
};

#endif
