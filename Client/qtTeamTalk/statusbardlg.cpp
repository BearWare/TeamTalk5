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
    connect(ui.statusBarTableView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &StatusBarDlg::statusBarEventSelected);
    m_SBVarMenu = new QMenu(this);
    connect(ui.SBVarButton, &QPushButton::clicked, this, [this]()
    {
        m_SBVarMenu->exec(QCursor::pos());
    });
    connect(ui.SBDefValButton, &QPushButton::clicked, this, &StatusBarDlg::statusBarRestoreDefaultMessage);
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
    {
        m_statusbarmodel->setStatusBarEvents(events & ~e);
        ui.sbmsg_groupbox->hide();
    }
    else
    {
        m_statusbarmodel->setStatusBarEvents(events | e);
        ui.sbmsg_groupbox->show();
    }
}

void StatusBarDlg::statusBarEventSelected(const QModelIndex &index)
{
    saveCurrentMessage();

    m_currentIndex = index;
    if (!index.isValid()) return;

    bool customizable = true;
    auto eventMap = UtilUI::eventToSettingMap();
    StatusBarEvents eventId = static_cast<StatusBarEvents>(index.internalId());

    if (eventMap.contains(eventId))
    {
        const StatusBarEventInfo& eventInfo = eventMap[eventId];
        QString paramKey = eventInfo.settingKey;
        QString defaultValue = UtilUI::getDefaultValue(paramKey);
        QString currentMessage = ttSettings->value(paramKey, defaultValue).toString();
        ui.SBMsgLabel->setText(eventInfo.eventName.size() > 0?tr("Message for Event \"%1\"").arg(eventInfo.eventName):tr("Message"));
        ui.SBMsgEdit->setText(currentMessage);

        m_SBVarMenu->clear();
        for (auto it = eventInfo.variables.constBegin(); it != eventInfo.variables.constEnd(); ++it)
        {
            QAction* action = m_SBVarMenu->addAction(it.value());
            action->setData(it.key());
            connect(action, &QAction::triggered, this, &StatusBarDlg::insertVariable);
        }
    }
    else
    {
        customizable = false;
    }
    auto events = m_statusbarmodel->getStatusBarEvents();
    StatusBarEvent e = StatusBarEvent(index.internalId());
    ui.sbmsg_groupbox->setVisible(customizable&&(e & events));
}

void StatusBarDlg::insertVariable()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        QString variable = action->data().toString();
        int cursorPos = ui.SBMsgEdit->cursorPosition();
        ui.SBMsgEdit->insert(variable);
        ui.SBMsgEdit->setCursorPosition(cursorPos + variable.length());
    }
}

void StatusBarDlg::saveCurrentMessage()
{
    if (!m_currentIndex.isValid()) return;

    auto eventMap = UtilUI::eventToSettingMap();
    StatusBarEvents eventId = static_cast<StatusBarEvents>(m_currentIndex.internalId());

    if (eventMap.contains(eventId))
    {
        const StatusBarEventInfo& eventInfo = eventMap[eventId];
        QString paramKey = eventInfo.settingKey;
        QString text = ui.SBMsgEdit->text();

        if (!text.isEmpty() && text != ttSettings->value(paramKey))
        {
            ttSettings->setValue(paramKey, text);
        }
    }
}

void StatusBarDlg::statusBarRestoreDefaultMessage()
{
    if (!m_currentIndex.isValid()) return;

    auto eventMap = UtilUI::eventToSettingMap();
    StatusBarEvents eventId = static_cast<StatusBarEvents>(m_currentIndex.internalId());

    if (eventMap.contains(eventId))
    {
        const StatusBarEventInfo& eventInfo = eventMap[eventId];
        QString defaultValue = UtilUI::getDefaultValue(eventInfo.settingKey);
        ui.SBMsgEdit->setText(defaultValue);
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
            const StatusBarEventInfo& eventInfo = eventMap[eventId];
            QString defaultValue = UtilUI::getDefaultValue(eventInfo.settingKey);
            ttSettings->setValue(eventInfo.settingKey, defaultValue);
            if (m_currentIndex.isValid() && m_currentIndex.internalId() == eventId)
                ui.SBMsgEdit->setText(defaultValue);
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
    ttSettings->setValue(SETTINGS_STATUSBAR_ACTIVEEVENTS, m_statusbarmodel->getStatusBarEvents());
    ttSettings->setValue(SETTINGS_DISPLAY_STATUSBAR_EVENTS_HEADER, ui.statusBarTableView->horizontalHeader()->saveState());
    ttSettings->setValue(SETTINGS_DISPLAY_STATUSBARDLG_SIZE, saveGeometry());
    saveCurrentMessage();
}
