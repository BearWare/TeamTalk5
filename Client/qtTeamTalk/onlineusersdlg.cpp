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
#include "settings.h"

#include <QHeaderView>
#include <QMenu>
#include <QClipboard>
#include <QKeyEvent>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

OnlineUsersDlg::OnlineUsersDlg(QWidget* parent/* = 0 */)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_ONLINEUSERSWINDOWPOS).toByteArray());

    ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.treeView, &QWidget::customContextMenuRequested,
            this, &OnlineUsersDlg::slotTreeContextMenu);
    ui.keepDisconnectedUsersCheckBox->setChecked(ttSettings->value(SETTINGS_KEEP_DISCONNECTED_USERS, SETTINGS_KEEP_DISCONNECTED_USERS_DEFAULT).toBool());
    connect(ui.keepDisconnectedUsersCheckBox, &QAbstractButton::clicked, this, &OnlineUsersDlg::slotUpdateSettings);

    m_model = new OnlineUsersModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    ui.treeView->setModel(m_proxyModel);

    ui.treeView->header()->resizeSection(COLUMN_USERID, 40);
    //ui.treeView->header()->resizeSection(COLUMN_STATUSMSG, 70);
    ui.treeView->header()->resizeSection(COLUMN_USERNAME, 65);

    m_model->resetUsers();
    updateTitle();

#if QT_VERSION >= QT_VERSION_CHECK(6,3,0)
    addAction(tr("&View User Information"), QKeySequence(tr("Ctrl+I")), this, [&](){ menuAction(VIEW_USERINFORMATION); });
    addAction(tr("M&essages"), QKeySequence(tr("Ctrl+E")), this, [&]() { menuAction(SEND_TEXTMESSAGE); });
    addAction(tr("&Op"), QKeySequence(tr("Ctrl+O")), this, [&]() { menuAction(OP); });
    addAction(tr("&Kick"), QKeySequence(tr("Ctrl+K")), this, [&]() { menuAction(KICK); });
    addAction(tr("Kick and &Ban"), QKeySequence(tr("Ctrl+B")), this, [&]() { menuAction(BAN); });
    addAction(tr("Select User(s) for Move"), QKeySequence(tr("Ctrl+Alt+X")), this, [&]() { menuAction(MOVE); });
#endif
}

