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

#include "statusbardlg.h"
#include "appinfo.h"
#include "statusbareventsmodel.h"
#include "settings.h"

extern QSettings* ttSettings;

StatusBarDlg::StatusBarDlg(QWidget* parent, StatusBarEvents events)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
, m_events(events)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_STATUSBARDLG_SIZE).toByteArray());

    m_statusbarmodel = new StatusBarEventsModel(this);
    ui.statusBarTreeView->setModel(m_statusbarmodel);
    m_statusbarmodel->setStatusBarEvents(m_events);

    ui.statusBarTreeView->header()->restoreState(ttSettings->value(SETTINGS_DISPLAY_STATUSBAR_EVENTS_HEADER).toByteArray());

    connect(ui.statusBarTreeView, &QAbstractItemView::doubleClicked, this, &StatusBarDlg::slotStatusBarEventToggled);
    connect(ui.statusBarEnableallButton, &QAbstractButton::clicked, this, &StatusBarDlg::slotStatusBarEnableAll);
    connect(ui.statusBarClearallButton, &QAbstractButton::clicked, this, &StatusBarDlg::slotStatusBarClearAll);
    connect(ui.statusBarRevertButton, &QAbstractButton::clicked, this, &StatusBarDlg::slotStatusBarRevert);
    connect(this, &QDialog::accepted, this, &StatusBarDlg::slotAccept);
}

void StatusBarDlg::slotStatusBarEventToggled(const QModelIndex &index)
{
    auto events = m_statusbarmodel->getStatusBarEvents();
    StatusBarEvent e = StatusBarEvent(index.internalId());
    if (e & events)
        m_statusbarmodel->setStatusBarEvents(events & ~e);
    else
        m_statusbarmodel->setStatusBarEvents(events | e);
}

void StatusBarDlg::slotStatusBarEnableAll(bool /*checked*/)
{
    m_statusbarmodel->setStatusBarEvents(~STATUSBAR_NONE);
}

void StatusBarDlg::slotStatusBarClearAll(bool /*checked*/)
{
    m_statusbarmodel->setStatusBarEvents(STATUSBAR_NONE);
}

void StatusBarDlg::slotStatusBarRevert(bool /*checked*/)
{
    m_statusbarmodel->setStatusBarEvents(m_events);
}

void StatusBarDlg::slotAccept()
{
    ttSettings->setValue(SETTINGS_STATUSBAR_ACTIVEEVENTS, m_statusbarmodel->getStatusBarEvents());
    ttSettings->setValue(SETTINGS_DISPLAY_STATUSBAR_EVENTS_HEADER, ui.statusBarTreeView->header()->saveState());
    ttSettings->setValue(SETTINGS_DISPLAY_STATUSBARDLG_SIZE, saveGeometry());
}
