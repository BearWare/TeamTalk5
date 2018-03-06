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

#include "onlineusersdlg.h"
#include "appinfo.h"

#include <QHeaderView>
#include <QMenu>

extern TTInstance* ttInst;

OnlineUsersDlg::OnlineUsersDlg(QWidget* parent/* = 0 */)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.treeView, SIGNAL(customContextMenuRequested(const QPoint&)),
            SLOT(slotTreeContextMenu(const QPoint&)));

    m_model = new OnlineUsersModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    ui.treeView->setModel(m_proxyModel);

    ui.treeView->header()->resizeSection(COLUMN_USERID, 40);
    //ui.treeView->header()->resizeSection(COLUMN_STATUSMSG, 70);
    ui.treeView->header()->resizeSection(COLUMN_USERNAME, 65);

    m_model->resetUsers();
    updateTitle();
}

void OnlineUsersDlg::updateTitle()
{
    QString title = tr("Users Currently on Server");
    title += QString(" (%1)").arg(m_model->rowCount());
    ui.groupBox->setTitle(title);
}

void OnlineUsersDlg::slotUserLoggedIn(const User& user)
{
    m_model->addUser(user.nUserID);
    updateTitle();
}

void OnlineUsersDlg::slotUserLoggedOut(const User& user)
{
    m_model->removeUser(user.nUserID);
    updateTitle();
}

void OnlineUsersDlg::slotUserUpdate(const User& user)
{
    m_model->updateUser(user.nUserID);
    QModelIndex index = m_model->userRow(user.nUserID);
    if(index.isValid())
        ui.treeView->update(index);
}

void OnlineUsersDlg::slotUserJoin(int /*channelid*/, const User& user)
{
    m_model->updateUser(user.nUserID);
    QModelIndex index = m_model->userRow(user.nUserID);
    if(index.isValid())
        ui.treeView->update(index);
}

void OnlineUsersDlg::slotUserLeft(int /*channelid*/, const User& user)
{
    m_model->updateUser(user.nUserID);
    QModelIndex index = m_model->userRow(user.nUserID);
    if(index.isValid())
        ui.treeView->update(index);
}

void OnlineUsersDlg::slotTreeContextMenu(const QPoint& /*point*/)
{
    QMenu menu(this);
    QAction* info = menu.addAction(tr("&View User Information"));
    QAction* messages = menu.addAction(tr("M&essages"));
    //QAction* mute = menu.addAction(tr("&Mute"));
    //QAction* volume = menu.addAction(tr("&Volume"));
    QAction* op = menu.addAction(tr("&Op"));
    QAction* kick = menu.addAction(tr("&Kick"));
    QAction* ban = menu.addAction(tr("Kick and &Ban"));    
    QAction* action = menu.exec(QCursor::pos());

    QItemSelectionModel* selModel = ui.treeView->selectionModel();
    QModelIndexList indexes = selModel->selectedRows();
    QVector<int> userids, chanids;
    for(int i=0;i<indexes.size();i++)
    {
        //QModelIndex index = ui.treeView->indexAt(point);
        QModelIndex index = m_proxyModel->mapToSource(indexes[i]);
        if(!index.isValid())
            return;

        int userid = index.internalId();
        User user;
        if(!TT_GetUser(ttInst, userid, &user))
            continue;
        userids.push_back(user.nUserID);
        chanids.push_back(user.nChannelID);
        qDebug() << "Processing " << user.nUserID;
    }
    
    for(int i=0;i<userids.size();i++)
    {
        if(action == info)
            emit(viewUserInformation(userids[i]));
        else if(action == messages)
            emit(sendUserMessage(userids[i]));
        //else if(action == mute)
        //    emit(muteUser(userid, !(user.uUserState & USERSTATE_MUTE)));
        //else if(action == volume)
        //    emit(changeUserVolume(userid));
        else if(action == op)
            emit(opUser(userids[i], chanids[i]));
        else if(action == kick)
            emit(kickUser(userids[i], chanids[i]));
        else if(action == ban)
            emit(kickbanUser(userids[i], chanids[i]));
    }
}
