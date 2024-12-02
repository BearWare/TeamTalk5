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

#include "aboutdlg.h"
#include "appinfo.h"
#include "utiltt.h"
#include "utilui.h"

#include <QLibraryInfo>

AboutDlg::AboutDlg(QWidget* parent)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    QString dateTimeString = QString("%1 %2").arg(__DATE__).arg(__TIME__);
    QString compile = QString(tr("Compiled on %1 using Qt %2 (Qt %3 used by this instance).")).arg(getFormattedDateTime(dateTimeString, "MMM d yyyy HH:mm:ss")).arg(QT_VERSION_STR).arg(qVersion()) + "\r\n" +
        tr("Version ") + (TEAMTALK_VERSION ".\r\n");
    if(sizeof(void*) == 8)
        compile += QString(tr("TeamTalk 64-bit DLL version %1.")).arg(_Q(TT_GetVersion()));
    else
        compile += QString(tr("TeamTalk 32-bit DLL version %1.")).arg(_Q(TT_GetVersion()));
    ui.compileLabel->setText(compile);
    ui.compileLabel->setAccessibleName(compile);
    ui.versionLabel->setText(APPVERSION);
    ui.versionLabel->setAccessibleName(APPVERSION);
    ui.iconLabel->setPixmap(QPixmap(APPICON));
}
