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

#include "useraccountsdlg.h"
#include "appinfo.h"
#include "common.h"

#include <QMessageBox>
#include <QInputDialog>

extern TTInstance* ttInst;

enum
{
    COLUMN_INDEX_USERNAME,
    COLUMN_INDEX_PASSWORD,
    COLUMN_INDEX_USERTYPE,
    COLUMN_INDEX_NOTE,
    COLUMN_INDEX_CHANNEL,
    COLUMN_COUNT_USERACCOUNTS,
};
UserAccountsModel::UserAccountsModel(QObject* parent)
: QAbstractItemModel(parent)
{
}

QVariant UserAccountsModel::headerData ( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if(orientation == Qt::Horizontal)
            switch(section)
        {
            case COLUMN_INDEX_USERNAME: return tr("Username");
            case COLUMN_INDEX_PASSWORD: return tr("Password");
            case COLUMN_INDEX_USERTYPE: return tr("User Type");
            case COLUMN_INDEX_NOTE: return tr("Note");
            case COLUMN_INDEX_CHANNEL: return tr("Channel");
        }
    }
    return QVariant();
}

int UserAccountsModel::columnCount ( const QModelIndex & parent /*= QModelIndex() */) const
{
    if(!parent.isValid())
        return COLUMN_COUNT_USERACCOUNTS;
    return 0;
}

QVariant UserAccountsModel::data ( const QModelIndex & index, int role /*= Qt::DisplayRole */) const
{
    Q_ASSERT(index.isValid());
    switch(role)
    {
    case Qt::DisplayRole :
        Q_ASSERT(index.row() < m_users.size());
        switch(index.column())
        {
        case COLUMN_INDEX_USERNAME :
            return _Q(m_users[index.row()].szUsername);
        case COLUMN_INDEX_PASSWORD :
            return _Q(m_users[index.row()].szPassword);
        case COLUMN_INDEX_USERTYPE :
            if(m_users[index.row()].uUserType & USERTYPE_ADMIN)
                return tr("Administrator");
            else
                return tr("Default User");
        case COLUMN_INDEX_NOTE :
            return _Q(m_users[index.row()].szNote);
        case COLUMN_INDEX_CHANNEL :
            return _Q(m_users[index.row()].szInitChannel);
        }
        break;
    }
    return QVariant();
}

QModelIndex UserAccountsModel::index ( int row, int column, const QModelIndex & parent /*= QModelIndex()*/ ) const
{
    if(!parent.isValid() && row<m_users.size())
        return createIndex(row, column);
    return QModelIndex();
}

QModelIndex UserAccountsModel::parent ( const QModelIndex &/* index */) const
{
    return QModelIndex();
}

int UserAccountsModel::rowCount ( const QModelIndex & /*parent = QModelIndex() */) const
{
    return m_users.size();
}

void UserAccountsModel::addRegUser(const UserAccount& user, bool do_reset)
{
    for(int i=0;i<m_users.size();i++)
    {
        if(_Q(m_users[i].szUsername) == _Q(user.szUsername))
        {
            m_users.erase(m_users.begin()+i);
            break;
        }
    }
    m_users.push_back(user);
    if(do_reset)
    {
        this->beginResetModel();
        this->endResetModel();
    }
}

void UserAccountsModel::delRegUser(int index)
{
    if(m_users.size())
        m_users.erase(m_users.begin()+index);
    this->beginResetModel();
    this->endResetModel();
}

void UserAccountsModel::delRegUser(const QString& username)
{
    this->beginResetModel();

    for(int i=0;i<m_users.size();i++)
    {
        if(_Q(m_users[i].szUsername) == username)
        {
            m_users.erase(m_users.begin()+i);
            break;
        }
    }
    this->endResetModel();
}

enum
{
    LIMITCMD_DISABLED       = 0,
    LIMITCMD_10_PER_10SEC   = 1,
    LIMITCMD_10_PER_MINUTE  = 2,
    LIMITCMD_60_PER_MINUTE  = 3,
    LIMITCMD_CUSTOM         = 4
};

UserAccountsDlg::UserAccountsDlg(const useraccounts_t& useraccounts, UserAccountsDisplay uad, QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS | Qt::WindowMinMaxButtonsHint | Qt::WindowSystemMenuHint)
    , m_add_cmdid(0)
    , m_del_cmdid(0)
    , m_add_user()
    , m_abuse()
    , m_uad(uad)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    m_model = new UserAccountsModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    ui.usersTreeView->setModel(m_proxyModel);

