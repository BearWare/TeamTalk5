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

#include "serverlistdlg.h"
#include "common.h"
#include "appinfo.h"
#include "settings.h"
#include "generatettfiledlg.h"
#include "utilui.h"
#include "utilxml.h"
#include "serverdlg.h"

#include <QUrl>
#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileDialog>
#include <QMenu>
#include <QDebug>

extern NonDefaultSettings* ttSettings;

enum
{
    COLUMN_INDEX_SERVERNAME,
    COLUMN_INDEX_USERCOUNT,
    COLUMN_INDEX_COUNTRY,
    COLUMN_COUNT,
};

void processHostEntry(const QDomElement& hostElement, HostEntryEx& entry)
{
    QDomElement tmp = hostElement.firstChildElement("listing");
    if (!tmp.isNull())
    {
        if (tmp.text() == "official")
            entry.srvtype = SERVERTYPE_OFFICIAL;
        if (tmp.text() == "public")
            entry.srvtype = SERVERTYPE_PUBLIC;
        if (tmp.text() == "private")
            entry.srvtype = SERVERTYPE_UNOFFICIAL;
    }
}

void processStatsXML(const QDomElement& hostElement, HostEntryEx& entry)
{
    QDomElement stats = hostElement.firstChildElement("stats");
    if (!stats.isNull())
    {
        QDomElement tmp = stats.firstChildElement("user-count");
        if (!tmp.isNull())
            entry.usercount = tmp.text().toInt();
        tmp = stats.firstChildElement("country");
        if (!tmp.isNull())
            entry.country = tmp.text();
        tmp = stats.firstChildElement("motd");
        if (!tmp.isNull())
            entry.motd = tmp.text();
        tmp = stats.firstChildElement("servername");
        if (!tmp.isNull())
            entry.servername = tmp.text();
    }
}

ServerListModel::ServerListModel(QObject* parent) : QAbstractTableModel(parent)
{
}

QVariant ServerListModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if(orientation == Qt::Horizontal)
        switch(section)
        {
            case COLUMN_INDEX_SERVERNAME: return tr("Name");
            case COLUMN_INDEX_USERCOUNT: return tr("Users");
            case COLUMN_INDEX_COUNTRY: return tr("Country");
        }
    }
    return QVariant();
}

int ServerListModel::columnCount(const QModelIndex & /*parent = QModelIndex()*/) const
{
    return COLUMN_COUNT;
}

QVariant ServerListModel::data(const QModelIndex & index, int role /*= Qt::DisplayRole*/) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        switch (index.column())
        {
        case COLUMN_INDEX_SERVERNAME :
            return getServers()[index.row()].name;
        case COLUMN_INDEX_USERCOUNT :
            return getServers()[index.row()].usercount;
        case COLUMN_INDEX_COUNTRY :
            return getServers()[index.row()].country;
        }
        break;
    case Qt::AccessibleTextRole :
    {
        auto srv = getServers()[index.row()];
        if (index.column() == COLUMN_INDEX_SERVERNAME)
        {
            QString srvtype;
            switch (getServerType(srv))
            {
            case SERVERTYPE_LOCAL:
                return tr("Local server, Name: %1").arg(srv.name);
            case SERVERTYPE_OFFICIAL:
                srvtype = tr("Official server");
                break;
            case SERVERTYPE_PUBLIC:
                srvtype = tr("Public server");
                break;
            case SERVERTYPE_UNOFFICIAL:
                srvtype = tr("Unofficial server");
                break;
            }
            return QString(tr("%1, Name: %2, Users: %3, Country: %4, MOTD: %5").arg(srvtype).arg(srv.name).arg(data(createIndex(index.row(), COLUMN_INDEX_USERCOUNT, index.internalId()), Qt::DisplayRole).toString()).arg(data(createIndex(index.row(), COLUMN_INDEX_COUNTRY, index.internalId()), Qt::DisplayRole).toString()).arg(srv.motd));
        }
        if (index.column() == COLUMN_INDEX_USERCOUNT && getServerType(srv) != SERVERTYPE_LOCAL)
            return data(createIndex(index.row(), COLUMN_INDEX_USERCOUNT, index.internalId()), Qt::DisplayRole).toString();
        if (index.column() == COLUMN_INDEX_COUNTRY && getServerType(srv) != SERVERTYPE_LOCAL)
            return data(createIndex(index.row(), COLUMN_INDEX_COUNTRY, index.internalId()), Qt::DisplayRole).toString();
    }
    case Qt::ToolTipRole :
        return getServers()[index.row()].motd;
    case Qt::BackgroundRole :
        switch (getServerType(getServers()[index.row()]))
        {
        case SERVERTYPE_LOCAL :
            return QVariant();
        case SERVERTYPE_OFFICIAL :
            return QColor(0x0,0x4A,0x7F);
        case SERVERTYPE_PUBLIC :
            return QColor(0x0C,0x52,0x28);
        case SERVERTYPE_UNOFFICIAL :
            return QColor(0xFF,0x61,0xC);
        }
        break;
    case Qt::UserRole :
        if (index.column() == COLUMN_INDEX_SERVERNAME)
        {
            QString name = data(index, Qt::DisplayRole).toString();
            int category, id;
            switch (getServerType(getServers()[index.row()]))
            {
            case SERVERTYPE_LOCAL :
                category = getServerType(getServers()[index.row()]);
                id = 0; // sort by name
                break;
            case SERVERTYPE_OFFICIAL :
            case SERVERTYPE_PUBLIC :
            case SERVERTYPE_UNOFFICIAL :
                category = getServerType(getServers()[index.row()]);
                id = getServers()[index.row()].id; // sort by id (order from www-server)
                break;
            }
            name = QString("%1-%2-%3").arg(category, 9, 16, QLatin1Char('0')).arg(id, 9, 10, QLatin1Char('0')).arg(name);
            return name;
        }
        return data(index, Qt::DisplayRole);
    }
    return QVariant();
}

