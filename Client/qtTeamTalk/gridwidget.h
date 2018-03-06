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
