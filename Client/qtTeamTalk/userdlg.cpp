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

#include "userdlg.h"
#include "ui_userdlg.h"
#include "appinfo.h"
#include "encryptionsetupdlg.h"
#include "settings.h"

#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>

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

UserDlg::UserDlg(UserDlgType type, const UserAccount& user, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UserDlg)
    , m_type(type)
    , m_user(user)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(APPICON));

    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_USERWINDOWPOS).toByteArray());

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    m_userrightsModel = new UserRightsModel(this);
    ui->userrightsTreeView->setModel(m_userrightsModel);
    
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
            ui->channelComboBox->addItem(str_channels[i]);
            if(str_channels[i].size())
                ui->opchanComboBox->addItem(str_channels[i]);
        }
    }

    ui->limitcmdComboBox->addItem(tr("Disabled"), LIMITCMD_DISABLED);
    ui->limitcmdComboBox->addItem(tr("10 commands in 10 sec."), LIMITCMD_10_PER_10SEC);
    ui->limitcmdComboBox->addItem(tr("10 commands in 1 minute"), LIMITCMD_10_PER_MINUTE);
    ui->limitcmdComboBox->addItem(tr("60 commands in 1 minute"), LIMITCMD_60_PER_MINUTE);
    ui->limitcmdComboBox->addItem(tr("Custom specified"), LIMITCMD_CUSTOM);

    connect(ui->limitcmdComboBox, QOverload<int>::of(&QComboBox::activated),
            this, &UserDlg::slotCustomCmdLimit);

    connect(ui->addopBtn, &QAbstractButton::clicked, this, &UserDlg::slotAddOpChannel);
    connect(ui->rmopBtn, &QAbstractButton::clicked, this, &UserDlg::slotRemoveOpChannel);

    connect(ui->defaultuserBtn, &QAbstractButton::clicked, this, &UserDlg::slotUserTypeChanged);
    connect(ui->adminBtn, &QAbstractButton::clicked, this, &UserDlg::slotUserTypeChanged);
    connect(ui->disableduserBtn, &QAbstractButton::clicked, this, &UserDlg::slotUserTypeChanged);

    connect(ui->usernameEdit, &QLineEdit::textChanged, this, &UserDlg::slotUsernameChanged);

    if (type != USER_READONLY)
        connect(ui->userrightsTreeView, &QAbstractItemView::doubleClicked, this, &UserDlg::toggleUserRights);

    switch(type)
    {
    case USER_CREATE :
        setWindowTitle(tr("Add User"));
        this->setAccessibleDescription(tr("Add User on Server"));
        break;
    case USER_UPDATE :
    {
        setWindowTitle(tr("Edit User"));
        this->setAccessibleDescription(tr("Edit User %1").arg(m_user.szUsername));
    }
    break;
    case USER_READONLY :
        setWindowTitle(tr("View User Information"));
        this->setAccessibleDescription(tr("View %1 Information").arg(m_user.szUsername));
        ui->usernameEdit->setReadOnly(true);
        ui->passwordEdit->setReadOnly(true);
        ui->defaultuserBtn->setEnabled(false);
        ui->adminBtn->setEnabled(false);
        ui->disableduserBtn->setEnabled(false);
        ui->noteEdit->setReadOnly(true);
        ui->channelComboBox->setEnabled(false);
        ui->opchannelsListWidget->setEnabled(false);
        ui->opchanComboBox->setEnabled(false);
        ui->addopBtn->setEnabled(false);
        ui->rmopBtn->setEnabled(false);
        ui->audmaxbpsSpinBox->setReadOnly(true);
        ui->limitcmdComboBox->setEnabled(false);
        break;
    }
    showUserAccount(m_user);
}

UserDlg::~UserDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_USERWINDOWPOS, saveGeometry());
    delete ui;
}

UserAccount UserDlg::getUser() const
{
    UserAccount newUser = m_user;
    COPY_TTSTR(newUser.szUsername, ui->usernameEdit->text().trimmed());
    COPY_TTSTR(newUser.szPassword, ui->passwordEdit->text());
    newUser.uUserType = getUserType();

    newUser.uUserRights = m_userrightsModel->getUserRights();

    COPY_TTSTR(newUser.szNote, ui->noteEdit->toPlainText());
    COPY_TTSTR(newUser.szInitChannel, ui->channelComboBox->lineEdit()->text().trimmed());

    int opchan_idx = 0;
    for(int i=0;i<ui->opchannelsListWidget->count();i++)
    {
        QListWidgetItem* item = ui->opchannelsListWidget->item(i);
        if(!item)
            continue;
        int chanid = TT_GetChannelIDFromPath(ttInst, _W(item->text()));
        if(chanid>0)
            newUser.autoOperatorChannels[opchan_idx++] = chanid;
    }

    newUser.nAudioCodecBpsLimit = ui->audmaxbpsSpinBox->value() * 1000;

    newUser.abusePrevent = m_abuse;

    return newUser;
}

void UserDlg::accept()
{
    if(ui->usernameEdit->text().isEmpty())
    {
        QMessageBox answer;
        answer.setText(tr("Create anonymous user account?"));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        Q_UNUSED(NoButton);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle(tr("Anonymous User"));
        answer.exec();
        if(answer.clickedButton() != YesButton)
            return;
    }
    
    QDialog::accept();
}

void UserDlg::updateUserRights(const UserAccount& useraccount)
{
    m_userrightsModel->setUserRights(useraccount.uUserType, useraccount.uUserRights);
}

void UserDlg::toggleUserRights(const QModelIndex &index)
{
    auto events = m_userrightsModel->getUserRights();
    UserRight e = UserRight(index.internalId());
    if (e & events)
        m_userrightsModel->setUserRights(getUserType(), events & ~e);
    else
        m_userrightsModel->setUserRights(getUserType(), events | e);
}

