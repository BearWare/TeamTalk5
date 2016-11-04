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

#include "uservideodlg.h"
#include "appinfo.h"

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
    p.setBrush(QPalette::Background, QBrush(Qt::black));
    uservideoWidget->setPalette(p);

    
#if defined(Q_OS_DARWIN)
    setMacResizeMargins(this, ui.horizontalLayout);
#endif
}

UserVideoDlg::~UserVideoDlg()
{
    emit(userVideoDlgClosing(uservideoWidget->getUserID()));
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
