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

#ifndef CHATTEMPLATESDLG_H
#define CHATTEMPLATESDLG_H

#include "ui_chattemplatesdlg.h"
#include "utilui.h"

class ChatTemplatesDlg : public QDialog
{
    Q_OBJECT

public:
    ChatTemplatesDlg(QWidget* parent);

private:
    void slotAccept();

private:
    Ui::ChatTemplatesDlg ui;
    class ChatTemplatesModel* m_chattemplatesmodel = nullptr;
    void chatTemplateSelected(const QModelIndex &index);
    void insertVariable();
    void saveCurrentTemplate();
    void chatTemplatesRestoreDefaultTemplate();
    void chatTemplatesRestoreAllDefaultTemplate();
    QMenu* m_CTVarMenu;
    QModelIndex m_currentIndex;
};


#include <QAbstractTableModel>
#include <QVector>
#include <QTableView>

class ChatTemplatesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ChatTemplatesModel(QObject* parent);
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const override;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const override;
    QModelIndex parent ( const QModelIndex & index ) const override;
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const override;
private:
    QVector<ChatTemplate> m_chattemplates;
};

#endif
