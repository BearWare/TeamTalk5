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

#include "statusbareventeditdlg.h"
#include "settings.h"

extern NonDefaultSettings* ttSettings;

StatusBarEventEditDlg::StatusBarEventEditDlg(StatusBarEvents eventId, QWidget* parent/* = nullptr*/)
: QDialog(parent)
, m_eventId(eventId)
{
    ui.setupUi(this);

    m_SBVarMenu = new QMenu(this);

    auto eventMap = UtilUI::eventToSettingMap();
    if (eventMap.contains(m_eventId))
    {
        const StatusBarEventInfo& eventInfo = eventMap[m_eventId];
        QString paramKey = eventInfo.settingKey;
        QString defaultValue = UtilUI::getDefaultValue(paramKey);
        QString currentMessage = ttSettings->value(paramKey, defaultValue).toString();

        setWindowTitle(eventInfo.eventName.size() > 0 ? tr("Edit Message for Event \"%1\"").arg(eventInfo.eventName) : tr("Edit Status Bar Message"));
        ui.SBMsgLabel->setText(eventInfo.eventName.size() > 0 ? tr("Message for Event \"%1\"").arg(eventInfo.eventName) : tr("Message"));
        ui.SBMsgEdit->setText(currentMessage);

        for (auto it = eventInfo.variables.constBegin(); it != eventInfo.variables.constEnd(); ++it)
        {
            QAction* action = m_SBVarMenu->addAction(it.value());
            action->setData(it.key());
            connect(action, &QAction::triggered, this, &StatusBarEventEditDlg::insertVariable);
        }
    }

    connect(ui.SBVarButton, &QPushButton::clicked, this, [this]()
    {
        m_SBVarMenu->exec(QCursor::pos());
    });
    connect(ui.SBDefValButton, &QPushButton::clicked, this, &StatusBarEventEditDlg::slotRestoreDefault);
}

void StatusBarEventEditDlg::insertVariable()
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

void StatusBarEventEditDlg::slotRestoreDefault()
{
    auto eventMap = UtilUI::eventToSettingMap();
    if (eventMap.contains(m_eventId))
    {
        const StatusBarEventInfo& eventInfo = eventMap[m_eventId];
        QString defaultValue = UtilUI::getDefaultValue(eventInfo.settingKey);
        ui.SBMsgEdit->setText(defaultValue);
    }
}
