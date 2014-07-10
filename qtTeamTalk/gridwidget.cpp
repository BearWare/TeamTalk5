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
    return NULL;
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

void GridWidget::ResetGrid()
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


