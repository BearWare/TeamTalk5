/*
 * Copyright (c) 2005-2014, BearWare.dk
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

#include "bannedusersdlg.h"
#include "appinfo.h"

enum
{
    COLUMN_INDEX_NICKNAME = 0,
    COLUMN_INDEX_USERNAME,
    COLUMN_INDEX_BANTIME,
    COLUMN_INDEX_CHANPATH,
    COLUMN_INDEX_IPADDRESS,
    COLUMN_COUNT_BANNEDUSERS
};

extern TTInstance* ttInst;

BannedUsersModel::BannedUsersModel(QObject* parent)
: QAbstractItemModel(parent)
{
}

QVariant BannedUsersModel::headerData ( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if(orientation == Qt::Horizontal)
            switch(section)
        {
            case COLUMN_INDEX_NICKNAME: return "Nickname";
            case COLUMN_INDEX_USERNAME: return "Username";
            case COLUMN_INDEX_BANTIME: return "Ban Time";
            case COLUMN_INDEX_CHANPATH: return "Channel";
            case COLUMN_INDEX_IPADDRESS: return "IP-address";
        }
    }
    return QVariant();
}

int BannedUsersModel::columnCount ( const QModelIndex & parent /*= QModelIndex() */) const
{
    if(!parent.isValid())
        return 5;
    return 0;
}

QVariant BannedUsersModel::data ( const QModelIndex & index, int role /*= Qt::DisplayRole */) const
{
    Q_ASSERT(index.isValid());
    switch(role)
    {
    case Qt::DisplayRole :
        Q_ASSERT(index.row() < m_users.size());
        switch(index.column())
        {
        case COLUMN_INDEX_NICKNAME :
            return _Q(m_users[index.row()].szNickname);
        case COLUMN_INDEX_USERNAME :
            return _Q(m_users[index.row()].szUsername);
        case COLUMN_INDEX_BANTIME :
            return _Q(m_users[index.row()].szBanTime);
        case COLUMN_INDEX_CHANPATH :
            return _Q(m_users[index.row()].szChannelPath);
        case COLUMN_INDEX_IPADDRESS : 
            return _Q(m_users[index.row()].szIPAddress);
        }
        break;
    }
    return QVariant();
}

QModelIndex BannedUsersModel::index ( int row, int column, const QModelIndex & parent /*= QModelIndex()*/ ) const
{
    if(!parent.isValid() && row<m_users.size())
        return createIndex(row, column);
    return QModelIndex();
}

QModelIndex BannedUsersModel::parent ( const QModelIndex &/* index */) const
{
    return QModelIndex();
}

int BannedUsersModel::rowCount ( const QModelIndex & /*parent = QModelIndex() */) const
{
    return m_users.size();
}

void BannedUsersModel::addBannedUser(const BannedUser& user, bool do_reset)
{
    m_users.push_back(user);
    if(do_reset)
    {
        this->beginResetModel();
        this->endResetModel();
    }
}

void BannedUsersModel::delBannedUser(int index)
{
    if(m_users.size())
        m_users.erase(m_users.begin()+index);

    this->beginResetModel();
    this->endResetModel();
}

BannedUsersDlg::BannedUsersDlg(const bannedusers_t& bannedusers, QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    m_bannedmodel = new BannedUsersModel(this);
    m_unbannedmodel = new BannedUsersModel(this);

    for(int i=0;i<bannedusers.size();i++)
        m_bannedmodel->addBannedUser(bannedusers[i], i+1 == bannedusers.size());

    ui.bannedTreeView->setModel(m_bannedmodel);
    for(int i=0;i<COLUMN_COUNT_BANNEDUSERS;i++)
        ui.bannedTreeView->resizeColumnToContents(i);
    ui.unbannedTreeView->setModel(m_unbannedmodel);
    for(int i=0;i<COLUMN_COUNT_BANNEDUSERS;i++)
        ui.unbannedTreeView->resizeColumnToContents(i);

    connect(ui.banipBtn, SIGNAL(clicked()), SLOT(slotBanIPAddress()));
    connect(ui.buttonBox, SIGNAL(accepted()), SLOT(slotClose()));
    connect(ui.leftButton, SIGNAL(clicked()), SLOT(slotBanUser()));
    connect(ui.rightButton, SIGNAL(clicked()), SLOT(slotUnbanUser()));
}

void BannedUsersDlg::slotClose()
{
    bannedusers_t users = m_unbannedmodel->getUsers();
    for(int i=0;i<users.size();i++)
        TT_DoUnBanUser(ttInst, users[i].szIPAddress, 0);
}

void BannedUsersDlg::slotUnbanUser()
{
    int index = ui.bannedTreeView->currentIndex().row();
    if(index<0)
        return;
    m_unbannedmodel->addBannedUser(m_bannedmodel->getUsers()[index], true);
    m_bannedmodel->delBannedUser(index);
}

void BannedUsersDlg::slotBanUser()
{
    int index = ui.unbannedTreeView->currentIndex().row();
    if(index<0)
        return;
    m_bannedmodel->addBannedUser(m_unbannedmodel->getUsers()[index], true);
    m_unbannedmodel->delBannedUser(index);
}

void BannedUsersDlg::slotBanIPAddress()
{
    if(TT_DoBanIPAddress(ttInst, _W(ui.ipaddrEdit->text()), 0)>0)
    {
        BannedUser user;
        ZERO_STRUCT(user);
        COPY_TTSTR(user.szIPAddress, ui.ipaddrEdit->text());
        ui.ipaddrEdit->setText("");
        m_bannedmodel->addBannedUser(user, true);
    }
}
