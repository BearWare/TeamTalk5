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

#ifndef SERVERPROPERTIES_H
#define SERVERPROPERTIES_H

#include "common.h"

#include "ui_serverproperties.h"

class ServerPropertiesDlg : public QDialog
{
    Q_OBJECT

public:
    ServerPropertiesDlg(QWidget * parent = 0);
    ~ServerPropertiesDlg();

private:
    void slotAccepted();
    void slotShowMOTDVars(bool checked);
    void slotServerLogToggled(const QModelIndex &index);
    void insertVariable();
private:
    Ui::ServerPropertiesDlg ui;
    ServerProperties m_srvprop = {};
    class ServerLogEventsModel* m_serverlogmodel;
    QMenu* m_varMenu;
};


#endif