QModelIndex ServerListModel::index(int row, int column, const QModelIndex & parent /*= QModelIndex()*/) const
{
    if (!parent.isValid() && row < getServers().size())
        return createIndex(row, column);
    return QModelIndex();
}

QModelIndex ServerListModel::parent(const QModelIndex&/* index*/) const
{
    return QModelIndex();
}

int ServerListModel::rowCount(const QModelIndex& /*parent = QModelIndex()*/) const
{
    return getServers().size();
}

void ServerListModel::addServer(const HostEntryEx& host, ServerType srvtype)
{
    m_servers[srvtype].append(host);
    filterServers();
}

void ServerListModel::clearServers()
{
    m_servers.clear();
    filterServers();
}

const QVector<HostEntryEx>& ServerListModel::getServers() const
{
    return m_servercache;
}

void ServerListModel::setServerFilter(ServerTypes srvtypes, const QRegularExpression& regex, int n_users)
{
    m_srvtypes = srvtypes;
    m_name_regex = regex;
    m_nusers = n_users;
    filterServers();
}

void ServerListModel::filterServers()
{
    this->beginResetModel();
    m_servercache.clear();

    ServerTypes srvtype = SERVERTYPE_MIN;
    for (; srvtype <= SERVERTYPE_MAX; srvtype <<= 1)
    {
        if ((m_srvtypes & srvtype))
        {
            auto hosts = m_servers[ServerType(srvtype)];
            hosts.erase(std::remove_if(hosts.begin(), hosts.end(), [&](const HostEntryEx& entry) { return entry.usercount < m_nusers && (entry.srvtype & SERVERTYPE_LOCAL) != SERVERTYPE_LOCAL; }), hosts.end());
            hosts.erase(std::remove_if(hosts.begin(), hosts.end(), [&](const HostEntryEx& entry) { return !m_name_regex.match(entry.name).hasMatch(); }), hosts.end());
            m_servercache.append(hosts);
        }
    }
    this->endResetModel();
}

ServerType ServerListModel::getServerType(const HostEntryEx& host) const
{
    ServerTypes srvtype = SERVERTYPE_MIN;
    for (; srvtype <= SERVERTYPE_MAX; srvtype <<= 1)
    {
        const auto i = m_servers.find(ServerType(srvtype));
        if (i != m_servers.end() && std::find_if((*i).begin(), (*i).end(),
                                                 [host](const HostEntry& h) { return h.sameHostEntry(host); }) != (*i).end())
        {
            return ServerType(srvtype);
        }
    }
    return SERVERTYPE_MAX;
}

