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
#include <QKeyEvent>

#include "chattemplateeditdlg.h"
#include "settings.h"

extern NonDefaultSettings* ttSettings;

ChatTemplateEditDlg::ChatTemplateEditDlg(ChatTemplates templateId, QWidget* parent/* = nullptr*/)
: QDialog(parent)
, m_templateId(templateId)
{
    ui.setupUi(this);

    m_CTVarMenu = new QMenu(this);

    auto templatesMap = UtilUI::templatesToSettingMap();
    if (templatesMap.contains(m_templateId))
    {
        const ChatTemplateInfo& templateInfo = templatesMap[m_templateId];
        QString paramKey = templateInfo.settingKey;
        QString defaultValue = UtilUI::getDefaultTemplate(paramKey);
        QString currentMessage = ttSettings->value(paramKey, defaultValue).toString();

        setWindowTitle(templateInfo.templateName.size() > 0 ? tr("Edit Template for \"%1\"").arg(templateInfo.templateName) : tr("Edit Chat Template"));
        ui.CTMsgLabel->setText(templateInfo.templateName.size() > 0 ? tr("Template for \"%1\"").arg(templateInfo.templateName) : tr("Template"));
        ui.CTMsgEdit->setText(currentMessage);

        for (auto it = templateInfo.variables.constBegin(); it != templateInfo.variables.constEnd(); ++it)
        {
            QAction* action = m_CTVarMenu->addAction(it.value());
            action->setData(it.key());
            connect(action, &QAction::triggered, this, &ChatTemplateEditDlg::insertVariable);
        }
    }

    connect(ui.CTVarButton, &QPushButton::clicked, this, [this]()
    {
        m_CTVarMenu->exec(QCursor::pos());
    });
    connect(ui.CTDefValButton, &QPushButton::clicked, this, &ChatTemplateEditDlg::slotRestoreDefault);
}

void ChatTemplateEditDlg::insertVariable()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        QString variable = action->data().toString();
        int cursorPos = ui.CTMsgEdit->cursorPosition();
        ui.CTMsgEdit->insert(variable);
        ui.CTMsgEdit->setCursorPosition(cursorPos + variable.length());
    }
}

void ChatTemplateEditDlg::slotRestoreDefault()
{
    auto templatesMap = UtilUI::templatesToSettingMap();
    if (templatesMap.contains(m_templateId))
    {
        const ChatTemplateInfo& templateInfo = templatesMap[m_templateId];
        QString defaultValue = UtilUI::getDefaultTemplate(templateInfo.settingKey);
        ui.CTMsgEdit->setText(defaultValue);
    }
}

void ChatTemplateEditDlg::keyPressEvent(QKeyEvent* event)
{
    if (ui.CTMsgEdit->hasFocus() && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
        (event->modifiers() & Qt::ShiftModifier) != 0)
    {
        ui.CTMsgEdit->insert("\n");
    }
    QDialog::keyPressEvent(event);
}
