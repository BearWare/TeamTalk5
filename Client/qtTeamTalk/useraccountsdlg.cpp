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
#include "settings.h"
#include "utilui.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QKeyEvent>
#include <QSettings>
#include <QMenu>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

enum
{
    COLUMN_INDEX_USERNAME,
    COLUMN_INDEX_PASSWORD,
    COLUMN_INDEX_USERTYPE,
    COLUMN_INDEX_NOTE,
    COLUMN_INDEX_CHANNEL,
    COLUMN_INDEX_MODIFIED,
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
            case COLUMN_INDEX_MODIFIED : return tr("Modified");
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
            else if(m_users[index.row()].uUserType & USERTYPE_DEFAULT)
                return tr("Default User");
            else if(m_users[index.row()].uUserType == USERTYPE_NONE)
                return tr("Disabled");
            else
                return tr("Unknown");
        case COLUMN_INDEX_NOTE :
            return _Q(m_users[index.row()].szNote);
        case COLUMN_INDEX_CHANNEL :
            return _Q(m_users[index.row()].szInitChannel);
        case COLUMN_INDEX_MODIFIED :
            return _Q(m_users[index.row()].szLastModified);
        }
        break;
    case Qt::AccessibleTextRole :
    {
        QString result;
        if(m_users[index.row()].uUserType & USERTYPE_ADMIN)
            result = tr("Administrator");
        else if(m_users[index.row()].uUserType & USERTYPE_DEFAULT)
            result = tr("Default User");
        else if(m_users[index.row()].uUserType == USERTYPE_NONE)
            result = tr("Disabled");
        else
            result = tr("Unknown");
        return QString(tr("Username: %1, Password: %2, Type: %3, Note: %4, Initial channel: %5, Modified: %6").arg(_Q(m_users[index.row()].szUsername)).arg(_Q(m_users[index.row()].szPassword)).arg(result).arg(_Q(m_users[index.row()].szNote)).arg(_Q(m_users[index.row()].szInitChannel)).arg(_Q(m_users[index.row()].szLastModified)));
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
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->sort(COLUMN_INDEX_USERNAME, Qt::AscendingOrder);
    ui.checkBox->hide();

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

    connect(ui.limitcmdComboBox, QOverload<int>::of(&QComboBox::activated),
            this, &UserAccountsDlg::slotCustomCmdLimit);

    for(int i=0;i<useraccounts.size();i++)
        m_model->addRegUser(useraccounts[i], i+1 == useraccounts.size());

    for(int i=0;i<COLUMN_COUNT_USERACCOUNTS;i++)
        ui.usersTreeView->resizeColumnToContents(i);

    connect(ui.addopBtn, &QAbstractButton::clicked, this, &UserAccountsDlg::slotAddOpChannel);
    connect(ui.rmopBtn, &QAbstractButton::clicked, this, &UserAccountsDlg::slotRemoveOpChannel);
    connect(ui.newButton, &QAbstractButton::clicked, this, &UserAccountsDlg::slotClearUser);
    connect(ui.addButton, &QAbstractButton::clicked, this, &UserAccountsDlg::slotAddUser);
    connect(ui.closeBtn, &QAbstractButton::clicked, this, &QWidget::close);
    connect(ui.defaultuserBtn, &QAbstractButton::clicked, this, &UserAccountsDlg::slotUserTypeChanged);
    connect(ui.adminBtn, &QAbstractButton::clicked, this, &UserAccountsDlg::slotUserTypeChanged);
    connect(ui.disableduserBtn, &QAbstractButton::clicked, this, &UserAccountsDlg::slotUserTypeChanged);
    connect(ui.usernameEdit, &QLineEdit::textChanged, this, &UserAccountsDlg::slotUsernameChanged);
    connect(ui.usersTreeView, &QAbstractItemView::activated, this, &UserAccountsDlg::slotUserSelected);
    connect(ui.usersTreeView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &UserAccountsDlg::slotUserSelected);
    ui.usersTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.usersTreeView, &QWidget::customContextMenuRequested,
            this, &UserAccountsDlg::slotTreeContextMenu);

    if(m_uad == UAD_READONLY)
    {
        if(useraccounts.size() == 1)
            showUserAccount(useraccounts[0]);
        lockUI(true);
    }
    else
        slotClearUser();
    ui.usersTreeView->header()->restoreState(ttSettings->value(SETTINGS_DISPLAY_USERACCOUNTS_HEADERSIZES).toByteArray());
    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_USERACCOUNTSDLG_SIZE).toByteArray());
}

