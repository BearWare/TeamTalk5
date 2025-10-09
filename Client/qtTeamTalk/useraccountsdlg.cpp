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

#include "useraccountsdlg.h"
#include "appinfo.h"
#include "settings.h"
#include "useraccountdlg.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QKeyEvent>
#include <QSettings>
#include <QMenu>

extern TTInstance* ttInst;
extern NonDefaultSettings* ttSettings;

UserAccountsDlg::UserAccountsDlg(const useraccounts_t& useraccounts, QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
    , m_add_cmdid(0)
    , m_del_cmdid(0)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    using std::placeholders::_1;
    m_useraccountsModel = new UserAccountsModel(this, std::bind(&QHeaderView::logicalIndex, ui.usersTableView->horizontalHeader(), _1));
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_useraccountsModel);
    m_proxyModel->setSortRole(Qt::UserRole);
    ui.usersTableView->setModel(m_proxyModel);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->sort(COLUMN_INDEX_USERNAME, Qt::AscendingOrder);

#if defined(Q_OS_MAC)
    auto font = ui.usersTableView->font();
    font.setPointSize(13);
    ui.usersTableView->setFont(font);
#endif

    for(int i=0;i<useraccounts.size();i++)
        m_useraccountsModel->addRegUser(useraccounts[i], i+1 == useraccounts.size());

    for(int i=0;i<COLUMN_COUNT_USERACCOUNTS;i++)
        ui.usersTableView->resizeColumnToContents(i);

    connect(ui.addButton, &QAbstractButton::clicked, this, &UserAccountsDlg::slotAddUser);
    ui.usersTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.usersTableView, &QWidget::customContextMenuRequested,
            this, &UserAccountsDlg::slotTreeContextMenu);
    connect(ui.usersTableView, &QAbstractItemView::doubleClicked, this, &UserAccountsDlg::slotEditUser);

    ui.usersTableView->horizontalHeader()->restoreState(ttSettings->value(SETTINGS_DISPLAY_USERACCOUNTS_HEADERSIZES).toByteArray());
    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_USERACCOUNTSDLG_SIZE).toByteArray());
}

UserAccountsDlg::~UserAccountsDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_USERACCOUNTSDLG_SIZE, saveGeometry());
    ttSettings->setValue(SETTINGS_DISPLAY_USERACCOUNTS_HEADERSIZES, ui.usersTableView->horizontalHeader()->saveState());
}

void UserAccountsDlg::slotCmdSuccess(int cmdid)
{
    if(cmdid == m_add_cmdid)
    {
        m_useraccountsModel->addRegUser(m_user, true); //here we disregard that the command might fail
        m_add_cmdid = 0;
    }
    if(cmdid == m_del_cmdid)
    {
        m_useraccountsModel->delRegUser(m_del_username); //here we disregard that the command might fail
        m_del_cmdid = 0;
        m_del_username.clear();
    }
}

void UserAccountsDlg::slotCmdError(int /*error*/, int cmdid)
{
    if(cmdid == m_add_cmdid)
    {
        m_add_cmdid = 0;
    }
    if(cmdid == m_del_cmdid)
    {
        m_del_cmdid = 0;
    }
}

void UserAccountsDlg::slotAddUser()
{
    m_user = {};
    m_user.uUserRights = USERRIGHT_DEFAULT;
    UserAccountDlg dlg(UserAccountDlg::USER_CREATE, m_user, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        m_user = dlg.getUserAccount();
        m_add_cmdid = TT_DoNewUserAccount(ttInst, &m_user);
    }
}

void UserAccountsDlg::slotDelUser()
{
    auto proxySelection = ui.usersTableView->currentIndex();
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
}

void UserAccountsDlg::slotEditUser()
{
    auto proxySelection = ui.usersTableView->currentIndex();
    int index = m_proxyModel->mapToSource(proxySelection).row();
    if (index < 0)
        return;
    m_user = m_useraccountsModel->getUsers()[index];
    UserAccountDlg dlg(UserAccountDlg::USER_UPDATE, m_user, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        m_del_cmdid = TT_DoDeleteUserAccount(ttInst, m_user.szUsername);
        m_del_username = _Q(m_user.szUsername);
        m_user = dlg.getUserAccount();
        m_add_cmdid = TT_DoNewUserAccount(ttInst, &m_user);
    }
}

void UserAccountsDlg::keyPressEvent(QKeyEvent* e)
{
    if (ui.usersTableView->hasFocus())
    {
        if (e->matches(QKeySequence::Delete) || e->key() == Qt::Key_Backspace)
            slotDelUser();
        else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
            slotEditUser();
        else
            QDialog::keyPressEvent(e);
    }
    else
        QDialog::keyPressEvent(e);
}

