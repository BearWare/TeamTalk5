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

#include <QMenu>
#include <QMessageBox>

#include "statusbardlg.h"
#include "statusbareventeditdlg.h"
#include "appinfo.h"
#include "statusbareventsmodel.h"
#include "settings.h"

extern NonDefaultSettings* ttSettings;

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
    ui.statusBarTableView->setModel(m_statusbarmodel);
    m_statusbarmodel->setStatusBarEvents(m_events);

    ui.statusBarTableView->horizontalHeader()->restoreState(ttSettings->value(SETTINGS_DISPLAY_STATUSBAR_EVENTS_HEADER).toByteArray());
    ui.statusBarTableView->horizontalHeader()->setSectionsMovable(false);

    connect(ui.statusBarTableView, &QAbstractItemView::doubleClicked, this, &StatusBarDlg::slotStatusBarEventToggled);
    ui.statusBarTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.statusBarTableView, &QWidget::customContextMenuRequested,
            this, &StatusBarDlg::slotTableContextMenu);

    connect(ui.SBDefAllValButton, &QPushButton::clicked, this, &StatusBarDlg::statusBarRestoreAllDefaultMessage);
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

void StatusBarDlg::slotEditEvent()
{
    QModelIndex index = ui.statusBarTableView->currentIndex();
    if (!index.isValid()) return;

    StatusBarEvents eventId = static_cast<StatusBarEvents>(index.internalId());
    auto eventMap = UtilUI::eventToSettingMap();
    if (!eventMap.contains(eventId)) return;

    auto events = m_statusbarmodel->getStatusBarEvents();
    if (!(StatusBarEvent(eventId) & events)) return;

    StatusBarEventEditDlg dlg(eventId, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        QString paramKey = eventMap[eventId].settingKey;
        QString text = dlg.getMessage();

        if (!text.isEmpty() && text != ttSettings->value(paramKey))
        {
            ttSettings->setValueOrClear(paramKey, text, UtilUI::getDefaultValue(paramKey));
        }
    }
}

void StatusBarDlg::statusBarRestoreAllDefaultMessage()
{
    QMessageBox answer;
    answer.setText(tr("Are you sure you want to restore all Status bar messages to default values?"));
    QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
    QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
    Q_UNUSED(YesButton);
    answer.setIcon(QMessageBox::Information);
    answer.setWindowTitle(tr("Restore default values"));
    answer.exec();
    if(answer.clickedButton() == NoButton)
        return;
    auto eventMap = UtilUI::eventToSettingMap();
    for (StatusBarEvents event = STATUSBAR_USER_LOGGEDIN; event < STATUSBAR_NEXT_UNUSED; event <<= 1)
    {
        StatusBarEvents eventId = static_cast<StatusBarEvents>(event);
        if (eventMap.contains(eventId))
        {
            ttSettings->remove(eventMap[eventId].settingKey);
        }
    }
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
    ttSettings->setValueOrClear(SETTINGS_STATUSBAR_ACTIVEEVENTS, m_statusbarmodel->getStatusBarEvents(), SETTINGS_STATUSBAR_ACTIVEEVENTS_DEFAULT);
    ttSettings->setValue(SETTINGS_DISPLAY_STATUSBAR_EVENTS_HEADER, ui.statusBarTableView->horizontalHeader()->saveState());
    ttSettings->setValue(SETTINGS_DISPLAY_STATUSBARDLG_SIZE, saveGeometry());
}

void StatusBarDlg::slotTableContextMenu(const QPoint& /*point*/)
{
    QModelIndex index = ui.statusBarTableView->currentIndex();
    bool valid = index.isValid();
    bool customizable = valid && UtilUI::eventToSettingMap().contains(static_cast<StatusBarEvents>(index.internalId()));
    bool enabled = valid && (StatusBarEvent(index.internalId()) & m_statusbarmodel->getStatusBarEvents());

    QMenu menu(this);
    QAction* toggleEvent = menu.addAction(enabled ? tr("&Disable") : tr("&Enable"));
    toggleEvent->setEnabled(valid);
    QAction* editEvent = menu.addAction(tr("&Edit Message"));
    editEvent->setEnabled(customizable && enabled);

    if (QAction* action = menu.exec(QCursor::pos()))
    {
        if (action == toggleEvent)
            slotStatusBarEventToggled(index);
        else if (action == editEvent)
            slotEditEvent();
    }
}
