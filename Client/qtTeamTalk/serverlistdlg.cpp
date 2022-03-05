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

#include "serverlistdlg.h"
#include "common.h"
#include "appinfo.h"
#include "settings.h"
#include "generatettfiledlg.h"
#include "utilui.h"

#include <QUrl>
#include <QMessageBox>
#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileDialog>
#include <QMenu>

extern QSettings* ttSettings;

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
            entry.srvtype = SERVERTYPE_PRIVATE;
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

ServerListModel::ServerListModel(QObject* parent) : QAbstractItemModel(parent)
{
}

QVariant ServerListModel::headerData(int section, Qt::Orientation /*orientation*/, int role /*= Qt::DisplayRole*/) const
{
    switch(role)
    {
    case Qt::DisplayRole :
    {
        switch(section)
        {
            case COLUMN_INDEX_SERVERNAME: return tr("Name");
            case COLUMN_INDEX_USERCOUNT: return tr("Users");
            case COLUMN_INDEX_COUNTRY: return tr("Country");
        }
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
        QString srvtype;
        auto srv = getServers()[index.row()];
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
        case SERVERTYPE_PRIVATE:
            srvtype = tr("Private server");
            break;
        }
        return QString(tr("%1, Name: %2, Users: %3, Country: %4, MOTD: %5").arg(srvtype).arg(srv.name).arg(srv.usercount).arg(srv.country).arg(srv.motd));
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
        case SERVERTYPE_PRIVATE :
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
            case SERVERTYPE_PRIVATE :
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
    setServerTypes(m_srvtypes);
}

void ServerListModel::clearServers()
{
    m_servers.clear();
    setServerTypes(m_srvtypes);
}

const QVector<HostEntryEx>& ServerListModel::getServers() const
{
    return m_servercache;
}

void ServerListModel::setServerTypes(ServerTypes srvtypes)
{
    this->beginResetModel();
    m_srvtypes = srvtypes;
    m_servercache.clear();

    ServerTypes srvtype = SERVERTYPE_MIN;
    for (; srvtype <= SERVERTYPE_MAX; srvtype <<= 1)
    {
        if (m_srvtypes & srvtype)
            m_servercache.append(m_servers[ServerType(srvtype)]);
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

    m_model = new ServerListModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    ui.serverTreeView->setModel(m_proxyModel);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setSortRole(Qt::UserRole);
    m_proxyModel->sort(COLUMN_INDEX_SERVERNAME, Qt::AscendingOrder);

    ui.usernameBox->addItem(WEBLOGIN_BEARWARE_USERNAME);

    connect(ui.addupdButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotAddUpdServer);
    connect(ui.serverTreeView, &QAbstractItemView::activated,
            this, &ServerListDlg::showSelectedServer);
    connect(ui.serverTreeView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &ServerListDlg::showSelectedServer);
    connect(ui.connectButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotConnect);
    connect(ui.clearButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotClearServerClicked);
    connect(ui.serverTreeView, &QAbstractItemView::doubleClicked,
            this, &ServerListDlg::slotDoubleClicked);
    connect(ui.publicserverChkBox, &QAbstractButton::clicked,
            this, &ServerListDlg::refreshServerList);
    connect(ui.privateserverChkBox, &QAbstractButton::clicked,
            this, &ServerListDlg::refreshServerList);
    connect(ui.genttButton, &QAbstractButton::clicked,
            this, &ServerListDlg::saveTTFile);
    connect(ui.impttButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotImportTTFile);
    connect(ui.hostaddrBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ServerListDlg::showLatestHostEntry);
    connect(ui.nameEdit, &QLineEdit::textChanged,
            this, &ServerListDlg::hostEntryNameChanged);

    connect(ui.hostaddrBox, &QComboBox::editTextChanged,
            this, &ServerListDlg::slotGenerateEntryName);
    connect(ui.deleteBtn, &QAbstractButton::clicked,
            this, &ServerListDlg::deleteHostEntry);
    connect(ui.tcpportEdit, &QLineEdit::textChanged,
            this, &ServerListDlg::slotGenerateEntryName);
    connect(ui.usernameBox, &QComboBox::editTextChanged,
            this, &ServerListDlg::slotGenerateEntryName);
    connect(ui.publishButton, &QAbstractButton::clicked,
            this, &ServerListDlg::publishServer);
    ui.serverTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.serverTreeView, &QWidget::customContextMenuRequested,
            this, &ServerListDlg::slotTreeContextMenu);


    clearHostEntry();

    showLatestHosts();

    ui.publicserverChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_PUBLICSERVERS, SETTINGS_DISPLAY_PUBLICSERVERS_DEFAULT).toBool());
    ui.privateserverChkBox->setChecked(ttSettings->value(SETTINGS_DISPLAY_PRIVATESERVERS, SETTINGS_DISPLAY_PRIVATESERVERS_DEFAULT).toBool());

    refreshServerList();
    HostEntry lasthost;
    if (getLatestHost(0, lasthost))
    {
        ui.hostaddrBox->setFocus();
        auto servers = m_model->getServers();
        for (int i=0;i<servers.size();++i)
        {
            if (servers[i].sameHost(lasthost, false))
            {
                auto srcIndex = m_proxyModel->mapFromSource(m_model->index(i, 0));
                ui.serverTreeView->setCurrentIndex(srcIndex);
                ui.serverTreeView->setFocus();
            }
        }
    }

    ui.serverTreeView->header()->restoreState(ttSettings->value(SETTINGS_DISPLAY_SERVERLIST_HEADERSIZES).toByteArray());
    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_SERVERLISTDLG_SIZE).toByteArray());
}

