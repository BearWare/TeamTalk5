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
#include <QKeyEvent>

#include "chattemplatesdlg.h"
#include "appinfo.h"
#include "settings.h"

extern NonDefaultSettings* ttSettings;

ChatTemplatesDlg::ChatTemplatesDlg(QWidget* parent)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_CHATTEMPLATESDLG_SIZE).toByteArray());

    m_chattemplatesmodel = new ChatTemplatesModel(this);
    ui.chatTemplatesTableView->setModel(m_chattemplatesmodel);

    ui.chatTemplatesTableView->horizontalHeader()->restoreState(ttSettings->value(SETTINGS_DISPLAY_CHATTEMPLATES_MODEL_HEADER).toByteArray());
    ui.chatTemplatesTableView->horizontalHeader()->setSectionsMovable(false);

    connect(ui.chatTemplatesTableView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &ChatTemplatesDlg::chatTemplateSelected);
    m_CTVarMenu = new QMenu(this);
    connect(ui.CTVarButton, &QPushButton::clicked, this, [this]()
    {
        m_CTVarMenu->exec(QCursor::pos());
    });
    connect(ui.CTDefValButton, &QPushButton::clicked, this, &ChatTemplatesDlg::chatTemplatesRestoreDefaultTemplate);
    connect(ui.CTDefAllValButton, &QPushButton::clicked, this, &ChatTemplatesDlg::chatTemplatesRestoreAllDefaultTemplate);
    connect(this, &QDialog::accepted, this, &ChatTemplatesDlg::slotAccept);
}

void ChatTemplatesDlg::chatTemplateSelected(const QModelIndex &index)
{
    saveCurrentTemplate();

    m_currentIndex = index;
    if (!index.isValid()) return;

    auto templatesMap = UtilUI::templatesToSettingMap();
    ChatTemplates templateId = static_cast<ChatTemplates>(index.internalId());

    if (templatesMap.contains(templateId))
    {
        const ChatTemplateInfo& templateInfo = templatesMap[templateId];
        QString paramKey = templateInfo.settingKey;
        QString defaultValue = UtilUI::getDefaultTemplate(paramKey);
        QString currentMessage = ttSettings->value(paramKey, defaultValue).toString();
        ui.CTMsgLabel->setText(templateInfo.templateName.size() > 0?tr("Template for \"%1\"").arg(templateInfo.templateName):tr("Template"));
        ui.CTMsgEdit->setText(currentMessage);

        m_CTVarMenu->clear();
        for (auto it = templateInfo.variables.constBegin(); it != templateInfo.variables.constEnd(); ++it)
        {
            QAction* action = m_CTVarMenu->addAction(it.value());
            action->setData(it.key());
            connect(action, &QAction::triggered, this, &ChatTemplatesDlg::insertVariable);
        }
    }
}

void ChatTemplatesDlg::insertVariable()
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

void ChatTemplatesDlg::saveCurrentTemplate()
{
    if (!m_currentIndex.isValid()) return;

    auto templatesMap = UtilUI::templatesToSettingMap();
    ChatTemplates templateId = static_cast<ChatTemplates>(m_currentIndex.internalId());

    if (templatesMap.contains(templateId))
    {
        const ChatTemplateInfo& templateInfo = templatesMap[templateId];
        QString paramKey = templateInfo.settingKey;
        QString text = ui.CTMsgEdit->text();

        if (!text.isEmpty() && text != ttSettings->value(paramKey))
        {
            ttSettings->setValue(paramKey, text);
        }
    }
}

void ChatTemplatesDlg::chatTemplatesRestoreDefaultTemplate()
{
    if (!m_currentIndex.isValid()) return;

    auto templatesMap = UtilUI::templatesToSettingMap();
    ChatTemplates templateId = static_cast<ChatTemplates>(m_currentIndex.internalId());

    if (templatesMap.contains(templateId))
    {
        const ChatTemplateInfo& templateInfo = templatesMap[templateId];
        QString defaultValue = UtilUI::getDefaultTemplate(templateInfo.settingKey);
        ui.CTMsgEdit->setText(defaultValue);
    }
}

void ChatTemplatesDlg::chatTemplatesRestoreAllDefaultTemplate()
{
    QMessageBox answer;
    answer.setText(tr("Are you sure you want to restore all chat templates to default values?"));
    QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
    QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
    Q_UNUSED(YesButton);
    answer.setIcon(QMessageBox::Information);
    answer.setWindowTitle(tr("Restore default values"));
    answer.exec();
    if(answer.clickedButton() == NoButton)
        return;
    auto templatesMap = UtilUI::templatesToSettingMap();
    for (ChatTemplates tpl = CHATTEMPLATES_CHANNEL_MESSAGE; tpl < CHATTEMPLATES_NEXT_UNUSED; tpl <<= 1)
    {
        ChatTemplates templateId = static_cast<ChatTemplates>(tpl);
        if (templatesMap.contains(templateId))
        {
            const ChatTemplateInfo& templateInfo = templatesMap[templateId];
            QString defaultValue = UtilUI::getDefaultTemplate(templateInfo.settingKey);
            ttSettings->setValue(templateInfo.settingKey, defaultValue);
            if (m_currentIndex.isValid() && m_currentIndex.internalId() == templateId)
                ui.CTMsgEdit->setText(defaultValue);
        }
    }
}

