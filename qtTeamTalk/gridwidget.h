/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
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

    void ResetGrid();

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
