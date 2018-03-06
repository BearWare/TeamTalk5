/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
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

public slots:
    void slotTransferUpdate(const FileTransfer& transfer);

private slots:
    void slotOpenFile();
    void slotCancelTransfer();
};

#endif

