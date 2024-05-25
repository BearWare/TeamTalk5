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

#include "serverpropertiesdlg.h"
#include "appinfo.h"
#include "serverlogeventsmodel.h"
#include "settings.h"

#include <QMessageBox>
#include <QPushButton>
#include <QMenu>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

ServerPropertiesDlg::ServerPropertiesDlg(QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_SERVERPROPERTIESWINDOWPOS).toByteArray());

    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&OK"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    ui.serverlogTableView->horizontalHeader()->restoreState(ttSettings->value(SETTINGS_DISPLAY_SERVERLOG_EVENTS_HEADER).toByteArray());

    bool editable = (TT_GetMyUserRights(ttInst) & USERRIGHT_UPDATE_SERVERPROPERTIES);

    if(!editable)
    {
        ui.buttonBox->setStandardButtons(QDialogButtonBox::Close);
        ui.buttonBox->button(QDialogButtonBox::Close)->setText(tr("&Close"));
    }

    connect(ui.motdChkBox, &QAbstractButton::clicked, this, &ServerPropertiesDlg::slotShowMOTDVars);
    connect(this, &QDialog::accepted, this, &ServerPropertiesDlg::slotAccepted);

    TT_GetServerProperties(ttInst, &m_srvprop);
    this->setAccessibleDescription(tr("Properties of %1").arg(_Q(m_srvprop.szServerName)));

    ui.servernameEdit->setText(_Q(m_srvprop.szServerName));
    ui.maxusersSpinBox->setValue(m_srvprop.nMaxUsers);
    ui.motdTextEdit->setPlainText((editable?_Q(m_srvprop.szMOTDRaw):_Q(m_srvprop.szMOTD)));
    ui.tcpportSpinBox->setValue(m_srvprop.nTcpPort);
    ui.udpportSpinBox->setValue(m_srvprop.nUdpPort);
    ui.usertimeoutSpinBox->setValue(m_srvprop.nUserTimeout);
    ui.autosaveBox->setChecked(m_srvprop.bAutoSave);
    ui.maxloginattemptsSpinBox->setValue(m_srvprop.nMaxLoginAttempts);
    ui.maxiploginsSpinBox->setValue(m_srvprop.nMaxLoginsPerIPAddress);
    ui.logindelaySpinBox->setValue(m_srvprop.nLoginDelayMSec);
    ui.audtxSpinBox->setValue(m_srvprop.nMaxVoiceTxPerSecond/1024);
    ui.videotxSpinBox->setValue(m_srvprop.nMaxVideoCaptureTxPerSecond/1024);
    ui.mediafiletxSpinBox->setValue(m_srvprop.nMaxMediaFileTxPerSecond/1024);
    ui.desktoptxSpinBox->setValue(m_srvprop.nMaxDesktopTxPerSecond/1024);
    ui.totaltxSpinBox->setValue(m_srvprop.nMaxTotalTxPerSecond/1024);
    ui.serverversionEdit->setText(_Q(m_srvprop.szServerVersion));
    m_serverlogmodel = new ServerLogEventsModel(this);
    ui.serverlogTableView->setModel(m_serverlogmodel);
    m_serverlogmodel->setServerLogEvents(m_srvprop.uServerLogEvents);
    if (!versionSameOrLater(_Q(m_srvprop.szServerProtocolVersion), "5.10"))
        ui.serverlogGroupBox->hide();

    if(!editable)
    {
        ui.servernameEdit->setReadOnly(true);
        ui.maxusersSpinBox->setReadOnly(true);
        ui.motdTextEdit->setReadOnly(true);
        ui.MOTDVarButton->hide();
        ui.tcpportSpinBox->setReadOnly(true);
        ui.udpportSpinBox->setReadOnly(true);
        ui.usertimeoutSpinBox->setReadOnly(true);
        ui.autosaveBox->setEnabled(false);
        ui.maxloginattemptsSpinBox->setReadOnly(true);
        ui.maxiploginsSpinBox->setReadOnly(true);
        ui.logindelaySpinBox->setReadOnly(true);
        ui.audtxSpinBox->setReadOnly(true);
        ui.videotxSpinBox->setReadOnly(true);
        ui.mediafiletxSpinBox->setReadOnly(true);
        ui.desktoptxSpinBox->setReadOnly(true);
        ui.totaltxSpinBox->setReadOnly(true);
        ui.motdChkBox->hide();
    }
    else
    {
        connect(ui.serverlogTableView, &QAbstractItemView::doubleClicked, this, &ServerPropertiesDlg::slotServerLogToggled);
        m_varMenu = new QMenu(this);
        connect(ui.MOTDVarButton, &QPushButton::clicked, this, [this]()
        {
            m_varMenu->exec(QCursor::pos());
        });
        QHash<QString, QString> variables = {{"%users%", tr("Number of users on server")}, {"%admins%", tr("Number of admins on server")}, {"%uptime%", tr("Server's time online")}, {"%voicerx%", tr("KBytes received")}, {"%voicetx%", tr("KBytes sent")}, {"%lastuser%", tr("last user to log on")}};
        for (auto it = variables.constBegin(); it != variables.constEnd(); ++it)
        {
            QAction* action = m_varMenu->addAction(it.value());
            action->setData(it.key());
            connect(action, &QAction::triggered, this, &ServerPropertiesDlg::insertVariable);
        }
    }
}

