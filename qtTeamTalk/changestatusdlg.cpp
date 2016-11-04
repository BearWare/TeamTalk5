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

#include "changestatusdlg.h"
#include "appinfo.h"
#include "settings.h"

extern TTInstance* ttInst;
extern QSettings* ttSettings;

ChangeStatusDlg::ChangeStatusDlg(QWidget* parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    connect(this, SIGNAL(accepted()), SLOT(slotAccepted()));

    if(TT_GetUser(ttInst, TT_GetMyUserID(ttInst), &m_user))
    {
        switch(m_user.nStatusMode & STATUSMODE_MODE)
        {
        case STATUSMODE_AVAILABLE:
            ui.availBtn->setFocus();
            ui.availBtn->setChecked(true);break;
        case STATUSMODE_AWAY:
            ui.awayBtn->setFocus();
            ui.awayBtn->setChecked(true);break;
        case STATUSMODE_QUESTION :
            ui.questionBtn->setFocus();
            ui.questionBtn->setChecked(true);break;
        }
        ui.msgEdit->setText(_Q(m_user.szStatusMsg));
    }
}

void ChangeStatusDlg::slotAccepted()
{
    m_user.nStatusMode &= STATUSMODE_FLAGS;
    if(ui.availBtn->isChecked())
        m_user.nStatusMode |= STATUSMODE_AVAILABLE;
    else if(ui.awayBtn->isChecked())
        m_user.nStatusMode |= STATUSMODE_AWAY;
    else if(ui.questionBtn->isChecked())
        m_user.nStatusMode |= STATUSMODE_QUESTION;

    ttSettings->setValue(SETTINGS_GENERAL_STATUSMESSAGE, ui.msgEdit->text());

    TT_DoChangeStatus(ttInst, m_user.nStatusMode, _W(ui.msgEdit->text()));
}
