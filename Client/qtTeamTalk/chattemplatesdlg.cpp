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
#include "chattemplateeditdlg.h"
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

    connect(ui.chatTemplatesTableView, &QAbstractItemView::doubleClicked, this, &ChatTemplatesDlg::slotEditTemplate);
    ui.chatTemplatesTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.chatTemplatesTableView, &QWidget::customContextMenuRequested,
            this, &ChatTemplatesDlg::slotTableContextMenu);

    connect(ui.CTDefAllValButton, &QPushButton::clicked, this, &ChatTemplatesDlg::chatTemplatesRestoreAllDefaultTemplate);
    connect(this, &QDialog::accepted, this, &ChatTemplatesDlg::slotAccept);
}

void ChatTemplatesDlg::slotEditTemplate()
{
    QModelIndex index = ui.chatTemplatesTableView->currentIndex();
    if (!index.isValid()) return;

    ChatTemplates templateId = static_cast<ChatTemplates>(index.internalId());
    auto templatesMap = UtilUI::templatesToSettingMap();
    if (!templatesMap.contains(templateId)) return;

    ChatTemplateEditDlg dlg(templateId, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        QString paramKey = templatesMap[templateId].settingKey;
        QString text = dlg.getMessage();

        if (!text.isEmpty() && text != ttSettings->value(paramKey))
        {
            ttSettings->setValueOrClear(paramKey, text, UtilUI::getDefaultTemplate(paramKey));
        }
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
            ttSettings->remove(templatesMap[templateId].settingKey);
        }
    }
}

void ChatTemplatesDlg::slotAccept()
{
    ttSettings->setValue(SETTINGS_DISPLAY_CHATTEMPLATES_MODEL_HEADER, ui.chatTemplatesTableView->horizontalHeader()->saveState());
    ttSettings->setValue(SETTINGS_DISPLAY_CHATTEMPLATESDLG_SIZE, saveGeometry());
}

void ChatTemplatesDlg::keyPressEvent ( QKeyEvent * event )
{
    if (ui.chatTemplatesTableView->hasFocus() && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter))
        slotEditTemplate();
    else
        QDialog::keyPressEvent( event );
}

void ChatTemplatesDlg::slotTableContextMenu(const QPoint& /*point*/)
{
    QMenu menu(this);
    QAction* editTemplate = menu.addAction(tr("&Edit Template"));

    QModelIndex index = ui.chatTemplatesTableView->currentIndex();
    editTemplate->setEnabled(index.isValid());

    if (QAction* action = menu.exec(QCursor::pos()))
    {
        if (action == editTemplate)
            slotEditTemplate();
    }
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