void ChatTemplatesDlg::slotAccept()
{
    ttSettings->setValue(SETTINGS_DISPLAY_CHATTEMPLATES_MODEL_HEADER, ui.chatTemplatesTableView->horizontalHeader()->saveState());
    ttSettings->setValue(SETTINGS_DISPLAY_CHATTEMPLATESDLG_SIZE, saveGeometry());
    saveCurrentTemplate();
}

void ChatTemplatesDlg::keyPressEvent ( QKeyEvent * event )
{
    if (ui.CTMsgEdit->hasFocus() && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
        (event->modifiers() & Qt::ShiftModifier) != 0)
    {
        ui.CTMsgEdit->insert("\n");
    }
    QDialog::keyPressEvent( event );
}


enum
{
    COLUMN_TYPE = 0,
    COLUMN_TEMPLATE = 1,
    COLUMN_COUNT,
};

ChatTemplatesModel::ChatTemplatesModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    m_chattemplates.push_back(CHATTEMPLATES_CHANNEL_MESSAGE);
    m_chattemplates.push_back(CHATTEMPLATES_BROADCAST_MESSAGE);
    m_chattemplates.push_back(CHATTEMPLATES_PRIVATE_MESSAGE);
    m_chattemplates.push_back(CHATTEMPLATES_LOG_MESSAGE);
    m_chattemplates.push_back(CHATTEMPLATES_SERVER_NAME);
    m_chattemplates.push_back(CHATTEMPLATES_SERVER_MOTD);
    m_chattemplates.push_back(CHATTEMPLATES_JOINED_CHAN);
    m_chattemplates.push_back(CHATTEMPLATES_CHANNEL_TOPIC);
    m_chattemplates.push_back(CHATTEMPLATES_CHANNEL_QUOTA);
}

QVariant ChatTemplatesModel::headerData ( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case COLUMN_TYPE: return tr("Type");
            case COLUMN_TEMPLATE : return tr("Template");
            }
        }
        break;
    case Qt::TextAlignmentRole :
        return Qt::AlignLeft;
    }
    return QVariant();
}

int ChatTemplatesModel::columnCount ( const QModelIndex & /*parent*/ /*= QModelIndex() */) const
{
    return COLUMN_COUNT;
}

QVariant ChatTemplatesModel::data ( const QModelIndex & index, int role /*= Qt::DisplayRole*/ ) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        switch(index.column())
        {
        case COLUMN_TYPE :
            switch(m_chattemplates[index.row()])
            {
            case CHATTEMPLATES_CHANNEL_MESSAGE :
                return tr("Channel Message");
            case CHATTEMPLATES_BROADCAST_MESSAGE :
                return tr("Broadcast Message");
            case CHATTEMPLATES_PRIVATE_MESSAGE :
                return tr("Private Message");
            case CHATTEMPLATES_LOG_MESSAGE :
                return tr("Log Message");
            case CHATTEMPLATES_SERVER_NAME :
                return tr("Server Name");
            case CHATTEMPLATES_SERVER_MOTD :
                return tr("Server Message of the day");
            case CHATTEMPLATES_JOINED_CHAN :
                return tr("Joined Channel");
            case CHATTEMPLATES_CHANNEL_TOPIC :
                return tr("Channel topic");
            case CHATTEMPLATES_CHANNEL_QUOTA :
                return tr("Channel Disk Quota");
            case CHATTEMPLATES_NONE :
            case CHATTEMPLATES_NEXT_UNUSED :
                break;
            }
        case COLUMN_TEMPLATE :
        {
            auto templatesMap = UtilUI::templatesToSettingMap();
            if (templatesMap.contains(m_chattemplates[index.row()]))
            {
                QString paramKey = templatesMap[m_chattemplates[index.row()]].settingKey;
                return UtilUI::getRawChatTemplate(paramKey);
            }
            return QVariant();
        }
        }
        break;
    case Qt::AccessibleTextRole :
        switch (index.column())
        {
        case COLUMN_TYPE :
            QString result = data(index, Qt::DisplayRole).toString();
            QString tpl = data(createIndex(index.row(), COLUMN_TEMPLATE), Qt::DisplayRole).toString();
            if (tpl.size() > 0)
                result += " - " + tpl;
            return result;
        }
        break;
    }
    return QVariant();
}

Qt::ItemFlags ChatTemplatesModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable |  Qt::ItemIsEditable;
}

QModelIndex ChatTemplatesModel::index ( int row, int column, const QModelIndex & /*parent*/ /*= QModelIndex()*/ ) const
{
    return createIndex(row, column, m_chattemplates[row]);
}

QModelIndex ChatTemplatesModel::parent ( const QModelIndex & /*index*/ ) const
{
    return QModelIndex();
}

int ChatTemplatesModel::rowCount ( const QModelIndex & /*parent*/ /*= QModelIndex()*/ ) const
{
    return int(m_chattemplates.size());
}
