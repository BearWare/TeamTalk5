/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
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

    Channel GetChannel() const;
private slots:
    void slotAudioCodecChanged(int index);
    void slotUpdateSliderLabels();
    void slotUpdateChannelPath(const QString &);
    void slotAudioChannelChanged(int aud_channels);

private:
    Ui::ChannelDlg ui;
    void initValues();
    Channel m_channel;
};

#endif