ServerListDlg::ServerListDlg(QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    this->setAccessibleDescription(tr("Host manager"));

    m_model = new ServerListModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    ui.serverTableView->setModel(m_proxyModel);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setSortRole(Qt::UserRole);
    m_proxyModel->sort(COLUMN_INDEX_SERVERNAME, Qt::AscendingOrder);

    ui.filterusersSpinBox->setValue(ttSettings->value(SETTINGS_DISPLAY_SERVERLISTFILTER_USERSCOUNT, SETTINGS_DISPLAY_SERVERLISTFILTER_USERSCOUNT_DEFAULT).toInt());
    ui.filternameEdit->setText(ttSettings->value(SETTINGS_DISPLAY_SERVERLISTFILTER_NAME, SETTINGS_DISPLAY_SERVERLISTFILTER_NAME_DEFAULT).toString());
    ui.officialserverChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_OFFICIALSERVERS, SETTINGS_DISPLAY_OFFICIALSERVERS_DEFAULT).toBool());
    ui.unofficialserverChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_UNOFFICIALSERVERS, SETTINGS_DISPLAY_UNOFFICIALSERVERS_DEFAULT).toBool());

    connect(ui.newsrvButton, &QPushButton::clicked, this, &ServerListDlg::slotNewServer);
    connect(ui.impttButton, &QPushButton::clicked, this, &ServerListDlg::slotImportTTFile);
    connect(ui.expttButton, &QPushButton::clicked, this, &ServerListDlg::showExportMenu);

    connect(ui.officialserverChkBox, &QCheckBox::clicked, this, &ServerListDlg::refreshServerList);
    connect(ui.unofficialserverChkBox, &QCheckBox::clicked, this, &ServerListDlg::refreshServerList);

    connect(ui.filternameEdit, &QLineEdit::textChanged, this, &ServerListDlg::applyServerListFilter);
    connect(ui.filterusersSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ServerListDlg::applyServerListFilter);
    connect(ui.serverTableView, &QAbstractItemView::doubleClicked, this, &ServerListDlg::slotConnect);
    ui.serverTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.serverTableView, &QWidget::customContextMenuRequested, this, &ServerListDlg::slotTreeContextMenu);

    connect(ui.hostListWidget, &QAbstractItemView::doubleClicked, this, &ServerListDlg::slotConnect);
    ui.hostListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.hostListWidget, &QWidget::customContextMenuRequested, this, &ServerListDlg::slotLatestHostsContextMenu);

    showLatestHosts();
    refreshServerList();
    HostEntry lasthost;
    if (getServerEntry(0, lasthost, true))
    {
        restoreSelectedHost(lasthost);
    }

    ui.serverTableView->horizontalHeader()->restoreState(ttSettings->value(SETTINGS_DISPLAY_SERVERLIST_HEADERSIZES).toByteArray());
    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_SERVERLISTDLG_SIZE).toByteArray());
    ui.serverTableView->horizontalHeader()->setSectionsMovable(false);
}

ServerListDlg::~ServerListDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_SERVERLISTDLG_SIZE, saveGeometry());
    ttSettings->setValue(SETTINGS_DISPLAY_SERVERLIST_HEADERSIZES, ui.serverTableView->horizontalHeader()->saveState());
}

void ServerListDlg::restoreSelectedHost(const HostEntry& entry)
{
    ui.hostListWidget->setFocus();
    auto servers = m_model->getServers();
    for (int i=0;i<servers.size();++i)
    {
        if (servers[i].sameHost(entry, false) || (servers[i].channel != entry.channel || servers[i].chanpasswd != entry.chanpasswd))
        {
            auto srcIndex = m_proxyModel->mapFromSource(m_model->index(i, 0));
            ui.serverTableView->setCurrentIndex(srcIndex);
            ui.serverTableView->setFocus();
        }
    }
}

void ServerListDlg::showLatestHosts()
{
    ui.hostListWidget->clear();

    HostEntry host;
    int index = 0;
    while (getServerEntry(index++, host, true))
        ui.hostListWidget->addItem(host.name);
}

void ServerListDlg::deleteLatestHostEntry()
{
    HostEntry host;
    int i = ui.hostListWidget->currentRow();
    if (getServerEntry(i, host, true))
        deleteServerEntry(host);
    showLatestHosts();
}

void ServerListDlg::clearLatestHosts()
{
    for (int i=ui.hostListWidget->count()-1; i >= 0; --i)
    {
        HostEntry host;
        if (getServerEntry(i, host, true))
            deleteServerEntry(host);
    }
    showLatestHosts();
}

