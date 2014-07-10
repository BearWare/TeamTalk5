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

#ifndef DESKTOPACCESSDLG_H
#define DESKTOPACCESSDLG_H

#include <QAbstractItemModel>

#include "ui_desktopaccess.h"
#include "common.h"

class DesktopAccessModel;

class DesktopAccessDlg : public QDialog
{
    Q_OBJECT

public:
    DesktopAccessDlg(QWidget* parent);

public slots:
    void accept();

private slots:
    void slotDelEntry();
    void slotClearEntry();
    void slotAddEntry();

    void slotAddChannel();
    void slotDelChannel();
    void slotAddUsername();
    void slotDelUsername();

    void slotServerSelected(const QModelIndex& index);
private:
    Ui::DesktopAccessDlg ui;
    DesktopAccessModel* m_model;
};

class DesktopAccessModel : public QAbstractItemModel
{
public:
    DesktopAccessModel(QObject* parent) : QAbstractItemModel(parent) { }
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const { Q_UNUSED(parent); return 1; }
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex()) const;
    QModelIndex parent ( const QModelIndex & index ) const { Q_UNUSED(index); return QModelIndex(); }
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const { Q_UNUSED(parent); return m_entries.size(); }

    void setEntries(QVector<DesktopAccessEntry>& entries);
    void updateEntries();
    QVector<DesktopAccessEntry>& getEntries() { return m_entries; }
private:
    QVector<DesktopAccessEntry> m_entries;
};


#endif
