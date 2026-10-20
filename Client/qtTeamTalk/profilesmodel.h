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

#ifndef PROFILESMODEL_H
#define PROFILESMODEL_H

#include "utilui.h"

#include <QAbstractTableModel>
#include <QString>
#include <QVector>

enum
{
    COLUMN_INDEX_PROFILE_NAME,
    COLUMN_COUNT_PROFILES,
};

struct ProfileEntry
{
    QString name;
    QString inipath;
};

typedef QVector<ProfileEntry> profiles_t;

class ProfilesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ProfilesModel(QObject* parent, get_logical_index_t getindex);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & index) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;

    void addProfile(const ProfileEntry& profile);
    void delProfile(int index);
    void renameProfile(int index, const QString& newname);
    const profiles_t& getProfiles() const { return m_profiles; }

private:
    profiles_t m_profiles;
    get_logical_index_t m_logical_column;
};

#endif
