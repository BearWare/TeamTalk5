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

#ifndef SERVERLISTDLG_H
#define SERVERLISTDLG_H

#include "ui_serverlist.h"
#include "common.h"

#include <QAbstractTableModel>
#include <QNetworkAccessManager>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <QVector>
#include <memory>

enum ServerType
{
    SERVERTYPE_LOCAL    = 1 << 0,
    SERVERTYPE_OFFICIAL = 1 << 1,
    SERVERTYPE_PUBLIC   = 1 << 2,
    SERVERTYPE_UNOFFICIAL  = 1 << 3,

    SERVERTYPE_MIN      = SERVERTYPE_LOCAL,
    SERVERTYPE_MAX      = SERVERTYPE_UNOFFICIAL,
};

typedef quint32 ServerTypes;

struct HostEntryEx : HostEntry
{
    // public server settings
    int usercount = 0;
    QString country;
    QString motd;
    QString servername;
    int id = 0;
    ServerType srvtype = SERVERTYPE_LOCAL;
};

class ServerListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ServerListModel(QObject* parent);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex & index) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    void addServer(const HostEntryEx& host, ServerType srvtype);
    void clearServers();
    const QVector<HostEntryEx>& getServers() const;
    void setServerFilter(ServerTypes srvtypes, const QRegularExpression& regex, int n_users);
    ServerType getServerType(const HostEntryEx& host) const;
private:
    void filterServers();
    QMap<ServerType, QVector<HostEntryEx>> m_servers;
    // servers available after filter is applied
    QVector<HostEntryEx> m_servercache;
    // server filter variables
    ServerTypes m_srvtypes = ~0;
    QRegularExpression m_name_regex;
    int m_nusers = 0;
};

class ServerListDlg : public QDialog
{
    Q_OBJECT
public:
    ServerListDlg(QWidget * parent = 0);
    ~ServerListDlg();
    HostEntry getHostEntry() const;

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    Ui::ServerListDlg ui;
    ServerListModel* m_model;
    int m_nextid = 0;
    QSortFilterProxyModel* m_proxyModel;

    QNetworkAccessManager* m_httpsrvlist_manager = nullptr, *m_http_srvpublish_manager = nullptr;
    std::unique_ptr<HostEncryption> m_setup_encryption;
    HostEntry m_hostentry;

    void restoreSelectedHost(const HostEntry& entry);
    void showLatestHosts();
    void deleteLatestHostEntry();
    void clearLatestHosts();
    void slotNewServer();
    void slotImportTTFile();
    void slotConnect();
    void connectToHost(const HostEntry& = HostEntry());

    void refreshServerList();
    void applyServerListFilter();
    void deleteSelectedServer();
    void editSelectedServer();
    void duplicateSelectedServer();
    void requestServerList();
    void serverlistReply(QNetworkReply* reply);

    void saveTTFile();
    void exportSingleFile();
    void exportMultipleFiles();
    void showExportMenu(); // Nouvelle méthode
    void publishServer();
    void publishServerRequest(QNetworkReply* reply);
    bool getSelectedHost(HostEntryEx& host);

    void slotTreeContextMenu(const QPoint&);
    void slotLatestHostsContextMenu(const QPoint&);
};

#endif
