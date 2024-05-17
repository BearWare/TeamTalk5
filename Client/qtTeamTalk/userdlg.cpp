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
#include "settings.h"

#include <QPushButton>
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
    , m_userRightsTab(nullptr)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(APPICON));

    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_USERWINDOWPOS).toByteArray());

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    m_userrightsModel = new UserRightsModel(this);
    ui->userrightsTreeView->setModel(m_userrightsModel);

    m_userRightsTab = ui->tabWidget->widget(ui->tabWidget->indexOf(ui->tab_user_rights));

    ui->typeComboBox->addItem(tr("Default User"), USERTYPE_DEFAULT);
    ui->typeComboBox->addItem(tr("Administrator"), USERTYPE_ADMIN);
    ui->typeComboBox->addItem(tr("Disabled"), USERTYPE_NONE);

    connect(ui->typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UserDlg::slotUserTypeChanged);

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

    connect(ui->usernameEdit, &QLineEdit::textChanged, this, &UserDlg::slotUsernameChanged);

    if (type != USER_READONLY)
        connect(ui->userrightsTreeView, &QAbstractItemView::doubleClicked, this, &UserDlg::toggleUserRights);

    switch(type)
    {
    case USER_CREATE :
        setWindowTitle(tr("Add User"));
        this->setAccessibleDescription(tr("Add User on Server"));
        ui->typeComboBox->setCurrentIndex(ui->typeComboBox->findData(USERTYPE_DEFAULT));
        ui->lastEditLabel->setVisible(false);
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
        ui->typeComboBox->setEnabled(false);
        ui->usernameEdit->setReadOnly(true);
        ui->passwordEdit->setReadOnly(true);
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
    return static_cast<UserTypes>(ui->typeComboBox->currentData().toInt());
}

