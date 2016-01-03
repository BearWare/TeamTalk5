/*
 * Copyright (c) 2005-2016, BearWare.dk
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

#ifndef FILESVIEW_H
#define FILESVIEW_H

#include <QTreeView>
#include <QList>



class FilesView : public QTreeView
{
    Q_OBJECT
public:
    FilesView(QWidget* parent);

    QList<int> selectedFiles(QStringList* fileNames = NULL);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);
    void mousePressEvent(QMouseEvent* event);

signals:
    void filesSelected(bool);
    void uploadFiles(const QStringList& files);

public slots:
    void slotNewSelection(const QItemSelection & selected);
};
#endif