void ServerListDlg::slotNewServer()
{
    HostEntry entry;
    ServerDlg dlg(ServerDlg::SERVER_CREATE, entry, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        entry = dlg.GetHostEntry();
        addServerEntry(entry);
        if (dlg.connectToServer())
            connectToHost(entry);
        else
        {
            refreshServerList();
            restoreSelectedHost(entry);
            ui.serverTableView->setFocus();
        }
    }
    else
    {
        entry = dlg.GetHostEntry();
        if (dlg.connectToServer())
            connectToHost(entry);
    }
}

void ServerListDlg::slotImportTTFile()
{
    QString start_dir = ttSettings->value(SETTINGS_LAST_DIRECTORY, QDir::homePath()).toString();
    QString filepath = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    start_dir, tr("TT Files (*.tt)"));
    if(filepath.isEmpty())
        return;

    QFile ttfile(QDir::fromNativeSeparators(filepath));
    if(!ttfile.open(QFile::ReadOnly))
    {
        QMessageBox::information(this, tr("Load File"),
            tr("Failed to load file %1").arg(filepath));
        return;
    }

    QByteArray data = ttfile.readAll();
    QDomDocument doc;
    if(!doc.setContent(data))
    {
        QMessageBox::information(this, tr("Load File"),
            tr("Failed to load file %1").arg(filepath));
        return;
    }

    QDomElement rootElement = doc.documentElement();
    if (rootElement.tagName() == TTFILE_ROOT)
    {
        QString version = rootElement.attribute("version");
        if(!versionSameOrLater(version, TTFILE_VERSION))
        {
            QMessageBox::information(this, tr("Load File"),
                tr("The file \"%1\" is incompatible with %2")
                .arg(QDir::toNativeSeparators(filepath))
                .arg(APPTITLE));
            return;
        }

        QDomElement hostElement = rootElement.firstChildElement("host");
        while (!hostElement.isNull())
        {
            HostEntry entry;
            if (getServerEntry(hostElement, entry))
            {
                addServerEntry(entry);
            }
            hostElement = hostElement.nextSiblingElement("host");
        }
    }
    else
    {
        QMessageBox::information(this, tr("Load File"),
            tr("Failed to load file %1").arg(filepath));
        return;
    }

    refreshServerList();
}

void ServerListDlg::slotConnect()
{
    HostEntryEx host, latestHost;
    if (getServerEntry(0, latestHost, true) || ui.hostListWidget->count() == 0)
    {
        if (!getSelectedHost(host))
            return;
        if (!host.sameHost(latestHost, true))
            addLatestHost(host);
        m_hostentry = host;
        this->accept();
    }
}

void ServerListDlg::connectToHost(const HostEntry& host/* = HostEntry()*/)
{
    if (!host.ipaddr.isEmpty())
    {
        HostEntry latestHost;
        if (getServerEntry(0, latestHost, true) || ui.hostListWidget->count() == 0)
        {
            if (!host.sameHost(latestHost, true))
                addLatestHost(host);
        }
        m_hostentry = host;
        this->accept();
    }
    else
        slotConnect();
}

void ServerListDlg::refreshServerList()
{
    m_model->clearServers();
    applyServerListFilter();

    m_nextid = 0;
    int index = 0;
    HostEntryEx entry;
    while (getServerEntry(index++, entry, false))
    {
        entry.id = ++m_nextid;
        m_model->addServer(entry, SERVERTYPE_LOCAL);
        entry = HostEntryEx();
    }

    requestServerList();
}

void ServerListDlg::applyServerListFilter()
{
    ServerTypes typefilter = SERVERTYPE_LOCAL;
    if (ui.officialserverChkBox->isChecked())
        typefilter |= SERVERTYPE_OFFICIAL;
    if (ui.unofficialserverChkBox->isChecked())
        typefilter |= SERVERTYPE_UNOFFICIAL;

    m_model->setServerFilter(typefilter, QRegularExpression(ui.filternameEdit->text(), QRegularExpression::CaseInsensitiveOption), ui.filterusersSpinBox->value());
    ttSettings->setValueOrClear(SETTINGS_DISPLAY_SERVERLISTFILTER_NAME, ui.filternameEdit->text(), SETTINGS_DISPLAY_SERVERLISTFILTER_NAME_DEFAULT);
    ttSettings->setValueOrClear(SETTINGS_DISPLAY_SERVERLISTFILTER_USERSCOUNT, ui.filterusersSpinBox->value(), SETTINGS_DISPLAY_SERVERLISTFILTER_USERSCOUNT_DEFAULT);
}