#if defined(Q_OS_MAC)
    auto font = ui.usersTreeView->font();
    font.setPointSize(13);
    ui.usersTreeView->setFont(font);
#endif

    int count = 0;
    TT_GetServerChannels(ttInst, nullptr, &count);
    if(count)
    {
        QVector<Channel> channels;
        channels.resize(count);
        TT_GetServerChannels(ttInst, &channels[0], &count);
        QStringList str_channels;
        str_channels.append("");
        TTCHAR szChannel[TT_STRLEN];
        for(int i=0;i<count;i++)
        {
            if((channels[i].uChannelType & CHANNEL_PERMANENT) &&
               TT_GetChannelPath(ttInst, channels[i].nChannelID, szChannel))
               str_channels.append(_Q(szChannel));
        }
        str_channels.sort();
        for(int i=0;i<str_channels.size();i++)
        {
            ui.channelComboBox->addItem(str_channels[i]);
            if(str_channels[i].size())
                ui.opchanComboBox->addItem(str_channels[i]);
        }
    }

    ui.limitcmdComboBox->addItem(tr("Disabled"), LIMITCMD_DISABLED);
    ui.limitcmdComboBox->addItem(tr("10 commands in 10 sec."), LIMITCMD_10_PER_10SEC);
    ui.limitcmdComboBox->addItem(tr("10 commands in 1 minute"), LIMITCMD_10_PER_MINUTE);
    ui.limitcmdComboBox->addItem(tr("60 commands in 1 minute"), LIMITCMD_60_PER_MINUTE);
    ui.limitcmdComboBox->addItem(tr("Custom specified"), LIMITCMD_CUSTOM);

    connect(ui.limitcmdComboBox, SIGNAL(activated(int)),
            SLOT(slotCustomCmdLimit(int)));

    for(int i=0;i<useraccounts.size();i++)
        m_model->addRegUser(useraccounts[i], i+1 == useraccounts.size());

    for(int i=0;i<COLUMN_COUNT_USERACCOUNTS;i++)
        ui.usersTreeView->resizeColumnToContents(i);
    ui.delButton->setEnabled(false);

    connect(ui.addopBtn, SIGNAL(clicked()), SLOT(slotAddOpChannel()));
    connect(ui.rmopBtn, SIGNAL(clicked()), SLOT(slotRemoveOpChannel()));
    connect(ui.newButton, SIGNAL(clicked()), SLOT(slotClearUser()));
    connect(ui.addButton, SIGNAL(clicked()), SLOT(slotAddUser()));
    connect(ui.delButton, SIGNAL(clicked()), SLOT(slotDelUser()));
    connect(ui.closeBtn, SIGNAL(clicked()), SLOT(close()));
    connect(ui.defaultuserBtn, SIGNAL(clicked()), SLOT(slotUserTypeChanged()));
    connect(ui.adminBtn, SIGNAL(clicked()), SLOT(slotUserTypeChanged()));
    connect(ui.usernameEdit, SIGNAL(textChanged(const QString&)), SLOT(slotUsernameChanged(const QString&)));

    connect(ui.usersTreeView, SIGNAL(clicked(const QModelIndex&)), 
            SLOT(slotUserSelected(const QModelIndex&)));

    if(m_uad == UAD_READONLY)
    {
        if(useraccounts.size() == 1)
            showUserAccount(useraccounts[0]);
        lockUI(true);
    }
    else
        slotClearUser();
}

void UserAccountsDlg::slotCmdSuccess(int cmdid)
{
    if(cmdid == m_add_cmdid)
    {
        m_model->addRegUser(m_add_user, true); //here we disregard that the command might fail
        m_add_cmdid = 0;
        ZERO_STRUCT(m_add_user);
        lockUI(false);
    }
    if(cmdid == m_del_cmdid)
    {
        m_model->delRegUser(m_del_username); //here we disregard that the command might fail
        m_del_cmdid = 0;
        m_del_username.clear();
        lockUI(false);
    }
}

void UserAccountsDlg::slotCmdError(int /*error*/, int cmdid)
{
    if(cmdid == m_add_cmdid)
    {
        m_add_cmdid = 0;
        ZERO_STRUCT(m_add_user);
        lockUI(false);
    }
    if(cmdid == m_del_cmdid)
    {
        m_del_cmdid = 0;
        lockUI(false);
    }
}