ServerPropertiesDlg::~ServerPropertiesDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_SERVERPROPERTIESWINDOWPOS, saveGeometry());
    ttSettings->setValue(SETTINGS_DISPLAY_SERVERLOG_EVENTS_HEADER, ui.serverlogTableView->horizontalHeader()->saveState());
}

void ServerPropertiesDlg::slotAccepted()
{
    COPY_TTSTR(m_srvprop.szServerName, ui.servernameEdit->text());
    m_srvprop.nMaxUsers = ui.maxusersSpinBox->value();
    m_srvprop.nMaxLoginAttempts = ui.maxloginattemptsSpinBox->value();
    m_srvprop.nMaxLoginsPerIPAddress = ui.maxiploginsSpinBox->value();
    m_srvprop.nLoginDelayMSec = ui.logindelaySpinBox->value();
    if (_Q(m_srvprop.szMOTDRaw) != ui.motdTextEdit->toPlainText())
    {
        QMessageBox answer;
        answer.setText(tr("Change message of the day?"));
        QAbstractButton *YesButton = answer.addButton(tr("&Yes"), QMessageBox::YesRole);
        QAbstractButton *NoButton = answer.addButton(tr("&No"), QMessageBox::NoRole);
        Q_UNUSED(NoButton);
        answer.setIcon(QMessageBox::Question);
        answer.setWindowTitle(tr("Message of the day"));
        answer.exec();
        if(answer.clickedButton() == YesButton)
            COPY_TTSTR(m_srvprop.szMOTDRaw, ui.motdTextEdit->toPlainText());
    }
    m_srvprop.nTcpPort = ui.tcpportSpinBox->value();
    m_srvprop.nUdpPort = ui.udpportSpinBox->value();
    m_srvprop.nUserTimeout = ui.usertimeoutSpinBox->value();
    m_srvprop.bAutoSave = ui.autosaveBox->isChecked();
    m_srvprop.nMaxVoiceTxPerSecond = ui.audtxSpinBox->value()*1024;
    m_srvprop.nMaxVideoCaptureTxPerSecond = ui.videotxSpinBox->value()*1024;
    m_srvprop.nMaxMediaFileTxPerSecond = ui.mediafiletxSpinBox->value()*1024;
    m_srvprop.nMaxDesktopTxPerSecond = ui.desktoptxSpinBox->value()*1024;
    m_srvprop.nMaxTotalTxPerSecond = ui.totaltxSpinBox->value()*1024;
    m_srvprop.uServerLogEvents = m_serverlogmodel->getServerLogEvents();

    TT_DoUpdateServer(ttInst, &m_srvprop);
}

void ServerPropertiesDlg::slotShowMOTDVars(bool checked)
{
    if(checked)
        ui.motdTextEdit->setPlainText(_Q(m_srvprop.szMOTDRaw));
    else
        ui.motdTextEdit->setPlainText(_Q(m_srvprop.szMOTD));
}

void ServerPropertiesDlg::slotServerLogToggled(const QModelIndex &index)
{
    auto events = m_serverlogmodel->getServerLogEvents();
    ServerLogEvent e = ServerLogEvent(index.internalId());
    if (e & events)
        m_serverlogmodel->setServerLogEvents(events & ~e);
    else
        m_serverlogmodel->setServerLogEvents(events | e);
}

void ServerPropertiesDlg::insertVariable()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        QString variable = action->data().toString();
        QTextCursor cursor = ui.motdTextEdit->textCursor();
        int cursorPos = cursor.position();
        cursor.insertText(variable);
        cursor.setPosition(cursorPos + variable.length());
        ui.motdTextEdit->setTextCursor(cursor);
    }
}
