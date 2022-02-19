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
#include "bearwarelogindlg.h"
#include "utilui.h"

#include <QUrl>
#include <QMessageBox>
#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileDialog>

extern QSettings* ttSettings;

enum
{
    COLUMN_INDEX_SERVERNAME,
    COLUMN_INDEX_USERCOUNT,
    COLUMN_INDEX_COUNTRY,
    COLUMN_COUNT,
};

ServerListModel::ServerListModel(QObject* parent) : QAbstractItemModel(parent)
{
}

QVariant ServerListModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
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

int ServerListModel::columnCount(const QModelIndex & parent /*= QModelIndex()*/) const
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
        break;
    case Qt::ToolTipRole :
        return getServers()[index.row()].motd;
    case Qt::BackgroundRole :
        switch (getServerType(getServers()[index.row()]))
        {
        case SERVERTYPE_LOCAL :
            return QVariant();
        case SERVERTYPE_PUBLIC :
            return QColor(0x0C,0x52,0x28);
        }
        break;
    case Qt::UserRole :
        switch (getServerType(getServers()[index.row()]))
        {
        case SERVERTYPE_LOCAL :
            return "A" + data(index, Qt::DisplayRole).toString();
        case SERVERTYPE_PUBLIC :
            return "B" + data(index, Qt::DisplayRole).toString();
        }
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

void ServerListModel::addServer(const HostEntry& host, ServerType srvtype)
{
    m_servers[srvtype].append(host);
    setServerTypes(m_srvtypes);
}

void ServerListModel::clearServers()
{
    m_servers.clear();
    setServerTypes(m_srvtypes);
}

const QVector<HostEntry>& ServerListModel::getServers() const
{
    return m_servercache;
}

void ServerListModel::setServerTypes(ServerTypes srvtypes)
{
    this->beginResetModel();
    m_srvtypes = srvtypes;
    m_servercache.clear();

    ServerTypes srvtype = 0x1;
    for (; srvtype <= SERVERTYPE_MAX; srvtype <<= 1)
    {
        if (m_srvtypes & srvtype)
            m_servercache.append(m_servers[ServerType(srvtype)]);
    }
    this->endResetModel();
}

