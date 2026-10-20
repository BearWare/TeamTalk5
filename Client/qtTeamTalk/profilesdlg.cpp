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

#include "profilesdlg.h"
#include "appinfo.h"
#include "settings.h"

#include <QApplication>
#include <QFile>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>

extern NonDefaultSettings* ttSettings;

namespace
{
    const int MAX_PROFILES = 16;
}

ProfilesDlg::ProfilesDlg(QWidget* parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));

    using std::placeholders::_1;
    m_profilesModel = new ProfilesModel(this, std::bind(&QHeaderView::logicalIndex, ui.profilesTableView->horizontalHeader(), _1));
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_profilesModel);
    m_proxyModel->setSortRole(Qt::UserRole);
    ui.profilesTableView->setModel(m_proxyModel);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->sort(COLUMN_INDEX_PROFILE_NAME, Qt::AscendingOrder);

#if defined(Q_OS_MAC)
    auto font = ui.profilesTableView->font();
    font.setPointSize(13);
    ui.profilesTableView->setFont(font);
#endif

    loadProfiles();

    for(int i=0;i<COLUMN_COUNT_PROFILES;i++)
        ui.profilesTableView->resizeColumnToContents(i);

    connect(ui.addButton, &QAbstractButton::clicked, this, &ProfilesDlg::slotNewProfile);
    connect(ui.currentProfileButton, &QAbstractButton::clicked, this, &ProfilesDlg::slotCurrentProfile);
    ui.profilesTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.profilesTableView, &QWidget::customContextMenuRequested,
            this, &ProfilesDlg::slotTableContextMenu);
    connect(ui.profilesTableView, &QAbstractItemView::doubleClicked, this, &ProfilesDlg::slotOpenProfile);

    ui.profilesTableView->horizontalHeader()->restoreState(ttSettings->value(SETTINGS_DISPLAY_PROFILES_HEADERSIZES).toByteArray());
    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_PROFILESDLG_SIZE).toByteArray());
}

ProfilesDlg::~ProfilesDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_PROFILESDLG_SIZE, saveGeometry());
    ttSettings->setValue(SETTINGS_DISPLAY_PROFILES_HEADERSIZES, ui.profilesTableView->horizontalHeader()->saveState());
}

void ProfilesDlg::loadProfiles()
{
    m_baseinipath = ttSettings->fileName();

    if(ttSettings->value(SETTINGS_GENERAL_PROFILENAME).toString().size())
        m_baseinipath.remove(QRegularExpression("\\.\\d{1,2}$"));

    m_freeno = -1;
    for (int i = 1;i <= MAX_PROFILES;i++)
    {
        QString inifile = QString("%1.%2").arg(m_baseinipath).arg(i);
        if(QFile::exists(inifile))
        {
            QSettings settings(inifile, QSettings::IniFormat, this);
            ProfileEntry profile;
            profile.name = settings.value(SETTINGS_GENERAL_PROFILENAME).toString();
            profile.inipath = inifile;
            m_profilesModel->addProfile(profile);
        }
        else if(m_freeno < 0)
            m_freeno = i;
    }
}

void ProfilesDlg::launchInstance(const QString& inipath, bool noconnect)
{
    QString path = QApplication::applicationFilePath();
    QStringList args;
    args.push_back("-cfg");
    args.push_back(inipath);
    if(noconnect)
        args.push_back("-noconnect");

#if defined(_DEBUG)
    QProcess::startDetached(path, args);
#else
    QProcess::startDetached(path, args, QApplication::applicationDirPath());
#endif
}

void ProfilesDlg::slotNewProfile()
{
    if(m_freeno < 0)
    {
        QMessageBox::information(this, tr("New Profile"), tr("Maximum number of profiles reached."));
        return;
    }

    QInputDialog inputDialog(this);
    inputDialog.setOkButtonText(tr("&OK"));
    inputDialog.setCancelButtonText(tr("&Cancel"));
    inputDialog.setInputMode(QInputDialog::TextInput);
    inputDialog.setTextValue(QString(tr("Profile %1")).arg(m_freeno));
    inputDialog.setWindowTitle(tr("New Profile"));
    inputDialog.setLabelText(tr("Profile name"));
    if(inputDialog.exec() != QDialog::Accepted)
        return;

    QString newname = inputDialog.textValue();
    if(newname.isEmpty())
        return;

    QString inipath = QString("%1.%2").arg(m_baseinipath).arg(m_freeno);
    QSettings settings(inipath, QSettings::IniFormat, this);
    settings.setValue(SETTINGS_GENERAL_PROFILENAME, newname);
    settings.sync();

    launchInstance(inipath, false);
    accept();
}

