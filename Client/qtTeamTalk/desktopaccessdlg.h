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

public:
    void accept();

private:
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
