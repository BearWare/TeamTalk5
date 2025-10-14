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

#include "serverstatsdlg.h"
#include "appinfo.h"
#include "utilui.h"

#include <QTimer>

extern TTInstance* ttInst;

ServerStatisticsDlg::ServerStatisticsDlg(QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
    , m_cmdid(0)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    this->setAccessibleDescription(tr("Server statistics"));

    connect(ui.updateChkBox, &QAbstractButton::clicked,
            this, &ServerStatisticsDlg::slotUpdateCmd);
    connect(ui.updateChkBox, &QAbstractButton::toggled, ui.msecSpinBox,
            &QWidget::setEnabled);

    slotUpdateCmd();
}

void ServerStatisticsDlg::slotUpdateCmd()
{
    m_cmdid = TT_DoQueryServerStats(ttInst);
}

void ServerStatisticsDlg::slotCmdSuccess(int cmdid)
{
    Q_UNUSED(cmdid);
}

void ServerStatisticsDlg::slotUpdateStats(const ServerStatistics& stats)
{
    ui.totalRXTXLabel->setText(QString(tr("Total RX/TX") + ": %L1 / %L2")
        .arg(getFormattedSize(stats.nTotalBytesRX))
        .arg(getFormattedSize(stats.nTotalBytesTX)));
    ui.voiceRXTXLabel->setText(QString(tr("Voice RX/TX") + ": %L1 / %L2")
        .arg(getFormattedSize(stats.nVoiceBytesRX))
        .arg(getFormattedSize(stats.nVoiceBytesTX)));
    ui.videoRXTXLabel->setText(QString(tr("Video RX/TX") + ": %L1 / %L2")
        .arg(getFormattedSize(stats.nVideoCaptureBytesRX))
        .arg(getFormattedSize(stats.nVideoCaptureBytesTX)));
    ui.mediafileRXTXLabel->setText(QString(tr("Media File RX/TX") + ": %L1 / %L2")
        .arg(getFormattedSize(stats.nMediaFileBytesRX))
        .arg(getFormattedSize(stats.nMediaFileBytesTX)));
    ui.desktopRXTXLabel->setText(QString(tr("Desktop RX/TX") + ": %L1 / %L2")
        .arg(getFormattedSize(stats.nDesktopBytesRX))
        .arg(getFormattedSize(stats.nDesktopBytesTX)));
    if(m_lastStats.nUptimeMSec)
    {
        qint64 diff = stats.nUptimeMSec - m_lastStats.nUptimeMSec;

        if(diff)
        {
            double diff_rx = (stats.nTotalBytesRX - m_lastStats.nTotalBytesRX) / (double)diff;
            double diff_tx = (stats.nTotalBytesTX - m_lastStats.nTotalBytesTX) / (double)diff;
            ui.kbpsLabel->show();
            ui.kbpsLabel->setText(QString(tr("Throughput RX/TX") + ": %1 / %2 " + tr("KBytes/sec"))
            .arg( diff_rx, 0, 'f', 1)
            .arg( diff_tx, 0, 'f', 1));
        }
        else
            ui.kbpsLabel->hide();
    }
    else
        ui.kbpsLabel->hide();
    ui.filesRXTXLabel->setText(QString(tr("Files RX/TX") + ": %L1 / %L2")
        .arg(getFormattedSize(stats.nFilesRx))
        .arg(getFormattedSize(stats.nFilesTx)));
    ui.usersLabel->setText(QString(tr("Users served") + ": %1").arg(stats.nUsersServed));
    ui.usersPeakLabel->setText(QString(tr("Users peak") + ": %1").arg(stats.nUsersPeak));

    ui.uptimeLabel->setText(QString(tr("Uptime: %1 hours, %2 minutes, %3 seconds"))
        .arg(stats.nUptimeMSec / 1000 / 60 / 60)
        .arg(((stats.nUptimeMSec / 1000 / 60) % 60), 2, 10, (QChar)'0')
        .arg(((stats.nUptimeMSec / 1000) % 60), 2, 10, (QChar)'0'));

    m_lastStats = stats;

    if(ui.updateChkBox->isChecked())
        QTimer::singleShot(ui.msecSpinBox->value(), this, &ServerStatisticsDlg::slotUpdateCmd);
}
