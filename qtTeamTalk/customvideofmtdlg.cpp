/*
 * Copyright (c) 2005-2016, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
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
