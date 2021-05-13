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

#include <QUrl>
#include <QMessageBox>
#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileDialog>

extern QSettings* ttSettings;

ServerListDlg::ServerListDlg(QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
    , m_http_manager(nullptr)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    ui.usernameBox->addItem(WEBLOGIN_BEARWARE_USERNAME);

    connect(ui.addupdButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotAddUpdServer);
    connect(ui.delButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotDeleteServer);
    connect(ui.listWidget, &QListWidget::currentRowChanged,
            this, &ServerListDlg::slotShowServer);
    connect(ui.connectButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotConnect);
    connect(ui.clearButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotClearServerClicked);
    connect(ui.listWidget, &QListWidget::itemDoubleClicked,
            this, &ServerListDlg::slotDoubleClicked);
    connect(ui.freeserverChkBox, &QAbstractButton::clicked,
            this, &ServerListDlg::slotFreeServers);
    connect(ui.genttButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotGenerateFile);
    connect(ui.impttButton, &QAbstractButton::clicked,
            this, &ServerListDlg::slotLoadTTFile);
    connect(ui.hostaddrBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ServerListDlg::slotShowHost);
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

    clearServer();

    showLatestHosts();

    if(ttSettings->value(SETTINGS_DISPLAY_FREESERVERS, true).toBool())
        ui.freeserverChkBox->setChecked(true);

    ui.delButton->setEnabled(false);
    showServers();
}

void ServerListDlg::showLatestHosts()
{
    ui.hostaddrBox->clear();

    HostEntry host;
    int index = 0;
    while(getLatestHost(index++, host))
        ui.hostaddrBox->addItem(host.ipaddr);
    slotShowHost(0);
}

void ServerListDlg::slotClearServerClicked()
{
    clearServer();
    ui.hostaddrBox->setFocus();
}

void ServerListDlg::clearServer()
{
    ui.nameEdit->setText("");
    ui.hostaddrBox->lineEdit()->setText("");
    ui.tcpportEdit->setText(QString::number(DEFAULT_TCPPORT));
    ui.udpportEdit->setText(QString::number(DEFAULT_UDPPORT));
    ui.cryptChkBox->setChecked(false);
    ui.usernameBox->lineEdit()->setText("");
    ui.passwordEdit->setText("");
    ui.channelEdit->setText("");
    ui.chanpasswdEdit->setText("");

    ui.clearButton->setEnabled(false);
}

void ServerListDlg::slotShowHost(int index)
{
    HostEntry host;
    if(getLatestHost(index, host))
    {
        showHost(host);
        ui.delButton->setEnabled(false);
    }
}

void ServerListDlg::showHost(const HostEntry& entry)
{
    ui.nameEdit->setText(entry.name);
    ui.hostaddrBox->lineEdit()->setText(entry.ipaddr);
    ui.tcpportEdit->setText(QString::number(entry.tcpport));
    ui.udpportEdit->setText(QString::number(entry.udpport));
    ui.cryptChkBox->setChecked(entry.encrypted);
    ui.usernameBox->lineEdit()->setText(entry.username);
    if (entry.username == WEBLOGIN_BEARWARE_USERNAME)
        ui.passwordEdit->setText("");
    else
        ui.passwordEdit->setText(entry.password);
    ui.passwordEdit->setDisabled(entry.username == WEBLOGIN_BEARWARE_USERNAME ||
                                 entry.username.endsWith(WEBLOGIN_BEARWARE_USERNAMEPOSTFIX));
    ui.channelEdit->setText(entry.channel);
    ui.chanpasswdEdit->setText(entry.chanpasswd);

    ui.clearButton->setEnabled(true);
}

void ServerListDlg::showServers()
{
    m_servers.clear();
    ui.listWidget->clear();
    int index = 0;
    HostEntry entry;
    while(getServerEntry(index++, entry))
    {
        m_servers.push_back(entry);
        entry = HostEntry();
    }
    for(int i=0;i<m_servers.size();i++)
        ui.listWidget->addItem(m_servers[i].name);

    if(ui.freeserverChkBox->isChecked())
        slotFreeServers(true);
}

void ServerListDlg::slotShowServer(int index)
{
    clearServer();
    if(index >= 0 && index < m_servers.size())
    {
        showHost(m_servers[index]);
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
        int index = ui.listWidget->currentRow();
        deleteServerEntry(entry.name);
        addServerEntry(entry);
        showServers();
        ui.listWidget->setCurrentRow(index);
    }
}

void ServerListDlg::slotDeleteServer()
{
    QListWidgetItem* item = ui.listWidget->currentItem();
    if(item)
    {
        int index = ui.listWidget->currentRow();
        deleteServerEntry(item->text());
        clearServer();
        showServers();
        ui.delButton->setEnabled(false);
        ui.listWidget->setCurrentRow(index);
        ui.listWidget->setFocus();
    }
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
    entry.ipaddr = ui.hostaddrBox->lineEdit()->text();
    entry.tcpport = ui.tcpportEdit->text().toInt();
    entry.udpport = ui.udpportEdit->text().toInt();
    entry.encrypted = ui.cryptChkBox->isChecked();
    entry.username = ui.usernameBox->lineEdit()->text().trimmed();
    entry.password = ui.passwordEdit->text();
    entry.channel = ui.channelEdit->text().trimmed();
    entry.chanpasswd = ui.chanpasswdEdit->text();

    return true;
}

void ServerListDlg::slotConnect()
{
    HostEntry entry;
    if(getHostEntry(entry))
    {
        if (entry.username == WEBLOGIN_BEARWARE_USERNAME ||
            entry.username.endsWith(WEBLOGIN_BEARWARE_USERNAMEPOSTFIX))
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

void ServerListDlg::slotServerSelected(QListWidgetItem *)
{
    qDebug() << "Activated";
}

void ServerListDlg::slotDoubleClicked(QListWidgetItem*)
{
    slotConnect();
}

void ServerListDlg::slotFreeServers(bool checked)
{
    ttSettings->setValue(SETTINGS_DISPLAY_FREESERVERS, checked);

    if(!checked)
    {
        showServers();
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
    Q_ASSERT(m_http_manager);
    QByteArray data = reply->readAll();

    QDomDocument doc("foo");
    if(!doc.setContent(data))
        return;

    int index = m_servers.size();
	QDomElement rootElement(doc.documentElement());
	QDomElement element = rootElement.firstChildElement();
    while(!element.isNull())
    {
        HostEntry entry;
        if(getServerEntry(element, entry))
            m_servers.push_back(entry);
		element = element.nextSiblingElement();
    }
    for(;index<m_servers.size();index++)
    {
        QListWidgetItem* srvItem = new QListWidgetItem(ui.listWidget);
        srvItem->setText(m_servers[index].name);
        srvItem->setBackground(QColor(0x0C,0x52,0x28));
        ui.listWidget->addItem(srvItem);
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
    showServers();
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
    if (username == WEBLOGIN_BEARWARE_USERNAME)
        ui.passwordEdit->setText("");
}
