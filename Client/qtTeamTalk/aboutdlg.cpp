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
    m_ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    QString const DATE_TIME_STRING = QString("%1 %2").arg(__DATE__, __TIME__);
    QString compile = QString(tr("Compiled on %1 using Qt %2 (Qt %3 used by this instance).")).arg(getFormattedDateTime(DATE_TIME_STRING, "MMM d yyyy HH:mm:ss")).arg(QT_VERSION_STR).arg(qVersion()) + "\r\n" +
        tr("Version ") + (TEAMTALK_VERSION ".\r\n");
    if (sizeof(void*) == sizeof(qint64))
        compile += QString(tr("TeamTalk 64-bit DLL version %1.")).arg(_Q(TT_GetVersion()));
    else
        compile += QString(tr("TeamTalk 32-bit DLL version %1.")).arg(_Q(TT_GetVersion()));
    m_ui.compileLabel->setText(compile);
    m_ui.compileLabel->setAccessibleName(compile);
    m_ui.versionLabel->setText(APPVERSION);
    m_ui.versionLabel->setAccessibleName(APPVERSION);
    m_ui.iconLabel->setPixmap(QPixmap(APPICON));
}
