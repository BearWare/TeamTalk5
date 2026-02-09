#include "serverlistmodel.h"

ServerListModel::ServerListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ServerListModel::rowCount(const QModelIndex &) const {
    return m_servers.size();
}

QVariant ServerListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole)
        return {};

    return m_servers.at(index.row()).label;
}

void ServerListModel::addServer(const ServerEntry &entry) {
    beginInsertRows(QModelIndex(), m_servers.size(), m_servers.size());
    m_servers.append(entry);
    endInsertRows();
}