void ServerListDlg::deleteSelectedServer()
{
    auto servers = m_model->getServers();
    auto srcIndex = m_proxyModel->mapToSource(ui.serverTableView->currentIndex());
    if (srcIndex.isValid() && srcIndex.row() < servers.size())
    {
        QMessageBox answer;
        answer.setText(tr("Delete server named \"%1\"").arg(m_model->getServers()[srcIndex.row()].name));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        Q_UNUSED(NoButton);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle(tr("Delete Server"));
        answer.exec();
        if (answer.clickedButton() == YesButton)
        {
            RestoreIndex ri(ui.serverTableView);
            deleteServerEntry(servers[srcIndex.row()]);
            refreshServerList();
            ui.serverTableView->setFocus();
        }
    }
}

void ServerListDlg::editSelectedServer()
{
    HostEntryEx host;
    if (!getSelectedHost(host))
        return;
    ServerDlg dlg((host.srvtype == SERVERTYPE_LOCAL ? ServerDlg::SERVER_UPDATE : ServerDlg::SERVER_READONLY), host, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        HostEntry updatedHost = dlg.GetHostEntry();
        deleteServerEntry(host);
        addServerEntry(updatedHost);
        if (dlg.connectToServer())
            connectToHost(updatedHost);
        else
        {
            refreshServerList();
            restoreSelectedHost(updatedHost);
        }
    }
    else
    {
        HostEntry updatedHost = dlg.GetHostEntry();
        if (dlg.connectToServer())
            connectToHost(updatedHost);
    }
}

void ServerListDlg::duplicateSelectedServer()
{
    HostEntryEx host;
    if (!getSelectedHost(host))
        return;

    host.name = QInputDialog::getText(this, tr("Duplicate Server Entry"),
                          tr("Entry Name"),
                          QLineEdit::Normal, tr("%1 - COPY").arg(host.name));
    addServerEntry(host);
    refreshServerList();
    restoreSelectedHost(host);
}

void ServerListDlg::requestServerList()
{
    bool officialservers = ui.officialserverChkBox->isChecked();
    bool unofficialservers = ui.unofficialserverChkBox->isChecked();
    ttSettings->setValueOrClear(SETTINGS_DISPLAY_OFFICIALSERVERS, officialservers, SETTINGS_DISPLAY_OFFICIALSERVERS_DEFAULT);
    ttSettings->setValueOrClear(SETTINGS_DISPLAY_UNOFFICIALSERVERS, unofficialservers, SETTINGS_DISPLAY_UNOFFICIALSERVERS_DEFAULT);

    if (!officialservers && !unofficialservers)
        return;

    if (!m_httpsrvlist_manager)
        m_httpsrvlist_manager = new QNetworkAccessManager(this);

    QUrl url(URL_FREESERVER(officialservers, publicservers, unofficialservers));
    connect(m_httpsrvlist_manager, &QNetworkAccessManager::finished,
            this, &ServerListDlg::serverlistReply);

    QNetworkRequest request(url);
    m_httpsrvlist_manager->get(request);
}

void ServerListDlg::serverlistReply(QNetworkReply* reply)
{
    RestoreIndex ri(ui.serverTableView);

    Q_ASSERT(m_httpsrvlist_manager);
    QByteArray data = reply->readAll();

    QDomDocument doc("foo");
    if(!doc.setContent(data))
        return;

    QDomElement rootElement(doc.documentElement());
    QDomElement element = rootElement.firstChildElement();
    while(!element.isNull())
    {
        HostEntryEx entry;
        if (getServerEntry(element, entry))
        {
            processHostEntry(element, entry);
            processStatsXML(element, entry);
            entry.id = ++m_nextid;
            m_model->addServer(entry, entry.srvtype);
        }
        element = element.nextSiblingElement();
    }
}

void ServerListDlg::saveTTFile()
{
    HostEntry entry;
    auto servers = m_model->getServers();
    auto srcIndex = m_proxyModel->mapToSource(ui.serverTableView->currentIndex());
    if (srcIndex.isValid() && srcIndex.row() < servers.size())
    {
        entry = servers[srcIndex.row()];
        GenerateTTFileDlg dlg(entry, this);
        dlg.exec();
    }
    return;
}

