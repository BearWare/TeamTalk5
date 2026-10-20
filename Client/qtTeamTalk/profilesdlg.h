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

#ifndef PROFILESDLG_H
#define PROFILESDLG_H

#include "profilesmodel.h"

#include "ui_profiles.h"

#include <QSortFilterProxyModel>

class ProfilesDlg : public QDialog
{
    Q_OBJECT

public:
    ProfilesDlg(QWidget* parent = 0);
    ~ProfilesDlg();

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    Ui::ProfilesDlg ui;
    ProfilesModel* m_profilesModel;
    QSortFilterProxyModel* m_proxyModel;

    QString m_baseinipath;
    int m_freeno = -1;

private:
    void loadProfiles();
    void launchInstance(const QString& inipath, bool noconnect);
    void slotNewProfile();
    void slotCurrentProfile();
    void slotOpenProfile();
    void slotEditProfile();
    void slotDeleteProfile();
    void slotTableContextMenu(const QPoint&);
};

#endif
