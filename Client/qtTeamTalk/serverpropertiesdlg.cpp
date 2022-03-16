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

#include "serverpropertiesdlg.h"
#include "appinfo.h"
#include "serverlogeventsmodel.h"
#include "settings.h"

#include <QMessageBox>
#include <QPushButton>

extern TTInstance* ttInst;
extern QSettings* ttSettings;

ServerPropertiesDlg::ServerPropertiesDlg(QWidget * parent/* = 0*/)
    : QDialog(parent, QT_DEFAULT_DIALOG_HINTS)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(APPICON));
    restoreGeometry(ttSettings->value(SETTINGS_DISPLAY_SERVERPROPERTIESWINDOWPOS).toByteArray());

    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&Ok"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));

    ui.serverlogTreeView->header()->restoreState(ttSettings->value(SETTINGS_DISPLAY_SERVERLOG_EVENTS_HEADER).toByteArray());

    bool editable = (TT_GetMyUserRights(ttInst) & USERRIGHT_UPDATE_SERVERPROPERTIES);

    if(!editable)
    {
        ui.buttonBox->setStandardButtons(QDialogButtonBox::Close);
        ui.buttonBox->button(QDialogButtonBox::Close)->setText(tr("&Close"));
    }

    connect(ui.motdChkBox, &QAbstractButton::clicked, this, &ServerPropertiesDlg::slotShowMOTDVars);
    connect(this, &QDialog::accepted, this, &ServerPropertiesDlg::slotAccepted);

    TT_GetServerProperties(ttInst, &m_srvprop);

    ui.servernameEdit->setText(_Q(m_srvprop.szServerName));
    ui.maxusersSpinBox->setValue(m_srvprop.nMaxUsers);
    ui.motdTextEdit->setPlainText(_Q(m_srvprop.szMOTD));
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
    ui.serverlogTreeView->setModel(m_serverlogmodel);
    m_serverlogmodel->setServerLogEvents(m_srvprop.uServerLogEvents);
    if (!versionSameOrLater(_Q(m_srvprop.szServerProtocolVersion), "5.10"))
        ui.serverlogGroupBox->hide();

    if(!editable)
    {
        ui.servernameEdit->setEnabled(false);
        ui.maxusersSpinBox->setEnabled(false);
        ui.motdTextEdit->setEnabled(false);
        ui.tcpportSpinBox->setEnabled(false);
        ui.udpportSpinBox->setEnabled(false);
        ui.usertimeoutSpinBox->setEnabled(false);
        ui.autosaveBox->setEnabled(false);
        ui.maxloginattemptsSpinBox->setEnabled(false);
        ui.maxiploginsSpinBox->setEnabled(false);
        ui.logindelaySpinBox->setEnabled(false);
        ui.audtxSpinBox->setEnabled(false);
        ui.videotxSpinBox->setEnabled(false);
        ui.mediafiletxSpinBox->setEnabled(false);
        ui.desktoptxSpinBox->setEnabled(false);
        ui.totaltxSpinBox->setEnabled(false);
        ui.motdChkBox->hide();
    }
    else
    {
        ui.label_4->setTextInteractionFlags(Qt::TextBrowserInteraction|Qt::TextSelectableByKeyboard);
        connect(ui.serverlogTreeView, &QAbstractItemView::doubleClicked, this, &ServerPropertiesDlg::slotServerLogToggled);
    }
}

ServerPropertiesDlg::~ServerPropertiesDlg()
{
    ttSettings->setValue(SETTINGS_DISPLAY_SERVERPROPERTIESWINDOWPOS, saveGeometry());
    ttSettings->setValue(SETTINGS_DISPLAY_SERVERLOG_EVENTS_HEADER, ui.serverlogTreeView->header()->saveState());
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