void ServerListDlg::showExportMenu()
{
    QMenu menu(this);
    QAction *exportSingleAction = menu.addAction(tr("Export entire list in single file"));
    QAction *exportMultipleAction = menu.addAction(tr("Export one server per file"));

    connect(exportSingleAction, &QAction::triggered, this, &ServerListDlg::exportSingleFile);
    connect(exportMultipleAction, &QAction::triggered, this, &ServerListDlg::exportMultipleFiles);

    menu.exec(QCursor::pos());
}

void ServerListDlg::exportSingleFile()
{
    QVector<HostEntry> localServers;

    auto servers = m_model->getServers();
    for (const auto& server : servers)
    {
        if (m_model->getServerType(server) == SERVERTYPE_LOCAL)
        {
            localServers.append(server);
        }
    }

    if (localServers.isEmpty())
    {
        QMessageBox::information(this, tr("Export Server List"), tr("No server to export."));
        return;
    }

    QString start_dir = ttSettings->value(SETTINGS_LAST_DIRECTORY, QDir::homePath()).toString();
    QString defaultFileName = start_dir + QDir::separator() + APPNAME_SHORT + "Servers_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".tt";
    QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    defaultFileName, tr("TT Files (*.tt)"));

    if (filename.isEmpty())
        return;

    ttSettings->setValue(SETTINGS_LAST_DIRECTORY, QFileInfo(filename).absolutePath());

    QDomDocument doc;
    QDomProcessingInstruction xmlDecl = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlDecl);

    QDomElement root = doc.createElement(TTFILE_ROOT);
    root.setAttribute("version", TTFILE_VERSION);
    doc.appendChild(root);

    for (const auto& entry : localServers)
    {
        QByteArray xml = generateTTFile(entry);
        QDomDocument entryDoc;
        entryDoc.setContent(xml);
        QDomElement hostElement = entryDoc.documentElement().firstChildElement("host");
        root.appendChild(doc.importNode(hostElement, true));
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, tr("Save File"), tr("Unable to save file"));
        return;
    }
    file.write(doc.toByteArray());

    QMessageBox::information(this, tr("Export Server List"), tr("All servers have been exported successfully."));
}

void ServerListDlg::exportMultipleFiles()
{
    QVector<HostEntry> localServers;

    auto servers = m_model->getServers();
    for (const auto& server : servers)
    {
        if (m_model->getServerType(server) == SERVERTYPE_LOCAL)
        {
            localServers.append(server);
        }
    }

    if (localServers.isEmpty())
    {
        QMessageBox::information(this, tr("Export Server List"), tr("No server to export."));
        return;
    }

    QString start_dir = ttSettings->value(SETTINGS_LAST_DIRECTORY, QDir::homePath()).toString();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), start_dir);

    if (dir.isEmpty())
        return;

    ttSettings->setValue(SETTINGS_LAST_DIRECTORY, dir);

    for (const auto& entry : localServers)
    {
        GenerateTTFileDlg dlg(entry, this);
        dlg.exportTTFileToDirectory(dir);
    }

    QMessageBox::information(this, tr("Export Server List"), tr("All servers have been exported successfully."));
}

void ServerListDlg::publishServer()
{
    HostEntry entry;
    auto servers = m_model->getServers();
    auto srcIndex = m_proxyModel->mapToSource(ui.serverTableView->currentIndex());
    if (srcIndex.isValid() && srcIndex.row() < servers.size())
    {
        entry = servers[srcIndex.row()];
        if (entry.name.isEmpty())
            return;

        QMessageBox answer;
        answer.setText(tr("Are you sure you want to publish the server named \"%1\"").arg(entry.name));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        Q_UNUSED(NoButton);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle(tr("Publish Server"));
        answer.exec();
        if(answer.clickedButton() != YesButton)
            return;

        if (!m_http_srvpublish_manager)
            m_http_srvpublish_manager = new QNetworkAccessManager(this);

        connect(m_http_srvpublish_manager, &QNetworkAccessManager::finished,
                this, &ServerListDlg::publishServerRequest);

        QString username = getBearWareWebLogin(this);
        username = QUrl::toPercentEncoding(username);
        QString token = ttSettings->value(SETTINGS_GENERAL_BEARWARE_TOKEN, "").toString();
        token = QUrl::toPercentEncoding(token);
        QUrl url(URL_PUBLISHSERVER(username, token));
        QByteArray xml = generateTTFile(entry);

        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");
        m_http_srvpublish_manager->post(request, xml);
    }
}