UserAccountsDlg::~UserAccountsDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_USERACCOUNTSDLG_SIZE, saveGeometry());
    ttSettings->setValue(SETTINGS_DISPLAY_USERACCOUNTS_HEADERSIZES, ui.usersTreeView->header()->saveState());
}

void UserAccountsDlg::slotCmdSuccess(int cmdid)
{
    if(cmdid == m_add_cmdid)
    {
        m_model->addRegUser(m_add_user, true); //here we disregard that the command might fail
        m_add_cmdid = 0;
        m_add_user = {};
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
        m_add_user = {};
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
    //ui.usersTreeView->setEnabled(!locked);
    ui.usernameEdit->setEnabled(!locked);
    ui.passwordEdit->setEnabled(!locked);
    ui.defaultuserBtn->setEnabled(!locked);
    ui.adminBtn->setEnabled(!locked);
    ui.disableduserBtn->setEnabled(!locked);
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
    ui.newButton->setEnabled(!locked);
}

void UserAccountsDlg::showUserAccount(const UserAccount& useraccount)
{
    ui.usernameEdit->setText(_Q(useraccount.szUsername));
    ui.passwordEdit->setText(_Q(useraccount.szPassword));
    if (useraccount.uUserType & USERTYPE_ADMIN)
        ui.adminBtn->setChecked(useraccount.uUserType & USERTYPE_ADMIN);
    else if (useraccount.uUserType & USERTYPE_DEFAULT)
        ui.defaultuserBtn->setChecked(useraccount.uUserType & USERTYPE_DEFAULT);
    else if (useraccount.uUserType == USERTYPE_NONE)
        ui.disableduserBtn->setChecked(true);

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
    ui.viewhiddenchanBox->setChecked(useraccount.uUserRights & USERRIGHT_VIEW_HIDDEN_CHANNELS);
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
    ui.viewhiddenchanBox->setEnabled((useraccount.uUserType & USERTYPE_ADMIN) == USERTYPE_NONE);
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
    m_abuse = {};

    m_add_user = {};
    slotUserTypeChanged();
    updateUserRights(m_add_user);

}

void UserAccountsDlg::slotAddUser()
{
    if(ui.usernameEdit->text().isEmpty())
    {
        QMessageBox answer;
        answer.setText(tr("Create anonymous user account?"));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        Q_UNUSED(NoButton);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle(tr("Add/Update"));
        answer.exec();
        if(answer.clickedButton() != YesButton)
            return;
    }

    m_add_user = {};
    COPY_TTSTR(m_add_user.szUsername, ui.usernameEdit->text().trimmed());
    COPY_TTSTR(m_add_user.szPassword, ui.passwordEdit->text());
    if (ui.adminBtn->isChecked())
        m_add_user.uUserType = USERTYPE_ADMIN;
    else if (ui.defaultuserBtn->isChecked())
        m_add_user.uUserType = USERTYPE_DEFAULT;
    else if (ui.disableduserBtn->isChecked())
        m_add_user.uUserType = USERTYPE_NONE;

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
    if (ui.viewhiddenchanBox->isChecked())
        m_add_user.uUserRights |= USERRIGHT_VIEW_HIDDEN_CHANNELS;
    else
        m_add_user.uUserRights &= ~USERRIGHT_VIEW_HIDDEN_CHANNELS;
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
    QMessageBox answer;
    answer.setText(tr("Are you sure you want to delete user \"%1\"?").arg(_Q(m_model->getUsers()[index].szUsername)));
    QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
    QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
    Q_UNUSED(YesButton);
    answer.setIcon(QMessageBox::Information);
    answer.setWindowTitle(tr("Delete user"));
    answer.exec();
    if(answer.clickedButton() == NoButton)
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
        return;
    }
    showUserAccount(m_model->getUsers()[i]);
}

void UserAccountsDlg::slotEdited(const QString&)
{
    ui.addButton->setEnabled(ui.usernameEdit->text().size());
}

