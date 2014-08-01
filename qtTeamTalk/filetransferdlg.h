/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
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