void ServerListDlg::publishServerRequest(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        QMessageBox::critical(this, tr("Publish Server"),
            tr("Failed to publish server."));
    }
    else
    {
        QInputDialog::getText(this, tr("Publish Server Completed"),
                              tr("Update your server's properties so its server name includes the text #teamtalkpublish#.\n"
                              "This will verify that you're the owner of the server.\n"
                              "Once the server is verified your server will appear in a couple of minutes.\n\n"
                              "The #teamtalkpublish# notification can be removed once\n"
                              "the server has been verified.\n\n"
                              "Delete the published user account to unregister your server."),
                              QLineEdit::Normal, "#teamtalkpublish#");
    }
}

bool ServerListDlg::getSelectedHost(HostEntryEx& host)
{
    if (ui.hostListWidget->hasFocus())
    {
        int currentIndex = ui.hostListWidget->currentRow();
        if (currentIndex != -1)
        {
            getServerEntry(currentIndex, host, true);
        }
        return true;
    }
    else if (ui.serverTableView->hasFocus())
    {
        auto servers = m_model->getServers();
        auto srcIndex = m_proxyModel->mapToSource(ui.serverTableView->currentIndex());
        if (srcIndex.isValid() && srcIndex.row() < servers.size())
        {
            host = servers[srcIndex.row()];
        }
        return true;
    }
    return false;
}

HostEntry ServerListDlg::getHostEntry() const
{
    return m_hostentry;
}