void UserAccountsDlg::lockUI(bool locked)
{
    ui.usersTreeView->setEnabled(!locked);
    ui.usernameEdit->setEnabled(!locked);
    ui.passwordEdit->setEnabled(!locked);
    ui.defaultuserBtn->setEnabled(!locked);
    ui.adminBtn->setEnabled(!locked);
    ui.noteEdit->setEnabled(!locked);
    ui.channelComboBox->setEnabled(!locked);
    ui.opchannelsListWidget->setEnabled(!locked);
    ui.opchanComboBox->setEnabled(!locked);
    ui.addopBtn->setEnabled(!locked);
    ui.rmopBtn->setEnabled(!locked);
    ui.audmaxbpsSpinBox->setEnabled(!locked);
    ui.limitcmdComboBox->setEnabled(!locked);
    ui.groupBox_4->setEnabled(!locked);
    ui.addButton->setEnabled(!locked);
    ui.delButton->setEnabled(!locked);
    ui.newButton->setEnabled(!locked);
}

void UserAccountsDlg::showUserAccount(const UserAccount& useraccount)
{
    ui.usernameEdit->setText(_Q(useraccount.szUsername));
    ui.passwordEdit->setText(_Q(useraccount.szPassword));
    if(useraccount.uUserType & USERTYPE_ADMIN)
        ui.adminBtn->setChecked(useraccount.uUserType & USERTYPE_ADMIN);
    if(useraccount.uUserType & USERTYPE_DEFAULT)
        ui.defaultuserBtn->setChecked(useraccount.uUserType & USERTYPE_DEFAULT);

    ui.noteEdit->setPlainText(_Q(useraccount.szNote));
    ui.channelComboBox->lineEdit()->setText(_Q(useraccount.szInitChannel));

    // User Rights
    updateUserRights(useraccount);

    // Tab - Channel Operator

    ui.opchannelsListWidget->clear();
    for(int c = 0; c<TT_CHANNELS_OPERATOR_MAX; c++)
    {
        TTCHAR chanpath[TT_STRLEN];
        if(useraccount.autoOperatorChannels[c] &&
            TT_GetChannelPath(ttInst,
                useraccount.autoOperatorChannels[c],
                chanpath))
            ui.opchannelsListWidget->addItem(_Q(chanpath));
    }

    // Tab - Audio codec limit

    ui.audmaxbpsSpinBox->setValue(useraccount.nAudioCodecBpsLimit / 1000);

    // Tab - abuse prevention

    m_abuse = useraccount.abusePrevent;
    int i;
    switch(useraccount.abusePrevent.nCommandsLimit)
    {
    case 0:
        i = ui.limitcmdComboBox->findData(LIMITCMD_DISABLED);
        break;
    case 10:
        switch(useraccount.abusePrevent.nCommandsIntervalMSec)
        {
        case 10000:
            i = ui.limitcmdComboBox->findData(LIMITCMD_10_PER_10SEC);
            break;
        case 60000:
            i = ui.limitcmdComboBox->findData(LIMITCMD_10_PER_MINUTE);
            break;
        default:
            i = ui.limitcmdComboBox->findData(LIMITCMD_CUSTOM);
            break;
        }
        break;
    case 60:
        switch(useraccount.abusePrevent.nCommandsIntervalMSec)
        {
        case 60000:
            i = ui.limitcmdComboBox->findData(LIMITCMD_60_PER_MINUTE);
            break;
        default:
            i = ui.limitcmdComboBox->findData(LIMITCMD_CUSTOM);
            break;
        }
        break;
    default:
        switch(useraccount.abusePrevent.nCommandsIntervalMSec)
        {
        case 0:
            i = ui.limitcmdComboBox->findData(LIMITCMD_DISABLED);
            break;
        default:
            i = ui.limitcmdComboBox->findData(LIMITCMD_CUSTOM);
            break;
        }
    }

    if(i >= 0)
        ui.limitcmdComboBox->setCurrentIndex(i);
}

