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

#ifndef USERVOLUMEDLG_H
#define USERVOLUMEDLG_H

#include "ui_uservolume.h"

class UserVolumeDlg : public QDialog
{
    Q_OBJECT

public:
    UserVolumeDlg(int userid, QWidget * parent = 0);

private:
    void slotVolumeChanged(int);
    void slotMuteChannel();
    void slotDefaults();

private:
    int m_userid;
    Ui::UserVolumeDlg ui;
};
#endif