void ProfilesDlg::slotCurrentProfile()
{
    launchInstance(ttSettings->fileName(), true);
    accept();
}

void ProfilesDlg::slotOpenProfile()
{
    auto proxySelection = ui.profilesTableView->currentIndex();
    int index = m_proxyModel->mapToSource(proxySelection).row();
    if (index < 0)
        return;

    const ProfileEntry& profile = m_profilesModel->getProfiles()[index];
    launchInstance(profile.inipath, false);
    accept();
}

void ProfilesDlg::slotEditProfile()
{
    auto proxySelection = ui.profilesTableView->currentIndex();
    int index = m_proxyModel->mapToSource(proxySelection).row();
    if (index < 0)
        return;

    const ProfileEntry& profile = m_profilesModel->getProfiles()[index];

    QInputDialog inputDialog(this);
    inputDialog.setOkButtonText(tr("&OK"));
    inputDialog.setCancelButtonText(tr("&Cancel"));
    inputDialog.setInputMode(QInputDialog::TextInput);
    inputDialog.setTextValue(profile.name);
    inputDialog.setWindowTitle(tr("Edit Profile"));
    inputDialog.setLabelText(tr("Profile name"));
    if(inputDialog.exec() != QDialog::Accepted)
        return;

    QString newname = inputDialog.textValue();
    if(newname.isEmpty())
        return;

    QSettings settings(profile.inipath, QSettings::IniFormat, this);
    settings.setValue(SETTINGS_GENERAL_PROFILENAME, newname);
    settings.sync();

    m_profilesModel->renameProfile(index, newname);
}

void ProfilesDlg::slotDeleteProfile()
{
    auto proxySelection = ui.profilesTableView->currentIndex();
    int index = m_proxyModel->mapToSource(proxySelection).row();
    if (index < 0)
        return;

    const ProfileEntry& profile = m_profilesModel->getProfiles()[index];
    if(profile.inipath == ttSettings->fileName())
        return;

    QMessageBox answer;
    answer.setText(tr("Are you sure you want to delete profile \"%1\"?").arg(profile.name));
    QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
    QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
    Q_UNUSED(YesButton);
    answer.setIcon(QMessageBox::Information);
    answer.setWindowTitle(tr("Delete profile"));
    answer.exec();
    if(answer.clickedButton() == NoButton)
        return;

    QFile::remove(profile.inipath);

    QString deleted_inipath = profile.inipath;
    m_profilesModel->delProfile(index);

    QRegularExpressionMatch match = QRegularExpression("\\.(\\d{1,2})$").match(deleted_inipath);
    if(match.hasMatch())
    {
        int freedno = match.captured(1).toInt();
        if(m_freeno < 0 || freedno < m_freeno)
            m_freeno = freedno;
    }
}

void ProfilesDlg::keyPressEvent(QKeyEvent* e)
{
    if (ui.profilesTableView->hasFocus())
    {
        if (e->matches(QKeySequence::Delete) || e->key() == Qt::Key_Backspace)
            slotDeleteProfile();
        else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
            slotOpenProfile();
        else
            QDialog::keyPressEvent(e);
    }
    else
        QDialog::keyPressEvent(e);
}

void ProfilesDlg::slotTableContextMenu(const QPoint& /*point*/)
{
    QMenu menu(this);
    QAction* openProfile = menu.addAction(tr("&Open Profile"));
    QAction* editProfile = menu.addAction(tr("&Edit Profile"));
    QAction* delProfile = menu.addAction(tr("&Delete Profile"));

    auto srcIndex = m_proxyModel->mapToSource(ui.profilesTableView->currentIndex());
    bool valid = srcIndex.isValid();
    bool isactive = valid && m_profilesModel->getProfiles()[srcIndex.row()].inipath == ttSettings->fileName();

    openProfile->setEnabled(valid);
    editProfile->setEnabled(valid && !isactive);
    delProfile->setEnabled(valid && !isactive);

    if (QAction* action = menu.exec(QCursor::pos()))
    {
        if (action == openProfile)
            slotOpenProfile();
        else if (action == editProfile)
            slotEditProfile();
        else if (action == delProfile)
            slotDeleteProfile();
    }
}