void UserAccountsDlg::updateUserRights(const UserAccount& useraccount)
{
    ui.multiloginBox->setChecked(useraccount.uUserRights & USERRIGHT_MULTI_LOGIN);
    ui.chnickBox->setChecked((useraccount.uUserRights & USERRIGHT_LOCKED_NICKNAME) == USERRIGHT_NONE);
    ui.viewallusersBox->setChecked(useraccount.uUserRights & USERRIGHT_VIEW_ALL_USERS);
    ui.permchannelsBox->setChecked(useraccount.uUserRights & USERRIGHT_MODIFY_CHANNELS);
    ui.tempchannelsBox->setChecked(useraccount.uUserRights & USERRIGHT_CREATE_TEMPORARY_CHANNEL);
    ui.clientbroadcastBox->setChecked(useraccount.uUserRights & USERRIGHT_TEXTMESSAGE_BROADCAST);
    ui.kickusersBox->setChecked(useraccount.uUserRights & USERRIGHT_KICK_USERS);
    ui.banusersBox->setChecked(useraccount.uUserRights & USERRIGHT_BAN_USERS);
    ui.moveusersBox->setChecked(useraccount.uUserRights & USERRIGHT_MOVE_USERS);
    ui.chanopBox->setChecked(useraccount.uUserRights & USERRIGHT_OPERATOR_ENABLE);
    ui.uploadfilesBox->setChecked(useraccount.uUserRights & USERRIGHT_UPLOAD_FILES);
    ui.downloadfilesBox->setChecked(useraccount.uUserRights & USERRIGHT_DOWNLOAD_FILES);
    ui.recordBox->setChecked(useraccount.uUserRights & USERRIGHT_RECORD_VOICE);
    ui.srvpropBox->setChecked(useraccount.uUserRights & USERRIGHT_UPDATE_SERVERPROPERTIES);
    ui.transmitvoiceBox->setChecked(useraccount.uUserRights & USERRIGHT_TRANSMIT_VOICE);
    ui.transmitvideoBox->setChecked(useraccount.uUserRights & USERRIGHT_TRANSMIT_VIDEOCAPTURE);
    ui.transmitdesktopBox->setChecked(useraccount.uUserRights & USERRIGHT_TRANSMIT_DESKTOP);
    ui.transmitdesktopaccessBox->setChecked(useraccount.uUserRights & USERRIGHT_TRANSMIT_DESKTOPINPUT);
    ui.transmitaudiofileBox->setChecked(useraccount.uUserRights & USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO);
    ui.transmitvideofileBox->setChecked(useraccount.uUserRights & USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO);

    ui.multiloginBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.chnickBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.viewallusersBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.permchannelsBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.tempchannelsBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.clientbroadcastBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.kickusersBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.banusersBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.moveusersBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.chanopBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.uploadfilesBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.downloadfilesBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.recordBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.srvpropBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.transmitvoiceBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.transmitvideoBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.transmitdesktopBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.transmitdesktopaccessBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.transmitaudiofileBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
    ui.transmitvideofileBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
}

void UserAccountsDlg::slotClearUser()
{
    ui.usernameEdit->clear();
    ui.passwordEdit->clear();
    ui.defaultuserBtn->setChecked(true);
    ui.adminBtn->setChecked(false);
    ui.noteEdit->clear();
    ui.channelComboBox->setCurrentIndex(0);
    ui.opchannelsListWidget->clear();
    ui.opchanComboBox->setCurrentIndex(0);
    ui.audmaxbpsSpinBox->setValue(0);
    ui.limitcmdComboBox->setCurrentIndex(0);
    ZERO_STRUCT(m_abuse);

    ZERO_STRUCT(m_add_user);
    slotUserTypeChanged();
    updateUserRights(m_add_user);

    ui.delButton->setEnabled(false);
}

