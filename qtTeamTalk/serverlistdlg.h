/*
 * Copyright (c) 2005-2016, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#ifndef SERVERLISTDLG_H
#define SERVERLISTDLG_H

#include "ui_serverlist.h"
#include "common.h"
#include <QVector>
#include <QNetworkAccessManager>

class ServerListDlg : public QDialog
{
    Q_OBJECT
public:
    ServerListDlg(QWidget * parent = 0);

private:
    Ui::ServerListDlg ui;
    void showServers();
    void showLatestHosts();
    QVector<HostEntry> m_servers, m_freeservers;
    QNetworkAccessManager* m_http_manager;

    bool getHostEntry(HostEntry& entry);
    void showHost(const HostEntry& entry);
    void clearServer();

private slots:
    void slotShowHost(int index);
    void slotShowServer(int index);
    void slotAddUpdServer();
    void slotDeleteServer();
    void slotClearServerClicked();
    void slotConnect();
    void slotServerSelected(QListWidgetItem * item);
    void slotDoubleClicked(QListWidgetItem*);
    void slotFreeServers(bool checked);
    void slotFreeServerRequest(QNetworkReply* reply);
    void slotGenerateFile();
    void slotDeleteLatestHost();

    void slotSaveEntryChanged(const QString& text);
    void slotGenerateEntryName(const QString&);
};

#endif
