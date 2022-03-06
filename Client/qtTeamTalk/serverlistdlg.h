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

#ifndef SERVERLISTDLG_H
#define SERVERLISTDLG_H

#include "ui_serverlist.h"
#include "common.h"
#include <QVector>
#include <QNetworkAccessManager>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

enum ServerType
{
    SERVERTYPE_LOCAL    = 1 << 0,
    SERVERTYPE_OFFICIAL = 1 << 1,
    SERVERTYPE_PUBLIC   = 1 << 2,
    SERVERTYPE_PRIVATE  = 1 << 3,

    SERVERTYPE_MIN      = SERVERTYPE_LOCAL,
    SERVERTYPE_MAX      = SERVERTYPE_PRIVATE,
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

class ServerListModel : public QAbstractItemModel
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
    void setServerTypes(ServerTypes srvtypes);
    const QVector<HostEntryEx>& getServers() const;
private:
    QMap<ServerType, QVector<HostEntryEx>> m_servers;
    QVector<HostEntryEx> m_servercache;
    ServerTypes m_srvtypes = ~0;
    ServerType getServerType(const HostEntryEx& host) const;
};

class ServerListDlg : public QDialog
{
    Q_OBJECT
public:
    ServerListDlg(QWidget * parent = 0);
    ~ServerListDlg();

private:
    Ui::ServerListDlg ui;
    ServerListModel* m_model;
    int m_nextid = 0;
    QSortFilterProxyModel* m_proxyModel;

    QNetworkAccessManager* m_httpsrvlist_manager = nullptr, *m_http_srvpublish_manager = nullptr;

    void showHostEntry(const HostEntry& entry);
    bool getHostEntry(HostEntry& entry);
    void clearHostEntry();
    void showLatestHosts();
    void showLatestHostEntry(int index);
    void deleteHostEntry();
    void slotClearServerClicked();
    void slotImportTTFile();
    void slotConnect();

    void refreshServerList();
    void showSelectedServer(const QModelIndex &index);
    void slotAddUpdServer();
    void deleteSelectedServer();
    void slotDoubleClicked(const QModelIndex& index);
    void requestServerList();
    void serverlistReply(QNetworkReply* reply);

    void saveTTFile();
    void publishServer();
    void publishServerRequest(QNetworkReply* reply);

    void hostEntryNameChanged(const QString& text);
    void slotGenerateEntryName(const QString&);
    void slotTreeContextMenu(const QPoint&);
};

#endif