void UserAccountsDlg::slotAddUser()
{
    if(ui.usernameEdit->text().isEmpty())
    {
        QMessageBox::StandardButton answer = 
            QMessageBox::question(this, tr("Add/Update"), 
                                  tr("Create anonymous user account?"),
                                  QMessageBox::Yes | QMessageBox::No);
        if(answer != QMessageBox::Yes)
            return;
    }

    ZERO_STRUCT(m_add_user);
    COPY_TTSTR(m_add_user.szUsername, ui.usernameEdit->text().trimmed());
    COPY_TTSTR(m_add_user.szPassword, ui.passwordEdit->text());
    if(ui.adminBtn->isChecked())
        m_add_user.uUserType = USERTYPE_ADMIN;
    else
        m_add_user.uUserType = USERTYPE_DEFAULT;
    if(ui.multiloginBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_MULTI_LOGIN;
    else
        m_add_user.uUserRights &= ~USERRIGHT_MULTI_LOGIN;
    if(ui.chnickBox->isChecked())
        m_add_user.uUserRights &= ~USERRIGHT_LOCKED_NICKNAME;
    else
        m_add_user.uUserRights |= USERRIGHT_LOCKED_NICKNAME;
    if(ui.viewallusersBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_VIEW_ALL_USERS;
    else
        m_add_user.uUserRights &= ~USERRIGHT_VIEW_ALL_USERS;
    if(ui.permchannelsBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_MODIFY_CHANNELS;
    else
        m_add_user.uUserRights &= ~USERRIGHT_MODIFY_CHANNELS;
    if(ui.tempchannelsBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_CREATE_TEMPORARY_CHANNEL;
    else
        m_add_user.uUserRights &= ~USERRIGHT_CREATE_TEMPORARY_CHANNEL;
    if(ui.clientbroadcastBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_TEXTMESSAGE_BROADCAST;
    else
        m_add_user.uUserRights &= ~USERRIGHT_TEXTMESSAGE_BROADCAST;
    if(ui.kickusersBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_KICK_USERS;
    else
        m_add_user.uUserRights &= ~USERRIGHT_KICK_USERS;
    if(ui.banusersBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_BAN_USERS;
    else
        m_add_user.uUserRights &= ~USERRIGHT_BAN_USERS;
    if(ui.moveusersBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_MOVE_USERS;
    else
        m_add_user.uUserRights &= ~USERRIGHT_MOVE_USERS;
    if(ui.chanopBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_OPERATOR_ENABLE;
    else
        m_add_user.uUserRights &= ~USERRIGHT_OPERATOR_ENABLE;
    if(ui.uploadfilesBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_UPLOAD_FILES;
    else
        m_add_user.uUserRights &= ~USERRIGHT_UPLOAD_FILES;
    if(ui.downloadfilesBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_DOWNLOAD_FILES;
    else
        m_add_user.uUserRights &= ~USERRIGHT_DOWNLOAD_FILES;
    if(ui.recordBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_RECORD_VOICE;
    else
        m_add_user.uUserRights &= ~USERRIGHT_RECORD_VOICE;
    if(ui.srvpropBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_UPDATE_SERVERPROPERTIES;
    else
        m_add_user.uUserRights &= ~USERRIGHT_UPDATE_SERVERPROPERTIES;
    if(ui.transmitvoiceBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_TRANSMIT_VOICE;
    else
        m_add_user.uUserRights &= ~USERRIGHT_TRANSMIT_VOICE;
    if(ui.transmitvideoBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_TRANSMIT_VIDEOCAPTURE;
    else
        m_add_user.uUserRights &= ~USERRIGHT_TRANSMIT_VIDEOCAPTURE;
    if(ui.transmitdesktopBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_TRANSMIT_DESKTOP;
    else
        m_add_user.uUserRights &= ~USERRIGHT_TRANSMIT_DESKTOP;
    if(ui.transmitdesktopaccessBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_TRANSMIT_DESKTOPINPUT;
    else
        m_add_user.uUserRights &= ~USERRIGHT_TRANSMIT_DESKTOPINPUT;
    if(ui.transmitaudiofileBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;
    else
        m_add_user.uUserRights &= ~USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;
    if(ui.transmitvideofileBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO;
    else
        m_add_user.uUserRights &= ~USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO;

    COPY_TTSTR(m_add_user.szNote, ui.noteEdit->toPlainText());
    COPY_TTSTR(m_add_user.szInitChannel, ui.channelComboBox->lineEdit()->text().trimmed());

    // Tab - Channel operator
    int opchan_idx = 0;
    for(int i=0;i<ui.opchannelsListWidget->count();i++)
    {
        QListWidgetItem* item = ui.opchannelsListWidget->item(i);
        if(!item)
            continue;
        int chanid = TT_GetChannelIDFromPath(ttInst, _W(item->text()));
        if(chanid>0)
            m_add_user.autoOperatorChannels[opchan_idx++] = chanid;
    }

    // Tab - Audio codec bitrate limit

    m_add_user.nAudioCodecBpsLimit = ui.audmaxbpsSpinBox->value() * 1000;

    // Tab - Abuse Prevention - Flood protection

    m_add_user.abusePrevent = m_abuse;

    m_add_cmdid = TT_DoNewUserAccount(ttInst, &m_add_user);
    lockUI(true);
}

void UserAccountsDlg::slotDelUser()
{
    auto proxySelection = ui.usersTreeView->currentIndex();
    int index = m_proxyModel->mapToSource(proxySelection).row();
    if (index < 0)
        return;

    m_del_cmdid = TT_DoDeleteUserAccount(ttInst, m_model->getUsers()[index].szUsername);
    m_del_username = _Q(m_model->getUsers()[index].szUsername);

    lockUI(true);
}

void UserAccountsDlg::slotUserSelected(const QModelIndex & index )
{
    int i = m_proxyModel->mapToSource(index).row();
    if (i < 0)
    {
        ui.delButton->setEnabled(false);
        return;
    }
    showUserAccount(m_model->getUsers()[i]);
    ui.delButton->setEnabled(true);
}

void UserAccountsDlg::slotEdited(const QString&)
{
    ui.delButton->setEnabled(false);
    ui.addButton->setEnabled(ui.usernameEdit->text().size());
}

void UserAccountsDlg::slotUserTypeChanged()
{
    if(ui.adminBtn->isChecked())
    {
        m_add_user.uUserType = USERTYPE_ADMIN;
        m_add_user.uUserRights = 0;
    }
    if(ui.defaultuserBtn->isChecked())
    {
        m_add_user.uUserType = USERTYPE_DEFAULT;
        m_add_user.uUserRights = USERRIGHT_DEFAULT;
    }
    updateUserRights(m_add_user);
}

void UserAccountsDlg::slotAddOpChannel()
{
    if(ui.opchannelsListWidget->count() + 1 > TT_CHANNELS_OPERATOR_MAX)
    {
        QMessageBox::information(this, tr("Channel Operator"),
            tr("The maximum number of channels where a user can automatically "
               "become channel operator is %1.").arg(TT_CHANNELS_OPERATOR_MAX));
        return;
    }
    if(ui.opchannelsListWidget->findItems(ui.opchanComboBox->currentText(),
                                          Qt::MatchCaseSensitive).empty())
        ui.opchannelsListWidget->addItem(ui.opchanComboBox->currentText());
}

void UserAccountsDlg::slotRemoveOpChannel()
{
    if(ui.opchannelsListWidget->hasFocus())
        delete ui.opchannelsListWidget->currentItem();
    else
    {
        QList<QListWidgetItem*> items = ui.opchannelsListWidget->findItems(ui.opchanComboBox->currentText(),
            Qt::MatchCaseSensitive);
        while(items.size())
        {
            delete *items.begin();
            items.erase(items.begin());
        }
    }
}

void UserAccountsDlg::slotCustomCmdLimit(int index)
{
    switch(ui.limitcmdComboBox->itemData(index).toInt())
    {
    case LIMITCMD_DISABLED :
        m_abuse.nCommandsIntervalMSec = m_abuse.nCommandsLimit = 0;
        break;
    case LIMITCMD_10_PER_10SEC :
        m_abuse.nCommandsLimit = 10;
        m_abuse.nCommandsIntervalMSec = 10000;
        break;
    case LIMITCMD_10_PER_MINUTE :
        m_abuse.nCommandsLimit = 10;
        m_abuse.nCommandsIntervalMSec = 60000;
        break;
    case LIMITCMD_60_PER_MINUTE :
        m_abuse.nCommandsLimit = 60;
        m_abuse.nCommandsIntervalMSec = 60000;
        break;
    case LIMITCMD_CUSTOM :
        m_abuse.nCommandsLimit =
                QInputDialog::getInt(this, tr("Limit issued commands"),
                                     tr("Number of commands to allow (0 = disabled)"),
                                     m_abuse.nCommandsLimit, 0);
        if(m_abuse.nCommandsLimit)
        {
            m_abuse.nCommandsIntervalMSec =
                    QInputDialog::getInt(this, tr("Limit issued commands"),
                                         tr("Timeframe to allow %1 commands (in seconds)")
                                         .arg(m_abuse.nCommandsLimit),
                                         m_abuse.nCommandsIntervalMSec/1000, 1);
            m_abuse.nCommandsIntervalMSec *= 1000;
        }
        break;
    default :
        Q_ASSERT(0);
    }
}

void UserAccountsDlg::slotUsernameChanged(const QString& text)
{
    ui.passwordEdit->setReadOnly(text == WEBLOGIN_FACEBOOK_USERNAME ||
        text.endsWith(WEBLOGIN_FACEBOOK_USERNAMEPOSTFIX));
}