ServerListDlg::~ServerListDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_SERVERLISTDLG_SIZE, saveGeometry());
    ttSettings->setValue(SETTINGS_DISPLAY_SERVERLIST_HEADERSIZES, ui.serverTreeView->header()->saveState());
}

void ServerListDlg::showHostEntry(const HostEntry& entry)
{
    ui.nameEdit->setText(entry.name);
    ui.hostaddrBox->lineEdit()->setText(entry.ipaddr);
    ui.tcpportEdit->setText(QString::number(entry.tcpport));
    ui.udpportEdit->setText(QString::number(entry.udpport));
    ui.cryptChkBox->setChecked(entry.encrypted);
    ui.usernameBox->lineEdit()->setText(entry.username);
    if (isWebLogin(entry.username, true))
        ui.passwordEdit->setText("");
    else
        ui.passwordEdit->setText(entry.password);
    ui.passwordEdit->setDisabled(isWebLogin(entry.username, true));
    ui.nicknameEdit->setText(entry.nickname);
    ui.channelEdit->setText(entry.channel);
    ui.chanpasswdEdit->setText(entry.chanpasswd);

    ui.clearButton->setEnabled(true);
}

bool ServerListDlg::getHostEntry(HostEntry& entry)
{
    if(ui.hostaddrBox->lineEdit()->text().isEmpty() ||
       ui.tcpportEdit->text().isEmpty() ||
       ui.udpportEdit->text().isEmpty())
    {
        QMessageBox::information(this, tr("Missing fields"),
            tr("Please fill the fields 'Host IP-address', 'TCP port' and 'UDP port'"));
            return false;
    }

    entry.name = ui.nameEdit->text();
    entry.ipaddr = ui.hostaddrBox->lineEdit()->text().trimmed();
    entry.tcpport = ui.tcpportEdit->text().toInt();
    entry.udpport = ui.udpportEdit->text().toInt();
    entry.encrypted = ui.cryptChkBox->isChecked();
    entry.username = ui.usernameBox->lineEdit()->text().trimmed();
    entry.password = ui.passwordEdit->text();
    entry.nickname = ui.nicknameEdit->text();
    entry.channel = ui.channelEdit->text().trimmed();
    entry.chanpasswd = ui.chanpasswdEdit->text();

    return true;
}

void ServerListDlg::clearHostEntry()
{
    showHostEntry(HostEntry());
    ui.clearButton->setEnabled(false);
}

void ServerListDlg::slotClearServerClicked()
{
    clearHostEntry();
    ui.hostaddrBox->setFocus();
}

void ServerListDlg::showLatestHosts()
{
    ui.hostaddrBox->clear();

    HostEntry host;
    int index = 0;
    while (getLatestHost(index++, host))
        ui.hostaddrBox->addItem(host.ipaddr);
    showLatestHostEntry(0);
}

void ServerListDlg::showLatestHostEntry(int index)
{
    HostEntry host;
    if(getLatestHost(index, host))
    {
        showHostEntry(host);
    }
}

