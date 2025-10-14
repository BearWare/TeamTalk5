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

#include "desktopaccessdlg.h"
#include "appinfo.h"

#include <QMessageBox>

DesktopAccessDlg::DesktopAccessDlg(QWidget* parent)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    connect(ui.delBtn, &QAbstractButton::clicked,
            this, &DesktopAccessDlg::slotDelEntry);
    connect(ui.clearBtn, &QAbstractButton::clicked,
            this, &DesktopAccessDlg::slotClearEntry);
    connect(ui.addBtn, &QAbstractButton::clicked,
            this, &DesktopAccessDlg::slotAddEntry);

    connect(ui.addchanBtn, &QAbstractButton::clicked,
            this, &DesktopAccessDlg::slotAddChannel);
    connect(ui.delchanBtn, &QAbstractButton::clicked,
            this, &DesktopAccessDlg::slotDelChannel);
    connect(ui.adduserBtn, &QAbstractButton::clicked,
            this, &DesktopAccessDlg::slotAddUsername);
    connect(ui.deluserBtn, &QAbstractButton::clicked,
            this, &DesktopAccessDlg::slotDelUsername);

    connect(ui.listView, &QAbstractItemView::clicked,
            this, &DesktopAccessDlg::slotServerSelected);

    m_model = new DesktopAccessModel(this);

    QVector<DesktopAccessEntry> entries;
    getDesktopAccessList(entries);
    ui.listView->setModel(m_model);
    m_model->setEntries(entries);

    HostEntry host;
    if(getServerEntry(0, host, true))
    {
        int i;
        for(i=0;i<m_model->getEntries().size();i++)
        {
            if(m_model->getEntries()[i].ipaddr == host.ipaddr &&
               m_model->getEntries()[i].tcpport == host.tcpport)
            {
                QModelIndex index = m_model->index(i, 0);
                ui.listView->setCurrentIndex(index);
                slotServerSelected(index);
                break;
            }
        }
        if(i>=m_model->getEntries().size())
        {
            ui.hostaddrEdit->setText(host.ipaddr);
            ui.tcpportSpinBox->setValue(host.tcpport);
        }
    }
}

void DesktopAccessDlg::accept()
{
    deleteDesktopAccessEntries();
    for(int i=0;i<m_model->getEntries().size();i++)
        addDesktopAccessEntry(m_model->getEntries()[i]);

    QDialog::accept();
}

void DesktopAccessDlg::slotDelEntry()
{
    QModelIndex ii = ui.listView->currentIndex();
    if(ii.isValid())
        m_model->getEntries().remove(ii.row());
    m_model->updateEntries();
    if(ii.row() < m_model->getEntries().size())
        ui.listView->setCurrentIndex(ii);
}

void DesktopAccessDlg::slotClearEntry()
{
    ui.hostaddrEdit->clear();
    ui.tcpportSpinBox->setValue(DEFAULT_TCPPORT);
    ui.channelsComboBox->clear();
    ui.channelsComboBox->setEditText(QString());
    ui.usernamesComboBox->clear();
    ui.usernamesComboBox->setEditText(QString());
}

void DesktopAccessDlg::slotAddEntry()
{
    if(ui.hostaddrEdit->text().isEmpty())
    {
        QMessageBox::information(this, tr("Missing fields"),
                                 tr("Please fill the field 'Host IP-address'"));
        return;
    }

    DesktopAccessEntry entry;
    entry.ipaddr = ui.hostaddrEdit->text();
    entry.tcpport = ui.tcpportSpinBox->value();
    int n_items = ui.channelsComboBox->count();
    for(int i=0;i<n_items;i++)
        entry.channels.push_back(ui.channelsComboBox->itemText(i));
    n_items = ui.usernamesComboBox->count();
    for(int i=0;i<n_items;i++)
        entry.usernames.push_back(ui.usernamesComboBox->itemText(i));

    QVector<DesktopAccessEntry>& entries = m_model->getEntries();
    for(int i=0;i<entries.size();)
    {
        if(entries[i].ipaddr == entry.ipaddr && entries[i].tcpport == entry.tcpport)
            entries.remove(i);
        else i++;
    }
    m_model->getEntries().push_back(entry);
    m_model->updateEntries();
}

void DesktopAccessDlg::slotAddChannel()
{
    QString text = ui.channelsComboBox->currentText();
    if(!text.isEmpty())
        ui.channelsComboBox->addItem(text);
    ui.channelsComboBox->setEditText(QString());
}

void DesktopAccessDlg::slotDelChannel()
{
    int index = ui.channelsComboBox->findText(ui.channelsComboBox->currentText());
    if(index >= 0)
        ui.channelsComboBox->removeItem(index);
}

void DesktopAccessDlg::slotAddUsername()
{
    QString text = ui.usernamesComboBox->currentText();
    if(!text.isEmpty())
        ui.usernamesComboBox->addItem(text);
    ui.usernamesComboBox->setEditText(QString());
}

void DesktopAccessDlg::slotDelUsername()
{
    int index = ui.usernamesComboBox->findText(ui.usernamesComboBox->currentText());
    if(index >= 0)
        ui.usernamesComboBox->removeItem(index);
}

void DesktopAccessDlg::slotServerSelected(const QModelIndex& index)
{
    slotClearEntry();

    const DesktopAccessEntry& entry = m_model->getEntries()[index.row()];
    ui.hostaddrEdit->setText(entry.ipaddr);
    ui.tcpportSpinBox->setValue(entry.tcpport);
    ui.channelsComboBox->addItems(entry.channels);
    ui.usernamesComboBox->addItems(entry.usernames);
}

QVariant DesktopAccessModel::data(const QModelIndex & index, int role/* = Qt::DisplayRole*/) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        return QString("%1:%2").arg(m_entries[index.row()].ipaddr).arg(m_entries[index.row()].tcpport);
    }
    return QVariant();
}

QModelIndex DesktopAccessModel::index(int row, int column, const QModelIndex & parent/* = QModelIndex()*/) const
{
    if(!parent.isValid() && row<m_entries.size())
        return createIndex(row, column);
    return QModelIndex();
}

void DesktopAccessModel::setEntries(QVector<DesktopAccessEntry>& entries)
{
    m_entries = entries;
    updateEntries();
}

void DesktopAccessModel::updateEntries()
{
    this->beginResetModel();
    this->endResetModel();
}