void UserAccountsDlg::slotTreeContextMenu(const QPoint& /*point*/)
{
    QMenu menu(this);
    QMenu* sortMenu = menu.addMenu(tr("Sort By..."));
    QString asc = tr("Ascending"), desc = tr("Descending");
    QAction* sortUsername = new QAction(sortMenu);
    sortUsername->setText(tr("&Username (%1)").arg(m_proxyModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortUsername->setCheckable(true);
    const QString username = "username";
    sortUsername->setChecked((ttSettings->value(SETTINGS_DISPLAY_USERACCOUNT_SORT, SETTINGS_DISPLAY_USERACCOUNT_SORT_DEFAULT).toString() == username)?true:false);
    sortMenu->addAction(sortUsername);
    QAction* sortUserType = new QAction(sortMenu);
    sortUserType->setText(tr("User &Type (%1)").arg(m_proxyModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortUserType->setCheckable(true);
    const QString usertype = "usertype";
    sortUserType->setChecked((ttSettings->value(SETTINGS_DISPLAY_USERACCOUNT_SORT, SETTINGS_DISPLAY_USERACCOUNT_SORT_DEFAULT).toString() == usertype)?true:false);
    sortMenu->addAction(sortUserType);
    QAction* sortChannel = new QAction(sortMenu);
    sortChannel->setText(tr("&Channel (%1)").arg(m_proxyModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortChannel->setCheckable(true);
    const QString channel = "channel";
    sortChannel->setChecked((ttSettings->value(SETTINGS_DISPLAY_USERACCOUNT_SORT, SETTINGS_DISPLAY_USERACCOUNT_SORT_DEFAULT).toString() == channel)?true:false);
    sortMenu->addAction(sortChannel);
    QAction* sortModified = new QAction(sortMenu);
    sortModified->setText(tr("&Modified (%1)").arg(m_proxyModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortModified->setCheckable(true);
    const QString modified = "modified";
    sortModified->setChecked((ttSettings->value(SETTINGS_DISPLAY_USERACCOUNT_SORT, SETTINGS_DISPLAY_USERACCOUNT_SORT_DEFAULT).toString() == modified)?true:false);
    sortMenu->addAction(sortModified);
    QAction* sortLogin = new QAction(sortMenu);
    sortLogin->setText(tr("&Last Login Time (%1)").arg(m_proxyModel->sortOrder() == Qt::AscendingOrder?asc:desc));
    sortLogin->setCheckable(true);
    const QString lastLogin = "lastLogin";
    sortLogin->setChecked((ttSettings->value(SETTINGS_DISPLAY_USERACCOUNT_SORT, SETTINGS_DISPLAY_USERACCOUNT_SORT_DEFAULT).toString() == lastLogin)?true:false);
    sortMenu->addAction(sortLogin);
    QAction* addUser = menu.addAction(tr("&Create New User Account"));
    QAction* delUser = menu.addAction(tr("&Delete Selected User Account"));
    QAction* editUser = menu.addAction(tr("&Edit Selected User Account"));
    auto srcIndex = m_proxyModel->mapToSource(ui.usersTableView->currentIndex());
    delUser->setEnabled(srcIndex.isValid());
    editUser->setEnabled(srcIndex.isValid());
    if (QAction* action = menu.exec(QCursor::pos()))
    {
        auto sortToggle = m_proxyModel->sortOrder() == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        if (action == sortUsername)
        {
            ui.usersTableView->horizontalHeader()->setSortIndicator(COLUMN_INDEX_USERNAME, m_proxyModel->sortColumn() == COLUMN_INDEX_USERNAME ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValueOrClear(SETTINGS_DISPLAY_USERACCOUNT_SORT, username, SETTINGS_DISPLAY_USERACCOUNT_SORT_DEFAULT);
        }
        else if (action == sortUserType)
        {
            ui.usersTableView->horizontalHeader()->setSortIndicator(COLUMN_INDEX_USERTYPE, m_proxyModel->sortColumn() == COLUMN_INDEX_USERTYPE ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValueOrClear(SETTINGS_DISPLAY_USERACCOUNT_SORT, usertype, SETTINGS_DISPLAY_USERACCOUNT_SORT_DEFAULT);
        }
        else if (action == sortChannel)
        {
            ui.usersTableView->horizontalHeader()->setSortIndicator(COLUMN_INDEX_CHANNEL, m_proxyModel->sortColumn() == COLUMN_INDEX_CHANNEL? sortToggle : Qt::AscendingOrder);
            ttSettings->setValueOrClear(SETTINGS_DISPLAY_USERACCOUNT_SORT, channel, SETTINGS_DISPLAY_USERACCOUNT_SORT_DEFAULT);
        }
        else if (action == sortModified)
        {
            ui.usersTableView->horizontalHeader()->setSortIndicator(COLUMN_INDEX_MODIFIED, m_proxyModel->sortColumn() == COLUMN_INDEX_MODIFIED ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValueOrClear(SETTINGS_DISPLAY_USERACCOUNT_SORT, modified, SETTINGS_DISPLAY_USERACCOUNT_SORT_DEFAULT);
        }
        else if (action == sortLogin)
        {
            ui.usersTableView->horizontalHeader()->setSortIndicator(COLUMN_INDEX_LASTLOGIN, m_proxyModel->sortColumn() == COLUMN_INDEX_LASTLOGIN ? sortToggle : Qt::AscendingOrder);
            ttSettings->setValueOrClear(SETTINGS_DISPLAY_USERACCOUNT_SORT, lastLogin, SETTINGS_DISPLAY_USERACCOUNT_SORT_DEFAULT);
        }
        else if (action == addUser)
            slotAddUser();
        else if (action == delUser)
            slotDelUser();
        else if (action == editUser)
            slotEditUser();
    }
}