void UserAccountsDlg::slotUserTypeChanged()
{
    if (ui.adminBtn->isChecked())
    {
        m_add_user.uUserType = USERTYPE_ADMIN;
        m_add_user.uUserRights = USERRIGHT_NONE;
    }
    else if (ui.defaultuserBtn->isChecked())
    {
        m_add_user.uUserType = USERTYPE_DEFAULT;
        m_add_user.uUserRights = USERRIGHT_DEFAULT;
    }
    else
    {
        m_add_user.uUserType = USERTYPE_NONE;
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
    QInputDialog inputDialog;
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
        inputDialog.setOkButtonText(tr("&Ok"));
        inputDialog.setCancelButtonText(tr("&Cancel"));
        inputDialog.setInputMode(QInputDialog::IntInput);
        inputDialog.setIntValue(m_abuse.nCommandsLimit);
        inputDialog.setWindowTitle(tr("Limit issued commands"));
        inputDialog.setLabelText(tr("Number of commands to allow (0 = disabled)"));
        inputDialog.setIntMinimum(0);
        inputDialog.exec();
        m_abuse.nCommandsLimit = inputDialog.intValue();
        if(m_abuse.nCommandsLimit)
        {
            inputDialog.setIntValue(m_abuse.nCommandsIntervalMSec/1000);
            inputDialog.setIntMinimum(1);
            inputDialog.setWindowTitle(tr("Limit issued commands"));
            inputDialog.setLabelText(tr("Timeframe to allow %1 commands (in seconds)").arg(m_abuse.nCommandsLimit));
            inputDialog.exec();
            m_abuse.nCommandsIntervalMSec = inputDialog.intValue();
            m_abuse.nCommandsIntervalMSec *= 1000;
        }
        break;
    default :
        Q_ASSERT(0);
    }
}

void UserAccountsDlg::slotUsernameChanged(const QString& /*text*/)
{
}

void UserAccountsDlg::keyPressEvent(QKeyEvent* e)
{
    if (ui.tabWidget->hasFocus())
    {
        if (e->key() == Qt::Key_Home && ui.tabWidget->currentIndex() != 0)
            ui.tabWidget->setCurrentIndex(0);
        else if (e->key() == Qt::Key_End && ui.tabWidget->currentIndex() != ui.tabWidget->count())
            ui.tabWidget->setCurrentIndex(ui.tabWidget->count()-1);
    }
    QDialog::keyPressEvent(e);
}

void UserAccountsDlg::slotTreeContextMenu(const QPoint& /*point*/)
{
    QMenu menu(this);
    QMenu* sortMenu = menu.addMenu(tr("Sort By..."));
    QAction* sortUsername = sortMenu->addAction(tr("&Username"));
    QAction* sortUserType = sortMenu->addAction(tr("User &Type"));
    QAction* sortChannel = sortMenu->addAction(tr("&Channel"));
    QAction* sortModified = sortMenu->addAction(tr("&Modified"));
    QAction* delUser = menu.addAction(tr("&Delete Selected User"));
    if (QAction* action = menu.exec(QCursor::pos()))
    {
        auto sortToggle = m_proxyModel->sortOrder() == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        if (action == sortUsername)
            m_proxyModel->sort(COLUMN_INDEX_USERNAME, m_proxyModel->sortColumn() == COLUMN_INDEX_USERNAME ? sortToggle : Qt::AscendingOrder);
        else if (action == sortUserType)
            m_proxyModel->sort(COLUMN_INDEX_USERTYPE, m_proxyModel->sortColumn() == COLUMN_INDEX_USERTYPE ? sortToggle : Qt::AscendingOrder);
        else if (action == sortChannel)
            m_proxyModel->sort(COLUMN_INDEX_CHANNEL, m_proxyModel->sortColumn() == COLUMN_INDEX_CHANNEL? sortToggle : Qt::AscendingOrder);
        else if (action == sortModified)
            m_proxyModel->sort(COLUMN_INDEX_MODIFIED, m_proxyModel->sortColumn() == COLUMN_INDEX_MODIFIED ? sortToggle : Qt::AscendingOrder);
        else if (action == delUser)
            emit(slotDelUser());
        ttSettings->setValue(SETTINGS_DISPLAY_USERACCOUNTS_HEADERSIZES, ui.usersTreeView->header()->saveState());
    }
}
