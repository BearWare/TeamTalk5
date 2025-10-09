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

#include "customvideofmtdlg.h"
#include "appinfo.h"
#include <QPushButton>

CustomVideoFmtDlg::CustomVideoFmtDlg(QWidget* parent, VideoFormat& vidfmt)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
, m_vidfmt(vidfmt)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    if(vidfmt.nFPS_Denominator>0)
        ui.fpsEdit->setText(QString::number(vidfmt.nFPS_Numerator/vidfmt.nFPS_Denominator));
    else
        ui.fpsEdit->setText(QString::number(10));

    ui.widthEdit->setText(QString::number(vidfmt.nWidth));
    ui.heightEdit->setText(QString::number(vidfmt.nHeight));


    connect(this, &QDialog::accepted, this, &CustomVideoFmtDlg::slotAccept);
}

void CustomVideoFmtDlg::slotAccept()
{
    m_vidfmt.nWidth = ui.widthEdit->text().toInt();
    m_vidfmt.nHeight = ui.heightEdit->text().toInt();
    m_vidfmt.nFPS_Numerator = ui.fpsEdit->text().toInt();
    m_vidfmt.nFPS_Denominator = 1;
}
