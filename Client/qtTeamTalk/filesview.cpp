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
#include <QDragEnterEvent>
#include <QUrl>
#include <QFileInfo>
#include <QMimeData>
#include "common.h"
#include "filesmodel.h"

extern TTInstance* ttInst;

FilesView::FilesView(QWidget* parent)
: QListView(parent)
{
    setAcceptDrops(true);
}

QList<int> FilesView::selectedFiles(QStringList* fileNames/* = nullptr*/)
{
    QItemSelectionModel* sel = selectionModel();
    QModelIndexList indexes = sel->selectedRows();//selectedIndexes ();
    QList<int> files;
    for(int i=0;i<indexes.size();i++)
    {
        files.push_back(indexes[i].internalId());
        if(fileNames)
            fileNames->push_back(indexes[i].data(COLUMN_INDEX_NAME).toString());
    }
    return files;
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

void FilesView::mousePressEvent(QMouseEvent* event )
{
    QTreeView::mousePressEvent(event);

    //QDrag* drag = new QDrag(this);
    //QMimeData* mimedata = new QMimeData();;
    ////QList<QUrl> urls;
    ////urls.push_back(QUrl("foo.txt"));
    ////mimedata->setUrls(urls);
    ////drag->setMimeData(mimedata);
    //int ret = drag->exec(Qt::CopyAction);
    //mimedata = drag->mimeData();
    //ret = ret;
}

void FilesView::slotNewSelection(const QItemSelection & selected)
{
    emit(filesSelected(selected.size()>0));
}