void ServerListDlg::deleteHostEntry()
{
    int i = ui.hostaddrBox->currentIndex();
    deleteLatestHost(i);
    showLatestHosts();
}

void ServerListDlg::slotImportTTFile()
{
    QString start_dir = ttSettings->value(SETTINGS_LAST_DIRECTORY, QDir::homePath()).toString();
    QString filepath = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                   start_dir/*, tr("TT Files (*.tt)")*/);
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
    QDomDocument doc(TTFILE_ROOT);
    if(!doc.setContent(data))
    {
        QMessageBox::information(this, tr("Load File"),
            tr("Failed to load file %1").arg(filepath));
        return;
    }

    QDomElement rootElement(doc.documentElement());
    QString version = rootElement.attribute("version");

    if(!versionSameOrLater(version, TTFILE_VERSION))
    {
        QMessageBox::information(this, tr("Load File"),
            tr("The file \"%1\" is incompatible with %2")
            .arg(QDir::toNativeSeparators(filepath))
            .arg(APPTITLE));
        return;
    }

    QDomElement element = rootElement.firstChildElement("host");
    HostEntry entry;
    if(!getServerEntry(element, entry))
    {
        QMessageBox::information(this, tr("Load File"),
            tr("Failed to extract host-information from %1").arg(filepath));
        return;
    }

    addServerEntry(entry);
    refreshServerList();
}

void ServerListDlg::slotConnect()
{
    HostEntry entry;
    if(getHostEntry(entry))
    {
        if (isWebLogin(entry.username, true))
        {
            ui.usernameBox->lineEdit()->setText(getBearWareWebLogin(this));
            ui.passwordEdit->setText("");
        }

        addLatestHost(entry);
        this->accept();
    }
}

void ServerListDlg::refreshServerList()
{
    m_model->clearServers();

    m_nextid = 0;
    int index = 0;
    HostEntryEx entry;
    while (getServerEntry(index++, entry))
    {
        entry.id = ++m_nextid;
        m_model->addServer(entry, SERVERTYPE_LOCAL);
        entry = HostEntryEx();
    }

    requestServerList();
}

void ServerListDlg::showSelectedServer(const QModelIndex &index)
{
    clearHostEntry();
    auto servers = m_model->getServers();
    auto srcIndex = m_proxyModel->mapToSource(index);
    if (srcIndex.isValid() && srcIndex.row() < servers.size())
    {
        showHostEntry(servers[srcIndex.row()]);
    }
    else
    {
        ui.clearButton->setEnabled(false);
    }
}

void ServerListDlg::slotAddUpdServer()
{
    HostEntry entry;
    if(getHostEntry(entry))
    {
        RestoreIndex g(ui.serverTreeView);
        deleteServerEntry(entry.name);
        addServerEntry(entry);
        refreshServerList();
        ui.serverTreeView->setFocus();
    }
}

void ServerListDlg::deleteSelectedServer()
{
    auto servers = m_model->getServers();
    auto srcIndex = m_proxyModel->mapToSource(ui.serverTreeView->currentIndex());
    if (srcIndex.isValid() && srcIndex.row() < servers.size())
    {
        RestoreIndex ri(ui.serverTreeView);

        deleteServerEntry(servers[srcIndex.row()].name);
        clearHostEntry();
        refreshServerList();
        ui.serverTreeView->setFocus();
    }
}

void ServerListDlg::slotDoubleClicked(const QModelIndex& /*index*/)
{
    slotConnect();
}

void ServerListDlg::requestServerList()
{
    bool publicservers = ui.publicserverChkBox->isChecked();
    bool privateservers = ui.privateserverChkBox->isChecked();
    ttSettings->setValue(SETTINGS_DISPLAY_PUBLICSERVERS, publicservers);
    ttSettings->setValue(SETTINGS_DISPLAY_PRIVATESERVERS, privateservers);

    if (!privateservers && !publicservers)
        return;

    if (!m_httpsrvlist_manager)
        m_httpsrvlist_manager = new QNetworkAccessManager(this);

    QUrl url(URL_FREESERVER(privateservers, publicservers));
    connect(m_httpsrvlist_manager, &QNetworkAccessManager::finished,
            this, &ServerListDlg::serverlistReply);

    QNetworkRequest request(url);
    m_httpsrvlist_manager->get(request);
}

