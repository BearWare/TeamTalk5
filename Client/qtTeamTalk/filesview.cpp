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

#include "filesview.h"
#include "filesmodel.h"

#include <QDragEnterEvent>
#include <QUrl>
#include <QFileInfo>
#include <QMimeData>
#include <QSettings>

extern TTInstance* ttInst;

FilesView::FilesView(QWidget* parent) : MyTableView(parent)
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
    emit uploadFiles(files);
}

void FilesView::slotNewSelection(const QItemSelection & selected)
{
    emit filesSelected(selected.size()>0);
}
