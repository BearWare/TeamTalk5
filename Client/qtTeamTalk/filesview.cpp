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

#include "filesview.h"
#include "common.h"
#include "filesmodel.h"
#include "settings.h"
#include "utiltts.h"

#include <QDragEnterEvent>
#include <QUrl>
#include <QFileInfo>
#include <QMimeData>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

FilesView::FilesView(QWidget* parent) : MyTreeView(parent)
{
    setAcceptDrops(true);
}

QStringList FilesView::selectedFiles()
{
    QStringList fileNames;
    QItemSelectionModel* sel = selectionModel();
    QModelIndexList indexes = sel->selectedRows();
    for(int i=0;i<indexes.size();i++)
    {
        fileNames.push_back(indexes[i].data(COLUMN_INDEX_NAME).toString());
    }
    return fileNames;
}

void FilesView::dragEnterEvent(QDragEnterEvent *event)
{
    if((TT_GetFlags(ttInst) & CLIENT_AUTHORIZED) == 0)
        return;
    foreach(QUrl url, event->mimeData()->urls())
        if (!url.isEmpty())
        {
            event->acceptProposedAction();
            return;
        }
}

void FilesView::dragMoveEvent(QDragMoveEvent * event)
{
    if((TT_GetFlags(ttInst) & CLIENT_AUTHORIZED) == 0)
        return;

    foreach(QUrl url, event->mimeData()->urls())
        if (!url.isEmpty())
        {
            event->acceptProposedAction();
            return;
        }
}

void FilesView::dropEvent(QDropEvent *event)
{
    QStringList files;
    foreach(QUrl url, event->mimeData()->urls())
        files.push_back(url.toLocalFile());
    emit(uploadFiles(files));
}

void FilesView::slotNewSelection(const QItemSelection & selected)
{
    emit(filesSelected(selected.size()>0));
}
