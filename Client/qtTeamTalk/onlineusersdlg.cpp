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

#include "onlineusersdlg.h"
#include "appinfo.h"
#include "settings.h"
#include "utilui.h"

#include <QHeaderView>
#include <QMenu>
#include <QClipboard>
#include <QKeyEvent>

extern TTInstance* ttInst;
extern NonDefaultSettings* ttSettings;

OnlineUsersDlg::OnlineUsersDlg(QWidget* parent/* = 0 */)
: QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_ONLINEUSERSWINDOWPOS).toByteArray());

    ui.tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.tableView, &QWidget::customContextMenuRequested,
            this, &OnlineUsersDlg::slotTreeContextMenu);
    ui.keepDisconnectedUsersCheckBox->setChecked(ttSettings->value(SETTINGS_KEEP_DISCONNECTED_USERS, SETTINGS_KEEP_DISCONNECTED_USERS_DEFAULT).toBool());
    connect(ui.keepDisconnectedUsersCheckBox, &QAbstractButton::clicked, this, &OnlineUsersDlg::slotUpdateSettings);

    using std::placeholders::_1;
    m_model = new OnlineUsersModel(this, std::bind(&QHeaderView::logicalIndex, ui.tableView->horizontalHeader(), _1));
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    ui.tableView->setModel(m_proxyModel);

    ui.tableView->horizontalHeader()->resizeSection(COLUMN_USERID, 40);
    //ui.tableView->horizontalHeader()->resizeSection(COLUMN_STATUSMSG, 70);
    ui.tableView->horizontalHeader()->resizeSection(COLUMN_USERNAME, 65);

    m_model->resetUsers();
    updateTitle();

    m_viewInfoAct = new QShortcut(QKeySequence(tr("Ctrl+I")), this);
    connect(m_viewInfoAct, &QShortcut::activated, this, [this]
    {
        menuAction(VIEW_USERINFORMATION);
    });
    m_msgAct = new QShortcut(QKeySequence(tr("Ctrl+E")), this);
    connect(m_msgAct, &QShortcut::activated, this, [this]
    {
        menuAction(SEND_TEXTMESSAGE);
    });
    m_opAct = new QShortcut(QKeySequence(tr("Ctrl+O")), this);
    connect(m_opAct, &QShortcut::activated, this, [this]
    {
        menuAction(OP);
    });
    m_kickAct = new QShortcut(QKeySequence(tr("Ctrl+K")), this);
    connect(m_kickAct, &QShortcut::activated, this, [this]
    {
        menuAction(KICK_FROM_CHANNEL);
    });
    m_kickServAct = new QShortcut(QKeySequence(tr("Ctrl+Alt+K")), this);
    connect(m_kickServAct, &QShortcut::activated, this, [this]
    {
        menuAction(KICK_FROM_SERVER);
    });
    m_kickBanAct = new QShortcut(QKeySequence(tr("Ctrl+B")), this);
    connect(m_kickBanAct, &QShortcut::activated, this, [this]
    {
        menuAction(BAN_FROM_CHANNEL);
    });
    m_kickBanServAct = new QShortcut(QKeySequence(tr("Ctrl+Alt+B")), this);
    connect(m_kickBanServAct, &QShortcut::activated, this, [this]
    {
        menuAction(BAN_FROM_SERVER);
    });
    m_selMoveAct = new QShortcut(QKeySequence(tr("Ctrl+Alt+X")), this);
    connect(m_selMoveAct, &QShortcut::activated, this, [this]
    {
        menuAction(MOVE);
    });
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->sort(COLUMN_NICKNAME, Qt::AscendingOrder);
    ui.tableView->horizontalHeader()->restoreState(ttSettings->value(SETTINGS_DISPLAY_ONLINEUSERS_HEADERSIZES).toByteArray());
}

OnlineUsersDlg::~OnlineUsersDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_ONLINEUSERS_HEADERSIZES, ui.tableView->horizontalHeader()->saveState());
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
    RestoreItemData r(ui.tableView, m_proxyModel);
    m_model->addUser(user);
    updateTitle();
}

void OnlineUsersDlg::slotUserLoggedOut(const User& user)
{
    RestoreItemData r(ui.tableView, m_proxyModel);
    m_model->removeUser(user, ttSettings->value(SETTINGS_KEEP_DISCONNECTED_USERS, SETTINGS_KEEP_DISCONNECTED_USERS_DEFAULT).toBool());
    updateTitle();
}

void OnlineUsersDlg::slotUserUpdate(const User& user)
{
    RestoreItemData r(ui.tableView, m_proxyModel);
    m_model->updateUser(user);
    QModelIndex index = m_model->userRow(user.nUserID);
    if(index.isValid())
        ui.tableView->update(index);
}

void OnlineUsersDlg::slotUserJoin(int /*channelid*/, const User& user)
{
    RestoreItemData r(ui.tableView, m_proxyModel);
    m_model->updateUser(user);
    QModelIndex index = m_model->userRow(user.nUserID);
    if(index.isValid())
        ui.tableView->update(index);
}

