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

#include "moveusersdlg.h"
#include "utilui.h"
#include "appinfo.h"

extern TTInstance* ttInst;

MoveUsersDlg::MoveUsersDlg(const QVector<int>& userIds, const QVector<int>& channelIds, QWidget *parent)
    : QDialog(parent), userIds(userIds), channelIds(channelIds)
{
    setupUi();
    populateUsers();
    populateChannels();
}

void MoveUsersDlg::setupUi()
{
    setWindowTitle(tr("Move Users"));
    setWindowIcon(QIcon(APPICON));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QGroupBox* usersGroup = new QGroupBox(tr("Users"), this);
    QVBoxLayout* usersLayout = new QVBoxLayout(usersGroup);
    for (int i = 0; i < userIds.size(); ++i)
    {
        QCheckBox* userChkBox = new QCheckBox(this);
        usersChkBox.append(userChkBox);
        usersLayout->addWidget(userChkBox);
    }
    mainLayout->addWidget(usersGroup);

    QLabel* channelLabel = new QLabel(tr("&Channel"), this);
    userCombobox = new QComboBox(this);
    channelLabel->setBuddy(userCombobox);
    mainLayout->addWidget(channelLabel);
    mainLayout->addWidget(userCombobox);

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    QPushButton* moveBtn = new QPushButton(tr("Move Users"), this);
    QPushButton* cancelBtn = new QPushButton(tr("Cancel"), this);
    buttonsLayout->addWidget(moveBtn);
    buttonsLayout->addWidget(cancelBtn);
    mainLayout->addLayout(buttonsLayout);

    connect(moveBtn, &QPushButton::clicked, this, &MoveUsersDlg::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &MoveUsersDlg::reject);
}

void MoveUsersDlg::populateUsers()
{
    for (int i = 0; i < userIds.size(); ++i)
    {
        User u;
        if (TT_GetUser(ttInst, userIds[i], &u))
            usersChkBox[i]->setText(getDisplayName(u));
    }
}

void MoveUsersDlg::populateChannels()
{
    for (int channelId : channelIds)
    {
        Channel c;
        if (TT_GetChannel(ttInst, channelId, &c))
            userCombobox->addItem((channelId == TT_GetRootChannelID(ttInst)?tr("Root Channel"):_Q(c.szName)), channelId);
    }
}

QVector<int> MoveUsersDlg::getSelectedUserIds() const
{
    QVector<int> selectedUserIds;
    for (int i = 0; i < usersChkBox.size(); ++i)
    {
        if (usersChkBox[i]->isChecked())
            selectedUserIds.append(userIds[i]);
    }
    return selectedUserIds;
}

int MoveUsersDlg::getSelectedChannelId() const
{
    return userCombobox->currentData().toInt();
}
