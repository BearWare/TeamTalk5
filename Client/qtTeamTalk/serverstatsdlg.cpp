/*
 * Copyright (c) 2005-2017, BearWare.dk
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

#include "serverstatsdlg.h"
#include "appinfo.h"

#include <QTimer>

extern TTInstance* ttInst;

ServerStatisticsDlg::ServerStatisticsDlg(QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
    , m_cmdid(0)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(ui.closeBtn, SIGNAL(clicked()), SLOT(accept()));
    connect(ui.updateChkBox, SIGNAL(clicked()),
            SLOT(slotUpdateCmd()));
    connect(ui.updateChkBox, SIGNAL(toggled(bool)), ui.msecSpinBox,
            SLOT(setEnabled(bool)));

    ZERO_STRUCT(m_lastStats);

    //ServerProperties prop;
    //TT_GetServerProperties(ttInst, &prop);
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
    ui.totalEdit->setText(QString("%L1 / %L2")
        .arg(stats.nTotalBytesRX/1024)
        .arg(stats.nTotalBytesTX/1024));
    ui.voiceEdit->setText(QString("%L1 / %L2")
        .arg(stats.nVoiceBytesRX/1024)
        .arg(stats.nVoiceBytesTX/1024));
    ui.videoEdit->setText(QString("%L1 / %L2")
        .arg(stats.nVideoCaptureBytesRX/1024)
        .arg(stats.nVideoCaptureBytesTX/1024));
    ui.mediafileEdit->setText(QString("%L1 / %L2")
        .arg(stats.nMediaFileBytesRX/1024)
        .arg(stats.nMediaFileBytesTX/1024));
    ui.desktopEdit->setText(QString("%L1 / %L2")
        .arg(stats.nDesktopBytesRX/1024)
        .arg(stats.nDesktopBytesTX/1024));
    ui.usersservedEdit->setText(QString("%1").arg(stats.nUsersServed));
    ui.userspeakEdit->setText(QString("%1").arg(stats.nUsersPeak));
    ui.filesEdit->setText(QString("%L1 / %L2")
        .arg(stats.nFilesRx/1024)
        .arg(stats.nFilesTx/1024));

    if(m_lastStats.nUptimeMSec)
    {
        qint64 diff = stats.nUptimeMSec - m_lastStats.nUptimeMSec;

        if(diff)
        {
            double diff_rx = (stats.nTotalBytesRX - m_lastStats.nTotalBytesRX) / (double)diff;
            double diff_tx = (stats.nTotalBytesTX - m_lastStats.nTotalBytesTX) / (double)diff;
            ui.kbpsEdit->setText(QString("%1 / %2")
            .arg( diff_rx, 0, 'f', 1)
            .arg( diff_tx, 0, 'f', 1));
        }
    }

    ui.uptimeEdit->setText(QString("%1:%2:%3")
        .arg(stats.nUptimeMSec / 1000 / 60 / 60)
        .arg(((stats.nUptimeMSec / 1000 / 60) % 60), 2, 10, (QChar)'0')
        .arg(((stats.nUptimeMSec / 1000) % 60), 2, 10, (QChar)'0'));

    m_lastStats = stats;

    if(ui.updateChkBox->isChecked())
        QTimer::singleShot(ui.msecSpinBox->value(), this, SLOT(slotUpdateCmd()));
}
