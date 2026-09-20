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

#ifndef STATUSBAREVENTEDITDLG_H
#define STATUSBAREVENTEDITDLG_H

#include "ui_statusbareventeditdlg.h"
#include "utilui.h"

class StatusBarEventEditDlg : public QDialog
{
    Q_OBJECT

public:
    StatusBarEventEditDlg(StatusBarEvents eventId, QWidget* parent = nullptr);

    QString getMessage() const { return ui.SBMsgEdit->text(); }

private:
    void insertVariable();
    void slotRestoreDefault();

private:
    Ui::StatusBarEventEditDlg ui;
    QMenu* m_SBVarMenu;
    StatusBarEvents m_eventId;
};

#endif
