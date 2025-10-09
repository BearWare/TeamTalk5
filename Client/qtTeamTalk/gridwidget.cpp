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

#include "gridwidget.h"
#include <math.h>

GridWidget::GridWidget(QWidget * parent)
: QWidget(parent)
{
    m_grid = new QGridLayout(this);
}

void GridWidget::addUserWidget(int userid, UserImageWidget* widget)
{
    m_activewidgets.insert(userid, widget);
    RepositionWidgets();
}

void GridWidget::removeUserWidget(int userid)
{
    userwidgets_t::iterator ite = m_activewidgets.find(userid);
    if(ite != m_activewidgets.end())
    {
        QWidget* vid = *ite;
        m_activewidgets.erase(ite);
        vid->close();
        RepositionWidgets();
    }
}

UserImageWidget* GridWidget::getUserWidget(int userid) const
{
    userwidgets_t::const_iterator ite = m_activewidgets.find(userid);
    if(ite != m_activewidgets.end())
        return *ite;
    return nullptr;
}

bool GridWidget::userExists(int userid) const
{
    return m_activewidgets.find(userid) != m_activewidgets.end();
}

int GridWidget::selectedUser() const
{
    userwidgets_t::const_iterator ite = m_activewidgets.begin();
    while(ite != m_activewidgets.end())
    {
        if((*ite)->hasFocus())
            return ite.key();
        ite++;
    }
    return -1;
}

QSize GridWidget::getUserImageSize(int userid) const
{
    UserImageWidget* widget = getUserWidget(userid);
    if(widget)
        return widget->imageSize();
    return QSize();
}

void GridWidget::paintEvent(QPaintEvent *p)
{
    QWidget::paintEvent(p);

    if(m_activewidgets.isEmpty() && !getEmptyGridText().isEmpty())
    {
        QPainter painter(this);
        painter.drawText(rect(), Qt::AlignCenter, getEmptyGridText());
    }
}

QVector<int> GridWidget::activeUsers() const
{
    QVector<int> users;
    userwidgets_t::const_iterator ite = m_activewidgets.begin();
    while(ite != m_activewidgets.end())
    {
        users.push_back(ite.key());
        ite++;
    }
    return users;
}

int GridWidget::getActiveUsersCount() const
{
    return m_activewidgets.size();
}

void GridWidget::resetGrid()
{
    QVector<int> userids = activeUsers();
    for(int i=0;i<userids.size();i++)
        removeUserWidget(userids[i]);
}

void GridWidget::RepositionWidgets()
{
    int colums = (int)sqrt((double)m_activewidgets.size());
    int rows = (int)sqrt((double)m_activewidgets.size());
    rows = (m_activewidgets.size() <= rows*colums)? rows : rows + 1;
    colums = (m_activewidgets.size() <= rows*colums)? colums : colums + 1;

    if(m_grid->columnCount() > colums || m_grid->rowCount() > rows)
    {
        int x = m_grid->columnCount();
        for(int r = m_grid->rowCount()-1;r>=0;r--)
        {
            while(x > colums)
            {
                QLayoutItem* item = m_grid->itemAtPosition(r, x);
                if(item)
                    m_grid->removeItem(item);
                x--;
            }
        }
        int y = m_grid->rowCount();
        for(int c = m_grid->columnCount()-1;c>=0;c--)
        {
            while(y > rows)
            {
                QLayoutItem* item = m_grid->itemAtPosition(c, y);
                if(item)
                    m_grid->removeItem(item);
                y--;
            }
        }
    }
    userwidgets_t::iterator ite = m_activewidgets.begin();
    for(int x=0;x<colums;x++)
    {
        for(int y=0;y<rows;y++)
        {
            QLayoutItem* item = m_grid->itemAtPosition(y, x);
            if(item)
                m_grid->removeItem(item);
            if(ite != m_activewidgets.end())
            {
                m_grid->addWidget(*ite, y, x);
                ite++;
            }
        }
    }
}


