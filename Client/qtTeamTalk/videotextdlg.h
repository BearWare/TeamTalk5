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

#ifndef VIDEOTEXTDLG_H
#define VIDEOTEXTDLG_H

#include "ui_videotext.h"

class VideoTextDlg : public QDialog
{
    Q_OBJECT

public:
    VideoTextDlg(QWidget* parent = 0);

public:
    void accept();

private:
    void slotSelectFont();
    void slotSelectBackground();
    void slotUpdateExample();

private:
    quint32 getVideoTextPos() const;
    Ui::VideoTextDlg ui;
    QColor m_bgColor, m_fgColor;
};
#endif