void ServerListDlg::slotTreeContextMenu(const QPoint& /*point*/)
{
    QMenu menu(this);
    QMenu* sortMenu = menu.addMenu(tr("Sort By..."));
    QString asc = tr("Ascending"), desc = tr("Descending");
    QAction* sortDefault = new QAction(sortMenu);
    sortDefault->setText(tr("De&fault (%1)").arg(m_proxyModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortDefault->setCheckable(true);
    const QString defaultstr = "default";
    sortDefault->setChecked((ttSettings->value(SETTINGS_DISPLAY_SERVERLIST_SORT, SETTINGS_DISPLAY_SERVERLIST_SORT_DEFAULT).toString() == defaultstr)?true:false);
    sortMenu->addAction(sortDefault);
    QAction* sortName = new QAction(sortMenu);
    sortName->setText(tr("&Name (%1)").arg(m_proxyModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortName->setCheckable(true);
    const QString name = "name";
    sortName->setChecked((ttSettings->value(SETTINGS_DISPLAY_SERVERLIST_SORT, SETTINGS_DISPLAY_SERVERLIST_SORT_DEFAULT).toString() == name)?true:false);
    sortMenu->addAction(sortName);
    QAction* sortUserCount = new QAction(sortMenu);
    sortUserCount->setText(tr("&User Count (%1)").arg(m_proxyModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortUserCount->setCheckable(true);
    const QString usercount = "usercount";
    sortUserCount->setChecked((ttSettings->value(SETTINGS_DISPLAY_SERVERLIST_SORT, SETTINGS_DISPLAY_SERVERLIST_SORT_DEFAULT).toString() == usercount)?true:false);
    sortMenu->addAction(sortUserCount);
    QAction* sortCountry = new QAction(sortMenu);
    sortCountry->setText(tr("Country (%1)").arg(m_proxyModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortCountry->setCheckable(true);
    const QString country = "country";
    sortCountry->setChecked((ttSettings->value(SETTINGS_DISPLAY_SERVERLIST_SORT, SETTINGS_DISPLAY_SERVERLIST_SORT_DEFAULT).toString() == country)?true:false);
    sortMenu->addAction(sortCountry);
    QAction* connectServ = menu.addAction(tr("&Connect"));
    QAction* delServ = menu.addAction(tr("&Delete"));
    QAction* editServ = menu.addAction(tr("&Edit"));
    QAction* dupServ = menu.addAction(tr("D&uplicate"));
    QAction* genTTServ = menu.addAction(tr("&Generate .tt file"));
    QAction* publishServ = menu.addAction(tr("&Publish Publicly"));
    auto srcIndex = m_proxyModel->mapToSource(ui.serverTableView->currentIndex());
    connectServ->setEnabled(srcIndex.isValid());
    delServ->setEnabled(srcIndex.isValid() && m_model->getServers()[srcIndex.row()].srvtype == SERVERTYPE_LOCAL);
    editServ->setEnabled(srcIndex.isValid());
    dupServ->setEnabled(srcIndex.isValid());
    genTTServ->setEnabled(srcIndex.isValid());
    publishServ->setEnabled(srcIndex.isValid() && m_model->getServers()[srcIndex.row()].srvtype == SERVERTYPE_LOCAL);
    if (QAction* action = menu.exec(QCursor::pos()))
    {
        auto sortToggle = m_proxyModel->sortOrder() == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        if (action == sortDefault)
        {
            m_proxyModel->setSortRole(Qt::UserRole);
            ui.serverTableView->horizontalHeader()->setSortIndicator(COLUMN_INDEX_SERVERNAME, m_proxyModel->sortColumn() == COLUMN_INDEX_SERVERNAME ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValueOrClear(SETTINGS_DISPLAY_SERVERLIST_SORT, defaultstr, SETTINGS_DISPLAY_SERVERLIST_SORT_DEFAULT);
        }
        else if (action == sortName)
        {
            m_proxyModel->setSortRole(Qt::DisplayRole);
            ui.serverTableView->horizontalHeader()->setSortIndicator(COLUMN_INDEX_SERVERNAME, m_proxyModel->sortColumn() == COLUMN_INDEX_SERVERNAME ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValueOrClear(SETTINGS_DISPLAY_SERVERLIST_SORT, name, SETTINGS_DISPLAY_SERVERLIST_SORT_DEFAULT);
        }
        else if (action == sortUserCount)
        {
            m_proxyModel->setSortRole(Qt::DisplayRole);
            ui.serverTableView->horizontalHeader()->setSortIndicator(COLUMN_INDEX_USERCOUNT, m_proxyModel->sortColumn() == COLUMN_INDEX_USERCOUNT ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValueOrClear(SETTINGS_DISPLAY_SERVERLIST_SORT, usercount, SETTINGS_DISPLAY_SERVERLIST_SORT_DEFAULT);
        }
        else if (action == sortCountry)
        {
            m_proxyModel->setSortRole(Qt::DisplayRole);
            ui.serverTableView->horizontalHeader()->setSortIndicator(COLUMN_INDEX_COUNTRY, m_proxyModel->sortColumn() == COLUMN_INDEX_COUNTRY ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValueOrClear(SETTINGS_DISPLAY_SERVERLIST_SORT, country, SETTINGS_DISPLAY_SERVERLIST_SORT_DEFAULT);
        }
        else if (action == connectServ)
            connectToHost();
        else if (action == delServ)
            deleteSelectedServer();
        else if (action == editServ)
            editSelectedServer();
        else if (action == dupServ)
            duplicateSelectedServer();
        else if (action == genTTServ)
            saveTTFile();
        else if (action == publishServ)
            publishServer();
    }
}

void ServerListDlg::slotLatestHostsContextMenu(const QPoint& /*point*/)
{
    QMenu menu(this);
    QAction* connectHost = menu.addAction(tr("Co&nnect"));
    QAction* delHost = menu.addAction(tr("&Remove from Latest Hosts"));
    QAction* addHost = menu.addAction(tr("&Add to Saved Hosts"));
    QAction* clearList = menu.addAction(tr("&Clear Latest Hosts"));
    int i = ui.hostListWidget->currentRow();
    connectHost->setEnabled(i>=0);
    delHost->setEnabled(i>=0);
    addHost->setEnabled(i>=0);
    clearList->setEnabled(ui.hostListWidget->count() > 0);
    if (QAction* action = menu.exec(QCursor::pos()))
    {
        if (action == connectHost)
            connectToHost();
        else if (action == delHost)
            deleteLatestHostEntry();
        else if (action == addHost)
            editSelectedServer();
        else if (action == clearList)
            clearLatestHosts();
    }
}

void ServerListDlg::keyPressEvent(QKeyEvent* e)
{
    if (ui.serverTableView->hasFocus())
    {
        if (e->matches(QKeySequence::Delete) || e->key() == Qt::Key_Backspace)
            deleteSelectedServer();
        else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
            slotConnect();
        else
            QDialog::keyPressEvent(e);
    }
    else if (ui.hostListWidget->hasFocus())
    {
        if (e->matches(QKeySequence::Delete) || e->key() == Qt::Key_Backspace)
            deleteLatestHostEntry();
        else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
            slotConnect();
        else
            QDialog::keyPressEvent(e);
    }
    else
        QDialog::keyPressEvent(e);
}
