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

#ifndef FILETRANSFERDLG_H
#define FILETRANSFERDLG_H

#include "ui_filetransfer.h"

#include "common.h"

#include <QTime>

class FileTransferDlg : public QDialog
{
    Q_OBJECT

public:
    FileTransferDlg(const FileTransfer& transfer, QWidget* parent);

protected:
    void timerEvent(QTimerEvent *event);

private:
    void updateFileTransfer(const FileTransfer& transfer);
    Ui::FileTransferDlg ui;
    int m_transferid;
    QTime m_start;
    qint64 m_lastTransferred;
    int m_timerid;
    QString m_localfilename;

public:
    void slotTransferUpdate(const FileTransfer& transfer);

private:
    void slotOpenFile();
    void slotCancelTransfer();
    void slotUpdateSettings();
};

#endif