OnlineUsersDlg::~OnlineUsersDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_ONLINEUSERSWINDOWPOS, saveGeometry());
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
    m_model->removeUser(user.nUserID, ttSettings->value(SETTINGS_KEEP_DISCONNECTED_USERS, SETTINGS_KEEP_DISCONNECTED_USERS_DEFAULT).toBool());
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
    menu.addAction(tr("&View User Information"), this, [&]() { menuAction(VIEW_USERINFORMATION); },
                   QKeySequence(tr("Ctrl+I")));
    menu.addAction(tr("M&essages"), this, [&]() { menuAction(SEND_TEXTMESSAGE); },
                   QKeySequence(tr("Ctrl+E")));
    menu.addAction(tr("&Op"), this, [&]() { menuAction(OP); },
                   QKeySequence(tr("Ctrl+O")));
    menu.addAction(tr("&Kick"), this, [&]() { menuAction(KICK); },
                   QKeySequence(tr("Ctrl+K")));
    menu.addAction(tr("Kick and &Ban"), this, [&]() { menuAction(BAN); },
                   QKeySequence(tr("Ctrl+B")));
    menu.addAction(tr("Select User(s) for Move"), this, [&]() { menuAction(MOVE); },
                   QKeySequence(tr("Ctrl+Alt+X")));
    QMenu* sortMenu = menu.addMenu(tr("Sort By..."));
    QString asc = tr("Ascending"), desc = tr("Descending");
    QAction* sortId = new QAction(sortMenu);
    sortId->setText(tr("&Id (%1)").arg(m_proxyModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortId->setCheckable(true);
    const QString id = "id";
    sortId->setChecked((ttSettings->value(SETTINGS_DISPLAY_ONLINEUSERS_SORT, SETTINGS_DISPLAY_ONLINEUSERS_SORT_DEFAULT).toString() == id)?true:false);
    sortMenu->addAction(sortId);
    QAction* sortNickname = new QAction(sortMenu);
    sortNickname->setText(tr("&Nickname (%1)").arg(m_proxyModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortNickname->setCheckable(true);
    const QString nickname = "nickname";
    sortNickname->setChecked((ttSettings->value(SETTINGS_DISPLAY_ONLINEUSERS_SORT, SETTINGS_DISPLAY_ONLINEUSERS_SORT_DEFAULT).toString() == nickname)?true:false);
    sortMenu->addAction(sortNickname);
    if (QAction* action = menu.exec(QCursor::pos()))
    {
        auto sortToggle = m_proxyModel->sortOrder() == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        if (action == sortId)
        {
            ui.treeView->header()->setSortIndicator(COLUMN_USERID, m_proxyModel->sortColumn() == COLUMN_USERID ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValue(SETTINGS_DISPLAY_ONLINEUSERS_SORT, id);
        }
        else if (action == sortNickname)
        {
            ui.treeView->header()->setSortIndicator(COLUMN_NICKNAME, m_proxyModel->sortColumn() == COLUMN_NICKNAME ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValue(SETTINGS_DISPLAY_ONLINEUSERS_SORT, nickname);
        }
    }
}

void OnlineUsersDlg::menuAction(MenuAction ma)
{
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
    }

    for(int i=0;i<userids.size();i++)
    {
        switch (ma)
        {
        case VIEW_USERINFORMATION :
            emit(viewUserInformation(userids[i]));
            break;
        case SEND_TEXTMESSAGE :
            emit(sendUserMessage(userids[i]));
            break;
        case OP :
            emit(opUser(userids[i], chanids[i]));
            break;
        case KICK :
            emit(kickUser(userids[i], chanids[i]));
            break;
        case BAN :
            emit(kickbanUser(userids[i], chanids[i]));
            break;
        case MOVE :
            emit(moveUser(userids[i]));
            break;
        }
    }
}

void OnlineUsersDlg::keyPressEvent(QKeyEvent* e)
{
    if (ui.treeView->hasFocus())
    {
        if (e->matches(QKeySequence::Copy))
        {
            QItemSelectionModel* selModel = ui.treeView->selectionModel();
            QModelIndexList indexes = selModel->selectedRows();
            QVector<int> userids, chanids;
            for(int i=0;i<indexes.size();i++)
            {
                QModelIndex index = m_proxyModel->mapToSource(indexes[i]);
                if(!index.isValid())
                    return;
                int userid = index.internalId();
                User user;
                if(!TT_GetUser(ttInst, userid, &user))
                    continue;
                userids.push_back(user.nUserID);
                chanids.push_back(user.nChannelID);
            }
            for(int i=0;i<userids.size();i++)
            {
                User user;
                if(TT_GetUser(ttInst, userids[i], &user))
                {
                    TTCHAR channel[TT_STRLEN] = {};
                    TT_GetChannelPath(ttInst, user.nChannelID, channel);
                    QClipboard* clipboard = QApplication::clipboard();
                    clipboard->setText(QString(tr("Nickname: %2, Status message: %3, Username: %4, Channel: %5, IP address: %6, Version: %7, ID: %1").arg(user.nUserID).arg(getDisplayName(user)).arg(_Q(user.szStatusMsg)).arg(_Q(user.szUsername)).arg(_Q(channel)).arg(_Q(user.szIPAddress)).arg(getVersion(user))));
                }
            }
        }
    }
    QDialog::keyPressEvent(e);
}

void OnlineUsersDlg::slotUpdateSettings()
{
    ttSettings->setValue(SETTINGS_KEEP_DISCONNECTED_USERS, ui.keepDisconnectedUsersCheckBox->isChecked());
    if (!ui.keepDisconnectedUsersCheckBox->isChecked())
        m_model->removeDisconnected();
}
