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
 * This source code is part of the TeamTalk 4 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#include "filetransferdlg.h"
#include "appinfo.h"

#include <QDir>
#include <QUrl>
#include <QMessageBox>
#include <QDesktopServices>

extern TTInstance* ttInst;

FileTransferDlg::FileTransferDlg(const FileTransfer& transfer, QWidget* parent)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
, m_transferid(transfer.nTransferID)
, m_lastTransferred(0)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(ui.cancelButton, SIGNAL(clicked()), 
            SLOT(slotCancelTransfer()));
    connect(ui.openButton, SIGNAL(clicked()),
        SLOT(slotOpenFile()));

    m_start = QTime::currentTime();

    ui.openButton->setEnabled(false);

    m_timerid = startTimer(1000);

    updateFileTransfer(transfer);
}

void FileTransferDlg::timerEvent(QTimerEvent* event)
{
    if(event->timerId() == m_timerid)
    {
        FileTransfer transfer;
        if(!TT_GetFileTransferInfo(ttInst, m_transferid, &transfer))
        {
            killTimer(m_timerid);
            m_timerid = -1;
            QMessageBox::critical(this, tr("File Transfer"), 
                tr("File tranfer failed."));
        }
        else
            updateFileTransfer(transfer);
    }
}

void FileTransferDlg::updateFileTransfer(const FileTransfer& transfer)
{
    if(m_localfilename.isEmpty())
        m_localfilename = _Q(transfer.szLocalFilePath);

    setWindowTitle(_Q(transfer.szRemoteFileName));
    ui.filenameLabel->setText(_Q(transfer.szRemoteFileName));
    if(transfer.nFileSize>=1024)
        ui.filesizeLabel->setText(QString("%1 KBytes")
                                    .arg(transfer.nFileSize/1024));
    else
        ui.filesizeLabel->setText(QString("%1 Bytes").arg(transfer.nFileSize));
    double percent = 100.0;
    if(transfer.nFileSize)
        percent = transfer.nTransferred * 100 / transfer.nFileSize;
    ui.transferredLabel->setText(QString("%1/%2 - %3 %")
        .arg(transfer.nTransferred).arg(transfer.nFileSize).arg(percent));

    ui.progressBar->setValue((int)percent);

    int diff = transfer.nTransferred - m_lastTransferred;
    m_lastTransferred = transfer.nTransferred;

    double elapsed = m_start.secsTo(QTime::currentTime());
    double throughput = 0.0;
    if(elapsed)
    {
        throughput = transfer.nTransferred / elapsed;
        throughput /= 1024.0;
    }
    ui.throughputLabel->setText(tr("%1 KBytes/second, last second %2 bytes")
                                .arg(throughput).arg(diff));

    if(transfer.bInbound)
        ui.destinationLabel->setText(_Q(transfer.szLocalFilePath));
    else
    {
        TTCHAR chanpath[TT_STRLEN] = {0};
        TT_GetChannelPath(ttInst, transfer.nChannelID, chanpath);
        ui.destinationLabel->setText(_Q(chanpath));
    }
}

void FileTransferDlg::slotTransferUpdate(const FileTransfer& transfer)
{
    if(transfer.nTransferID == m_transferid)
    {
        updateFileTransfer(transfer);

        switch(transfer.nStatus)
        {
        case FILETRANSFER_FINISHED :
            ui.cancelButton->setText(tr("&Close"));
            ui.openButton->setEnabled(true);
            PlaySoundEvent(SOUNDEVENT_FILETXDONE);

            killTimer(m_timerid);
            m_timerid = -1;

            if(ui.closeChkBox->isChecked())
                this->close();
            break;
        case FILETRANSFER_ERROR :
            killTimer(m_timerid);
            m_timerid = -1;
            break;
        }
    }
}

void FileTransferDlg::slotOpenFile()
{
    QString filename = QDir::fromNativeSeparators(m_localfilename);
    filename = "file:///" + filename;
    if(!QDesktopServices::openUrl(QUrl(filename, QUrl::TolerantMode)))
        QMessageBox::information(this, ui.openButton->text(),
        tr("Unable to open \"%1\". File does not have a default file association").arg(filename));
}

void FileTransferDlg::slotCancelTransfer()
{
    if(m_timerid >= 0)
        TT_CancelFileTransfer(ttInst, m_transferid);
    close();
}