void OnlineUsersDlg::slotUserLeft(int /*channelid*/, const User& user)
{
    RestoreItemData r(ui.tableView, m_proxyModel);
    m_model->updateUser(user);
    QModelIndex index = m_model->userRow(user.nUserID);
    if(index.isValid())
        ui.tableView->update(index);
}

void OnlineUsersDlg::slotTreeContextMenu(const QPoint& /*point*/)
{
    QMenu menu(this);
    auto add = [&](const QString &txt, const QKeySequence &seq, MenuAction ma)
    {
        QAction *act = menu.addAction(txt, [this, ma]{ menuAction(ma); });
        act->setShortcut(seq);
    };
    add(tr("&View User Information"), QKeySequence(tr("Ctrl+I")), VIEW_USERINFORMATION);
    add(tr("M&essages"), QKeySequence(tr("Ctrl+E")), SEND_TEXTMESSAGE);
    add(tr("&Op"), QKeySequence(tr("Ctrl+O")), OP);
    add(tr("&Kick from Channel"), QKeySequence(tr("Ctrl+K")), KICK_FROM_CHANNEL);
    add(tr("K&ick from Server"), QKeySequence(tr("Ctrl+Alt+K")), KICK_FROM_SERVER);
    add(tr("Kick and &Ban from Channel"), QKeySequence(tr("Ctrl+B")), BAN_FROM_CHANNEL);
    add(tr("Kick and B&an from Server"), QKeySequence(tr("Ctrl+Alt+B")), BAN_FROM_SERVER);
    add(tr("&Select User(s) for Move"), QKeySequence(tr("Ctrl+Alt+X")), MOVE);
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
            ui.tableView->horizontalHeader()->setSortIndicator(COLUMN_USERID, m_proxyModel->sortColumn() == COLUMN_USERID ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValueOrClear(SETTINGS_DISPLAY_ONLINEUSERS_SORT, id, SETTINGS_DISPLAY_ONLINEUSERS_SORT_DEFAULT);
        }
        else if (action == sortNickname)
        {
            ui.tableView->horizontalHeader()->setSortIndicator(COLUMN_NICKNAME, m_proxyModel->sortColumn() == COLUMN_NICKNAME ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValueOrClear(SETTINGS_DISPLAY_ONLINEUSERS_SORT, nickname, SETTINGS_DISPLAY_ONLINEUSERS_SORT_DEFAULT);
        }
    }
}

void OnlineUsersDlg::menuAction(MenuAction ma)
{
    QItemSelectionModel* selModel = ui.tableView->selectionModel();
    QModelIndexList indexes = selModel->selectedRows();
    QVector<int> userids, chanids;
    for(int i=0;i<indexes.size();i++)
    {
        //QModelIndex index = ui.tableView->indexAt(point);
        QModelIndex index = m_proxyModel->mapToSource(indexes[i]);
        if(!index.isValid())
            return;

        int userid = index.internalId();
        User user = m_model->getUser(userid);
        if (user.nUserID <= 0)
            continue;
        userids.push_back(user.nUserID);
        chanids.push_back(user.nChannelID);
    }

    for(int i=0;i<userids.size();i++)
    {
        switch (ma)
        {
        case VIEW_USERINFORMATION :
            emit viewUserInformation(userids[i]);
            break;
        case SEND_TEXTMESSAGE :
            emit sendUserMessage(userids[i]);
            break;
        case OP :
            emit opUser(userids[i], chanids[i]);
            break;
        case KICK_FROM_CHANNEL :
            emit kickUser(userids[i], chanids[i]);
            break;
        case KICK_FROM_SERVER :
            emit kickUser(userids[i], 0);
            break;
        case BAN_FROM_CHANNEL :
            emit kickbanUser(m_model->getUser(userids[i]));
            break;
        case BAN_FROM_SERVER :
        {
            auto user = m_model->getUser(userids[i]);
            user.nChannelID = 0;
            emit kickbanUser(user);
            break;
        }
        case MOVE :
            emit moveUser(userids[i]);
            break;
        }
    }
}

void OnlineUsersDlg::keyPressEvent(QKeyEvent* e)
{
    if (ui.tableView->hasFocus())
    {
        if (e->matches(QKeySequence::Copy))
        {
            QItemSelectionModel* selModel = ui.tableView->selectionModel();
            QModelIndexList indexes = selModel->selectedRows();
            QVector<int> userids, chanids;
            for(int i=0;i<indexes.size();i++)
            {
                QModelIndex index = m_proxyModel->mapToSource(indexes[i]);
                if(!index.isValid())
                    return;
                int userid = index.internalId();
                User user = m_model->getUser(userid);
                if (user.nUserID <= 0)
                    continue;
                userids.push_back(user.nUserID);
                chanids.push_back(user.nChannelID);
            }
            for(int i=0;i<userids.size();i++)
            {
                User user = m_model->getUser(userids[i]);
                if (user.nUserID > 0)
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
    RestoreItemData r(ui.tableView, m_proxyModel);
    ttSettings->setValueOrClear(SETTINGS_KEEP_DISCONNECTED_USERS, ui.keepDisconnectedUsersCheckBox->isChecked(), SETTINGS_KEEP_DISCONNECTED_USERS_DEFAULT);
    if (!ui.keepDisconnectedUsersCheckBox->isChecked())
        m_model->removeDisconnected();
}
