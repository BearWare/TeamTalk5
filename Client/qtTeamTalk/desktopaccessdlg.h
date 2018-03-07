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
