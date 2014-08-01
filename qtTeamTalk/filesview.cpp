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

#include "filesview.h"
#include <QDragEnterEvent>
#include <QUrl>
#include <QFileInfo>
#include <QMimeData>
#include "common.h"
#include "filesmodel.h"

extern TTInstance* ttInst;

FilesView::FilesView(QWidget* parent)
: QTreeView(parent)
{
    setAcceptDrops(true);
}

QList<int> FilesView::selectedFiles(QStringList* fileNames/* = NULL*/)
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
