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

#include "changestatusdlg.h"
#include "appinfo.h"
#include "settings.h"
#include <QPushButton>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

ChangeStatusDlg::ChangeStatusDlg(QWidget* parent/* = 0*/)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    connect(this, &QDialog::accepted, this, &ChangeStatusDlg::slotAccepted);

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
        ui.streamChkBox->setChecked(ttSettings->value(SETTINGS_GENERAL_STREAMING_STATUS, SETTINGS_GENERAL_STREAMING_STATUS_DEFAULT).toBool());
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
    ttSettings->setValue(SETTINGS_GENERAL_STREAMING_STATUS, ui.streamChkBox->isChecked());

    TT_DoChangeStatus(ttInst, m_user.nStatusMode, _W(ui.msgEdit->text()));
}