UserTypes UserDlg::getUserType() const
{
    if (ui->adminBtn->isChecked())
        return USERTYPE_ADMIN;
    else if (ui->defaultuserBtn->isChecked())
        return USERTYPE_DEFAULT;
    else if (ui->disableduserBtn->isChecked())
        return USERTYPE_NONE;
    else
        return USERTYPE_NONE;
}

void UserDlg::slotUserTypeChanged()
{
    newUser.uUserType = getUserType();
    if (ui->adminBtn->isChecked())
    {
        newUser.uUserRights = USERRIGHT_NONE;
    }
    else if (ui->defaultuserBtn->isChecked())
    {
        newUser.uUserRights = USERRIGHT_DEFAULT;
    }
    else
    {
        newUser.uUserRights = USERRIGHT_DEFAULT;
    }
    updateUserRights(newUser);
}

void UserDlg::slotAddOpChannel()
{
    if(ui->opchannelsListWidget->count() + 1 > TT_CHANNELS_OPERATOR_MAX)
    {
        QMessageBox::information(this, tr("Channel Operator"),
            tr("The maximum number of channels where a user can automatically "
               "become channel operator is %1.").arg(TT_CHANNELS_OPERATOR_MAX));
        return;
    }
    if(ui->opchannelsListWidget->findItems(ui->opchanComboBox->currentText(),
                                          Qt::MatchCaseSensitive).empty())
        ui->opchannelsListWidget->addItem(ui->opchanComboBox->currentText());
}

void UserDlg::slotRemoveOpChannel()
{
    if(ui->opchannelsListWidget->hasFocus())
        delete ui->opchannelsListWidget->currentItem();
    else
    {
        QList<QListWidgetItem*> items = ui->opchannelsListWidget->findItems(ui->opchanComboBox->currentText(),
            Qt::MatchCaseSensitive);
        while(items.size())
        {
            delete *items.begin();
            items.erase(items.begin());
        }
    }
}

void UserDlg::slotCustomCmdLimit(int index)
{
    QInputDialog inputDialog;
    switch(ui->limitcmdComboBox->itemData(index).toInt())
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

void UserDlg::slotUsernameChanged(const QString& /*text*/)
{
}

void UserDlg::keyPressEvent(QKeyEvent* e)
{
    if (ui->tabWidget->hasFocus())
    {
        if (e->key() == Qt::Key_Home && ui->tabWidget->currentIndex() != 0)
            ui->tabWidget->setCurrentIndex(0);
        else if (e->key() == Qt::Key_End && ui->tabWidget->currentIndex() != ui->tabWidget->count())
            ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    }
    QDialog::keyPressEvent(e);
}

void UserDlg::showUserAccount(const UserAccount& useraccount)
{
    ui->usernameEdit->setText(_Q(useraccount.szUsername));
    ui->passwordEdit->setText(_Q(useraccount.szPassword));
    if (useraccount.uUserType & USERTYPE_ADMIN)
        ui->adminBtn->setChecked(useraccount.uUserType & USERTYPE_ADMIN);
    else if (useraccount.uUserType & USERTYPE_DEFAULT)
        ui->defaultuserBtn->setChecked(useraccount.uUserType & USERTYPE_DEFAULT);
    else if (useraccount.uUserType == USERTYPE_NONE)
        ui->disableduserBtn->setChecked(true);

    ui->noteEdit->setPlainText(_Q(useraccount.szNote));
    ui->channelComboBox->lineEdit()->setText(_Q(useraccount.szInitChannel));

    updateUserRights(useraccount);

    ui->opchannelsListWidget->clear();
    for(int c = 0; c<TT_CHANNELS_OPERATOR_MAX; c++)
    {
        TTCHAR chanpath[TT_STRLEN];
        if(useraccount.autoOperatorChannels[c] &&
            TT_GetChannelPath(ttInst,
                useraccount.autoOperatorChannels[c],
                chanpath))
            ui->opchannelsListWidget->addItem(_Q(chanpath));
    }

    ui->audmaxbpsSpinBox->setValue(useraccount.nAudioCodecBpsLimit / 1000);

    m_abuse = useraccount.abusePrevent;
    int i;
    switch(useraccount.abusePrevent.nCommandsLimit)
    {
    case 0:
        i = ui->limitcmdComboBox->findData(LIMITCMD_DISABLED);
        break;
    case 10:
        switch(useraccount.abusePrevent.nCommandsIntervalMSec)
        {
        case 10000:
            i = ui->limitcmdComboBox->findData(LIMITCMD_10_PER_10SEC);
            break;
        case 60000:
            i = ui->limitcmdComboBox->findData(LIMITCMD_10_PER_MINUTE);
            break;
        default:
            i = ui->limitcmdComboBox->findData(LIMITCMD_CUSTOM);
            break;
        }
        break;
    case 60:
        switch(useraccount.abusePrevent.nCommandsIntervalMSec)
        {
        case 60000:
            i = ui->limitcmdComboBox->findData(LIMITCMD_60_PER_MINUTE);
            break;
        default:
            i = ui->limitcmdComboBox->findData(LIMITCMD_CUSTOM);
            break;
        }
        break;
    default:
        switch(useraccount.abusePrevent.nCommandsIntervalMSec)
        {
        case 0:
            i = ui->limitcmdComboBox->findData(LIMITCMD_DISABLED);
            break;
        default:
            i = ui->limitcmdComboBox->findData(LIMITCMD_CUSTOM);
            break;
        }
    }

    if(i >= 0)
        ui->limitcmdComboBox->setCurrentIndex(i);
}