void ServerListDlg::serverlistReply(QNetworkReply* reply)
{
    RestoreIndex ri(ui.serverTreeView);

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
    if(!getHostEntry(entry))
        return;

    GenerateTTFileDlg dlg(entry, this);
    dlg.exec();
}

void ServerListDlg::publishServer()
{
    HostEntry entry;
    if (!getHostEntry(entry) || entry.name.isEmpty())
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

void ServerListDlg::publishServerRequest(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        QMessageBox::critical(this, tr("Publish Server"),
            tr("Failed to publish server."));
    }
    else
    {
        QMessageBox::information(this, tr("Publish Server"),
            tr("Change your server's name to include the text #teamtalkpublish#.\n"
                "This will verify that you're the owner of the server.\n"
                "Once this is done your private server will appear in a couple of minutes.\n\n"
                "Delete the published user account to unregister your server.\n\n"
                "The #teamtalkpublish# notification can be removed once\n"
                "the server has been verified."
            ));
    }
}

void ServerListDlg::hostEntryNameChanged(const QString& text)
{
    ui.addupdButton->setEnabled(text.size());
    ui.publishButton->setEnabled(text.size());
}

void ServerListDlg::slotGenerateEntryName(const QString&)
{
    QString username = ui.usernameBox->lineEdit()->text();
    if(username.size())
        ui.nameEdit->setText(QString("%1@%2:%3")
                             .arg(username)
                             .arg(ui.hostaddrBox->lineEdit()->text())
                             .arg(ui.tcpportEdit->text()));
    else if(ui.hostaddrBox->lineEdit()->text().size())
        ui.nameEdit->setText(QString("%1:%2")
                             .arg(ui.hostaddrBox->lineEdit()->text())
                             .arg(ui.tcpportEdit->text()));
    else
        ui.nameEdit->setText(QString());

    ui.passwordEdit->setDisabled(username == WEBLOGIN_BEARWARE_USERNAME);
    if (isWebLogin(username, true))
        ui.passwordEdit->setText("");
}

void ServerListDlg::slotTreeContextMenu(const QPoint& /*point*/)
{
    QMenu menu(this);
    QMenu* sortMenu = menu.addMenu(tr("Sort By..."));
    QAction* sortDefault = sortMenu->addAction(tr("De&fault"));
    QAction* sortName = sortMenu->addAction(tr("&Name"));
    QAction* sortUserCount = sortMenu->addAction(tr("&User Count"));
    QAction* sortCountry = sortMenu->addAction(tr("Country"));
    QAction* delServ = menu.addAction(tr("&Delete Selected Server"));
    auto srcIndex = m_proxyModel->mapToSource(ui.serverTreeView->currentIndex());
    if (srcIndex.isValid())
        delServ->setEnabled(m_model->getServers()[srcIndex.row()].srvtype == SERVERTYPE_LOCAL);
    if (QAction* action = menu.exec(QCursor::pos()))
    {

        auto sortToggle = m_proxyModel->sortOrder() == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        if (action == sortDefault)
        {
            m_proxyModel->setSortRole(Qt::UserRole);
            m_proxyModel->sort(COLUMN_INDEX_SERVERNAME, m_proxyModel->sortColumn() == COLUMN_INDEX_SERVERNAME ? sortToggle : Qt::AscendingOrder);
        }
        else if (action == sortName)
        {
            m_proxyModel->setSortRole(Qt::DisplayRole);
            m_proxyModel->sort(COLUMN_INDEX_SERVERNAME, m_proxyModel->sortColumn() == COLUMN_INDEX_SERVERNAME ? sortToggle : Qt::AscendingOrder);
        }
        else if (action == sortUserCount)
        {
            m_proxyModel->sort(COLUMN_INDEX_USERCOUNT, m_proxyModel->sortColumn() == COLUMN_INDEX_USERCOUNT ? sortToggle : Qt::AscendingOrder);
        }
        else if (action == sortCountry)
        {
            m_proxyModel->sort(COLUMN_INDEX_COUNTRY, m_proxyModel->sortColumn() == COLUMN_INDEX_COUNTRY ? sortToggle : Qt::AscendingOrder);
        }
        else if (action == delServ)
            emit(deleteSelectedServer());
        ttSettings->setValue(SETTINGS_DISPLAY_SERVERLIST_HEADERSIZES, ui.serverTreeView->header()->saveState());
    }
}
