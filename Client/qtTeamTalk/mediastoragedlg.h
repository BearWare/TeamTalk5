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

#ifndef AUDIOSTORAGEDLG_H
#define AUDIOSTORAGEDLG_H

#include "ui_mediastorage.h"

class MediaStorageDlg : public QDialog
{
    Q_OBJECT

public:
    MediaStorageDlg(QWidget * parent = 0);
    ~MediaStorageDlg();

public:
    void accept();
private:
    void slotSetMediaFolder();
    void slotSetChanLogFolder();
    void slotSetUserLogFolder();
    void slotStop();
    void slotUpdateUI();

    QString getFolder();
    Ui::MediaStorageDlg ui;
};

#endif
