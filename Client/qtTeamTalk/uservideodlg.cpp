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

#include "uservideodlg.h"
#include "appinfo.h"
#include "utilui.h"

extern TTInstance* ttInst;

UserVideoDlg::UserVideoDlg(int userid_masked, const User& user, const QSize& size,
                           QWidget * parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
, m_autosize(false)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    initCommon(userid_masked);

    this->resize(size);
    slotUserUpdate(user);

}

UserVideoDlg::UserVideoDlg(int userid_masked, const User& user, QWidget * parent/* = 0*/)
: QDialog(parent, Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
, m_autosize(true)
{
    Q_ASSERT(userid_masked & VIDEOTYPE_MASK);

    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    initCommon(userid_masked);

    QSize img_size = uservideoWidget->imageSize();
    if(img_size.isValid())
        this->resize(img_size);
    else
        this->resize(QSize(100,100));

    slotUserUpdate(user);

}

void UserVideoDlg::initCommon(int userid)
{
    Q_ASSERT(userid & VIDEOTYPE_MASK);

    uservideoWidget = new UserVideoWidget(this, userid, 0);
    ui.horizontalLayout->addWidget(uservideoWidget);

    //set black as background color
    uservideoWidget->setAutoFillBackground(true);
    QPalette p = uservideoWidget->palette();
    p.setBrush(QPalette::Window, QBrush(Qt::black));
    uservideoWidget->setPalette(p);

    
#if defined(Q_OS_DARWIN)
    setMacResizeMargins(this, ui.horizontalLayout);
#endif
}

UserVideoDlg::~UserVideoDlg()
{
    emit userVideoDlgClosing(uservideoWidget->getUserID());
}

void UserVideoDlg::slotUserUpdate(const User& user)
{
    if(user.nUserID != uservideoWidget->getUserID())
        return;

    if(user.nUserID == 0)
        setWindowTitle(tr("Video") + QString(" - ") + tr("Myself"));
    else
        setWindowTitle(tr("Video") + QString(" - ") + getDisplayName(user));
}

void UserVideoDlg::slotNewVideoFrame(int userid, int /*streamid*/)
{
    Q_ASSERT(userid & VIDEOTYPE_MASK);
    if(userid != uservideoWidget->getUserID())
        return;

    if(m_autosize)
    {
        this->resize(uservideoWidget->imageSize());
        m_autosize = false;

#if defined(Q_OS_DARWIN)
        setMacResizeMargins(this, ui.horizontalLayout);
#endif
    }
}
