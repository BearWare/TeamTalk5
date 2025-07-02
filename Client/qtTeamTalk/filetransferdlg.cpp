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

#include "filetransferdlg.h"
#include "appinfo.h"
#include "settings.h"
#include "utilsound.h"
#include "utilui.h"

#include <QDir>
#include <QUrl>
#include <QMessageBox>
#include <QDesktopServices>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

FileTransferDlg::FileTransferDlg(const FileTransfer& transfer, QWidget* parent)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
, m_transferid(transfer.nTransferID)
, m_lastTransferred(0)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(ui.cancelButton, &QAbstractButton::clicked,
            this, &FileTransferDlg::slotCancelTransfer);
    connect(ui.openButton, &QAbstractButton::clicked,
        this, &FileTransferDlg::slotOpenFile);
    connect(ui.closeChkBox, &QAbstractButton::clicked,
            this, &FileTransferDlg::slotUpdateSettings);

    m_start = QTime::currentTime();

    ui.openButton->setEnabled(false);

    m_timerid = startTimer(1000);

    updateFileTransfer(transfer);

    ui.closeChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_CLOSE_FILEDIALOG, SETTINGS_DISPLAY_CLOSE_FILEDIALOG_DEFAULT).toBool());
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
                tr("File transfer failed."));
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
    ui.filenameLabel->setAccessibleName(QString("%1 %2").arg(ui.label->text()).arg(_Q(transfer.szRemoteFileName)));
    ui.filesizeLabel->setText(getFormattedSize(transfer.nFileSize));
    ui.filesizeLabel->setAccessibleName(QString("%1 %2").arg(ui.label_2->text()).arg(getFormattedSize(transfer.nFileSize)));
    double percent = 100.0;
    if(transfer.nFileSize)
        percent = transfer.nTransferred * 100 / transfer.nFileSize;
    ui.transferredLabel->setText(QString("%1/%2 - %3 %")
        .arg(getFormattedSize(transfer.nTransferred)).arg(getFormattedSize(transfer.nFileSize)).arg(percent));
    ui.transferredLabel->setAccessibleName(QString("%1/%2 - %3 %").arg(getFormattedSize(transfer.nTransferred)).arg(getFormattedSize(transfer.nFileSize)).arg(percent));

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
    QString lt = QString(tr("%1/second, last second %2").arg(getFormattedSize(transfer.nTransferred / elapsed)).arg(getFormattedSize(diff)));
    ui.throughputLabel->setText(lt);
    ui.throughputLabel->setAccessibleName(QString("%1 %2").arg(ui.label_3->text()).arg(lt));

    if(transfer.bInbound)
    {
        ui.destinationLabel->setText(_Q(transfer.szLocalFilePath));
        ui.destinationLabel->setAccessibleName(QString("%1 %2").arg(ui.label_4->text()).arg(_Q(transfer.szLocalFilePath)));
    }
    else
    {
        TTCHAR chanpath[TT_STRLEN] = {};
        TT_GetChannelPath(ttInst, transfer.nChannelID, chanpath);
        ui.destinationLabel->setText(_Q(chanpath));
        ui.destinationLabel->setAccessibleName(QString("%1 %2").arg(ui.label_4->text()).arg(_Q(chanpath)));
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
            playSoundEvent(SOUNDEVENT_FILETXDONE);

            killTimer(m_timerid);
            m_timerid = -1;

            if(ttSettings->value(SETTINGS_DISPLAY_CLOSE_FILEDIALOG, SETTINGS_DISPLAY_CLOSE_FILEDIALOG_DEFAULT).toBool() == true)
                this->close();
            break;
        case FILETRANSFER_ERROR :
            killTimer(m_timerid);
            m_timerid = -1;
            break;
        default : break;
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

void FileTransferDlg::slotUpdateSettings()
{
    ttSettings->setValue(SETTINGS_DISPLAY_CLOSE_FILEDIALOG, ui.closeChkBox->isChecked());
}
