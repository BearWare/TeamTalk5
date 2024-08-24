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

#ifndef STATUSBARDLG_H
#define STATUSBARDLG_H

#include "ui_statusbardlg.h"
#include "utilui.h"

class StatusBarDlg : public QDialog
{
    Q_OBJECT

public:
    StatusBarDlg(QWidget* parent, StatusBarEvents events);

private:
    void slotAccept();

private:
    Ui::StatusBarDlg ui;
    class StatusBarEventsModel* m_statusbarmodel = nullptr;
    void slotStatusBarEventToggled(const QModelIndex &index);
    void statusBarEventSelected(const QModelIndex &index);
    void insertVariable();
    void saveCurrentMessage();
    void statusBarRestoreDefaultMessage();
    void statusBarRestoreAllDefaultMessage();
    QMenu* m_SBVarMenu;
    QModelIndex m_currentIndex;
    void slotStatusBarEnableAll(bool checked);
    void slotStatusBarClearAll(bool checked);
    void slotStatusBarRevert(bool checked);
    StatusBarEvents m_events;
};

#endif
