#pragma once
#include <QAbstractListModel>
#include "serverentry.h"

class ServerListModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit ServerListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void addServer(const ServerEntry &entry);

private:
    QList<ServerEntry> m_servers;
};
