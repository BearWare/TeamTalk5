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

#ifndef GENERATETTFILEDLG_H
#define GENERATETTFILEDLG_H

#include "ui_generatettfile.h"

#include "common.h"

class GenerateTTFileDlg : public QDialog
{
    Q_OBJECT

public:
    GenerateTTFileDlg(const HostEntry& entry, QWidget * parent = 0);
    void exportTTFileToDirectory(const QString& dir);

private:
    void slotSetupHotkey();
    void slotVideoCodecChange(int);
    void slotSaveTTFile();
    void slotSaveAllTTFiles();

private:
    void loadVideoFormats();
    Ui::GenTTFileDlg ui;
    QVector<VideoFormat> m_vidcap_fmts;
    HostEntry m_hostentry;
};

#endif
