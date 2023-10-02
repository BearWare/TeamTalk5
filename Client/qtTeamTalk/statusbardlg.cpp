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
