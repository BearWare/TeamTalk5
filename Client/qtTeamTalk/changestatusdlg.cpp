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
        if (m_user.nStatusMode & STATUSMODE_FEMALE)
        {
            ui.availBtn->setText(tr("&Available", "For female"));
            ui.awayBtn->setText(tr("A&way", "For female"));
        }
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