ServerType ServerListModel::getServerType(const HostEntry& host) const
{
    ServerTypes srvtype = SERVERTYPE_MIN;
    for (; srvtype <= SERVERTYPE_MAX; srvtype <<= 1)
    {
        Q_ASSERT(srvtype == SERVERTYPE_LOCAL || srvtype == SERVERTYPE_PUBLIC);
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
    , m_http_manager(nullptr)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    m_model = new ServerListModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setSortRole(Qt::UserRole);
    m_proxyModel->setSourceModel(m_model);
    ui.serverTreeView->setModel(m_proxyModel);

    ui.usernameBox->addItem(WEBLOGIN_BEARWARE_USERNAME);

    connect(ui.addupdButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotAddUpdServer);
    connect(ui.delButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotDeleteServer);
    connect(ui.serverTreeView, &QAbstractItemView::activated,
            this, &ServerListDlg::slotShowSelectedServer);
    connect(ui.serverTreeView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &ServerListDlg::slotShowSelectedServer);
    connect(ui.connectButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotConnect);
    connect(ui.clearButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotClearServerClicked);
    connect(ui.serverTreeView, &QAbstractItemView::doubleClicked,
            this, &ServerListDlg::slotDoubleClicked);
    connect(ui.freeserverChkBox, &QAbstractButton::clicked,
            this, &ServerListDlg::slotFreeServers);
    connect(ui.genttButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotGenerateFile);
    connect(ui.impttButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotLoadTTFile);
    connect(ui.hostaddrBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ServerListDlg::showLatestHostEntry);
    connect(ui.nameEdit, &QLineEdit::textChanged,
            this, &ServerListDlg::slotSaveEntryChanged);

    connect(ui.hostaddrBox, &QComboBox::editTextChanged,
            this, &ServerListDlg::slotGenerateEntryName);
    connect(ui.deleteBtn, &QAbstractButton::clicked,
            this, &ServerListDlg::slotDeleteLatestHost);
    connect(ui.tcpportEdit, &QLineEdit::textChanged,
            this, &ServerListDlg::slotGenerateEntryName);
    connect(ui.usernameBox, &QComboBox::editTextChanged,
            this, &ServerListDlg::slotGenerateEntryName);

    clearHostEntry();

    showLatestHosts();

    if(ttSettings->value(SETTINGS_DISPLAY_FREESERVERS, true).toBool())
        ui.freeserverChkBox->setChecked(true);

    ui.delButton->setEnabled(false);
    slotRefreshServers();
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

void ServerListDlg::showLatestHosts()
{
    ui.hostaddrBox->clear();

    HostEntry host;
    int index = 0;
    while (getLatestHost(index++, host))
        ui.hostaddrBox->addItem(host.ipaddr);
    showLatestHostEntry(0);
}

void ServerListDlg::slotClearServerClicked()
{
    clearHostEntry();
    ui.hostaddrBox->setFocus();
}

void ServerListDlg::showLatestHostEntry(int index)
{
    HostEntry host;
    if(getLatestHost(index, host))
    {
        showHostEntry(host);
        ui.delButton->setEnabled(false);
    }
}

void ServerListDlg::slotConnect()
{
    HostEntry entry;
    if(getHostEntry(entry))
    {
        if (isWebLogin(entry.username, true))
        {
            QString username = ttSettings->value(SETTINGS_GENERAL_BEARWARE_USERNAME).toString();
            if (username.isEmpty())
            {
                BearWareLoginDlg dlg(this);
                if (dlg.exec())
                {
                    username = ttSettings->value(SETTINGS_GENERAL_BEARWARE_USERNAME).toString();
                }
            }
            ui.usernameBox->lineEdit()->setText(username);
            ui.passwordEdit->setText("");
        }

        addLatestHost(entry);
        this->accept();
    }
}

void ServerListDlg::slotRefreshServers()
{
    m_model->clearServers();

    int index = 0;
    HostEntry entry;
    while (getServerEntry(index++, entry))
    {
        m_model->addServer(entry, SERVERTYPE_LOCAL);
        entry = HostEntry();
    }

    if (ui.freeserverChkBox->isChecked())
        slotFreeServers(true);
}

void ServerListDlg::slotShowSelectedServer(const QModelIndex &index)
{
    clearHostEntry();
    auto servers = m_model->getServers();
    auto srcIndex = m_proxyModel->mapToSource(index);
    if (srcIndex.isValid() && srcIndex.row() < servers.size())
    {
        showHostEntry(servers[srcIndex.row()]);
        ui.delButton->setEnabled(true);
    }
    else
    {
        ui.clearButton->setEnabled(false);
        ui.delButton->setEnabled(false);
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
        slotRefreshServers();
        ui.serverTreeView->setFocus();
    }
}

void ServerListDlg::slotDeleteServer()
{
    auto servers = m_model->getServers();
    auto srcIndex = m_proxyModel->mapToSource(ui.serverTreeView->currentIndex());
    if (srcIndex.isValid() && srcIndex.row() < servers.size())
    {
        RestoreIndex ri(ui.serverTreeView);

        deleteServerEntry(servers[srcIndex.row()].name);
        clearHostEntry();
        slotRefreshServers();
        ui.serverTreeView->setFocus();
    }
    ui.delButton->setEnabled(ui.serverTreeView->currentIndex().isValid());
}

void ServerListDlg::slotDoubleClicked(const QModelIndex& /*index*/)
{
    slotConnect();
}

void ServerListDlg::slotFreeServers(bool checked)
{
    ttSettings->setValue(SETTINGS_DISPLAY_FREESERVERS, checked);

    if(!checked)
    {
        slotRefreshServers();
        return;
    }
    if(!m_http_manager)
        m_http_manager = new QNetworkAccessManager(this);

    QUrl url(URL_FREESERVER);
    connect(m_http_manager, &QNetworkAccessManager::finished,
            this, &ServerListDlg::slotFreeServerRequest);

    QNetworkRequest request(url);
    m_http_manager->get(request);
    //QByteArray path = QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/");
    //if (path.isEmpty())
    //    path = "/";
}

void ServerListDlg::slotFreeServerRequest(QNetworkReply* reply)
{
    RestoreIndex ri(ui.serverTreeView);

    Q_ASSERT(m_http_manager);
    QByteArray data = reply->readAll();

    QDomDocument doc("foo");
    if(!doc.setContent(data))
        return;

    QDomElement rootElement(doc.documentElement());
    QDomElement element = rootElement.firstChildElement();
    while(!element.isNull())
    {
        HostEntry entry;
        if(getServerEntry(element, entry))
            m_model->addServer(entry, SERVERTYPE_PUBLIC);
		element = element.nextSiblingElement();
    }
}

void ServerListDlg::slotGenerateFile()
{
    HostEntry entry;
    if(!getHostEntry(entry))
        return;

    GenerateTTFileDlg dlg(entry, this);
    dlg.exec();
}

void ServerListDlg::slotLoadTTFile()
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
    slotRefreshServers();
}

void ServerListDlg::slotDeleteLatestHost()
{
    int i = ui.hostaddrBox->currentIndex();
    deleteLatestHost(i);
    showLatestHosts();
}

void ServerListDlg::slotSaveEntryChanged(const QString& text)
{
    ui.addupdButton->setEnabled(text.size());
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
