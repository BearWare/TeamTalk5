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
            case COLUMN_INDEX_NICKNAME: return tr("Nickname");
            case COLUMN_INDEX_USERNAME: return tr("Username");
            case COLUMN_INDEX_BANTIME: return tr("Ban Time");
            case COLUMN_INDEX_CHANPATH: return tr("Channel");
            case COLUMN_INDEX_IPADDRESS: return tr("IP-address");
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

BannedUsersDlg::BannedUsersDlg(const bannedusers_t& bannedusers, const QString& chanpath, QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
    , m_chanpath(chanpath)
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

    ui.bantypeBox->addItem(tr("Ban IP-address"), BanTypes(BANTYPE_IPADDR));
    ui.bantypeBox->addItem(tr("Ban Username"), BanTypes(BANTYPE_USERNAME));
    connect(ui.newbanBtn, SIGNAL(clicked()), SLOT(slotNewBan()));
    connect(ui.buttonBox, SIGNAL(accepted()), SLOT(slotClose()));
    connect(ui.leftButton, SIGNAL(clicked()), SLOT(slotBanUser()));
    connect(ui.rightButton, SIGNAL(clicked()), SLOT(slotUnbanUser()));
    connect(ui.bannedTreeView, &QTreeView::doubleClicked, this, &BannedUsersDlg::slotUnbanUser);
    connect(ui.unbannedTreeView, &QTreeView::doubleClicked, this, &BannedUsersDlg::slotBanUser);
}

void BannedUsersDlg::slotClose()
{
    bannedusers_t users = m_unbannedmodel->getUsers();
    for(int i=0;i<users.size();i++)
        TT_DoUnBanUserEx(ttInst, &users[i]);
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

void BannedUsersDlg::slotNewBan()
{
    BannedUser ban;
    ZERO_STRUCT(ban);
    ban.uBanTypes = BanTypes(ui.bantypeBox->currentData().toInt());
    if (m_chanpath.size())
    {
        ban.uBanTypes |= BANTYPE_CHANNEL;
        COPY_TTSTR(ban.szChannelPath, m_chanpath);
    }
    if (ui.bantypeBox->currentData().toInt() & BANTYPE_IPADDR)
        COPY_TTSTR(ban.szIPAddress, ui.banEdit->text());
    if (ui.bantypeBox->currentData().toInt() & BANTYPE_USERNAME)
        COPY_TTSTR(ban.szUsername, ui.banEdit->text());
    ui.banEdit->setText("");

    if(TT_DoBan(ttInst, &ban) > 0)
    {
        m_bannedmodel->addBannedUser(ban, true);
    }
}
