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

#include "customvideofmtdlg.h"
#include "appinfo.h"

CustomVideoFmtDlg::CustomVideoFmtDlg(QWidget* parent, VideoFormat& vidfmt)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
, m_vidfmt(vidfmt)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    if(vidfmt.nFPS_Denominator>0)
        ui.fpsEdit->setText(QString::number(vidfmt.nFPS_Numerator/vidfmt.nFPS_Denominator));
    else
        ui.fpsEdit->setText(QString::number(10));

    ui.widthEdit->setText(QString::number(vidfmt.nWidth));
    ui.heightEdit->setText(QString::number(vidfmt.nHeight));


    connect(this, SIGNAL(accepted()), SLOT(slotAccept()));
}

void CustomVideoFmtDlg::slotAccept()
{
    m_vidfmt.nWidth = ui.widthEdit->text().toInt();
    m_vidfmt.nHeight = ui.heightEdit->text().toInt();
    m_vidfmt.nFPS_Numerator = ui.fpsEdit->text().toInt();
    m_vidfmt.nFPS_Denominator = 1;
}