void UserDlg::slotUserTypeChanged()
{
    newUser.uUserType = getUserType();
    if (newUser.uUserType == USERTYPE_ADMIN)
    {
        newUser.uUserRights = USERRIGHT_NONE;
        int index = ui->tabWidget->indexOf(m_userRightsTab);
        if (index != -1)
        {
            ui->tabWidget->removeTab(index);
        }
    }
    else
    {
        newUser.uUserRights = USERRIGHT_DEFAULT;
        if (ui->tabWidget->indexOf(m_userRightsTab) == -1)
        {
            ui->tabWidget->addTab(m_userRightsTab, tr("User Rights"));
        }
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
    switch(ui->limitcmdComboBox->itemData(index).toInt())
    {
    case LIMITCMD_DISABLED:
        m_abuse.nCommandsIntervalMSec = m_abuse.nCommandsLimit = 0;
        break;
    case LIMITCMD_10_PER_10SEC:
        m_abuse.nCommandsLimit = 10;
        m_abuse.nCommandsIntervalMSec = 10000;
        break;
    case LIMITCMD_10_PER_MINUTE:
        m_abuse.nCommandsLimit = 10;
        m_abuse.nCommandsIntervalMSec = 60000;
        break;
    case LIMITCMD_60_PER_MINUTE:
        m_abuse.nCommandsLimit = 60;
        m_abuse.nCommandsIntervalMSec = 60000;
        break;
    case LIMITCMD_CUSTOM:
    {
        CustomCmdLimitDialog dlg(m_abuse.nCommandsLimit, m_abuse.nCommandsIntervalMSec / 1000, this);
        if (dlg.exec() == QDialog::Accepted)
        {
            m_abuse.nCommandsLimit = dlg.getCommandLimit();
            if (m_abuse.nCommandsLimit)
            {
                m_abuse.nCommandsIntervalMSec = dlg.getIntervalSec() * 1000;
            }
            else
            {
                m_abuse.nCommandsIntervalMSec = 0;
            }
            updateCustomLimitText(m_abuse.nCommandsLimit, m_abuse.nCommandsIntervalMSec);
        }
        break;
    }
    default:
        Q_ASSERT(0);
    }
}

void UserDlg::slotUsernameChanged()
{
    if (ui->usernameEdit->text() == WEBLOGIN_BEARWARE_USERNAME || ui->usernameEdit->text().contains(QString("@%1.dk").arg(WEBLOGIN_BEARWARE_USERNAME)))
    {
        ui->label_2->setVisible(false);
        ui->passwordEdit->setVisible(false);
    }
    else
    {
        ui->label_2->setVisible(true);
        ui->passwordEdit->setVisible(true);
    }
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

    if (m_type != USER_CREATE)
    {
        if (useraccount.uUserType & USERTYPE_ADMIN)
            ui->typeComboBox->setCurrentIndex(ui->typeComboBox->findData(USERTYPE_ADMIN));
        else if (useraccount.uUserType & USERTYPE_DEFAULT)
            ui->typeComboBox->setCurrentIndex(ui->typeComboBox->findData(USERTYPE_DEFAULT));
        else if (useraccount.uUserType == USERTYPE_NONE)
            ui->typeComboBox->setCurrentIndex(ui->typeComboBox->findData(USERTYPE_NONE));
    }
    if (useraccount.uUserType & USERTYPE_ADMIN)
    {
        int index = ui->tabWidget->indexOf(m_userRightsTab);
        if (index != -1)
        {
            ui->tabWidget->removeTab(index);
        }
    }
    else
    {
        if (ui->tabWidget->indexOf(m_userRightsTab) == -1)
        {
            ui->tabWidget->addTab(m_userRightsTab, tr("User Rights"));
        }
    }

    ui->noteEdit->setPlainText(_Q(useraccount.szNote));
    ui->channelComboBox->lineEdit()->setText(_Q(useraccount.szInitChannel));
    ui->lastEditLabel->setText(tr("Last edited: %1").arg(getFormattedDateTime(_Q(useraccount.szLastModified), "yyyy/MM/dd hh:mm")));

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
    int i = -1;  // Default value for index
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
            updateCustomLimitText(useraccount.abusePrevent.nCommandsLimit, useraccount.abusePrevent.nCommandsIntervalMSec);
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
            updateCustomLimitText(useraccount.abusePrevent.nCommandsLimit, useraccount.abusePrevent.nCommandsIntervalMSec);
            i = ui->limitcmdComboBox->findData(LIMITCMD_CUSTOM);
            break;
        }
        break;
    default:
        updateCustomLimitText(useraccount.abusePrevent.nCommandsLimit, useraccount.abusePrevent.nCommandsIntervalMSec);
        i = ui->limitcmdComboBox->findData(LIMITCMD_CUSTOM);
        break;
    }

    if(i >= 0)
        ui->limitcmdComboBox->setCurrentIndex(i);

    slotUsernameChanged();
}

void UserDlg::updateCustomLimitText(int nCommandsLimit, int nCommandsIntervalMSec)
{
    QString customText = tr("Custom (%1 commands per %2 seconds)")
                             .arg(nCommandsLimit)
                             .arg(nCommandsIntervalMSec / 1000);
    int index = ui->limitcmdComboBox->findData(LIMITCMD_CUSTOM);
    if (index != -1) {
        ui->limitcmdComboBox->setItemText(index, customText);
    }
}

CustomCmdLimitDialog::CustomCmdLimitDialog(int currentLimit, int currentIntervalSec, QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_cmdLimitSpinBox = new QSpinBox(this);
    m_cmdLimitSpinBox->setMinimum(0);
    m_cmdLimitSpinBox->setValue(currentLimit);
    m_cmdLimitSpinBox->setPrefix(tr("Command Limit: "));
    m_cmdLimitSpinBox->setAccessibleName(tr("Command Limit"));
    layout->addWidget(m_cmdLimitSpinBox);

    m_intervalSpinBox = new QSpinBox(this);
    m_intervalSpinBox->setMinimum(1);
    m_intervalSpinBox->setValue(currentIntervalSec);
    m_intervalSpinBox->setPrefix(("Interval: "));
    m_intervalSpinBox->setSuffix(tr("sec"));
    m_intervalSpinBox->setAccessibleName(("Interval"));
    layout->addWidget(m_intervalSpinBox);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    setLayout(layout);
    setWindowTitle(tr("Set Command Limits"));
}

int CustomCmdLimitDialog::getCommandLimit() const
{
    return m_cmdLimitSpinBox->value();
}

int CustomCmdLimitDialog::getIntervalSec() const
{
    return m_intervalSpinBox->value();
}
