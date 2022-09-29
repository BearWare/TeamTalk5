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
#include "settings.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QKeyEvent>
#include <QSettings>
#include <QMenu>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

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

    m_useraccountsModel = new UserAccountsModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_useraccountsModel);
    ui.usersTreeView->setModel(m_proxyModel);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->sort(COLUMN_INDEX_USERNAME, Qt::AscendingOrder);

    m_userrightsModel = new UserRightsModel(this);
    ui.userrightsTreeView->setModel(m_userrightsModel);

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
        m_useraccountsModel->addRegUser(useraccounts[i], i+1 == useraccounts.size());

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
    {
        connect(ui.userrightsTreeView, &QAbstractItemView::doubleClicked, this, &UserAccountsDlg::toggleUserRights);
        slotClearUser();
    }
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
        m_useraccountsModel->addRegUser(m_add_user, true); //here we disregard that the command might fail
        m_add_cmdid = 0;
        m_add_user = {};
        lockUI(false);
    }
    if(cmdid == m_del_cmdid)
    {
        m_useraccountsModel->delRegUser(m_del_username); //here we disregard that the command might fail
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
    m_userrightsModel->setUserRights(useraccount.uUserType, useraccount.uUserRights);
}

void UserAccountsDlg::toggleUserRights(const QModelIndex &index)
{
    auto events = m_userrightsModel->getUserRights();
    UserRight e = UserRight(index.internalId());
    if (e & events)
        m_userrightsModel->setUserRights(getUserType(), events & ~e);
    else
        m_userrightsModel->setUserRights(getUserType(), events | e);

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
    m_add_user.uUserType = getUserType();

    m_add_user.uUserRights = m_userrightsModel->getUserRights();

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
    answer.setText(tr("Are you sure you want to delete user \"%1\"?").arg(_Q(m_useraccountsModel->getUsers()[index].szUsername)));
    QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
    QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
    Q_UNUSED(YesButton);
    answer.setIcon(QMessageBox::Information);
    answer.setWindowTitle(tr("Delete user"));
    answer.exec();
    if(answer.clickedButton() == NoButton)
        return;
    m_del_cmdid = TT_DoDeleteUserAccount(ttInst, m_useraccountsModel->getUsers()[index].szUsername);
    m_del_username = _Q(m_useraccountsModel->getUsers()[index].szUsername);

    lockUI(true);
}

void UserAccountsDlg::slotUserSelected(const QModelIndex & index )
{
    int i = m_proxyModel->mapToSource(index).row();
    if (i < 0)
    {
        return;
    }
    showUserAccount(m_useraccountsModel->getUsers()[i]);
}

void UserAccountsDlg::slotEdited(const QString&)
{
    ui.addButton->setEnabled(ui.usernameEdit->text().size());
}

UserTypes UserAccountsDlg::getUserType()
{
    if (ui.adminBtn->isChecked())
        return USERTYPE_ADMIN;
    else if (ui.defaultuserBtn->isChecked())
        return USERTYPE_DEFAULT;
    else if (ui.disableduserBtn->isChecked())
        return USERTYPE_NONE;
    else
        return USERTYPE_NONE;
}

void UserAccountsDlg::slotUserTypeChanged()
{
    m_add_user.uUserType = getUserType();
    if (ui.adminBtn->isChecked())
    {
        m_add_user.uUserRights = USERRIGHT_NONE;
    }
    else if (ui.defaultuserBtn->isChecked())
    {
        m_add_user.uUserRights = USERRIGHT_DEFAULT;
    }
    else
    {
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
        inputDialog.setOkButtonText(tr("&OK"));
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
    else if (ui.usersTreeView->hasFocus())
    {
        if (e->matches(QKeySequence::Delete) || e->key() == Qt::Key_Backspace)
        {
            emit(slotDelUser());
        }
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
            ui.usersTreeView->header()->setSortIndicator(COLUMN_INDEX_USERNAME, m_proxyModel->sortColumn() == COLUMN_INDEX_USERNAME ? sortToggle : Qt::AscendingOrder);
        else if (action == sortUserType)
            ui.usersTreeView->header()->setSortIndicator(COLUMN_INDEX_USERTYPE, m_proxyModel->sortColumn() == COLUMN_INDEX_USERTYPE ? sortToggle : Qt::AscendingOrder);
        else if (action == sortChannel)
            ui.usersTreeView->header()->setSortIndicator(COLUMN_INDEX_CHANNEL, m_proxyModel->sortColumn() == COLUMN_INDEX_CHANNEL? sortToggle : Qt::AscendingOrder);
        else if (action == sortModified)
            ui.usersTreeView->header()->setSortIndicator(COLUMN_INDEX_MODIFIED, m_proxyModel->sortColumn() == COLUMN_INDEX_MODIFIED ? sortToggle : Qt::AscendingOrder);
        else if (action == delUser)
            emit(slotDelUser());
    }
}
