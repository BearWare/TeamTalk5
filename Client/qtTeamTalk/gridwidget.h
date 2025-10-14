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

#ifndef GRIDWIDGET_H
#define GRIDWIDGET_H

#include <QGridLayout>
#include <QWidget>
#include <QMap>

#include "userimagewidget.h"

class GridWidget : public QWidget
{
    Q_OBJECT

public:
    GridWidget(QWidget * parent);

    bool userExists(int userid) const;
    int selectedUser() const;
    QVector<int> activeUsers() const;
    int getActiveUsersCount() const;

    void resetGrid();

    QSize getUserImageSize(int userid) const;

    void paintEvent(QPaintEvent *p);

private:
    void RepositionWidgets();
    QGridLayout* m_grid;
    typedef QMap<int, UserImageWidget*> userwidgets_t;
    userwidgets_t m_activewidgets;

protected:
    void addUserWidget(int userid, UserImageWidget* widget);
    void removeUserWidget(int userid);
    UserImageWidget* getUserWidget(int userid) const;
    virtual QString getEmptyGridText() const = 0;
};

#endif
